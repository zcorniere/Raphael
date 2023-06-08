#include "Engine/Raphael.hxx"

#include "Engine/Containers/Array.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Array: Basic Operation")
{
    int Value1 = GENERATE(take(2, random(-42, 50)));
    int Value2 = GENERATE(take(2, random(-4200, 420)));
    int Value3 = GENERATE(take(2, random(42, 50)));
    int Value4 = GENERATE(take(2, random(0, 123)));

    Array<int> TestVec;
    CHECK(TestVec.IsEmpty());
    CHECK(TestVec.Size() == 0);

    CHECK_NOTHROW(TestVec.Add(Value1));
    CHECK_NOTHROW(TestVec.Size() == 1);
    CHECK_NOTHROW(TestVec[0] == Value1);

    CHECK_NOTHROW(TestVec.Add(Value2));
    CHECK_NOTHROW(TestVec.Size() == 2);
    CHECK_NOTHROW(TestVec[0] == Value1);
    CHECK_NOTHROW(TestVec[1] == Value2);

    CHECK_NOTHROW(TestVec.Emplace(Value3));
    CHECK_NOTHROW(TestVec.Size() == 3);
    CHECK_NOTHROW(TestVec[0] == Value1);
    CHECK_NOTHROW(TestVec[1] == Value2);
    CHECK_NOTHROW(TestVec[2] == Value3);

    SECTION("Test positive Resize")
    {
        CHECK_NOTHROW(TestVec.Resize(20));
        REQUIRE(TestVec.Size() == 20);

        CHECK_NOTHROW(TestVec[0] == Value1);
        CHECK_NOTHROW(TestVec.Add(100));
        CHECK_NOTHROW(TestVec[20] == 100);
    }

    SECTION("Test Negative resize")
    {
        CHECK_NOTHROW(TestVec.Resize(20));
        REQUIRE(TestVec.Size() == 20);

        CHECK_NOTHROW(TestVec[0] == Value1);
        CHECK_NOTHROW(TestVec[1] == Value2);
        CHECK_NOTHROW(TestVec[2] == Value3);
        for (unsigned Index = 3; Index < TestVec.Size(); Index++) {
            CHECK_NOTHROW(TestVec[Index] == 0);
        }

        CHECK_NOTHROW(TestVec.Add(Value4));
        CHECK_NOTHROW(TestVec[20] == Value4);
    }

    SECTION("Test Append")
    {
        const int SizeBefore = TestVec.Size();
        Array<int> TestVec2{10, 20, 30};

        TestVec.Append(TestVec2);
        CHECK(TestVec.Size() == SizeBefore + 3);
    }

    SECTION("Test simple Clear")
    {
        TestVec.Clear();

        CHECK(TestVec.IsEmpty());
    }

    SECTION("Test Back() and PopBack()")
    {
        SECTION("Test Back()")
        {
            int* Ref = &TestVec.Add(654);
            REQUIRE(Ref != nullptr);
            CHECK(Ref == &TestVec.Back());
            CHECK(*Ref == TestVec.Back());

            SECTION("Test Pop()")
            {
                auto old_size = TestVec.Size();
                int Value = TestVec.Pop();

                CHECK(Value == 654);
                CHECK(TestVec.Size() == old_size - 1);
            }
        }
    }
}

TEST_CASE("Array: Test Advanced Type")
{
    int DtorCounter = 0;
    struct ComplexType {
        int& Value;
        ComplexType(int& InValue): Value(InValue)
        {
            Value += 1;
        }
        ComplexType(const ComplexType &Other):  ComplexType(Other.Value) {}
        ~ComplexType()
        {
            Value -= 1;
        }
    };

    Array<ComplexType> Vec2;
    Vec2.Emplace(ComplexType(DtorCounter));
    Vec2.Add(ComplexType(DtorCounter));

    CHECK_NOTHROW(Vec2.Size() == 2);
    REQUIRE(DtorCounter == 2);

    SECTION("Test Append")
    {
        Array<ComplexType> TestVec2;

        TestVec2.Emplace(ComplexType(DtorCounter));
        TestVec2.Emplace(ComplexType(DtorCounter));
        REQUIRE(DtorCounter == 4);

        Vec2.Append(TestVec2);
        REQUIRE(DtorCounter == 6);
        CHECK(Vec2.Size() == 4);

        TestVec2.Clear();
        REQUIRE(DtorCounter == 4);

    }

    Vec2.Clear();
    REQUIRE(DtorCounter == 0);
}

TEST_CASE("Array: Test Find function")
{
    int Value1 = GENERATE(take(2, random(-42, 50)));
    int Value2 = GENERATE(take(2, random(-4200, 420)));
    int Value3 = GENERATE(take(2, random(42, 50)));
    Array<int> TestVec{Value1, Value2, Value3};

    SECTION("Make sur the vector is correctly init")
    {
        REQUIRE(!TestVec.IsEmpty());
        REQUIRE(TestVec.Size() == 3);
        REQUIRE_NOTHROW(TestVec[0] == Value1);
        REQUIRE_NOTHROW(TestVec[1] == Value2);
        REQUIRE_NOTHROW(TestVec[2] == Value3);
    }

    SECTION("Test Find()")
    {
        CHECK(TestVec.Find(Value1) == 0);
        CHECK(TestVec.Find(Value2) == 1);
        CHECK(TestVec.Find(Value3) == 2);

        // 10000 is not in the Vector
        CHECK(TestVec.Find(10000) == InvalidVectorIndex);
    }
}
