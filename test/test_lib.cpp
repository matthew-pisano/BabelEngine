//
// Created by matthew on 8/10/24.
//

#include <cmath>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "babel_engine.h"

TEST_CASE("Test getBaseCharset") {

    REQUIRE( getBaseCharset(29) == BASE29_CHARSET );
    REQUIRE( getBaseCharset(36) == BASE36_CHARSET );
    REQUIRE( getBaseCharset(64) == BASE64_CHARSET );
    std::string base128Charset;
    for (unsigned char i = 0; i < 128; ++i)
        base128Charset.push_back(i);
    REQUIRE( getBaseCharset(128) == base128Charset );
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
    SECTION("Test Base 36") {
        base = 36;
        REQUIRE( numToBase({0}, base) == "0");
        REQUIRE( numToBase({2}, base) == "2");
        REQUIRE( numToBase({36}, base) == "10");
        REQUIRE( numToBase({69}, base) == "1x");
        REQUIRE( numToBase({-33}, base) == "-x");
    }

    SECTION("Test Base 29") {
        base = 29;
        REQUIRE( numToBase({0}, base) == "a");
        REQUIRE( numToBase({2}, base) == "c");
        REQUIRE( numToBase({29}, base) == "ba");
        REQUIRE( numToBase({69}, base) == "cl");
        REQUIRE( numToBase({-33}, base) == "-be");
    }
}


TEST_CASE("Test baseToNum") {

    int base;
    SECTION("Test Base 36") {
        base = 36;
        REQUIRE( baseToNum("0", base) == 0);
        REQUIRE( baseToNum("2", base) == 2);
        REQUIRE( baseToNum("10", base) == 36);
        REQUIRE( baseToNum("1x", base) == 69);
        REQUIRE( baseToNum("-x", base) == -33);
    }

    SECTION("Test Base 29") {
        base = 29;
        REQUIRE( baseToNum("a", base) == 0);
        REQUIRE( baseToNum("c", base) == 2);
        REQUIRE( baseToNum("ba", base) == 29);
        REQUIRE( baseToNum("cl", base) == 69);
        REQUIRE( baseToNum("-be", base) == -33);
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


TEST_CASE("Test Reverse Search") {

    const std::string searchStr = "hello there general kenobi";
    SECTION("Test Text Base 29") {
        const std::string address = searchByContent(searchStr, 29, 36, true, true);
        const std::string content = searchByAddress(address, 29, 36);
        REQUIRE( content.length() == MAX_PAGE_LEN );
        REQUIRE( content.find(searchStr, 0) != std::string::npos );
    }

    SECTION("Test Text Base 128") {
        const std::string address = searchByContent(searchStr, 128, 36, false, true);
        const std::string content = searchByAddress(address, 128, 36);
        REQUIRE( content.length() == MAX_PAGE_LEN );
        REQUIRE( content.find(searchStr, 0) != std::string::npos );
    }
}


TEST_CASE("Test Address Search") {

    const std::string address = "simpleaddress:3:4:4:300";
    const std::string first_content = searchByAddress(address, 29, 36);
    const std::string second_content = searchByAddress(address, 29, 36);

    REQUIRE( first_content == second_content );
}


TEST_CASE("Test Content Search") {

    const std::string searchStr = "hello there general kenobi";
    const std::string first_address = searchByContent(searchStr, 29, 36, false, true);
    const std::string second_address = searchByContent(searchStr, 29, 36, false, true);

    const std::string first_content = searchByAddress(first_address, 29, 36);
    const std::string second_content = searchByAddress(second_address, 29, 36);

    REQUIRE( first_content == second_content );
}
