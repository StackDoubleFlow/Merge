#include "MetadataBuilder.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "Logger.h"

MetadataOffsets MetadataBuilder::CaptureNextOffsets() {
    MetadataOffsets offsets;
    offsets.stringLiteralOffset = stringLiteral.size();
    offsets.stringLiteralDataOffset = stringLiteralData.size();
    offsets.stringOffset = string.size();
    offsets.eventsOffset = events.size();
    offsets.propertiesOffset = properties.size();
    offsets.methodsOffset = methods.size();
    offsets.parameterDefaultValuesOffset = parameterDefaultValues.size();
    offsets.fieldDefaultValuesOffset = fieldDefaultValues.size();
    offsets.fieldAndParameterDefaultValueDataOffset = fieldAndParameterDefaultValueData.size();
    offsets.fieldMarshaledSizesOffset = fieldMarshaledSizes.size();
    offsets.parametersOffset = parameters.size();
    offsets.fieldsOffset = fields.size();
    offsets.genericParametersOffset = genericParameters.size();
    offsets.genericParameterConstraintsOffset = genericParameterConstraints.size();
    offsets.genericContainersOffset = genericContainers.size();
    offsets.nestedTypesOffset = nestedTypes.size();
    offsets.interfacesOffset = interfaces.size();
    offsets.vtableMethodsOffset = vtableMethods.size();
    offsets.interfaceOffsetsOffset = interfaceOffsets.size();
    offsets.typeDefinitionsOffset = typeDefinitions.size();
    offsets.imagesOffset = images.size();
    offsets.assembliesOffset = assemblies.size();
    offsets.metadataUsageListsOffset = metadataUsageLists.size();
    offsets.metadataUsagePairsOffset = metadataUsagePairs.size();
    offsets.fieldRefsOffset = fieldRefs.size();
    offsets.referencedAssembliesOffset = referencedAssemblies.size();
    offsets.attributesInfoOffset = attributesInfo.size();
    offsets.attributeTypesOffset = attributeTypes.size();
    offsets.unresolvedVirtualCallParameterTypesOffset = unresolvedVirtualCallParameterTypes.size();
    offsets.unresolvedVirtualCallParameterRangesOffset = unresolvedVirtualCallParameterRanges.size();
    offsets.windowsRuntimeTypeNamesOffset = windowsRuntimeTypeNames.size();
    offsets.exportedTypeDefinitionsOffset = exportedTypeDefinitions.size();
    return offsets;
}

template<typename T>
static T MetadataOffset(const void* metadata, size_t sectionOffset, size_t itemIndex) {
    return reinterpret_cast<T>(reinterpret_cast<const uint8_t*>(metadata) + sectionOffset) + itemIndex;
}

