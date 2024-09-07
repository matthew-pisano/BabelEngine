#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>


using namespace Babel;


using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;


/**
 * Find the index of a value in a vector
 * @param val The char to get the index of
 * @return The index of the value in the vector
 */
int charToIndex(const unsigned char &val, const int base) {
    if (base == 256) return val;

    for (int i = 0; i < BASE64_CHARSET.size(); ++i)
        if (BASE64_CHARSET[i] == val) return i;
    throw std::invalid_argument("Value not found in address charset: "+std::to_string(val));
}


/**
 * Concatenates the components of a library component and return their big integer representation
 */
mpz_class makeCoordSeed(const LibraryCoordinate& coord) {
    // Certain numbers crash mpz_set_str() without first using std::stoi()
    return {std::stoi(coord.page + coord.volume + coord.shelf + coord.wall)};
}


std::vector<unsigned char> Babel::getBaseCharset(const int base) {
    if (base == 64) return BASE64_CHARSET;
    if (base == 256) return BASE256_CHARSET;
    throw std::invalid_argument("Invalid base: "+std::to_string(base));
}


std::string Babel::genRandomPaddedInt(const int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Generate a random integer between 1 and maxValue inclusive
    std::uniform_int_distribution<> dist(1, maxValue);

    const int randInt = dist(gen);
    const int padding = static_cast<int>(std::to_string(maxValue).length());
    // Skip padding if the value is 1
    if (padding == 1) return std::to_string(randInt);

    // Pad the integer with zeros
    std::stringstream ss;
    ss << std::setw(padding) << std::setfill('0') << randInt;
    return ss.str();
}


LibraryCoordinate Babel::genRandomLibraryCoordinate() {
    LibraryCoordinate coord;
    coord.wall = genRandomPaddedInt(WALLS_PER_HEXAGON);
    coord.shelf = genRandomPaddedInt(SHELVES_PER_WALL);
    coord.volume = genRandomPaddedInt(VOLUMES_PER_SHELF);
    coord.page = genRandomPaddedInt(PAGES_PER_VOLUME);

    return coord;
}


LibraryCoordinate Babel::getAddressComponents(const std::string &address) {
    std::istringstream ss(address);
    LibraryCoordinate coord;
    std::getline(ss, coord.hexagon, ':');
    std::getline(ss, coord.wall, ':');
    std::getline(ss, coord.shelf, ':');
    std::getline(ss, coord.volume, ':');
    std::getline(ss, coord.page, ':');

    // Pad the volume and page with zeros
    while (coord.volume.length() < 2) coord.volume = "0" + coord.volume;
    while (coord.page.length() < 3) coord.page = "0" + coord.page;
    return coord;
}


std::vector<unsigned char> Babel::numToBase(mpz_class x, const int base) {
    std::vector<unsigned char> baseCharset = getBaseCharset(base);

    if (x == 0) return {baseCharset[0]};  // Zero is zero in any base

    const int sqrtBase = base == 256 ? 16 : 8;
    std::vector<unsigned char> chars;
    const int sign = x < 0 ? -1 : 1;
    x *= sign;  // Ensure the number is positive

    if (sign < 0) chars.push_back(45);  // Add the negative sign
    std::string xStr = x.get_str(sqrtBase);
    if (xStr.length() % 2 != 0) xStr = "0" + xStr;  // Ensure the string has an even length
    for (int i=0; i<xStr.length(); i+=2) {
        const int byte = std::stoi(xStr.substr(i, 2), nullptr, sqrtBase);  // Convert the hex string to an integer
        chars.push_back(baseCharset[byte]);
    }

    return chars;
}


mpz_class Babel::baseToNum(const std::vector<unsigned char> &vec, const int base) {
    const std::vector<unsigned char> baseCharset = getBaseCharset(base);

    if (vec.size() == 1 && vec[0] == baseCharset[0]) return {0};  // Zero is zero in any base

    const int baseShift = base == 256 ? 8 : 6;
    mpz_class x = {0};
    const bool isNeg = vec[0] == static_cast<char>(45);  // Check if the number is negative

    for (int i = (isNeg ? 1 : 0); i < vec.size(); ++i) {
        x = x << baseShift;
        x += charToIndex(vec[i], base);  // Add the value of the character to the number
    }

    return x * (isNeg? -1 : 1);
}


std::vector<unsigned char> fitToLength(const std::vector<unsigned char> &data, const int length, const bool padRandom) {
    if (data.size() >= length) {
        // Truncate the result
        return {data.begin(), data.begin() + length};
    }

    if (!padRandom) {
        // Pad the result with zeroes
        std::vector result(data);
        result.resize(length, 0);
        return result;
    }

    std::vector<unsigned char> result(length);
    // Generate random data to pad the result
    std::mt19937 generator(data.size());
    std::uniform_int_distribution<int> placementDistrib(0, length - static_cast<int>(data.size()) - 1);
    const int placement = placementDistrib(generator);

    random_bytes_engine randGenerator;
    // Add header of random size
    std::generate_n(begin(result), placement, std::ref(randGenerator));
    // Add the data
    std::copy(data.begin(), data.end(), result.begin() + placement);
    // Add footer of random size
    std::generate_n(begin(result) + placement + data.size(), length - placement - data.size(), std::ref(randGenerator));
    return result;
}


std::string Babel::computeAddress(const std::vector<unsigned char>& data, const bool padRandom) {
    const std::vector<unsigned char> paddedData = fitToLength(data, MAX_PAGE_LEN, padRandom);

    // Convert data to a number
    mpz_class dataSum = {0};
    for (int i = 0; i < paddedData.size(); ++i) {
        const mpz_class c = paddedData[paddedData.size() - 1 - i];
        dataSum += c << (8 * i);
    }

    // Generate a random library coordinate to serve as the basis for the address
    LibraryCoordinate coord = genRandomLibraryCoordinate();
    const mpz_class shifted_coord = makeCoordSeed(coord) << (8 * paddedData.size());
    const std::vector<unsigned char> hexagonAddr = numToBase(shifted_coord + dataSum, 64);
    // Encode the base-10 address as a string represented by the address charset
    const std::string hexagonAddrStr(hexagonAddr.begin(), hexagonAddr.end());
    return hexagonAddrStr + ":" + coord.wall + ":" + coord.shelf + ":" + coord.volume + ":" + coord.page;
}


std::string Babel::computeStreamAddress(std::istream& stream, const bool padRandom) {
    std::vector<unsigned char> signedData;
    unsigned char c;
    while (stream >> c) signedData.push_back(c);
    return computeAddress(signedData, padRandom);
}


std::vector<unsigned char> Babel::search(const std::string &address) {
    LibraryCoordinate coord = getAddressComponents(address);

    mpz_class mult;
    mpz_class bigBase = {256};
    // Exponentiate the base to the maximum page length and store the result in mult
    mpz_pow_ui(mult.get_mpz_t(), bigBase.get_mpz_t(), MAX_PAGE_LEN);

    const mpz_class numericalAddr = baseToNum({coord.hexagon.begin(), coord.hexagon.end()}, 64);
    const mpz_class seed = numericalAddr - makeCoordSeed(coord) * mult;
    // Convert the address base-encoded text to the text charset
    std::vector<unsigned char> resultText = numToBase(seed, 256);

    return {resultText.begin(), resultText.begin() + MAX_PAGE_LEN};
}


void Babel::searchStream(const std::string &address, std::ostream &stream) {
    std::vector<unsigned char> contentBytes = search(address);
    for (const unsigned char &c : contentBytes) stream << c;
}
