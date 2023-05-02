/// Resource Entry Implementation

#define RHI_CREATE_FUNCTION(Type)                                                                                  \
    if constexpr (ResourceType == RHIResourceType::Type) {                                                         \
        m_Resource = UnpackTupleAndCallRHIFunction(RHI::Create##Type, typename Gens<sizeof...(ArgTypes)>::type()); \
    } else

template <RHIResourceType ResourceType, typename... ArgTypes>
void TResourceEntry<ResourceType, ArgTypes...>::ConstructResource()
{
    verify(!m_Resource);

    RHI_CREATE_FUNCTION(Viewport)
    RHI_CREATE_FUNCTION(Shader)
    RHI_CREATE_FUNCTION(Texture) { static_assert(AlwaysFalse<ArgTypes...>); }
}

#undef RHI_CREATE_FUNCTION
