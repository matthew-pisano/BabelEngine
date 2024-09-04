#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>


int vectorFind(const std::vector<char> &vec, const char &val) {
    for (int i = 0; i < vec.size(); ++i)
        if (vec[i] == val) return i;
    throw std::invalid_argument("Value not found in vector");
}


std::vector<char> str2Vec(const std::string &str) {
    return {str.begin(), str.end()};
}

std::vector<char> range2Vec(const int start, const int end) {
    std::string charset;
    for (unsigned char i = start; i < end; ++i) charset.push_back(i);
    return {charset.begin(), charset.end()};
}


std::vector<char> getBaseCharset(const int base) {
    if (base == 29) return BASE29_CHARSET;
    if (base == 36) return BASE36_CHARSET;
    if (base == 64) return BASE64_CHARSET;
    if (base == 128) return BASE128_CHARSET;
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


std::vector<char> numToBase(mpz_class x, const int base) {
    std::vector<char> baseCharset = getBaseCharset(base);

    if (x == 0) return {baseCharset[0]};  // Zero is zero in any base

    std::vector<char> chars;
    const int sign = x < 0 ? -1 : 1;
    x *= sign;

    while (x > 0) {
        mpz_class rem = x % base;
        chars.push_back(baseCharset[rem.get_ui()]);
        x /= base;
    }

    if (sign < 0) chars.push_back(45);
    std::reverse(chars.begin(), chars.end());

    return chars;
}


mpz_class baseToNum(const std::vector<char> &s, const int base) {
    const std::vector<char> baseCharset = getBaseCharset(base);

    if (s.size() == 1 && s[0] == baseCharset[0]) return {0};  // Zero is zero in any base

    mpz_class x = {0};
    const bool isNeg = s[0] == static_cast<char>(45);

    for (int i = (isNeg ? 1 : 0); i < s.size(); ++i) {
        x *= base;
        x += vectorFind(baseCharset, s[i]);
    }

    return x * (isNeg? -1 : 1);
}


std::vector<char> fitToLength(const std::vector<char> &text, const int length, const std::vector<char> charset, const bool padRandom) {
    if (text.size() >= length) {
        // Truncate the result
        return {text.begin(), text.begin() + length};
    }

    if (!padRandom) {
        // Pad the result with spaces
        std::vector result(text);
        result.resize(length, 64);
        return result;
    }

    std::vector<char> result;
    // Generate random text to pad the result
    std::mt19937 generator(text.size());
    std::uniform_int_distribution<int> placementDistrib(0, length - text.size() - 1);
    int placement = placementDistrib(generator);
    while (result.size() < placement) {
        std::uniform_int_distribution<int> distribution(0, charset.size() - 1);
        result.push_back(charset[distribution(generator)]);
    }
    for (const char &c : text) result.push_back(c);
    while (result.size() < length) {
        std::uniform_int_distribution<int> distribution(0, charset.size() - 1);
        result.push_back(charset[distribution(generator)]);
    }
    return result;
}


std::string searchByContent(const std::vector<char>& rawText, const int textBase, const int addrBase, const bool padRandom) {
    const std::vector<char> text_charset = getBaseCharset(textBase);

    // Filter out non-alphanumeric characters and convert to lowercase
    std::vector<char> text;
    for (char current : rawText) {
        vectorFind(text_charset, current);  // Check if the character is in the charset
        text.push_back(current);
    }

    text = fitToLength(text, MAX_PAGE_LEN, text_charset, padRandom);

    // Convert text to a number
    mpz_class textSum = {0};
    mpz_class mult = {1};
    for (int i = 0; i < text.size(); ++i) {
        char c = text[text.size() - 1 - i];
        textSum += c * mult;

        mult *= text_charset.size();
    }

    // Generate a random library coordinate to serve as the basis for the address
    LibraryCoordinate coord = genRandomLibraryCoordinate();
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const std::vector<char> hexagonAddr = numToBase(coordSeed * mult + textSum, addrBase);
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


std::vector<char> searchByAddress(const std::string &address, const int textBase, const int addrBase) {
    LibraryCoordinate coord = getAddressComponents(address);

    mpz_class mult;
    mpz_class bigTextBase = textBase;
    mpz_pow_ui(mult.get_mpz_t(), bigTextBase.get_mpz_t(), MAX_PAGE_LEN);

    const mpz_class numericalAddr = baseToNum(str2Vec(coord.hexagon), addrBase);
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const mpz_class seed = numericalAddr - coordSeed * mult;
    const std::vector<char> baseEncodedText = numToBase(seed, addrBase);
    // Convert the address base-encoded text to the text charset
    std::vector<char> resultText = numToBase(baseToNum(baseEncodedText, addrBase), textBase);

    return {resultText.begin(), resultText.begin() + MAX_PAGE_LEN};
}
