#include "Engine/Raphael.hxx"

#include "Engine/Misc/CommandLine.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Sanity test : Simple command line")
{
    CommandLine::Set("Test");
    REQUIRE(strcmp(CommandLine::Get(), "Test") == 0);
    CommandLine::Reset();
}

TEST_CASE("Command line with multiple argument")
{
    CommandLine::Set("-intarg=42 -qargument=\"Test Value Argument\" -argument=TestValue Argument");

    SECTION("Command line with quote argument")
    {
        std::string value;
        REQUIRE(CommandLine::Parse("-qargument=", value));
        REQUIRE(value == "Test Value Argument");
    }
    SECTION("Command line without quote argument")
    {
        std::string value;
        REQUIRE(CommandLine::Parse("-argument=", value));
        REQUIRE(value == "TestValue");
    }
    SECTION("Command line with int argument")
    {
        int value;
        REQUIRE(CommandLine::Parse("-intarg=", value));
        REQUIRE(value == 42);
    }
    CommandLine::Reset();
}
