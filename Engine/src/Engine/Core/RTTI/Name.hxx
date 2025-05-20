#pragma once

#include "Engine/Core/RTTI/TypeName.hxx"

#include "Engine/Containers/Array.hxx"

namespace RTTI
{

class FName
{
public:
    FName();
    FName(const FName&) = default;
    explicit FName(const std::string& name);
    ~FName() = default;

    void Set(const std::string& Name);
    int32 GetIndex() const
    {
        return Index;
    }
    std::string ToString() const;

    bool operator==(const FName& Other) const
    {
        return Index == Other.Index;
    }

private:
    int32 Index;
    const char* Name = nullptr;
};

class FNamePool
{
public:
    static constexpr int32 InvalidIndex = -1;
    struct FNameHolder {
        std::string Name;
        int32 Index = 0;

        FNameHolder(const std::string& name, int32 index): Name(name), Index(index)
        {
            assert(!name.empty());
        }

        bool operator==(const FNameHolder& Other) const
        {
            return Name == Other.Name;
        }
    };

    struct FStaticNameHolder : public FNameHolder {
        FStaticNameHolder(const std::string& name, int32 index): FNameHolder(name, index)
        {
            FNamePool::Get().AddStaticName(*this);
        }
    };

public:
    static FNamePool& Get()
    {
        static FNamePool Instance;
        return Instance;
    }

public:
    FNamePool() = default;
    RPH_NONCOPYABLE(FNamePool)

    const std::string& FindName(uint32 Index) const;
    void AddStaticName(FNameHolder name);
    uint32 AddDynamicName(const std::string& Name);

private:
    static constexpr int32 MaxNamePoolSize = 1'000;
    TArray<FNameHolder, MaxNamePoolSize> NamePool = 0;
    mutable std::mutex Mutex;
};

inline std::string FName::ToString() const
{
    if (Index == FNamePool::InvalidIndex) {
        return "None";
    }
    return FNamePool::Get().FindName(Index);
}

}    // namespace RTTI

DEFINE_PRINTABLE_TYPE(RTTI::FName, "{}", RTTI::FNamePool::Get().FindName(Value.GetIndex()))

#define RTTI_DECLARE_NAME(Name)                            \
    namespace RTTI                                         \
    {                                                      \
        extern FNamePool::FStaticNameHolder holder_##Name; \
        namespace Names                                    \
        {                                                  \
            extern const FName name_##Name;                \
        }                                                  \
    }                                                      \
    RTTI_DEFINE_TYPENAME(Name);

#define RTTI_DECLARE_NAME_TEMPLATE(Name, TemplateParameter)                   \
    namespace RTTI                                                            \
    {                                                                         \
        extern FNamePool::FStaticNameHolder holder_##Name##TemplateParameter; \
        namespace Names                                                       \
        {                                                                     \
            extern const FName name_##Name##TemplateParameter;                \
        }                                                                     \
    }                                                                         \
    RTTI_DEFINE_TYPENAME(Name, TemplateParameter);

#define RTTI_DEFINE_NAME(Name)                                 \
    namespace RTTI                                             \
    {                                                          \
        FNamePool::FStaticNameHolder holder_##Name(#Name, -1); \
        namespace Names                                        \
        {                                                      \
            const FName name_##Name = FName(#Name);            \
        }                                                      \
    }

#define RTTI_DEFINE_NAME_TEMPLATE(Name, TemplateParameter)                                        \
    namespace RTTI                                                                                \
    {                                                                                             \
        FNamePool::FStaticNameHolder holder_##Name##TemplateParameter(#Name, -1);                 \
        namespace Names                                                                           \
        {                                                                                         \
            const FName name_##Name##TemplateParameter = FName(#Name "<" #TemplateParameter ">"); \
        }                                                                                         \
    }
