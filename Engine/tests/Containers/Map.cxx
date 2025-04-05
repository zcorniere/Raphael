#include "Engine/Raphael.hxx"

#include "Engine/Containers/Map.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Map Tests")
{
    TMap<int, std::string> TestMap;

    TestMap.Insert(42, "Hello");
    TestMap.Insert(43, "World");

    CHECK(TestMap.Size() == 2);
    CHECK(TestMap.Find(42) != nullptr);
    CHECK(TestMap.Find(43) != nullptr);
    CHECK(TestMap.Find(44) == nullptr);

    TestMap.Remove(42);
    CHECK(TestMap.Size() == 1);
    CHECK(TestMap.Find(42) == nullptr);
    CHECK(TestMap.Find(43) != nullptr);
    CHECK(TestMap.Find(44) == nullptr);

    const std::string* const Value = TestMap.Find(43);
    CHECK(*Value == "World");

    CHECK(TestMap.Contains(43));
    CHECK_FALSE(TestMap.Contains(42));

    SECTION("Find or add")
    {
        std::string& Value = TestMap.FindOrAdd(42);
        CHECK(Value == "");

        Value = "Hello";
        CHECK(TestMap.Find(42) != nullptr);
        CHECK(*TestMap.Find(42) == "Hello");

        Value = TestMap.FindOrAdd(43);
        CHECK(Value == "World");
    }

    SECTION("Emplace")
    {
        std::string& Value = TestMap.Emplace(42, "Hello");
        CHECK(Value == "Hello");

        Value = "World";
        CHECK(TestMap.Find(42) != nullptr);
        CHECK(*TestMap.Find(42) == "World");

        Value = TestMap.Emplace(43, "World");
        CHECK(Value == "World");
    }

    SECTION("Iterator")
    {
        TestMap.Insert(44, "Hello");
        TestMap.Insert(45, "World");

        int ExpectedKey = 43;
        for (TPair<int, std::string>& Pair: TestMap) {
            CHECK(Pair.Get<0>() == ExpectedKey);
            ExpectedKey++;
        }
    }

    SECTION("Const Iterator")
    {
        TestMap.Insert(44, "Hello");
        TestMap.Insert(45, "World");

        int ExpectedKey = 43;
        for (TPair<int, std::string>& Pair: TestMap) {
            CHECK(Pair.Get<0>() == ExpectedKey);
            ExpectedKey++;
        }
    }
}
