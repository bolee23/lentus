// This test file is AI generated
#include <catch2/catch_test_macros.hpp>
#include "../lentus/data.hpp"

using namespace lentus;

TEST_CASE("value move semantics")
{
    string *s = new string(std::string("hello"));
    value v(s);
    value v2(std::move(v));

    REQUIRE(v.get_type() == value_type::NONE);
    REQUIRE(v2.get_type() == value_type::STRING);
    REQUIRE(v2.get_string() == s);

    // cleanup
    delete s;
}

TEST_CASE("reference copy and modify")
{
    value v(42.0);
    reference r(&v);
    reference r2 = r;

    *r2 = value(100.0);
    REQUIRE(v.get_number() == 100.0);
}

TEST_CASE("list bounds")
{
    list *items = new list();
    items->push_back(value(5.0));

    REQUIRE(items->size() == 1);
    REQUIRE(items->operator[](0).get_number() == 5.0);
    REQUIRE_THROWS_AS(items->at(1), std::out_of_range);

    delete items;
}
