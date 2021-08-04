#pragma once
#include "il2cpp-metadata.h"
#include "il2cpp-runtime-metadata.h"
#include <string_view>
#include <vector>
#include <unordered_map>

class MetadataBuilder {
public:
    MetadataBuilder(const void *baseMetadata);
    void AppendMetadata(const void *metadata, std::string_view assemblyName, int typeOffset);
    StringIndex AppendString(const char *str);
    void *Finish();

private:
    TypeDefinitionIndex RedirectTypeDefinition(std::unordered_map<TypeDefinitionIndex, TypeDefinitionIndex> &typeRedirects,
                                               TypeDefinitionIndex modType, ImageIndex imageIndex, const void *metadata);

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
    std::vector<Il2CppMetadataUsageList> metadataUsageLists;
    std::vector<Il2CppMetadataUsagePair> metadataUsagePairs;
    std::vector<Il2CppFieldRef> fieldRefs;
    std::vector<int32_t> referencedAssemblies;
    std::vector<Il2CppCustomAttributeTypeRange> attributesInfo;
    std::vector<TypeIndex> attributeTypes;
    std::vector<TypeIndex> unresolvedVirtualCallParameterTypes;
    std::vector<Il2CppRange> unresolvedVirtualCallParameterRanges;
    std::vector<Il2CppWindowsRuntimeTypeNamePair> windowsRuntimeTypeNames;
    std::vector<TypeDefinitionIndex> exportedTypeDefinitions;
};