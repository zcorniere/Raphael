#pragma once

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
            .Type = TypeInfo::Type,                                                                           \
            .Size = sizeof(ParameterName),                                                                    \
            .Offset = offsetof(zzTThisStruct, ParameterName),                                                 \
            .Columns = TypeInfo::NumColumns,                                                                  \
            .Rows = TypeInfo::NumRows,                                                                        \
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

/// The base type of a shader parameter
enum class EShaderBufferType {
    Invalid,

    Struct,

    Int32,
    Uint32,
    Float,

};

/// This structure define a shader parameters (uniforms, storage buffers, etc)
struct ShaderParameter {
    std::string Name = "";
    EShaderBufferType Type = EShaderBufferType::Invalid;
    uint32 Size = 0;
    uint32 Offset = 0;
    uint32 Columns = 0;
    uint32 Rows = 0;

    /// If the parameter is a struct, this array contains the members of the struct
    Array<ShaderParameter> Members;

    bool operator==(const ShaderParameter& Other) const = default;
};
DECLARE_PRINTABLE_TYPE(ShaderParameter);

template <typename T>
struct TSupportedShaderType;

template <>
struct TSupportedShaderType<int32> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Int32;
    static constexpr uint32 NumColumns = 1;
    static constexpr uint32 NumRows = 1;
    static constexpr uint32 Alignment = 4;
    using AlignedType = int32;
};

template <>
struct TSupportedShaderType<uint32> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Uint32;
    static constexpr uint32 NumColumns = 1;
    static constexpr uint32 NumRows = 1;
    static constexpr uint32 Alignment = 4;
    using AlignedType = uint32;
};

template <>
struct TSupportedShaderType<float> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Float;
    static constexpr uint32 NumColumns = 1;
    static constexpr uint32 NumRows = 1;
    static constexpr uint32 Alignment = 4;
    using AlignedType = float;
};

template <>
struct TSupportedShaderType<glm::vec3> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Float;
    static constexpr uint32 NumColumns = 1;
    static constexpr uint32 NumRows = 3;
    static constexpr uint32 Alignment = 16;
    using AlignedType = glm::vec4;
};

template <>
struct TSupportedShaderType<glm::uvec2> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Uint32;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 2;
    static constexpr int32 Alignment = 16;
    using AlignedType = glm::uvec4;
};

template <>
struct TSupportedShaderType<glm::ivec2> {
    static constexpr EShaderBufferType Type = EShaderBufferType::Int32;
    static constexpr float NumColumns = 1;
    static constexpr float NumRows = 2;
    static constexpr int32 Alignment = 16;
    using AlignedType = glm::ivec4;
};

// your code for which the warning gets suppressed
inline std::string PrintShaderParameter(const ShaderParameter& Param, uint32 Indent)
{
    std::string Result = "";
    std::string Padding = "";
    for (uint32 i = 0; i < Indent; ++i) {
        Padding += ("\t");
    }

    Result += std::format("{0}Name: \"{1}\"\n", Padding, Param.Name);
    Padding += "\t";
    Result += std::format("{0}Type: {1}\n", Padding, magic_enum::enum_name(Param.Type));
    Result += std::format("{0}Size: {1}\n", Padding, Param.Size);
    Result += std::format("{0}Offset: {1}\n", Padding, Param.Offset);
    Result += std::format("{0}Columns: {1}\n", Padding, Param.Columns);
    Result += std::format("{0}Rows: {1}\n", Padding, Param.Rows);

    for (const ShaderParameter& Member: Param.Members) {
        Result += PrintShaderParameter(Member, Indent + 2);
    }
    return Result;
}
DEFINE_PRINTABLE_TYPE(ShaderParameter, "ShaderParameter{{\n{0}\n}}", PrintShaderParameter(Value, 0));
