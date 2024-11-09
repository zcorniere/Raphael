#include "Engine/Raphael.hxx"

#include "Engine/Misc/CommandLine.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Sanity test : Simple command line")
{
    FCommandLine::Set("Test");
    REQUIRE(strcmp(FCommandLine::Get(), "Test") == 0);
    FCommandLine::Reset();
}

TEST_CASE("Command line with multiple argument")
{
    FCommandLine::Set("-intarg=42 -qargument=\"Test Value Argument\" -argument=TestValue Argument");

    SECTION("Command line with quote argument")
    {
        std::string value;
        REQUIRE(FCommandLine::Parse("-qargument=", value));
        REQUIRE(value == "Test Value Argument");
    }
    SECTION("Command line without quote argument")
    {
        std::string value;
        REQUIRE(FCommandLine::Parse("-argument=", value));
        REQUIRE(value == "TestValue");
    }
    SECTION("Command line with int argument")
    {
        int value;
        REQUIRE(FCommandLine::Parse("-intarg=", value));
        REQUIRE(value == 42);
    }
    FCommandLine::Reset();
}
