#ifndef BABEL_ENGINE_LIBRARY_H
#define BABEL_ENGINE_LIBRARY_H

#include <string>
#include <BigInt/BigInt.hpp>

const std::string TEXT_CHARSET = "abcdefghijklmnopqrstuvwxyz, .";
const std::string ADDRESS_CHARSET = "0123456789abcdefghijklmnopqrstuvwxyz";

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
int genRandomLibraryCoordinate();


/**
 * Convert a number to a string in a given base
 * @param x The number to convert
 * @param base The base to convert the number to
 * @return The number as a string in the given base
 */
std::string NumToBase(BigInt x, int base);


/**
 * Convert a string in a given base to a number
 * @param s The string to convert
 * @param base The base of the string
 * @return The number represented by the string
 */
BigInt BaseToNum(const std::string &s, int base);


/**
 * Search for a page by its content
 * @param rawText The content to search for
 * @return The address of the page
 */
std::string searchByContent(const std::string& rawText);


/**
 * Search for a page by its address
 * @param address The address to search for
 * @return The content of the page
 */
std::string searchByAddress(const std::string &address);

#endif //BABEL_ENGINE_LIBRARY_H
