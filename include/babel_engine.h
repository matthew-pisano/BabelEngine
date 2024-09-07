#ifndef BABEL_ENGINE_LIBRARY_H
#define BABEL_ENGINE_LIBRARY_H


#include <gmpxx.h>
#include <string>
#include <vector>

namespace Babel {

    inline std::string BASE64_CHARSET_STR_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    inline std::vector<unsigned char> BASE64_CHARSET = {BASE64_CHARSET_STR_.begin(), BASE64_CHARSET_STR_.end()};

    // Build the charset for base 256
    inline std::vector<unsigned char> build256Charset() {
     std::vector<unsigned char> charset;
     charset.reserve(256);
     for (int i = 0; i < 256; ++i) charset.push_back(i);
     return charset;
    }
    inline std::vector<unsigned char> BASE256_CHARSET = build256Charset();

    constexpr int MAX_PAGE_LEN = 1024 * 100;

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
     * Convert a GMP number to a vector of bytes in a given base
     * @param x The number to convert
     * @param base The base to convert the number to
     * @return The number as a vector of bytes
     */
    std::vector<unsigned char> numToBase(mpz_class x, int base);


    /**
     * Convert a vector of bytes in a given base to a GMP number
     * @param vec The vector to convert
     * @param base The base of the string
     * @return The number represented by the vector
     */
    mpz_class baseToNum(const std::vector<unsigned char> &vec, int base);


    /**
     * Get the decomposed components of an address
     * @param address The address to get the components of
     * @return The coordinate components of the address
     */
    LibraryCoordinate getAddressComponents(const std::string &address);


    /**
     * Get the address of a given byte sequence
     * @param data The data to get the address of
     * @param padRandom Whether to pad with random bytes, otherwise pad with zeros
     * @return The address of the byte sequence
     */
    std::string computeAddress(const std::vector<unsigned char>& data, bool padRandom);


    /**
     * Compute the address of the data provided by a stream
     * @param stream The stream to get data from
     * @param padRandom Whether to pad with random bytes, otherwise pad with zeros
     * @return The address of the stream's data
     */
    std::string computeStreamAddress(std::istream& stream, bool padRandom);


    /**
     * Search for a byte sequence by its address
     * @param address The address to search for
     * @return The byte sequence at the given address
     */
    std::vector<unsigned char> search(const std::string &address);


    /**
     * Search for a byte sequence by its address
     * @param address The address to search for
     * @param stream The stream to write the byte sequence to
     */
    void searchStream(const std::string &address, std::ostream &stream);
}

#endif //BABEL_ENGINE_LIBRARY_H
