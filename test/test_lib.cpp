//
// Created by matthew on 8/10/24.
//

#include <cmath>
#include <iostream>
#include <strstream>

#include <catch2/catch_test_macros.hpp>

#include "babel_engine.h"


using namespace Babel;


class VectorStreamBuf final : public std::streambuf {
public:
    explicit VectorStreamBuf(std::vector<unsigned char>& vec) : vec_(vec) {}

protected:
    // Called when there is no space left in the buffer, forcing it to write to the vector.
    int overflow(const int ch) override {
        if (ch != EOF)
            vec_.push_back(static_cast<unsigned char>(ch));
        return ch;
    }

    // Write a sequence of characters to the buffer
    std::streamsize xsputn(const char* s, const std::streamsize count) override {
        vec_.insert(vec_.end(), s, s + count);
        return count;
    }

private:
    std::vector<unsigned char>& vec_;
};


class VectorOStream final : public std::ostream {
public:
    explicit VectorOStream(std::vector<unsigned char>& vec) : std::ostream(&buf_), buf_(vec) {}

private:
    VectorStreamBuf buf_;
};

TEST_CASE("Test getBaseCharset") {

    REQUIRE( getBaseCharset(64) == BASE64_CHARSET );
    REQUIRE( getBaseCharset(256) == BASE256_CHARSET );
    REQUIRE_THROWS( getBaseCharset(127) );
}


TEST_CASE("Test genRandomPaddedInt") {

    int maxValue;

    SECTION("Test Single Digit") {
        maxValue = 9;
        const std::string randInt = genRandomPaddedInt(maxValue);
        REQUIRE( randInt.length() == std::to_string(maxValue).length() );
        REQUIRE( std::stoi(randInt) <= maxValue );
    }

    SECTION("Test Double Digit") {
        maxValue = 99;
        const std::string randInt = genRandomPaddedInt(maxValue);
        REQUIRE( randInt.length() == std::to_string(maxValue).length() );
        REQUIRE( std::stoi(randInt) <= maxValue );
    }

    SECTION("Test Many Digits") {
        maxValue = 999999;
        const std::string randInt = genRandomPaddedInt(maxValue);
        REQUIRE( randInt.length() == std::to_string(maxValue).length() );
        REQUIRE( std::stoi(randInt) <= maxValue );
    }
}


TEST_CASE("Test genRandomLibraryCoordinate") {

    LibraryCoordinate coord = genRandomLibraryCoordinate();
    const int coordSeed = std::stoi(coord.page + coord.volume + coord.shelf + coord.wall);
    REQUIRE( coordSeed <= std::pow(10, 7) );
}


TEST_CASE("Test numToBase") {

    int base;
    std::vector<unsigned char> target;
    SECTION("Test Base 64") {
        base = 64;
        target = {'A'};
        REQUIRE( numToBase({0}, base) == target);
        target = {'C'};
        REQUIRE( numToBase({2}, base) == target);
        target = {'B', 'A'};
        REQUIRE( numToBase({64}, base) == target);
        target = {'B', 'F'};
        REQUIRE( numToBase({69}, base) == target);
        target = {'-', '/'};
        REQUIRE( numToBase({-63}, base) == target);
    }

    SECTION("Test Base 256") {
        base = 256;
        target = {'\0'};
        REQUIRE( numToBase({0}, base) == target);
        target = {'\x2'};
        REQUIRE( numToBase({2}, base) == target);
        target = {'\xf'};
        REQUIRE( numToBase({15}, base) == target);
        target = {'\x11'};
        REQUIRE( numToBase({17}, base) == target);
        target = {'-', '\x4b'};
        REQUIRE( numToBase({-75}, base) == target);
    }
}


