//
// Created by matthew on 8/10/24.
//

#include <catch2/catch_test_macros.hpp>

#include "babel_engine.h"


TEST_CASE( "Test getBaseCharset()") {
    REQUIRE( getBaseCharset(36) == ADDRESS_CHARSET );
    REQUIRE( getBaseCharset(29) == TEXT_CHARSET );
}