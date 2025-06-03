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

    TArray<int> TestVec;
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

    const std::string String = std::format("{}", TestVec);
    const std::string ExpectedString = std::format("[{0}, {1}, {2}]", Value1, Value2, Value3);
    CHECK(String == ExpectedString);

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
        for (unsigned Index = 3; Index < TestVec.Size(); Index++)
        {
            CHECK_NOTHROW(TestVec[Index] == 0);
        }

        CHECK_NOTHROW(TestVec.Add(Value4));
        CHECK_NOTHROW(TestVec[20] == Value4);
    }

    SECTION("Test Append")
    {
        const int SizeBefore = TestVec.Size();
        TArray<int> TestVec2{10, 20, 30};

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

struct ComplexType
{
    int* Value = nullptr;

    explicit ComplexType(int* const InValue): Value(InValue)
    {
        *Value += 1;
    }
    ComplexType(const ComplexType& Other): ComplexType(Other.Value)
    {
    }
    ComplexType(ComplexType&& Other) noexcept: Value(Other.Value)
    {
        *Value += 1;
    }
    ~ComplexType() noexcept
    {
        *Value -= 1;
    }

    ComplexType& operator=(const ComplexType& Other)
    {
        Value = Other.Value;
        *Value += 1;
        return *this;
    }

    ComplexType& operator=(ComplexType&& Other) noexcept
    {
        Value = Other.Value;
        Other.Value = nullptr;
        return *this;
    }
};
bool operator==(const ComplexType& Lhs, const ComplexType& Rhs)
{
    return Lhs.Value == Rhs.Value;
}

TEST_CASE("Array: Test Advanced Type")
{
    int DtorCounter = 0;

    static_assert(std::is_trivially_destructible<ComplexType>::value == false,
                  "ComplexType is trivially destructible and should not be");

    REQUIRE(DtorCounter == 0);
    ComplexType* TestType = new ComplexType(&DtorCounter);
    REQUIRE(DtorCounter == 1);

    TArray<ComplexType> Vec2;
    Vec2.Emplace(&DtorCounter);
    Vec2.Add(*TestType);

    CHECK_NOTHROW(Vec2.Size() == 2);
    REQUIRE(DtorCounter == 2 + 1);    // 1 for TestType, 2 for Vec2

    SECTION("Test Append")
    {
        TArray<ComplexType> TestVec2;

        TestVec2.Emplace(&DtorCounter);
        TestVec2.Emplace(&DtorCounter);
        REQUIRE(DtorCounter == 1 + 2 + 2);    // 1 for TestType, 2 for Vec2, and 2 for TestVec2

        Vec2.Append(TestVec2);
        REQUIRE(DtorCounter ==
                1 + 2 + 2 + 2    // 1 for TestType, 2 for Vec2, 2 for TestVec2, and 2 for the copy during the append
        );
        CHECK(Vec2.Size() == 4);

        TestVec2.Clear();
        REQUIRE(DtorCounter == 1 + 2 + 2);
    }

    SECTION("Test Remove")
    {
        ComplexType* TestType2 = TestType;
        Vec2.Remove(*TestType2);
        REQUIRE(DtorCounter == 1 + 1);    // 1 for TestType, 2 for Vec2
        CHECK(Vec2.Size() == 1);
    }
    SECTION("Test RemoveAt")
    {
        Vec2.RemoveAt(0);
        REQUIRE(DtorCounter == 1 + 1);    // 1 for TestType, 1 for Vec2
        CHECK(Vec2.Size() == 1);
    }

    Vec2.Clear();
    REQUIRE(Vec2.IsEmpty());
    REQUIRE(DtorCounter == 1);    // 1 for TestType
    delete TestType;
    REQUIRE(DtorCounter == 0);
}

TEST_CASE("Array: Test Find function")
{

    int Value1 = GENERATE(take(2, random(-10, 10)));
    int Value2 = GENERATE(take(2, random(100, 420)));
    int Value3 = GENERATE(take(2, random(-200, -11)));
    TArray<int> TestVec{Value1, Value2, Value3};

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
        CHECK(TestVec.Find(Value2) == 1);
        CHECK(TestVec.Find(Value3) == 2);
        CHECK(TestVec.Find(Value1) == 0);

        // 10000 is not in the Vector
        CHECK(TestVec.Find(10000) == std::nullopt);
    }

    SECTION("Test AddUnique()")
    {
        int Value4 = GENERATE(take(2, random(11, 99)));

        CHECK(TestVec.AddUnique(Value1) == false);
        CHECK(TestVec.AddUnique(Value2) == false);
        CHECK(TestVec.AddUnique(Value3) == false);
        CHECK(TestVec.AddUnique(Value4) == true);

        CHECK(TestVec.Size() == 4);
        CHECK(TestVec[3] == Value4);
    }
}
