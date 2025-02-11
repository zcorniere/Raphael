#pragma once

#include "Engine/Containers/Array.hxx"

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#define BEGIN_PARAMETER_STRUCT(StructureName)                                              \
    struct alignas(16) StructureName {                                                     \
        RTTI_DECLARE_TYPEINFO_MINIMAL(StructureName);                                      \
                                                                                           \
    public:                                                                                \
        static constexpr std::source_location Location = std::source_location::current();  \
        StructureName()                                                                    \
        {                                                                                  \
            static_assert(std::is_standard_layout<StructureName>::value,                   \
                          #StructureName " must be trivially constructible");              \
            std::memset(this, 0, sizeof(*this));                                           \
        }                                                                                  \
                                                                                           \
    private:                                                                               \
        typedef StructureName zzTThisStruct;                                               \
                                                                                           \
        struct zzFirstMemberId {                                                           \
        };                                                                                 \
        typedef void* zzFuncPtr;                                                           \
        typedef zzFuncPtr (*zzMemberFunc)(zzFirstMemberId, TArray<::RTTI::FParameter>*);   \
        static zzFuncPtr zzGetPreviousMember(zzFirstMemberId, TArray<::RTTI::FParameter>*) \
        {                                                                                  \
            return nullptr;                                                                \
        }                                                                                  \
        typedef zzFirstMemberId

#define PARAMETER(ParameterType, ParameterName) \
    SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, ::RTTI::TSupportedParameterType<ParameterType>)

#define SHADER_PARAMETER_INTERNAL(ParameterType, ParameterName, TypeInfo)                                         \
    zzMemberId##ParameterName;                                                                                    \
                                                                                                                  \
public:                                                                                                           \
    alignas(TypeInfo::Alignment) TypeInfo::AlignedType ParameterName;                                             \
                                                                                                                  \
private:                                                                                                          \
    struct zzNextMemberId##ParameterName {                                                                        \
    };                                                                                                            \
    static zzFuncPtr zzGetPreviousMember(zzNextMemberId##ParameterName, TArray<::RTTI::FParameter>* zzParameters) \
    {                                                                                                             \
        zzParameters->Add(::RTTI::FParameter{                                                                     \
            .Name = #ParameterName,                                                                               \
            .Type = TypeInfo::Type,                                                                               \
            .Size = sizeof(ParameterName),                                                                        \
            .Offset = offsetof(zzTThisStruct, ParameterName),                                                     \
            .Columns = TypeInfo::NumColumns,                                                                      \
            .Rows = TypeInfo::NumRows,                                                                            \
        });                                                                                                       \
        zzFuncPtr (*PrevFunc)(zzMemberId##ParameterName, TArray<::RTTI::FParameter>*);                            \
        PrevFunc = zzGetPreviousMember;                                                                           \
        return (zzFuncPtr)PrevFunc;                                                                               \
    }                                                                                                             \
    typedef zzNextMemberId##ParameterName

#define END_PARAMETER_STRUCT()                                                      \
    zzLastMemberId;                                                                 \
                                                                                    \
public:                                                                             \
    static TArray<::RTTI::FParameter> GetMembers()                                  \
    {                                                                               \
        TArray<::RTTI::FParameter> Members;                                         \
        zzFuncPtr (*LastFunc)(zzLastMemberId, TArray<::RTTI::FParameter>*);         \
        LastFunc = zzGetPreviousMember;                                             \
        zzFuncPtr Ptr = (zzFuncPtr)LastFunc;                                        \
        do {                                                                        \
            Ptr = reinterpret_cast<zzMemberFunc>(Ptr)(zzFirstMemberId(), &Members); \
        } while (Ptr);                                                              \
        std::reverse(Members.begin(), Members.end());                               \
        return Members;                                                             \
    }                                                                               \
    }

namespace RTTI
{

struct FParameter;

template <typename T>
concept IsParameterType = requires(T a) { T::GetMembers()->std::template same_as<TArray<::RTTI::FParameter>>; };

/// The base type of a shader parameter
enum class EParameterType : uint8 {
    Invalid,

    Struct,

    Int32,
    Uint32,
    Float,

};

/// This structure define a shader parameters (uniforms, storage buffers, etc)
struct FParameter {
    std::string Name = "";
    EParameterType Type = EParameterType::Invalid;
    uint64 Size = 0;
    uint64 Offset = 0;
    uint64 Columns = 0;
    uint64 Rows = 0;

    /// If the parameter is a struct, this array contains the members of the struct
    TArray<FParameter> Members;

    bool operator==(const FParameter& Other) const
    {
        // Name is not compared, because it does not matter
        return Type == Other.Type && Size == Other.Size && Offset == Other.Offset && Columns == Other.Columns &&
               Rows == Other.Rows && Members == Other.Members;
    };

    static void Serialize(Serialization::FStreamWriter* Writer, const FParameter& Value)
    {
        Writer->WriteString(Value.Name);
        Writer->WriteRaw(Value.Type);
        Writer->WriteRaw(Value.Size);
        Writer->WriteRaw(Value.Offset);
        Writer->WriteRaw(Value.Columns);
        Writer->WriteRaw(Value.Rows);
        Writer->WriteArray(Value.Members);
    }

    static void Deserialize(Serialization::FStreamReader* Reader, FParameter& Value)
    {
        Reader->ReadString(Value.Name);
        Reader->ReadRaw(Value.Type);
        Reader->ReadRaw(Value.Size);
        Reader->ReadRaw(Value.Offset);
        Reader->ReadRaw(Value.Columns);
        Reader->ReadRaw(Value.Rows);
        Reader->ReadArray(Value.Members);
    }
};

struct FClass {
    std::string_view Name;
    TArray<FParameter> Members;
};

template <typename T>
struct TSupportedParameterType;

template <>
struct TSupportedParameterType<int32> {
    static constexpr EParameterType Type = EParameterType::Int32;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 1;
    static constexpr uint64 Alignment = 4;
    using AlignedType = int32;
};

template <>
struct TSupportedParameterType<uint64> {
    static constexpr EParameterType Type = EParameterType::Uint32;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 1;
    static constexpr uint64 Alignment = 4;
    using AlignedType = uint64;
};

template <>
struct TSupportedParameterType<float> {
    static constexpr EParameterType Type = EParameterType::Float;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 1;
    static constexpr uint64 Alignment = 4;
    using AlignedType = float;
};

template <>
struct TSupportedParameterType<FVector3> {
    static constexpr EParameterType Type = EParameterType::Float;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 3;
    static constexpr uint64 Alignment = 16;
    using AlignedType = FVector4;
};

template <>
struct TSupportedParameterType<FVector4> {
    static constexpr EParameterType Type = EParameterType::Float;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 4;
    static constexpr uint64 Alignment = 16;
    using AlignedType = FVector4;
};

template <>
struct TSupportedParameterType<UVector2> {
    static constexpr EParameterType Type = EParameterType::Uint32;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 2;
    static constexpr uint64 Alignment = 16;
    using AlignedType = UVector2;
};

template <>
struct TSupportedParameterType<IVector2> {
    static constexpr EParameterType Type = EParameterType::Int32;
    static constexpr uint64 NumColumns = 1;
    static constexpr uint64 NumRows = 2;
    static constexpr uint64 Alignment = 16;
    using AlignedType = IVector2;
};

template <>
struct TSupportedParameterType<FMatrix4> {
    static constexpr EParameterType Type = EParameterType::Float;
    static constexpr uint64 NumColumns = 4;
    static constexpr uint64 NumRows = 4;
    static constexpr uint64 Alignment = 16;
    using AlignedType = FMatrix4;
};

inline std::string PrintShaderParameter(const FParameter& Param, unsigned Indent, bool bSimple)
{
    if (bSimple) {
        return std::format("Name: \"{0}\", Type: {1}, Size: {2}, Offset: {3}, Columns: {4}, Rows: {5}", Param.Name,
                           magic_enum::enum_name(Param.Type), Param.Size, Param.Offset, Param.Columns, Param.Rows);
    }

    std::string Result = "";
    std::string Padding = "";
    for (unsigned i = 0; i < Indent; ++i) {
        Padding += ("\t");
    }

    Result += "\n";
    Result += std::format("{0}Name: \"{1}\"\n", Padding, Param.Name);
    Padding += "\t";
    Result += std::format("{0}Type: {1}\n", Padding, magic_enum::enum_name(Param.Type));
    Result += std::format("{0}Size: {1}\n", Padding, Param.Size);
    Result += std::format("{0}Offset: {1}\n", Padding, Param.Offset);
    Result += std::format("{0}Columns: {1}\n", Padding, Param.Columns);
    Result += std::format("{0}Rows: {1}\n", Padding, Param.Rows);

    for (const FParameter& Member: Param.Members) {
        Result += PrintShaderParameter(Member, Indent + 2, bSimple);
    }

    Result += "\n";
    return Result;
}
}    // namespace RTTI

template <>
struct std::formatter<RTTI::FParameter, char> {
    bool bSimple = false;

    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;

        if (*it == '#') {
            bSimple = true;
            ++it;
        }
        if (*it != '}')
            throw std::format_error("Invalid format args for cpplogger::Level.");

        return it;
    }
    template <class FormatContext>
    auto format(const RTTI::FParameter& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "ShaderParameter{{ {0} }}", PrintShaderParameter(Value, 0, bSimple));
        return out;
    }
};
inline std::ostream& operator<<(std::ostream& os, const RTTI::FParameter& m)
{
    os << std ::format("{}", m);
    return os;
};
