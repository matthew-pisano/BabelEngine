#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>


std::string getBaseCharset(const int base) {
    if (base == 29) return BASE29_CHARSET;
    if (base == 36) return BASE36_CHARSET;
    if (base == 64) return BASE64_CHARSET;
    if (base == 128) {
        std::string charset;
        for (unsigned char i = 0; i < 128; ++i) charset.push_back(i);
        return charset;
    }
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


std::string numToBase(mpz_class x, const int base) {
    std::string baseCharset = getBaseCharset(base);

    if (x == 0) return baseCharset.substr(0, 1);  // Zero is zero in any base

    std::string chars;
    const int sign = x < 0 ? -1 : 1;
    x *= sign;

    while (x > 0) {
        mpz_class rem = x % base;
        chars.push_back(baseCharset[rem.get_ui()]);
        x /= base;
    }

    if (sign < 0) chars.push_back('-');
    std::reverse(chars.begin(), chars.end());

    return chars;
}


mpz_class baseToNum(const std::string &s, const int base) {
    const std::string baseCharset = getBaseCharset(base);

    if (s.length() == 1 && s[0] == baseCharset[0]) return {0};  // Zero is zero in any base

    mpz_class x = {0};
    const bool isNeg = s[0] == '-';

    for (int i = (isNeg ? 1 : 0); i < s.length(); ++i) {
        x *= base;
        x += baseCharset.find(s[i]);
    }

    return x * (isNeg? -1 : 1);
}


std::string fitToLength(const std::string &text, const int length, const std::string &charset, const bool padRandom) {
    if (text.length() >= length)
        // Truncate the result
        return text.substr(text.length() - length);

    if (!padRandom)
        // Pad the result with spaces
        return text + std::string(length - text.length(), ' ');

    std::string result;
    // Generate random text to pad the result
    std::mt19937 generator(text.length());
    std::uniform_int_distribution<int> placementDistrib(0, length - text.length() - 1);
    int placement = placementDistrib(generator);
    while (result.length() < placement) {
        std::uniform_int_distribution<int> distribution(0, charset.length() - 1);
        result += charset[distribution(generator)];
    }
    result += text;
    while (result.length() < length) {
        std::uniform_int_distribution<int> distribution(0, charset.length() - 1);
        result += charset[distribution(generator)];
    }
    return result;
}


std::string searchByContent(const std::string& rawText, const int textBase, const int addrBase, const bool padRandom, const bool ignoreCase) {

    const std::string text_charset = getBaseCharset(textBase);

    // Filter out non-alphanumeric characters and convert to lowercase
    std::string text;
    for (int i = 0; i < rawText.length(); i++) {
        char current = rawText[i];
        if (ignoreCase) current = std::tolower(current);
        if (text_charset.find(current) != std::string::npos)
            text += current;
    }

    text = fitToLength(text, MAX_PAGE_LEN, text_charset, padRandom);

    // Convert text to a number
    mpz_class textSum = {0};
    mpz_class mult = {1};
    for (int i = 0; i < text.length(); ++i) {
        const char c = text[text.length() - 1 - i];
        const int charValue = std::isalpha(c) ? c - 'a' : c == '.' ? 28 : 27;
        textSum += charValue * mult;

        mult *= text_charset.length();
    }

    // Generate a random library coordinate to serve as the basis for the address
    LibraryCoordinate coord = genRandomLibraryCoordinate();
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const std::string hexagonAddr = numToBase(coordSeed * mult + textSum, addrBase);
    // Encode the base-10 address as a string represented by the address charset
    return hexagonAddr + ":" + coord.wall + ":" + coord.shelf + ":" + coord.volume + ":" + coord.page;
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


std::string searchByAddress(const std::string &address, const int textBase, const int addrBase) {
    LibraryCoordinate coord = getAddressComponents(address);

    mpz_class mult;
    mpz_class bigTextBase = textBase;
    mpz_pow_ui(mult.get_mpz_t(), bigTextBase.get_mpz_t(), MAX_PAGE_LEN);

    const mpz_class numericalAddr = baseToNum(coord.hexagon, addrBase);
    const mpz_class coordSeed(coord.page + coord.volume + coord.shelf + coord.wall);
    const mpz_class seed = numericalAddr - coordSeed * mult;
    const std::string baseEncodedText = numToBase(seed, addrBase);
    // Convert the address base-encoded text to the text charset
    std::string resultText = numToBase(baseToNum(baseEncodedText, addrBase), textBase);

    return resultText;
}
