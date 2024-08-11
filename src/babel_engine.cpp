#include "babel_engine.h"

#include <algorithm>
#include <iomanip>
#include <random>
#include <string>
#include <sstream>


const int TEXT_BASE = TEXT_CHARSET.length();
const int ADDRESS_BASE = ADDRESS_CHARSET.length();
constexpr int MAX_PAGE_LEN = 3200;

constexpr int WALLS_PER_HEXAGON = 4;
constexpr int SHELVES_PER_WALL = 5;
constexpr int VOLUMES_PER_SHELF = 32;
constexpr int PAGES_PER_VOLUME = 410;


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


std::string NumToBase(BigInt x, const int base) {
    std::string baseCharset = getBaseCharset(base);

    if (x == 0) return {1, baseCharset[0]};  // Zero is zero in any base

    std::string chars;
    const int sign = x < 0 ? -1 : 1;
    x *= sign;

    while (x > 0) {
        chars.push_back(baseCharset[(x % base).to_int()]);
        x /= base;
    }

    if (sign < 0) chars.push_back('-');
    std::reverse(chars.begin(), chars.end());

    return chars;
}


BigInt BaseToNum(const std::string &s, const int base) {
    std::string baseCharset = getBaseCharset(base);

    if (s == baseCharset[0]) return {0};  // Zero is zero in any base

    BigInt x = {0};
    const bool isNeg = s[0] == '-';

    for (int i = (isNeg ? 1 : 0); i < s.length(); ++i) {
        x *= base;
        x += baseCharset.find(s[i]);
    }

    return x * (isNeg? -1 : 1);
}


std::string searchByContent(const std::string& rawText) {

    // Generate a random library coordinate to serve as the basis for the address
    const int libraryCoordinate = genRandomLibraryCoordinate();
    // Filter out non-alphanumeric characters and convert to lowercase
    std::string text;
    for (int i = 0; i < rawText.length(); i++)
        if (TEXT_CHARSET.find(rawText[i]) != std::string::npos)
            text += std::tolower(rawText[i]);

    text = text.substr(0, MAX_PAGE_LEN);  // Truncate text to max_page_content_length
    text.resize(MAX_PAGE_LEN, ' ');  // Pad text with spaces to max_page_content_length

    // Convert text to a number
    BigInt textSum = {0};
    for (int i = 0; i < text.length(); ++i) {
        const char c = text[text.length() - 1 - i];
        const int charValue = std::isalpha(c) ? c - 'a' : c == '.' ? 28 : 27;
        textSum += charValue * pow(TEXT_BASE, i);
    }

    const BigInt address = libraryCoordinate * pow(TEXT_BASE, MAX_PAGE_LEN) + textSum;
    // Encode the base-10 address as a string represented by the address charset
    return NumToBase(address, ADDRESS_BASE);
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

    BigInt addrAsBase = BaseToNum(hexagonAddress, ADDRESS_BASE);
    BigInt seed = addrAsBase - libraryCoordinate * pow(TEXT_BASE, MAX_PAGE_LEN);
    std::string baseEncodedText = NumToBase(seed, ADDRESS_BASE);
    // Convert the address base-encoded text to the text charset
    std::string resultText = NumToBase(BaseToNum(baseEncodedText, ADDRESS_BASE), TEXT_BASE);

    if (resultText.length() < MAX_PAGE_LEN) {
        // Generate random text to pad the result
        std::mt19937 generator(resultText.length());
        while (resultText.length() < MAX_PAGE_LEN) {
            std::uniform_int_distribution<int> distribution(0, TEXT_BASE - 1);
            resultText += TEXT_CHARSET[distribution(generator)];
        }
    } else if (resultText.length() > MAX_PAGE_LEN) {
        // Truncate the result
        resultText = resultText.substr(resultText.length() - MAX_PAGE_LEN);
    }

    return resultText;
}
