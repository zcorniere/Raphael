#include "Engine/Core/RTTI/Registrar.hxx"

#include "Engine/Core/RTTI/Name.hxx"
#include <unordered_set>

#define DECLARE_PRIMITIVE_RTTI_REGISTRATION(Type)               \
    RTTI_DEFINE_NAME(Type);                                     \
    namespace RTTI                                              \
    {                                                           \
    const TPrimitiveType##Type gRegistrator##Type##Registrator; \
    }

DECLARE_PRIMITIVE_RTTI_REGISTRATION(char)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(bool)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int8)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int16)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int32)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(int64)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint8)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint16)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint32)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(uint64)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(float)
DECLARE_PRIMITIVE_RTTI_REGISTRATION(double)

#undef DECLARE_PRIMITIVE_RTTI_REGISTRATION

namespace RTTI
{

void Registrar::Init()
{
    LOG(LogRegistrar, Trace, "Registering RTTI types");
    // Register the primitive types
    for (IClassBuilder* classBuilder: RegisteredClassBuilders) {
        LOG(LogRegistrar, Trace, "Registering class: {}", classBuilder->GetName());
        FClass* NewClass = classBuilder->InitClass();
        if (NewClass) {
            LOG(LogRegistrar, Trace, "Registered class: {}", NewClass->GetName());
        } else {
            LOG(LogRegistrar, Error, "Failed to register class");
        }
    }
}

void Registrar::RegisteredClassBuilder(IClassBuilder* classBuilder)
{
    RegisteredClassBuilders.Add(classBuilder);
}
void Registrar::UnregisterClassBuilder(IClassBuilder* classBuilder)
{
    RegisteredClassBuilders.Remove(classBuilder);
}

/// Registers a type in the RTTI system.
/// @param type The type to register.
void Registrar::RegisterType(IType* type)
{
    RegisteredTypes.Add(type);
}

/// Unregisters a type from the RTTI system.
/// @param type The type to unregister.
void Registrar::UnregisterType(IType* type)
{
    RegisteredTypes.Remove(type);
}

/// Finds a type by name.
/// @param name The name of the type to find.
/// @return The type if found, nullptr otherwise.
[[nodiscard]] IType* Registrar::FindType(const FName& name) const
{
    for (IType* type: RegisteredTypes) {
        LOG(LogRegistrar, Trace, "Found type: {} == {}", type->GetName(), name);
        if (type->GetName() == name) {
            return type;
        }
    }
    return nullptr;
}

std::string Registrar::PrintGraph(const FName& name) const
{
    const IType* type = FindType(name);
    if (!type)
        return std::format("Type {} not found", name);
    const FClass* root = dynamic_cast<const FClass*>(type);
    if (!root)
        return std::format("Type {} is not a class", name);

    std::ostringstream out;
    out << "digraph RTTIGraph {\n"
        << "    node [shape=record];\n";

    std::unordered_set<std::string> emittedNodes;
    std::unordered_set<std::string> emittedEdges;

    const auto escapeDot = [](const std::string& s) -> std::string {
        std::string result;
        for (const char c: s)
            switch (c) {
                case '"':
                    result += "\\\"";
                    break;
                case '<':
                    result += "\\<";
                    break;
                case '>':
                    result += "\\>";
                    break;
                default:
                    result += c;
            }
        return result;
    };

    const auto classLabel = [&](const FClass* klass) -> std::string {
        std::string label = "{" + escapeDot(klass->GetName().ToString()) + "|";
        for (const auto& member: klass->GetProperties())
            label += std::format("{} : {}\\l", escapeDot(member.Name.ToString()),
                                 escapeDot(member.Type ? member.Type->GetName().ToString() : "unknown"));
        label += "}";
        return label;
    };

    const auto emitTypeGraph = [&](this const auto& self, const IType* t) -> void {
        const std::string typeName = t->GetName().ToString();

        if (const auto* klass = dynamic_cast<const FClass*>(t)) {
            // Emit class node
            if (emittedNodes.insert(typeName).second)
                out << std::format("    \"{}\" [label=\"{}\"];\n", escapeDot(typeName), classLabel(klass));
            // Emit inheritance edges (dotted)
            for (const FClass* parent: klass->GetParentClass()) {
                if (!parent)
                    continue;
                const std::string parentName = parent->GetName().ToString();
                if (emittedNodes.find(parentName) == emittedNodes.end())
                    self(parent);
                const std::string inhEdge = std::format("\"{}\" -> \"{}\" [label=\"inherits\", style=dotted]",
                                                        escapeDot(parentName), escapeDot(typeName));
                if (emittedEdges.insert(inhEdge).second)
                    out << "    " << inhEdge << ";\n";
            }
            // Emit property-type edges and recurse on property types
            for (const auto& member: klass->GetProperties()) {
                if (!member.Type)
                    continue;
                const std::string memberTypeName = member.Type->GetName().ToString();
                const std::string memberEdge = std::format("\"{}\" -> \"{}\" [label=\"member type\"]",
                                                           escapeDot(typeName), escapeDot(memberTypeName));
                if (emittedEdges.insert(memberEdge).second)
                    out << "    " << memberEdge << ";\n";
                if (emittedNodes.find(memberTypeName) == emittedNodes.end())
                    self(member.Type);
            }
        } else {
            // Non-class types: emit simple node if needed
            if (emittedNodes.insert(typeName).second)
                out << std::format("    \"{}\" [label=\"{}\"];\n", escapeDot(typeName), escapeDot(typeName));
        }
    };

    emitTypeGraph(root);

    out << "}\n";
    return out.str();
}

}    // namespace RTTI
