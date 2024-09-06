#ifndef BABEL_ENGINE_LIBRARY_H
#define BABEL_ENGINE_LIBRARY_H


#include <gmpxx.h>
#include <string>
#include <vector>


std::vector<unsigned char> str2Vec(const std::string &str);

std::vector<unsigned char> range2Vec(int start, int end);

inline std::vector<unsigned char> BASE64_CHARSET = str2Vec("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
inline std::vector<unsigned char> BASE256_CHARSET = range2Vec(0, 256);

constexpr int MAX_PAGE_LEN = 10000;

constexpr int WALLS_PER_HEXAGON = 4;
constexpr int SHELVES_PER_WALL = 5;
constexpr int VOLUMES_PER_SHELF = 32;
constexpr int PAGES_PER_VOLUME = 410;


struct LibraryCoordinate {
   std::string hexagon;
   std::string wall;
   std::string shelf;
   std::string volume;
   std::string page;
};


/**
 * Find the index of a value in a vector
 * @param vec The vector to search
 * @param val The value to search for
 * @return The index of the value in the vector
 */
int vectorFind(const std::vector<unsigned char> &vec, const unsigned char &val);


/**
 * Get the characters that compose a number encoded in a given base
 * @param base The base to get the charset for
 * @return The charset for the given base
 */
std::vector<unsigned char> getBaseCharset(int base);


/**
 * Generate a random integer in the range [1, maxValue], left-padded with zeros
 * @param maxValue The maximum value of the random integer
 * @return A random integer in the range [1, maxValue], left-padded with zeros
 */
std::string genRandomPaddedInt(int maxValue);


/**
 * Generate a random library coordinate
 * @return A random library coordinate
 */
LibraryCoordinate genRandomLibraryCoordinate();


/**
 * Convert a number to a string in a given base
 * @param x The number to convert
 * @param base The base to convert the number to
 * @return The number as a string in the given base
 */
std::vector<unsigned char> numToBase(mpz_class x, int base);


/**
 * Convert a string in a given base to a number
 * @param s The string to convert
 * @param base The base of the string
 * @return The number represented by the string
 */
mpz_class baseToNum(const std::vector<unsigned char> &s, int base);


/**
 * Get the address components of a library coordinate
 * @param address The address to get the components of
 * @return The coordinate components of the address
 */
LibraryCoordinate getAddressComponents(const std::string &address);


/**
 * Search for a page by its content
 * @param rawText The content to search for
 * @param padRandom Whether to pad the text with random characters
 * @return The address of the page
 */
std::string searchByContent(const std::vector<unsigned char>& rawText, bool padRandom);


/**
 * Search for a page by its address
 * @param address The address to search for
 * @return The content of the page
 */
std::vector<unsigned char> searchByAddress(const std::string &address);

#endif //BABEL_ENGINE_LIBRARY_H
