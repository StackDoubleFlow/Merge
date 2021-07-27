#pragma once
#include "il2cpp-metadata.h"
#include "il2cpp-runtime-metadata.h"
#include <vector>

struct MetadataOffsets {
    int32_t stringLiteralOffset;
    int32_t stringLiteralDataOffset;
    int32_t stringOffset;
    int32_t eventsOffset;
    int32_t propertiesOffset;
    int32_t methodsOffset;
    int32_t parameterDefaultValuesOffset;
    int32_t fieldDefaultValuesOffset;
    int32_t fieldAndParameterDefaultValueDataOffset;
    int32_t fieldMarshaledSizesOffset;
    int32_t parametersOffset;
    int32_t fieldsOffset;
    int32_t genericParametersOffset;
    int32_t genericParameterConstraintsOffset;
    int32_t genericContainersOffset;
    int32_t nestedTypesOffset;
    int32_t interfacesOffset;
    int32_t vtableMethodsOffset;
    int32_t interfaceOffsetsOffset;
    int32_t typeDefinitionsOffset;
    int32_t imagesOffset;
    int32_t assembliesOffset;
    int32_t metadataUsageListsOffset;
    int32_t metadataUsagePairsOffset;
    int32_t fieldRefsOffset;
    int32_t referencedAssembliesOffset;
    int32_t attributesInfoOffset;
    int32_t attributeTypesOffset;
    int32_t unresolvedVirtualCallParameterTypesOffset;
    int32_t unresolvedVirtualCallParameterRangesOffset;
    int32_t windowsRuntimeTypeNamesOffset;
    int32_t exportedTypeDefinitionsOffset;
};

class MetadataBuilder {
public:
    MetadataOffsets CaptureNextOffsets();
    // Appends a metadata to the builder. Returns the offsets that were added to the indexes.
    MetadataOffsets AppendMetadata(const void *metadata);
    void *Finish();

private:
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