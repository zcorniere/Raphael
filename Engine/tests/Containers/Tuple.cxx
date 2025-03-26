#include "Engine/Raphael.hxx"

#include "Engine/Containers/Tuple.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Tuple Tests")
{

    SECTION("Test Get")
    {
        TTuple<int, float, std::string> Tuple(42, 3.14f, "Hello World");
        CHECK(Tuple.Get<0>() == 42);
        CHECK(Tuple.Get<1>() == 3.14f);
        CHECK(Tuple.Get<2>() == "Hello World");
    }

    SECTION("Test Const Get")
    {
        const TTuple<int, float, std::string> ConstTuple(42, 3.14f, "Hello World");

        CHECK(ConstTuple.Get<0>() == 42);
        CHECK(ConstTuple.Get<1>() == 3.14f);
        CHECK(ConstTuple.Get<2>() == "Hello World");
    }
}
