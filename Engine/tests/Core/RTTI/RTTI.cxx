#include "Engine/Raphael.hxx"

#include "Engine/Core/RTTI/RTTI.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

class A : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(A);

public:
    virtual int TestFunction()
    {
        return 0;
    }
};

class B : public A
{
    RTTI_DECLARE_TYPEINFO(B, A);

public:
    virtual int TestFunction() override
    {
        return 1;
    }
};

class C : public B
{
    RTTI_DECLARE_TYPEINFO(C, B);

public:
    virtual int TestFunction() override
    {
        return 2;
    }
};

class D : public A
{
    RTTI_DECLARE_TYPEINFO(D, A);

public:
    virtual int TestFunction() override
    {
        return 3;
    }
};

class E : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(E);
};

TEST_CASE("RTTI")
{
    A aInstance;
    B bInstance;
    C cInstance;
    D dInstance;
    E eInstance;

    CHECK(aInstance.Is<A>());
    CHECK_FALSE(aInstance.Is<B>());
    CHECK_FALSE(aInstance.Is<C>());
    CHECK_FALSE(aInstance.Is<D>());

    CHECK(bInstance.Is<A>());
    CHECK(bInstance.Is<B>());
    CHECK_FALSE(bInstance.Is<C>());
    CHECK_FALSE(bInstance.Is<D>());

    CHECK(cInstance.Is<A>());
    CHECK(cInstance.Is<B>());
    CHECK(cInstance.Is<C>());
    CHECK_FALSE(cInstance.Is<D>());

    CHECK(dInstance.Is<A>());
    CHECK_FALSE(dInstance.Is<B>());
    CHECK_FALSE(dInstance.Is<C>());
    CHECK(dInstance.Is<D>());

    CHECK(aInstance.Cast<A>() != nullptr);
    CHECK(aInstance.Cast<B>() == nullptr);
    CHECK(aInstance.Cast<C>() == nullptr);
    CHECK(aInstance.Cast<D>() == nullptr);
    CHECK(aInstance.Cast<E>() == nullptr);

    CHECK(bInstance.Cast<A>() != nullptr);
    CHECK(bInstance.Cast<B>() != nullptr);
    CHECK(bInstance.Cast<C>() == nullptr);
    CHECK(bInstance.Cast<D>() == nullptr);
    CHECK(bInstance.Cast<E>() == nullptr);

    CHECK(cInstance.Cast<A>() != nullptr);
    CHECK(cInstance.Cast<B>() != nullptr);
    CHECK(cInstance.Cast<C>() != nullptr);
    CHECK(cInstance.Cast<D>() == nullptr);
    CHECK(cInstance.Cast<E>() == nullptr);

    CHECK(dInstance.Cast<A>() != nullptr);
    CHECK(dInstance.Cast<B>() == nullptr);
    CHECK(dInstance.Cast<C>() == nullptr);
    CHECK(dInstance.Cast<D>() != nullptr);
    CHECK(dInstance.Cast<E>() == nullptr);

    CHECK(eInstance.Cast<A>() == nullptr);
    CHECK(eInstance.Cast<B>() == nullptr);
    CHECK(eInstance.Cast<C>() == nullptr);
    CHECK(eInstance.Cast<D>() == nullptr);
    CHECK(eInstance.Cast<E>() != nullptr);

    SECTION("Test if type info is correct")
    {
        CHECK(aInstance.GetTypeName() == "A");
        CHECK(bInstance.GetTypeName() == "B");
        CHECK(cInstance.GetTypeName() == "C");
        CHECK(dInstance.GetTypeName() == "D");
        CHECK(eInstance.GetTypeName() == "E");
    }

    SECTION("Test if cast works correctly")
    {
        // Test if the correct function is called
        A* pCinstance = cInstance.Cast<A>();
        REQUIRE(pCinstance != nullptr);
        CHECK(pCinstance->TestFunction() == 2);

        B* pCBinstance = pCinstance->Cast<B>();
        REQUIRE(pCBinstance != nullptr);
        CHECK(pCBinstance->TestFunction() == 2);

        C* pCCinstance = pCBinstance->Cast<C>();
        REQUIRE(pCCinstance != nullptr);
        CHECK(pCCinstance->TestFunction() == 2);
    }
}
