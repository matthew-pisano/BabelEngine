#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>


int vectorFind(const std::vector<unsigned char> &vec, const unsigned char &val) {
    for (int i = 0; i < vec.size(); ++i)
        if (vec[i] == val) return i;
    throw std::invalid_argument("Value not found in vector");
}


std::vector<unsigned char> str2Vec(const std::string &str) {
    return {str.begin(), str.end()};
}


std::vector<unsigned char> range2Vec(const int start, const int end) {
    std::vector<unsigned char> charset;
    for (unsigned char i = start; i < end-1; ++i)
        charset.push_back(i);
    charset.push_back(end-1);
    return charset;
}


std::vector<unsigned char> getBaseCharset(const int base) {
    if (base == 64) return BASE64_CHARSET;
    if (base == 256) return BASE256_CHARSET;
    throw std::invalid_argument("Invalid base: "+std::to_string(base));
}


std::string genRandomPaddedInt(const int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Generate a random integer between 1 and maxValue inclusive
    std::uniform_int_distribution<> dist(1, maxValue);

    const int randInt = dist(gen);
    const int padding = std::to_string(maxValue).length();
    // Skip padding if the value is 1
    if (padding == 1) return std::to_string(randInt);

    // Pad the integer with zeros
    std::stringstream ss;
    ss << std::setw(padding) << std::setfill('0') << randInt;
    return ss.str();
}


LibraryCoordinate genRandomLibraryCoordinate() {
    LibraryCoordinate coord;
    coord.wall = genRandomPaddedInt(WALLS_PER_HEXAGON);
    coord.shelf = genRandomPaddedInt(SHELVES_PER_WALL);
    coord.volume = genRandomPaddedInt(VOLUMES_PER_SHELF);
    coord.page = genRandomPaddedInt(PAGES_PER_VOLUME);

    return coord;
}


std::vector<unsigned char> numToBase(mpz_class x, const int base) {
    std::vector<unsigned char> baseCharset = getBaseCharset(base);

    if (x == 0) return {baseCharset[0]};  // Zero is zero in any base

    std::vector<unsigned char> chars;
    const int sign = x < 0 ? -1 : 1;
    x *= sign;

    while (x > 0) {
        mpz_class rem = x % base;
        chars.push_back(baseCharset[rem.get_ui()]);
        x /= base;
    }

    if (sign < 0) chars.push_back(45);  // Add the negative sign
    std::reverse(chars.begin(), chars.end());

    return chars;
}


mpz_class baseToNum(const std::vector<unsigned char> &vec, const int base) {
    const std::vector<unsigned char> baseCharset = getBaseCharset(base);

    if (vec.size() == 1 && vec[0] == baseCharset[0]) return {0};  // Zero is zero in any base

    mpz_class x = {0};
    const bool isNeg = vec[0] == static_cast<char>(45);  // Check if the number is negative

    for (int i = (isNeg ? 1 : 0); i < vec.size(); ++i) {
        x *= base;
        x += vectorFind(baseCharset, vec[i]);  // Add the value of the character to the number
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

    const std::vector<unsigned char> dataCharset = getBaseCharset(256);
    std::vector<unsigned char> result;
    // Generate random data to pad the result
    std::mt19937 generator(data.size());
    std::uniform_int_distribution<int> placementDistrib(0, length - data.size() - 1);
    const int placement = placementDistrib(generator);
    // Add header of random size
    while (result.size() < placement) {
        std::uniform_int_distribution<int> distribution(0, dataCharset.size() - 1);
        result.push_back(dataCharset[distribution(generator)]);
    }
    for (const unsigned char &c : data) result.push_back(c);  // Add the data
    // Add footer of random size
    while (result.size() < length) {
        std::uniform_int_distribution<int> distribution(0, dataCharset.size() - 1);
        result.push_back(dataCharset[distribution(generator)]);
    }
    return result;
}


std::string computeAddress(const std::vector<unsigned char>& data, const bool padRandom) {
    const std::vector<unsigned char> paddedData = fitToLength(data, MAX_PAGE_LEN, padRandom);

    // Convert data to a number
    mpz_class dataSum = {0};
    mpz_class mult = {1};
    for (int i = 0; i < paddedData.size(); ++i) {
        const unsigned char c = paddedData[paddedData.size() - 1 - i];
        dataSum += c * mult;
        mult *= 256;  // Multiply by the base
    }

    // Generate a random library coordinate to serve as the basis for the address
    LibraryCoordinate coord = genRandomLibraryCoordinate();
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const std::vector<unsigned char> hexagonAddr = numToBase(coordSeed * mult + dataSum, 64);
    // Encode the base-10 address as a string represented by the address charset
    const std::string hexagonAddrStr(hexagonAddr.begin(), hexagonAddr.end());
    return hexagonAddrStr + ":" + coord.wall + ":" + coord.shelf + ":" + coord.volume + ":" + coord.page;
}


LibraryCoordinate getAddressComponents(const std::string &address) {
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


std::vector<unsigned char> search(const std::string &address) {
    LibraryCoordinate coord = getAddressComponents(address);

    mpz_class mult;
    mpz_class bigBase = {256};
    // Exponentiate the base to the maximum page length and store the result in mult
    mpz_pow_ui(mult.get_mpz_t(), bigBase.get_mpz_t(), MAX_PAGE_LEN);

    const mpz_class numericalAddr = baseToNum(str2Vec(coord.hexagon), 64);
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const mpz_class seed = numericalAddr - coordSeed * mult;
    // Convert the address base-encoded text to the text charset
    std::vector<unsigned char> resultText = numToBase(seed, 256);

    return {resultText.begin(), resultText.begin() + MAX_PAGE_LEN};
}
