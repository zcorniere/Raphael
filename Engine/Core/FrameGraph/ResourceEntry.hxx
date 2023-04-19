#pragma once

#include "Engine/Core/UUID.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

#include <functional>

class PassNode;

class ResourceEntry : public Raphael::UUID, public RObject
{
public:
    ResourceEntry(Ref<RHIResource> InResource = nullptr, uint32 InVersion = 1);

    virtual void ConstructResource() = 0;
    virtual void DestroyResource() = 0;

    uint32 GetVersion() const { return m_Version; }

protected:
    uint32 m_Version;
    Ref<RHIResource> m_Resource;

    Ref<PassNode> Producer;
    Ref<PassNode> Last;

    friend class FrameGraph;
};

template <RHIResourceType Type, typename... ArgTypes>
class TResourceEntry : public ResourceEntry
{
private:
    // Helper templates that are used to call a function using
    template <int...>
    struct Sequence {
    };

    template <int N, int... S>
    struct Gens : Gens<N - 1, N - 1, S...> {
    };

    template <int... S>
    struct Gens<0, S...> {
        typedef Sequence<S...> type;
    };

public:
    TResourceEntry(uint32 InVersion, ArgTypes &&...args)
        : ResourceEntry(nullptr, InVersion), Arguments(std::forward<ArgTypes>(args)...)
    {
    }

    void ConstructResource() override
    {
        verify(m_Resource);
        m_Resource = CallRHIFunctionAndUnpackTuple(typename Gens<sizeof...(ArgTypes)>::type());
    }
    void DestroyResource() override { m_Resource = nullptr; }

private:
    template <int... S>
    Ref<RHIResource> CallRHIFunctionAndUnpackTuple(Sequence<S...>)
    {
        return RHI::Create<Type>(std::get<S>(Arguments)...);
    }

private:
    const std::tuple<ArgTypes...> Arguments;
};
