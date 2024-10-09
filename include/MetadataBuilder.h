#pragma once
#include "vm/GlobalMetadataFileInternals.h"
#include "il2cpp-class-internals.h"
// #include "il2cpp-metadata.h"
// #include "il2cpp-runtime-metadata.h"
#include <string_view>
#include <unordered_map>
#include <vector>

struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Same hash seed il2cpp uses, idk if it's any good
        return h1 * 486187739 + h2;
    }
};

class MetadataBuilder {
public:
    void Initialize(const void *baseMetadata);
    void AppendMetadata(const void *metadata, std::string_view assemblyName,
                        int typeOffset);
    StringIndex AppendString(const char *str);
    std::optional<TypeDefinitionIndex>
    FindTypeDefinition(const char *namespaze, const char *name);
    void *Finish();

private:
    TypeDefinitionIndex RedirectTypeDefinition(
        std::unordered_map<TypeDefinitionIndex, TypeDefinitionIndex>
            &typeRedirects,
        TypeDefinitionIndex modType, ImageIndex imageIndex,
        const void *metadata);

public:
    std::vector<Il2CppStringLiteral> stringLiteral;
    std::vector<char> stringLiteralData;
    std::vector<char> string;
    std::vector<Il2CppEventDefinition> events;
    std::vector<Il2CppPropertyDefinition> properties;
    std::vector<Il2CppMethodDefinition> methods;
    std::vector<Il2CppParameterDefaultValue> parameterDefaultValues;
    std::vector<Il2CppFieldDefaultValue> fieldDefaultValues;
    std::vector<uint8_t> fieldAndParameterDefaultValueData;
    std::vector<Il2CppFieldMarshaledSize> fieldMarshaledSizes;
    std::vector<Il2CppParameterDefinition> parameters;
    std::vector<Il2CppFieldDefinition> fields;
    std::vector<Il2CppGenericParameter> genericParameters;
    std::vector<TypeIndex> genericParameterConstraints;
    std::vector<Il2CppGenericContainer> genericContainers;
    std::vector<TypeDefinitionIndex> nestedTypes;
    std::vector<TypeIndex> interfaces;
    std::vector<EncodedMethodIndex> vtableMethods;
    std::vector<Il2CppInterfaceOffsetPair> interfaceOffsets;
    std::vector<Il2CppTypeDefinition> typeDefinitions;
    std::vector<Il2CppImageDefinition> images;
    std::vector<Il2CppAssemblyDefinition> assemblies;
    std::vector<Il2CppFieldRef> fieldRefs;
    std::vector<int32_t> referencedAssemblies;
    std::vector<uint8_t> attributeData;
    std::vector<Il2CppCustomAttributeDataRange> attributeDataRange;
    std::vector<TypeIndex> unresolvedIndirectCallParameterTypes;
    std::vector<Il2CppMetadataRange> unresolvedIndirectCallParameterRanges;
    std::vector<Il2CppWindowsRuntimeTypeNamePair> windowsRuntimeTypeNames;
    std::vector<char> windowsRuntimeStrings;
    std::vector<TypeDefinitionIndex> exportedTypeDefinitions;

private:
    std::unordered_map<std::pair<std::string, std::string>, TypeDefinitionIndex,
                       PairHash>
        typeNameMap;
};