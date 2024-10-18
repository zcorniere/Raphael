#include "Engine/Raphael.hxx"

#include "Engine/Containers/String.hxx"
#include "Engine/Containers/StringView.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("StringView: Basic functionality")
{
    String TestString("Hello, World!");
    StringView TestView(TestString);

    REQUIRE(TestView.Size() == TestString.Size());
    REQUIRE(strcmp(TestView.Raw(), TestString.Raw()) == 0);

    SECTION("Test ToLowerCase")
    {
        String ExpectedString("hello, world!");
        TestString.ToLowerCase();
        CHECK(TestString == ExpectedString);
    }

    SECTION("Test ToUpperCase")
    {
        String ExpectedString("HELLO, WORLD!");
        TestString.ToUpperCase();
        CHECK(TestString == ExpectedString);
    }
}
