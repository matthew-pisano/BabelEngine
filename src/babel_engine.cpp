#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>


const int TEXT_BASE = TEXT_CHARSET.length();
const int ADDRESS_BASE = ADDRESS_CHARSET.length();

std::string getBaseCharset(const int base) {
    if (base == ADDRESS_BASE) return ADDRESS_CHARSET;
    if (base == TEXT_BASE) return TEXT_CHARSET;
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


int genRandomLibraryCoordinate() {
    const std::string wall = genRandomPaddedInt(WALLS_PER_HEXAGON);
    const std::string shelf = genRandomPaddedInt(SHELVES_PER_WALL);
    const std::string volume = genRandomPaddedInt(VOLUMES_PER_SHELF);
    const std::string page = genRandomPaddedInt(PAGES_PER_VOLUME);

    return std::stoi(page + volume + shelf + wall);
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


std::string fitToLength(const std::string &text, const int length, const bool padRandom) {
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
        std::uniform_int_distribution<int> distribution(0, TEXT_BASE - 1);
        result += TEXT_CHARSET[distribution(generator)];
    }
    result += text;
    while (result.length() < length) {
        std::uniform_int_distribution<int> distribution(0, TEXT_BASE - 1);
        result += TEXT_CHARSET[distribution(generator)];
    }
    return result;
}


std::string searchByContent(const std::string& rawText) {

    // Generate a random library coordinate to serve as the basis for the address
    const int libraryCoordinate = genRandomLibraryCoordinate();
    // Filter out non-alphanumeric characters and convert to lowercase
    std::string text;
    for (int i = 0; i < rawText.length(); i++) {
        char lower = std::tolower(rawText[i]);
        if (TEXT_CHARSET.find(lower) != std::string::npos)
            text += lower;
    }

    text = fitToLength(text, MAX_PAGE_LEN, true);

    // Convert text to a number
    mpz_class textSum = {0};
    mpz_class mult = {1};
    for (int i = 0; i < text.length(); ++i) {
        const char c = text[text.length() - 1 - i];
        const int charValue = std::isalpha(c) ? c - 'a' : c == '.' ? 28 : 27;
        textSum += charValue * mult;

        mult *= TEXT_BASE;
    }

    const mpz_class address = libraryCoordinate * mult + textSum;
    // Encode the base-10 address as a string represented by the address charset
    return numToBase(address, ADDRESS_BASE);
}


std::string searchByAddress(const std::string &address) {
    std::istringstream ss(address);
    std::string hexagonAddress, wall, shelf, volume, page;
    std::getline(ss, hexagonAddress, ':');
    std::getline(ss, wall, ':');
    std::getline(ss, shelf, ':');
    std::getline(ss, volume, ':');
    std::getline(ss, page, ':');

    // Pad the volume and page with zeros
    volume.insert(0, 2 - volume.length(), '0');
    page.insert(0, 3 - page.length(), '0');

    int libraryCoordinate = std::stoi(page + volume + shelf + wall);

    mpz_class addrAsBase = baseToNum(hexagonAddress, ADDRESS_BASE);
    mpz_class mult;
    mpz_class bigTextBase = TEXT_BASE;
    mpz_pow_ui(mult.get_mpz_t(), bigTextBase.get_mpz_t(), MAX_PAGE_LEN);
    mpz_class seed = addrAsBase - libraryCoordinate * mult;
    std::string baseEncodedText = numToBase(seed, ADDRESS_BASE);
    // Convert the address base-encoded text to the text charset
    std::string resultText = numToBase(baseToNum(baseEncodedText, ADDRESS_BASE), TEXT_BASE);
    // Truncate the result to the maximum page length
    return resultText.substr(resultText.length() - MAX_PAGE_LEN);
}
