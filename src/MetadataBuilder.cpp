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

    for (size_t i = 0; i < header->eventsCount; i++) {
        Il2CppEventDefinition event = *MetadataOffset<const Il2CppEventDefinition*>(metadata, header->eventsOffset, i);
        events.push_back(event);
    }

    for (size_t i = 0; i < header->propertiesCount; i++) {
        Il2CppPropertyDefinition property = *MetadataOffset<const Il2CppPropertyDefinition*>(metadata, header->propertiesOffset, i);
        properties.push_back(property);
    }

    for (size_t i = 0; i < header->methodsCount; i++) {
        Il2CppMethodDefinition method = *MetadataOffset<const Il2CppMethodDefinition*>(metadata, header->methodsOffset, i);
        methods.push_back(method);
    }

    for (size_t i = 0; i < header->parameterDefaultValuesOffset; i++) {
        Il2CppParameterDefaultValue parameterDefaultValue = *MetadataOffset<const Il2CppParameterDefaultValue*>(metadata, header->eventsOffset, i);
        parameterDefaultValues.push_back(parameterDefaultValue);
    }

    for (size_t i = 0; i < header->fieldDefaultValuesCount; i++) {
        Il2CppFieldDefaultValue fieldDefaultValue = *MetadataOffset<const Il2CppFieldDefaultValue*>(metadata, header->eventsOffset, i);
        fieldDefaultValues.push_back(fieldDefaultValue);
    }

    for (size_t i = 0; i < header->fieldAndParameterDefaultValueDataCount; i++) {
        uint8_t defaultValue = *MetadataOffset<const uint8_t*>(metadata, header->eventsOffset, i);
        fieldAndParameterDefaultValueData.push_back(defaultValue);
    }

    for (size_t i = 0; i < header->fieldMarshaledSizesCount; i++) {
        Il2CppFieldMarshaledSize fieldMarshaledSize = *MetadataOffset<const Il2CppFieldMarshaledSize*>(metadata, header->eventsOffset, i);
        fieldMarshaledSizes.push_back(fieldMarshaledSize);
    }

    for (size_t i = 0; i < header->parametersCount; i++) {
        Il2CppParameterDefinition parameter = *MetadataOffset<const Il2CppParameterDefinition*>(metadata, header->eventsOffset, i);
        parameters.push_back(parameter);
    }

    for (size_t i = 0; i < header->fieldsCount; i++) {
        Il2CppFieldDefinition field = *MetadataOffset<const Il2CppFieldDefinition*>(metadata, header->eventsOffset, i);
        fields.push_back(field);
    }

    for (size_t i = 0; i < header->genericParametersCount; i++) {
        Il2CppGenericParameter genericParameter = *MetadataOffset<const Il2CppGenericParameter*>(metadata, header->eventsOffset, i);
        genericParameters.push_back(genericParameter);
    }

    for (size_t i = 0; i < header->genericParameterConstraintsCount; i++) {
        TypeIndex genericParameterConstraint = *MetadataOffset<const TypeIndex*>(metadata, header->eventsOffset, i);
        genericParameterConstraints.push_back(genericParameterConstraint);
    }

    for (size_t i = 0; i < header->genericContainersCount; i++) {
        Il2CppGenericContainer genericContainer = *MetadataOffset<const Il2CppGenericContainer*>(metadata, header->eventsOffset, i);
        genericContainers.push_back(genericContainer);
    }

    for (size_t i = 0; i < header->nestedTypesCount; i++) {
        TypeDefinitionIndex event = *MetadataOffset<const TypeDefinitionIndex*>(metadata, header->eventsOffset, i);
        nestedTypes.push_back(event);
    }

    for (size_t i = 0; i < header->interfacesCount; i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->eventsOffset, i);
        interfaces.push_back(event);
    }

    for (size_t i = 0; i < header->vtableMethodsCount; i++) {
        EncodedMethodIndex event = *MetadataOffset<const EncodedMethodIndex*>(metadata, header->eventsOffset, i);
        vtableMethods.push_back(event);
    }

    for (size_t i = 0; i < header->interfaceOffsetsCount; i++) {
        Il2CppInterfaceOffsetPair event = *MetadataOffset<const Il2CppInterfaceOffsetPair*>(metadata, header->eventsOffset, i);
        interfaceOffsets.push_back(event);
    }

    for (size_t i = 0; i < header->typeDefinitionsCount; i++) {
        Il2CppTypeDefinition event = *MetadataOffset<const Il2CppTypeDefinition*>(metadata, header->eventsOffset, i);
        typeDefinitions.push_back(event);
    }

    for (size_t i = 0; i < header->imagesCount; i++) {
        Il2CppImageDefinition event = *MetadataOffset<const Il2CppImageDefinition*>(metadata, header->eventsOffset, i);
        images.push_back(event);
    }

    for (size_t i = 0; i < header->assembliesCount; i++) {
        Il2CppAssemblyDefinition event = *MetadataOffset<const Il2CppAssemblyDefinition*>(metadata, header->eventsOffset, i);
        assemblies.push_back(event);
    }

    for (size_t i = 0; i < header->metadataUsageListsCount; i++) {
        Il2CppMetadataUsageList event = *MetadataOffset<const Il2CppMetadataUsageList*>(metadata, header->eventsOffset, i);
        metadataUsageLists.push_back(event);
    }

    for (size_t i = 0; i < header->metadataUsagePairsCount; i++) {
        Il2CppMetadataUsagePair event = *MetadataOffset<const Il2CppMetadataUsagePair*>(metadata, header->eventsOffset, i);
        metadataUsagePairs.push_back(event);
    }

    for (size_t i = 0; i < header->fieldRefsCount; i++) {
        Il2CppFieldRef event = *MetadataOffset<const Il2CppFieldRef*>(metadata, header->eventsOffset, i);
        fieldRefs.push_back(event);
    }

    for (size_t i = 0; i < header->referencedAssembliesCount; i++) {
        int32_t event = *MetadataOffset<const int32_t*>(metadata, header->eventsOffset, i);
        referencedAssemblies.push_back(event);
    }

    for (size_t i = 0; i < header->attributesInfoCount; i++) {
        Il2CppCustomAttributeTypeRange event = *MetadataOffset<const Il2CppCustomAttributeTypeRange*>(metadata, header->eventsOffset, i);
        attributesInfo.push_back(event);
    }

    for (size_t i = 0; i < header->attributeTypesCount; i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->eventsOffset, i);
        attributeTypes.push_back(event);
    }

    for (size_t i = 0; i < header->unresolvedVirtualCallParameterTypesCount; i++) {
        TypeIndex event = *MetadataOffset<const TypeIndex*>(metadata, header->eventsOffset, i);
        unresolvedVirtualCallParameterTypes.push_back(event);
    }

    for (size_t i = 0; i < header->unresolvedVirtualCallParameterRangesCount; i++) {
        Il2CppRange event = *MetadataOffset<const Il2CppRange*>(metadata, header->eventsOffset, i);
        unresolvedVirtualCallParameterRanges.push_back(event);
    }

    for (size_t i = 0; i < header->windowsRuntimeTypeNamesSize; i++) {
        Il2CppWindowsRuntimeTypeNamePair event = *MetadataOffset<const Il2CppWindowsRuntimeTypeNamePair*>(metadata, header->eventsOffset, i);
        windowsRuntimeTypeNames.push_back(event);
    }

    for (size_t i = 0; i < header->exportedTypeDefinitionsCount; i++) {
        TypeDefinitionIndex event = *MetadataOffset<const TypeDefinitionIndex*>(metadata, header->eventsOffset, i);
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