MetadataOffsets MetadataBuilder::AppendMetadata(const void *metadata) {
    MLogger::GetLogger().debug("Appending metadata to builder from %p", metadata);
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(metadata);
    CRASH_UNLESS(header->sanity == 0xFAB11BAF);
    CRASH_UNLESS(header->version == 24);

    MetadataOffsets offsets = CaptureNextOffsets();

    for (size_t i = 0; i < header->stringLiteralCount; i++) {
        Il2CppStringLiteral literal = *MetadataOffset<const Il2CppStringLiteral*>(metadata, header->stringLiteralOffset, i);
        literal.dataIndex += offsets.stringLiteralDataOffset;
        stringLiteral.push_back(literal);
    }

    for (size_t i = 0; i < header->stringLiteralDataCount; i++) {
        char c = *MetadataOffset<const char*>(metadata, header->stringLiteralOffset, i);
        stringLiteralData.push_back(c);
    }
    
    for (size_t i = 0; i < header->stringCount; i++) {
        char c = *MetadataOffset<const char*>(metadata, header->stringOffset, i);
        string.push_back(c);
    }

    for (size_t i = 0; i < header->eventsCount / sizeof(Il2CppEventDefinition); i++) {
        Il2CppEventDefinition event = *MetadataOffset<const Il2CppEventDefinition*>(metadata, header->eventsOffset, i);
        events.push_back(event);
    }

    for (size_t i = 0; i < header->propertiesCount / sizeof(Il2CppPropertyDefinition); i++) {
        Il2CppPropertyDefinition property = *MetadataOffset<const Il2CppPropertyDefinition*>(metadata, header->propertiesOffset, i);
        properties.push_back(property);
    }

    for (size_t i = 0; i < header->methodsCount / sizeof(Il2CppMethodDefinition); i++) {
        Il2CppMethodDefinition method = *MetadataOffset<const Il2CppMethodDefinition*>(metadata, header->methodsOffset, i);
        methods.push_back(method);
    }

    for (size_t i = 0; i < header->parameterDefaultValuesCount / sizeof(Il2CppParameterDefaultValue); i++) {
        Il2CppParameterDefaultValue parameterDefaultValue = *MetadataOffset<const Il2CppParameterDefaultValue*>(metadata, header->parameterDefaultValuesOffset, i);
        parameterDefaultValues.push_back(parameterDefaultValue);
    }

    for (size_t i = 0; i < header->fieldDefaultValuesCount / sizeof(Il2CppFieldDefaultValue); i++) {
        Il2CppFieldDefaultValue fieldDefaultValue = *MetadataOffset<const Il2CppFieldDefaultValue*>(metadata, header->fieldDefaultValuesOffset, i);
        fieldDefaultValues.push_back(fieldDefaultValue);
    }

    for (size_t i = 0; i < header->fieldAndParameterDefaultValueDataCount / sizeof(uint8_t); i++) {
        uint8_t defaultValue = *MetadataOffset<const uint8_t*>(metadata, header->eventsOffset, i);
        fieldAndParameterDefaultValueData.push_back(defaultValue);
    }

    for (size_t i = 0; i < header->fieldMarshaledSizesCount / sizeof(Il2CppFieldMarshaledSize); i++) {
        Il2CppFieldMarshaledSize fieldMarshaledSize = *MetadataOffset<const Il2CppFieldMarshaledSize*>(metadata, header->fieldMarshaledSizesOffset, i);
        fieldMarshaledSizes.push_back(fieldMarshaledSize);
    }

    for (size_t i = 0; i < header->parametersCount / sizeof(Il2CppParameterDefinition); i++) {
        Il2CppParameterDefinition parameter = *MetadataOffset<const Il2CppParameterDefinition*>(metadata, header->parametersOffset, i);
        parameters.push_back(parameter);
    }

    for (size_t i = 0; i < header->fieldsCount / sizeof(Il2CppFieldDefinition); i++) {
        Il2CppFieldDefinition field = *MetadataOffset<const Il2CppFieldDefinition*>(metadata, header->fieldsOffset, i);
        fields.push_back(field);
    }

    for (size_t i = 0; i < header->genericParametersCount / sizeof(Il2CppGenericParameter); i++) {
        Il2CppGenericParameter genericParameter = *MetadataOffset<const Il2CppGenericParameter*>(metadata, header->genericParametersOffset, i);
        genericParameters.push_back(genericParameter);
    }

    for (size_t i = 0; i < header->genericParameterConstraintsCount / sizeof(TypeIndex); i++) {
        TypeIndex genericParameterConstraint = *MetadataOffset<const TypeIndex*>(metadata, header->genericParameterConstraintsOffset, i);
        genericParameterConstraints.push_back(genericParameterConstraint);
    }

    for (size_t i = 0; i < header->genericContainersCount / sizeof(Il2CppGenericContainer); i++) {
        Il2CppGenericContainer genericContainer = *MetadataOffset<const Il2CppGenericContainer*>(metadata, header->genericContainersOffset, i);
        genericContainers.push_back(genericContainer);
    }

    for (size_t i = 0; i < header->nestedTypesCount / sizeof(TypeDefinitionIndex); i++) {
        TypeDefinitionIndex event = *MetadataOffset<const TypeDefinitionIndex*>(metadata, header->nestedTypesOffset, i);
        nestedTypes.push_back(event);
    }

    for (size_t i = 0; i < header->interfacesCount / sizeof(TypeIndex); i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->interfacesOffset, i);
        interfaces.push_back(event);
    }

    for (size_t i = 0; i < header->vtableMethodsCount / sizeof(EncodedMethodIndex); i++) {
        EncodedMethodIndex event = *MetadataOffset<const EncodedMethodIndex*>(metadata, header->vtableMethodsOffset, i);
        vtableMethods.push_back(event);
    }

    for (size_t i = 0; i < header->interfaceOffsetsCount / sizeof(Il2CppInterfaceOffsetPair); i++) {
        Il2CppInterfaceOffsetPair event = *MetadataOffset<const Il2CppInterfaceOffsetPair*>(metadata, header->interfaceOffsetsOffset, i);
        interfaceOffsets.push_back(event);
    }

    for (size_t i = 0; i < header->typeDefinitionsCount / sizeof(Il2CppTypeDefinition); i++) {
        Il2CppTypeDefinition event = *MetadataOffset<const Il2CppTypeDefinition*>(metadata, header->typeDefinitionsOffset, i);
        typeDefinitions.push_back(event);
    }

    for (size_t i = 0; i < header->imagesCount / sizeof(Il2CppImageDefinition); i++) {
        Il2CppImageDefinition event = *MetadataOffset<const Il2CppImageDefinition*>(metadata, header->imagesOffset, i);
        images.push_back(event);
    }

    for (size_t i = 0; i < header->assembliesCount / sizeof(Il2CppAssemblyDefinition); i++) {
        Il2CppAssemblyDefinition event = *MetadataOffset<const Il2CppAssemblyDefinition*>(metadata, header->assembliesOffset, i);
        assemblies.push_back(event);
    }

    for (size_t i = 0; i < header->metadataUsageListsCount / sizeof(Il2CppMetadataUsageList); i++) {
        Il2CppMetadataUsageList event = *MetadataOffset<const Il2CppMetadataUsageList*>(metadata, header->metadataUsageListsOffset, i);
        metadataUsageLists.push_back(event);
    }

    for (size_t i = 0; i < header->metadataUsagePairsCount / sizeof(Il2CppMetadataUsagePair); i++) {
        Il2CppMetadataUsagePair event = *MetadataOffset<const Il2CppMetadataUsagePair*>(metadata, header->metadataUsagePairsOffset, i);
        metadataUsagePairs.push_back(event);
    }

    for (size_t i = 0; i < header->fieldRefsCount / sizeof(Il2CppFieldRef); i++) {
        Il2CppFieldRef event = *MetadataOffset<const Il2CppFieldRef*>(metadata, header->fieldRefsOffset, i);
        fieldRefs.push_back(event);
    }

    for (size_t i = 0; i < header->referencedAssembliesCount / sizeof(int32_t); i++) {
        int32_t event = *MetadataOffset<const int32_t*>(metadata, header->referencedAssembliesOffset, i);
        referencedAssemblies.push_back(event);
    }

    for (size_t i = 0; i < header->attributesInfoCount / sizeof(Il2CppCustomAttributeTypeRange); i++) {
        Il2CppCustomAttributeTypeRange event = *MetadataOffset<const Il2CppCustomAttributeTypeRange*>(metadata, header->attributesInfoOffset, i);
        attributesInfo.push_back(event);
    }

    for (size_t i = 0; i < header->attributeTypesCount / sizeof(TypeIndex); i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->attributeTypesOffset, i);
        attributeTypes.push_back(event);
    }

    for (size_t i = 0; i < header->unresolvedVirtualCallParameterTypesCount / sizeof(TypeIndex); i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->unresolvedVirtualCallParameterTypesOffset, i);
        unresolvedVirtualCallParameterTypes.push_back(event);
    }

    for (size_t i = 0; i < header->unresolvedVirtualCallParameterRangesCount / sizeof(Il2CppRange); i++) {
        Il2CppRange event = *MetadataOffset<const Il2CppRange*>(metadata, header->unresolvedVirtualCallParameterRangesOffset, i);
        unresolvedVirtualCallParameterRanges.push_back(event);
    }

    for (size_t i = 0; i < header->windowsRuntimeTypeNamesSize / sizeof(Il2CppWindowsRuntimeTypeNamePair); i++) {
        Il2CppWindowsRuntimeTypeNamePair event = *MetadataOffset<const Il2CppWindowsRuntimeTypeNamePair*>(metadata, header->windowsRuntimeTypeNamesOffset, i);
        windowsRuntimeTypeNames.push_back(event);
    }

    for (size_t i = 0; i < header->exportedTypeDefinitionsCount / sizeof(TypeDefinitionIndex); i++) {
        TypeDefinitionIndex event = *MetadataOffset<const TypeDefinitionIndex*>(metadata, header->exportedTypeDefinitionsOffset, i);
        exportedTypeDefinitions.push_back(event);
    }

    return offsets;
}

void *MetadataBuilder::Finish() {
    // TODO: Calculate metadata size
    // but for right now I'll just do 20mb
    void *metadata = new char[20000000];
    Il2CppGlobalMetadataHeader *newHeader = static_cast<Il2CppGlobalMetadataHeader *>(metadata);
    newHeader->sanity = 0xFAB11BAF;
    newHeader->version = 24;

    // TODO: Write to the new metadata

    return newHeader;
}