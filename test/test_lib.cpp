//
// Created by matthew on 8/10/24.
//

#include <cmath>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "babel_engine.h"

TEST_CASE("Test getBaseCharset()") {
    REQUIRE( getBaseCharset(36) == ADDRESS_CHARSET );
    REQUIRE( getBaseCharset(29) == TEXT_CHARSET );
}


TEST_CASE("Test genRandomPaddedInt()") {
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


TEST_CASE("Test genRandomLibraryCoordinate()") {
    const int coord = genRandomLibraryCoordinate();
    REQUIRE( coord <= std::pow(10, 7) );
}


TEST_CASE("Test numToBase()") {

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


TEST_CASE("Test baseToNum()") {

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


TEST_CASE("Test Reverse Search") {

    const std::string searchStr = "hello there general kenobi";
    const std::string address = searchByContent(searchStr);
    const std::string content = searchByAddress(address);

    REQUIRE( content.length() == MAX_PAGE_LEN );
    REQUIRE( content.find(searchStr, 0) != std::string::npos );
}