#include "Engine/Raphael.hxx"

#include "Engine/Core/RTTI.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

class A : public RTTI::Enable
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

class E : public RTTI::Enable
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

    CHECK(aInstance.is<A>());
    CHECK_FALSE(aInstance.is<B>());
    CHECK_FALSE(aInstance.is<C>());
    CHECK_FALSE(aInstance.is<D>());

    CHECK(bInstance.is<A>());
    CHECK(bInstance.is<B>());
    CHECK_FALSE(bInstance.is<C>());
    CHECK_FALSE(bInstance.is<D>());

    CHECK(cInstance.is<A>());
    CHECK(cInstance.is<B>());
    CHECK(cInstance.is<C>());
    CHECK_FALSE(cInstance.is<D>());

    CHECK(dInstance.is<A>());
    CHECK_FALSE(dInstance.is<B>());
    CHECK_FALSE(dInstance.is<C>());
    CHECK(dInstance.is<D>());

    CHECK(aInstance.cast<A>() != nullptr);
    CHECK(aInstance.cast<B>() == nullptr);
    CHECK(aInstance.cast<C>() == nullptr);
    CHECK(aInstance.cast<D>() == nullptr);
    CHECK(aInstance.cast<E>() == nullptr);

    CHECK(bInstance.cast<A>() != nullptr);
    CHECK(bInstance.cast<B>() != nullptr);
    CHECK(bInstance.cast<C>() == nullptr);
    CHECK(bInstance.cast<D>() == nullptr);
    CHECK(bInstance.cast<E>() == nullptr);

    CHECK(cInstance.cast<A>() != nullptr);
    CHECK(cInstance.cast<B>() != nullptr);
    CHECK(cInstance.cast<C>() != nullptr);
    CHECK(cInstance.cast<D>() == nullptr);
    CHECK(cInstance.cast<E>() == nullptr);

    CHECK(dInstance.cast<A>() != nullptr);
    CHECK(dInstance.cast<B>() == nullptr);
    CHECK(dInstance.cast<C>() == nullptr);
    CHECK(dInstance.cast<D>() != nullptr);
    CHECK(dInstance.cast<E>() == nullptr);

    CHECK(eInstance.cast<A>() == nullptr);
    CHECK(eInstance.cast<B>() == nullptr);
    CHECK(eInstance.cast<C>() == nullptr);
    CHECK(eInstance.cast<D>() == nullptr);
    CHECK(eInstance.cast<E>() != nullptr);

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
        A* pCinstance = cInstance.cast<A>();
        REQUIRE(pCinstance != nullptr);
        CHECK(pCinstance->TestFunction() == 2);

        B* pCBinstance = pCinstance->cast<B>();
        REQUIRE(pCBinstance != nullptr);
        CHECK(pCBinstance->TestFunction() == 2);

        C* pCCinstance = pCBinstance->cast<C>();
        REQUIRE(pCCinstance != nullptr);
        CHECK(pCCinstance->TestFunction() == 2);
    }
}
