#include "Engine/Raphael.hxx"

#include "Engine/Containers/ArrayView.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("TArrayView")
{
    int Value1 = GENERATE(take(2, random(-42, 50)));
    int Value2 = GENERATE(take(2, random(-4200, 420)));
    int Value3 = GENERATE(take(2, random(42, 50)));
    int Value4 = GENERATE(take(2, random(0, 123)));
    int Array[4] = {Value1, Value2, Value3, Value4};
    TArrayView<int> View(Array);

    CHECK(View.Size() == 4);
    CHECK(View[0] == Value1);
    CHECK(View[1] == Value2);
    CHECK(View[2] == Value3);
    CHECK(View[3] == Value4);

    SECTION("Test begin() and end()")
    {
        CHECK(View.begin() == Array);
        CHECK(View.end() == Array + 4);
    }

    SECTION("Test Raw()")
    {
        CHECK(View.Raw() == Array);
    }

    SECTION("Test operator==")
    {
        int Array2[4] = {Value1, Value2, Value3, Value4};
        TArrayView<int> View2(Array2);
        CHECK(View == View2);
    }
}
