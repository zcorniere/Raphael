#include "Engine/Raphael.hxx"

#include "Engine/Containers/String.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("String: Basic functionality")
{
    std::string TestStringOriginal("Hello, World!");
    String TestString("Hello, World!");

    REQUIRE(TestString.Size() == TestStringOriginal.size());
    REQUIRE(strcmp(TestString.Raw(), TestStringOriginal.c_str()) == 0);

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

    SECTION("Test += operator")
    {
        SECTION("Test += operator with TString")
        {
            String ExpectedString("Hello, World! Goodbye World!");
            String TestString2(" Goodbye World!");
            TestString += TestString2;
            CHECK(TestString.Size() == ExpectedString.Size());
            CHECK(TestString == ExpectedString);
        }
        SECTION("Test += operator with char pointer")
        {
            String ExpectedString("Hello, World! Goodbye World!");
            TestString += " Goodbye World!";
            CHECK(TestString.Size() == ExpectedString.Size());
            CHECK(TestString == ExpectedString);
        }
    }
}