TEST_CASE("Test baseToNum") {

    int base;
    SECTION("Test Base 64") {
        base = 64;
        REQUIRE( baseToNum({'A'}, base) == 0 );
        REQUIRE( baseToNum({'C'}, base) == 2 );
        REQUIRE( baseToNum({'B', 'A'}, base) == 64 );
        REQUIRE( baseToNum({'B', 'F'}, base) == 69 );
        REQUIRE( baseToNum({'-', '/'}, base) == -63 );
    }

    SECTION("Test Base 256") {
        base = 256;
        REQUIRE( baseToNum({'\0'}, base) == 0 );
        REQUIRE( baseToNum({'\x2'}, base) == 2 );
        REQUIRE( baseToNum({'\xf'}, base) == 15 );
        REQUIRE( baseToNum({'\x11'}, base) == 17 );
        REQUIRE( baseToNum({'-', '\x4b'}, base) == -75 );
    }
}

TEST_CASE("Test getAddressComponents") {

    const std::string address = "simpleaddress:3322:4:4:300";
    LibraryCoordinate coord = getAddressComponents(address);
    REQUIRE( coord.hexagon == "simpleaddress" );
    REQUIRE( coord.wall == "3322" );
    REQUIRE( coord.shelf == "4" );
    REQUIRE( coord.volume == "04" );
    REQUIRE( coord.page == "300" );
}


void reverseSearch(const std::string &searchStr) {
    const std::vector<unsigned char> searchBytes = {searchStr.begin(), searchStr.end()};
    const std::string address = computeAddress(searchBytes, true);
    std::vector<unsigned char> contentBytes = search(address);

    REQUIRE( contentBytes.size() == MAX_PAGE_LEN );
    REQUIRE( std::search(contentBytes.begin(), contentBytes.end(),
        searchBytes.begin(), searchBytes.end()) != contentBytes.end() );
}


void reverseStreamSearch(const std::string &searchStr) {
    const std::vector<unsigned char> searchBytes = {searchStr.begin(), searchStr.end()};
    std::istrstream istream(reinterpret_cast<const char*>(searchBytes.data()), static_cast<int>(searchBytes.size()));
    const std::string address = computeStreamAddress(istream, true);
    std::vector<unsigned char> contentBytes;
    VectorOStream ostream(contentBytes);
    searchStream(address, ostream);

    REQUIRE( contentBytes.size() == MAX_PAGE_LEN );
    REQUIRE( std::search(contentBytes.begin(), contentBytes.end(),
        searchBytes.begin(), searchBytes.end()) != contentBytes.end() );
}


TEST_CASE("Test Reverse Search") {

    std::string searchStr;
    SECTION("Test ASCII Text") {
        searchStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        reverseSearch(searchStr);
        reverseStreamSearch(searchStr);
    }

    SECTION("Test Non-ASCII Text") {
        searchStr = "АА̀А̂А̄ӒБВГҐДЂЃЕЀЕ̄Е̂ЁЄЖЗЗ́ЅИІЇꙆЍИ̂ӢЙЈКЛЉМНЊОО̀О̂ŌӦПРСС́ТЋЌУУ̀У̂ӮЎӰФХЦЧЏШЩꙎЪЪ̀ЫЬѢЭЮЮ̀ЯЯ̀ѦѪѨѬѮѰѲѴѶѺѼѾѿ";
        reverseSearch(searchStr);
        reverseStreamSearch(searchStr);
    }

    SECTION("Test Random Bytes") {
        searchStr = "\xaf\xdb\x1c\x51\x24\x21\x1e\x87\x3b\x9d\x24\x60\xfe\xca\xf0\x60\x9a\x17\xc3\x18\x50\x8f\x13\xf2\xba\xdd\xdd\x6c\x29\xee\x1a\x99";
        reverseSearch(searchStr);
        reverseStreamSearch(searchStr);
    }
}


TEST_CASE("Test Address Search Consistency") {

    const std::string address = "simpleaddress:3:4:4:300";
    const std::vector<unsigned char> first_content = search(address);
    const std::vector<unsigned char> second_content = search(address);

    REQUIRE( first_content == second_content );
}


TEST_CASE("Test Content Search Consistency") {

    const std::string searchStr = "hello there general kenobi";
    const std::string first_address = computeAddress({searchStr.begin(), searchStr.end()}, false);
    const std::string second_address = computeAddress({searchStr.begin(), searchStr.end()}, false);

    const std::vector<unsigned char> first_content = search(first_address);
    const std::vector<unsigned char> second_content = search(second_address);

    REQUIRE( first_content == second_content );
}
