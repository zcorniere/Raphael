#pragma once

#include "Engine/Misc/MiscDefines.hxx"
#include "glm/fwd.hpp"

struct ShaderParameter {
    std::string Name;
    unsigned Offset = 0;
    unsigned Size = 0;

    bool operator==(const ShaderParameter& Other) const = default;
};

#define BEGIN_SHADER_PARAMETER_STRUCT(StructureName)                                      \
    struct alignas(16) StructureName {                                                    \
    public:                                                                               \
        static constexpr std::source_location Location = std::source_location::current(); \
        StructureName()                                                                   \
        {                                                                                 \
            memset(this, 0, sizeof(*this));                                               \
        }                                                                                 \
                                                                                          \
    private:                                                                              \
        typedef StructureName zzTThisStruct;                                              \
                                                                                          \
        struct zzFirstMemberId {                                                          \
        };                                                                                \
        typedef void* zzFuncPtr;                                                          \
        typedef zzFuncPtr (*zzMemberFunc)(zzFirstMemberId, Array<ShaderParameter>*);      \
        static zzFuncPtr zzGetPreviousMember(zzFirstMemberId, Array<ShaderParameter>*)    \
        {                                                                                 \
            return nullptr;                                                               \
        }                                                                                 \
        typedef zzFirstMemberId

#define SHADER_PARAMETER(ParameterType, ParameterName) \
    SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, TSupportedShaderType<ParameterType>)

#define SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, TypeInfo)                                     \
    zzMemberId##ParameterName;                                                                                \
                                                                                                              \
public:                                                                                                       \
    alignas(TypeInfo::Alignment) TypeInfo::AlignedType ParameterName;                                         \
                                                                                                              \
private:                                                                                                      \
    struct zzNextMemberId##ParameterName {                                                                    \
    };                                                                                                        \
    static zzFuncPtr zzGetPreviousMember(zzNextMemberId##ParameterName, Array<ShaderParameter>* zzParameters) \
    {                                                                                                         \
        zzParameters->Add(ShaderParameter{                                                                    \
            .Name = #ParameterName,                                                                           \
            .Offset = offsetof(zzTThisStruct, ParameterName),                                                 \
            .Size = sizeof(ParameterName),                                                                    \
        });                                                                                                   \
        zzFuncPtr (*PrevFunc)(zzMemberId##ParameterName, Array<ShaderParameter>*);                            \
        PrevFunc = zzGetPreviousMember;                                                                       \
        return (zzFuncPtr)PrevFunc;                                                                           \
    }                                                                                                         \
    typedef zzNextMemberId##ParameterName

#define END_SHADER_PARAMETER_STRUCT()                                               \
    zzLastMemberId;                                                                 \
                                                                                    \
public:                                                                             \
    static Array<ShaderParameter> GetMembers()                                      \
    {                                                                               \
        Array<ShaderParameter> Members;                                             \
        zzFuncPtr (*LastFunc)(zzLastMemberId, Array<ShaderParameter>*);             \
        LastFunc = zzGetPreviousMember;                                             \
        zzFuncPtr Ptr = (zzFuncPtr)LastFunc;                                        \
        do {                                                                        \
            Ptr = reinterpret_cast<zzMemberFunc>(Ptr)(zzFirstMemberId(), &Members); \
        } while (Ptr);                                                              \
        std::reverse(Members.begin(), Members.end());                               \
        return Members;                                                             \
    }                                                                               \
    }

enum class EUniformBufferType {
    Invalid,

    Int32,
    Uint32,
    Float,

};

template <typename T>
struct TSupportedShaderType;

template <>
struct TSupportedShaderType<int32> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Int32;
    static constexpr int32 NumColumns = 1;
    static constexpr int32 NumRows = 1;
    static constexpr int32 Alignment = 4;
    using AlignedType = int32;
};

template <>
struct TSupportedShaderType<uint32> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Uint32;
    static constexpr uint32 NumColumns = 1;
    static constexpr uint32 NumRows = 1;
    static constexpr int32 Alignment = 4;
    using AlignedType = uint32;
};

template <>
struct TSupportedShaderType<float> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Float;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 1;
    static constexpr int32 Alignment = 4;
    using AlignedType = float;
};

template <>
struct TSupportedShaderType<glm::vec3> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Float;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 3;
    static constexpr int32 Alignment = 16;
    using AlignedType = glm::vec4;
};

template <>
struct TSupportedShaderType<glm::uvec2> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Uint32;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 2;
    static constexpr int32 Alignment = 16;
    using AlignedType = glm::uvec4;
};

template <>
struct TSupportedShaderType<glm::ivec2> {
    static constexpr EUniformBufferType Type = EUniformBufferType::Int32;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 2;
    static constexpr int32 Alignment = 16;
    using AlignedType = glm::ivec4;
};
