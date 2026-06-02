// This test file is AI generated
#include <catch2/catch_test_macros.hpp>
#include "../lentus/variable.hpp"

TEST_CASE("variable move semantics")
{
    lentus_string *s = new lentus_string(std::string("hello"));
    variable v(s);
    variable v2(std::move(v));

    REQUIRE(v.get_type() == variable_type::NONE);
    REQUIRE(v2.get_type() == variable_type::STRING);
    REQUIRE(v2.get_string() == s);

    // cleanup
    delete s;
}

TEST_CASE("reference copy and modify")
{
    variable v(42.0);
    reference r(&v);
    reference r2 = r;

    *r2 = variable(100.0);
    REQUIRE(v.get_number() == 100.0);
}

TEST_CASE("lentus_list bounds")
{
    lentus_list *list = new lentus_list();
    list->push_back(variable(5.0));

    REQUIRE(list->size() == 1);
    REQUIRE(list->operator[](0).get_number() == 5.0);
    REQUIRE_THROWS_AS(list->at(1), std::out_of_range);

    delete list;
}
