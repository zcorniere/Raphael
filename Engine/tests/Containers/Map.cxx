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

    SECTION("Clear")
    {
        TestMap.Clear();
        CHECK(TestMap.Size() == 0);
        CHECK(TestMap.Find(42) == nullptr);
        CHECK(TestMap.Find(43) == nullptr);
        CHECK(TestMap.Find(44) == nullptr);
    }
}

TEST_CASE("Map Iterators")
{
    // Lower the min bucket size, to force rehashing
    TMap<int, std::string, std::hash<int>, 0.75f, uint32, 2> TestMap;
    TestMap.Insert(44, "Hello");
    TestMap.Insert(45, "World");
    TestMap.Insert(46, "!");
    TestMap.Insert(47, "Test");
    TestMap.Insert(48, "123");
    TestMap.Insert(49, "456");
    TestMap.Insert(50, "789");

    SECTION("Iterator")
    {
        for (TPair<int, std::string>& Pair: TestMap) {
            if (Pair.Get<0>() == 44) {
                CHECK(Pair.Get<1>() == "Hello");
            } else if (Pair.Get<0>() == 45) {
                CHECK(Pair.Get<1>() == "World");
            } else if (Pair.Get<0>() == 46) {
                CHECK(Pair.Get<1>() == "!");
            } else if (Pair.Get<0>() == 47) {
                CHECK(Pair.Get<1>() == "Test");
            } else if (Pair.Get<0>() == 48) {
                CHECK(Pair.Get<1>() == "123");
            } else if (Pair.Get<0>() == 49) {
                CHECK(Pair.Get<1>() == "456");
            } else if (Pair.Get<0>() == 50) {
                CHECK(Pair.Get<1>() == "789");
            } else {
                CHECK(false);
            }
        }
    }

    SECTION("Iterator with structured binding")
    {
        for (auto& [Key, Value]: TestMap) {
            if (Key == 44) {
                CHECK(Value == "Hello");
            } else if (Key == 45) {
                CHECK(Value == "World");
            } else if (Key == 46) {
                CHECK(Value == "!");
            } else if (Key == 47) {
                CHECK(Value == "Test");
            } else if (Key == 48) {
                CHECK(Value == "123");
            } else if (Key == 49) {
                CHECK(Value == "456");
            } else if (Key == 50) {
                CHECK(Value == "789");
            } else {
                CHECK(false);
            }
        }
    }

    SECTION("Const Iterator")
    {
        const TMap<int, std::string, std::hash<int>, 0.75f, uint32, 2>& ConstTestMap = TestMap;
        for (const TPair<int, std::string>& Pair: ConstTestMap) {
            if (Pair.Get<0>() == 44) {
                CHECK(Pair.Get<1>() == "Hello");
            } else if (Pair.Get<0>() == 45) {
                CHECK(Pair.Get<1>() == "World");
            } else if (Pair.Get<0>() == 46) {
                CHECK(Pair.Get<1>() == "!");
            } else if (Pair.Get<0>() == 47) {
                CHECK(Pair.Get<1>() == "Test");
            } else if (Pair.Get<0>() == 48) {
                CHECK(Pair.Get<1>() == "123");
            } else if (Pair.Get<0>() == 49) {
                CHECK(Pair.Get<1>() == "456");
            } else if (Pair.Get<0>() == 50) {
                CHECK(Pair.Get<1>() == "789");
            } else {
                CHECK(false);
            }
        }
    }

    SECTION("Const Iterator with structured binding")
    {
        const TMap<int, std::string, std::hash<int>, 0.75f, uint32, 2>& ConstTestMap = TestMap;
        for (const auto& [Key, Value]: ConstTestMap) {
            if (Key == 44) {
                CHECK(Value == "Hello");
            } else if (Key == 45) {
                CHECK(Value == "World");
            } else if (Key == 46) {
                CHECK(Value == "!");
            } else if (Key == 47) {
                CHECK(Value == "Test");
            } else if (Key == 48) {
                CHECK(Value == "123");
            } else if (Key == 49) {
                CHECK(Value == "456");
            } else if (Key == 50) {
                CHECK(Value == "789");
            } else {
                CHECK(false);
            }
        }
    }
}
