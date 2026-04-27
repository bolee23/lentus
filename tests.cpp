#include <catch2/catch_test_macros.hpp>
#include "lentus.hpp"
#include <iostream>

// Test the printMessage function
TEST_CASE("printMessage outputs correct message", "[printMessage]")
{
    REQUIRE_NOTHROW(printMessage());
}
