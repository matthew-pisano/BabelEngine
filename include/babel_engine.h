#ifndef BABEL_ENGINE_LIBRARY_H
#define BABEL_ENGINE_LIBRARY_H


#include <gmpxx.h>
#include <string>

inline std::string BASE29_CHARSET = "abcdefghijklmnopqrstuvwxyz, .";
inline std::string BASE36_CHARSET = "0123456789abcdefghijklmnopqrstuvwxyz";
inline std::string BASE64_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

constexpr int MAX_PAGE_LEN = 3200;

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
 * Get the characters that compose a number encoded in a given base
 * @param base The base to get the charset for
 * @return The charset for the given base
 */
std::string getBaseCharset(int base);


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
std::string numToBase(mpz_class x, int base);


/**
 * Convert a string in a given base to a number
 * @param s The string to convert
 * @param base The base of the string
 * @return The number represented by the string
 */
mpz_class baseToNum(const std::string &s, int base);


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
 * @param textBase The base that the text is encoded into
 * @param addrBase The base to encode the address into
 * @param ignoreCase Whether to encode text, regardless of case
 * @return The address of the page
 */
std::string searchByContent(const std::string& rawText, int textBase, int addrBase, bool padRandom, bool ignoreCase);


/**
 * Search for a page by its address
 * @param address The address to search for
 * @param textBase The base to encode the text into
 * @param addrBase The base that the address is encoded into
 * @return The content of the page
 */
std::string searchByAddress(const std::string &address, int textBase, int addrBase);

#endif //BABEL_ENGINE_LIBRARY_H
