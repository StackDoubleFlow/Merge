#include "merge-api.h"
#include "Logger.h"
#include "ModLoader.h"
#include "beatsaber-hook/shared/utils/utils.h"

namespace Merge::API {

void Initialize() { ModLoader::Initialize(); }

TypeDefinitionIndex FindTypeDefinitionIndex(std::string_view namespaze,
                                            std::string_view name) {
    auto td = ModLoader::metadataBuilder.FindTypeDefinition(namespaze.data(),
                                                            name.data());
    if (td) {
        return *td;
    } else {
        return -1;
    }
}

Il2CppTypeDefinition GetTypeDefinition(TypeDefinitionIndex idx) {
    return ModLoader::metadataBuilder.typeDefinitions[idx];
}

MethodIndex FindMethodDefinitionIndex(TypeDefinitionIndex typeIdx,
                                      std::string_view name,
                                      uint16_t paramCount) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    Il2CppTypeDefinition &type = builder.typeDefinitions[typeIdx];
    for (size_t i = 0; i < type.method_count; i++) {
        Il2CppMethodDefinition &method = builder.methods[type.methodStart + i];
        if (name == &builder.string[method.nameIndex] &&
            method.parameterCount == paramCount) {
            return type.methodStart + i;
        }
    }

    return -1;
}

Il2CppMethodDefinition GetMethodDefinition(MethodIndex idx) {
    return ModLoader::metadataBuilder.methods[idx];
}

TypeIndex CreateSZArrayType(TypeIndex elementType) {
    TypeIndex typeIdx = ModLoader::GetTypesCount();
    Il2CppType addedType;
    addedType.data.type = ModLoader::GetType(elementType);
    addedType.attrs = 0;
    addedType.type = IL2CPP_TYPE_SZARRAY;
    addedType.byref = false;
    ModLoader::addedTypes.push_back(new Il2CppType(addedType));
    return typeIdx;
}

TypeIndex CreatePointerType(TypeIndex type) {
    TypeIndex typeIdx = ModLoader::GetTypesCount();
    Il2CppType addedType;
    addedType.data.type = ModLoader::GetType(type);
    addedType.attrs = 0;
    addedType.type = IL2CPP_TYPE_PTR;
    addedType.byref = false;
    ModLoader::addedTypes.push_back(new Il2CppType(addedType));
    return typeIdx;
}

AssemblyIndex CreateAssembly(std::string_view name) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    Il2CppAssemblyDefinition assembly;
    assembly.aname.nameIndex = builder.AppendString(name.data());
    assembly.imageIndex = -1;
    assembly.referencedAssemblyStart = -1;
    assembly.referencedAssemblyCount = 0;
    // Is the assembly rid ever not 0?
    assembly.token = 0x20000000;

    AssemblyIndex idx = builder.assemblies.size();
    builder.assemblies.push_back(assembly);
    return idx;
}

ImageIndex CreateImage(AssemblyIndex assemblyIdx, std::string_view name) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    char *cname = new char[name.size() + 1];
    strcpy(cname, name.data());

    Il2CppImageDefinition image;
    image.nameIndex = builder.AppendString(cname);
    image.assemblyIndex = assemblyIdx;
    image.typeStart = -1;
    image.typeCount = 0;
    image.exportedTypeStart = -1;
    image.exportedTypeCount = 0;
    image.entryPointIndex = -1;
    // Is this the correct image token?
    image.token = 0;
    image.customAttributeStart = -1;
    image.customAttributeCount = 0;

    ImageIndex idx = builder.images.size();
    builder.images.push_back(image);

    builder.assemblies[assemblyIdx].imageIndex = idx;

    ModLoader::addedCodeGenModules.emplace(idx, cname);

    return idx;
}

namespace {

TypeDefinitionIndex GetInheritingDefinition(TypeIndex idx) {
    const Il2CppType *type = ModLoader::GetType(idx);
    if (type->type == IL2CPP_TYPE_GENERICINST) {
        return type->data.generic_class->typeDefinitionIndex;
    } else {
        return type->data.klassIndex;
    }
}

Il2CppTypeDefinition CreateType(ImageIndex image,
                                const MergeTypeDefinition &type) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    TypeDefinitionIndex idx = builder.typeDefinitions.size();

    Il2CppTypeDefinition typeDef;
    typeDef.nameIndex = builder.AppendString(type.name.c_str());
    typeDef.namespaceIndex = builder.AppendString(type.namespaze.c_str());
    typeDef.byvalTypeIndex = ModLoader::GetTypesCount();
    Il2CppType addedType;
    addedType.data.klassIndex = idx;
    addedType.attrs = 0;
    addedType.type = type.typeEnum;
    addedType.byref = false;
    ModLoader::addedTypes.push_back(new Il2CppType(addedType));
    typeDef.byrefTypeIndex = ModLoader::GetTypesCount();
    addedType.byref = true;
    ModLoader::addedTypes.push_back(new Il2CppType(addedType));
    typeDef.declaringTypeIndex = -1;
    typeDef.parentIndex = type.parent;
    typeDef.elementTypeIndex = -1;
    // TODO: Generics
    typeDef.genericContainerIndex = -1;
    typeDef.flags = type.attrs;
    typeDef.fieldStart = -1;
    typeDef.methodStart = -1;
    typeDef.eventStart = -1;
    typeDef.propertyStart = -1;
    typeDef.nestedTypesStart = -1;
    typeDef.method_count = 0;
    typeDef.property_count = 0;
    typeDef.field_count = 0;
    typeDef.event_count = 0;
    typeDef.nested_type_count = 0;
    typeDef.bitfield = 0;
    typeDef.bitfield |= type.valueType & 1;
    if (type.typeEnum == IL2CPP_TYPE_ENUM)
        typeDef.bitfield |= 2;
    typeDef.token = ModLoader::tokenGenerators[image].GetNextTypeDefToken();

    const Il2CppTypeDefinition &parentDef =
        builder.typeDefinitions[GetInheritingDefinition(type.parent)];

    typeDef.interfacesStart = builder.interfaces.size();
    // Copy interfaces of parent
    builder.interfaces.insert(
        builder.interfaces.end(),
        builder.interfaces.begin() + parentDef.interfacesStart,
        builder.interfaces.begin() + parentDef.interfacesStart +
            parentDef.interfaces_count);
    // Add new interfaces
    builder.interfaces.insert(builder.interfaces.end(), type.interfaces.begin(),
                              type.interfaces.end());
    typeDef.interfaces_count =
        builder.interfaceOffsets.size() - typeDef.interfacesStart;

    typeDef.interfaceOffsetsStart = builder.interfaceOffsets.size();
    typeDef.interface_offsets_count = typeDef.interfaces_count;
    // Copy interface offsets of parent
    builder.interfaceOffsets.insert(
        builder.interfaceOffsets.end(),
        builder.interfaceOffsets.begin() + parentDef.interfaceOffsetsStart,
        builder.interfaceOffsets.begin() + parentDef.interfaceOffsetsStart +
            parentDef.interface_offsets_count);

    typeDef.vtableStart = builder.vtableMethods.size();
    // Copy vtable of parent
    builder.vtableMethods.insert(
        builder.vtableMethods.end(),
        builder.vtableMethods.begin() + parentDef.vtableStart,
        builder.vtableMethods.begin() + parentDef.vtableStart +
            parentDef.vtable_count);

    for (TypeIndex interfaceidx : type.interfaces) {
        uint16_t slot = builder.vtableMethods.size() - typeDef.vtableStart;
        Il2CppInterfaceOffsetPair offsetPair{interfaceidx, slot};
        builder.interfaceOffsets.push_back(offsetPair);

        const Il2CppTypeDefinition &interfaceDef =
            builder.typeDefinitions[GetInheritingDefinition(interfaceidx)];
        for (uint16_t i = 0; i < interfaceDef.method_count; i++) {
            auto &method = builder.methods[interfaceDef.methodStart];
            MLogger::GetLogger().debug("Placing method with slot %i at slot %i",
                                       method.slot, i);
            // vtable should be populated later with a call to
            // SetMethodOverrides
            builder.vtableMethods.push_back(-1);
        }
    }
    typeDef.vtable_count = builder.vtableMethods.size() - typeDef.vtableStart;

    return typeDef;
}

} // namespace

static ImageIndex lastCreateTypesImage = -1;

TypeDefinitionIndex CreateTypes(ImageIndex image,
                                std::span<MergeTypeDefinition> types) {
    lastCreateTypesImage = image;
    auto logger = MLogger::GetLogger().WithContext("Merge::API::CreateTypes");
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    TypeDefinitionIndex startIdx = builder.typeDefinitions.size();
    for (auto &type : types) {
        Il2CppTypeDefinition typeDef = CreateType(image, type);
        logger.debug(
            "Creating type %s.%s, interfaces count: %hi, vtable size: %hi",
            type.namespaze.c_str(), type.name.c_str(), typeDef.interfaces_count,
            typeDef.vtable_count);
        builder.typeDefinitions.push_back(typeDef);
    }

    builder.images[image].typeStart = startIdx;
    builder.images[image].typeCount = types.size();

    return startIdx;
}

TypeDefinitionIndex AppendTypes(std::span<MergeTypeDefinition> types) {
    auto logger = MLogger::GetLogger().WithContext("Merge::API::AppendTypes");
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    ImageIndex image = lastCreateTypesImage;
    if (image == -1) {
        logger.error("You must call CreateTypes before using AppendTypes");
        SAFE_ABORT();
    }

    TypeDefinitionIndex startIdx = builder.typeDefinitions.size();
    for (auto &type : types) {
        Il2CppTypeDefinition typeDef = CreateType(image, type);
        logger.debug(
            "Appending type %s.%s, interfaces count: %hi, vtable size: %hi",
            type.namespaze.c_str(), type.name.c_str(), typeDef.interfaces_count,
            typeDef.vtable_count);
        builder.typeDefinitions.push_back(typeDef);
    }
    builder.images[image].typeCount += types.size();

    return startIdx;
}

MethodIndex CreateMethods(ImageIndex image, TypeDefinitionIndex type,
                          std::span<MergeMethodDefinition> methods) {
    auto logger = MLogger::GetLogger().WithContext("Merge::API::CreateMethods");
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    CodeGenModuleBuilder &moduleBuilder =
        ModLoader::addedCodeGenModules.at(image);

    MethodIndex startIdx = builder.methods.size();
    for (auto &method : methods) {
        Il2CppMethodDefinition methodDef;
        methodDef.nameIndex = builder.AppendString(method.name.c_str());
        methodDef.declaringType = type;
        methodDef.returnType = method.returnType;
        methodDef.parameterStart = builder.parameters.size();
        for (auto &param : method.parameters) {
            TypeIndex typeIdx = param.type;
            Il2CppType type = *ModLoader::GetType(typeIdx);
            if (type.attrs != param.attrs) {
                typeIdx = ModLoader::GetTypesCount();
                type.attrs = param.attrs;
                ModLoader::addedTypes.push_back(new Il2CppType(type));
            }

            Il2CppParameterDefinition paramDef;
            paramDef.nameIndex = builder.AppendString(param.name.c_str());
            paramDef.typeIndex = typeIdx;
            paramDef.token =
                ModLoader::tokenGenerators[image].GetNextParamToken();
        }
        // TODO: generics
        methodDef.genericContainerIndex = -1;
        int32_t rid = moduleBuilder.GetNextMethodRID();
        methodDef.token =
            ModLoader::tokenGenerators[image].GetNextMethodToken();
        // Check if the token matches the rid in the moduleBuilder. If it
        // doesn't, things are out of sync and will definitely go wrong.
        CRASH_UNLESS((methodDef.token & 0x00FFFFFF) == rid);
        methodDef.flags = method.flags;
        methodDef.slot = -1;
        methodDef.parameterCount = method.parameters.size();
        builder.methods.push_back(methodDef);
        int32_t invokerIdx = ModLoader::GetInvokersCount();
        ModLoader::addedInvokers.push_back(method.invoker);
        logger.debug("Adding method %s, RID: %i", method.name.c_str(), rid);
        moduleBuilder.AppendMethod(method.methodPointer, invokerIdx);

        Il2CppTypeDefinition &typeDef = builder.typeDefinitions[type];
        if (method.name == "Finalize") {
            typeDef.bitfield |= (1 << 2);
        } else if (method.name == ".cctor") {
            typeDef.bitfield |= (1 << 3);
        }
    }

    return startIdx;
}

FieldIndex CreateFields(ImageIndex image, TypeDefinitionIndex type,
                        std::span<MergeFieldDefinition> fields) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    FieldIndex startIdx = builder.fields.size();
    for (auto &field : fields) {
        TypeIndex typeIdx = field.type;
        Il2CppType type = *ModLoader::GetType(typeIdx);
        if (type.attrs != field.attrs) {
            typeIdx = ModLoader::GetTypesCount();
            type.attrs = field.attrs;
            ModLoader::addedTypes.push_back(new Il2CppType(type));
        }

        Il2CppFieldDefinition fieldDef;
        fieldDef.nameIndex = builder.AppendString(field.name.c_str());
        fieldDef.typeIndex = typeIdx;
        fieldDef.token = ModLoader::tokenGenerators[image].GetNextFieldToken();
        builder.fields.push_back(fieldDef);
    }

    return startIdx;
}

PropertyIndex CreateProperties(ImageIndex image, TypeDefinitionIndex type,
                               std::span<MergePropertyDefinition> properties) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    PropertyIndex startIdx = builder.properties.size();
    for (auto &prop : properties) {
        Il2CppPropertyDefinition propDef;
        propDef.nameIndex = builder.AppendString(prop.name.c_str());
        propDef.get = prop.get;
        propDef.set = prop.set;
        propDef.attrs = prop.attrs;
        propDef.token = ModLoader::tokenGenerators[image].GetNextPropToken();
        builder.properties.push_back(propDef);
    }

    return startIdx;
}

void SetMethodDeclaringType(MethodIndex method, TypeIndex type) {
    ModLoader::metadataBuilder.methods[method].declaringType = type;
}

namespace {

/// Check if parent is a parent of type
bool RecursiveCheckParents(TypeDefinitionIndex typeIdx,
                           TypeDefinitionIndex parentIdx) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    if (typeIdx == -1) {
        return false;
    }
    if (typeIdx == parentIdx) {
        return true;
    } else {
        Il2CppTypeDefinition &type = builder.typeDefinitions[typeIdx];
        return RecursiveCheckParents(type.parentIndex, parentIdx);
    }
}

} // namespace

void SetMethodOverrides(TypeDefinitionIndex typeIdx,
                        const OverridesMap &overrides) {
    auto logger =
        MLogger::GetLogger().WithContext("Merge::API::SetMethodOverrides");
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    Il2CppTypeDefinition &type = builder.typeDefinitions[typeIdx];
    for (auto &[oMethodIdx, vMethodIdx] : overrides) {
        auto &oMethod = builder.methods[oMethodIdx];
        if (oMethod.declaringType != typeIdx) {
            logger.error("Overriding method %i does not belong to type %i",
                         oMethodIdx, typeIdx);
            SAFE_ABORT();
        }
        auto &vMethod = builder.methods[vMethodIdx];
        if (static_cast<int16_t>(vMethod.slot) == -1) {
            logger.error("Tried to override non-virtual method %i", vMethodIdx);
            SAFE_ABORT();
        }
        // Check if vMethod exists in vtable interfaces offsets
        int interfaceOffset = -1;
        for (uint16_t i = 0; i < type.interfaces_count; i++) {
            auto &offsetPair =
                builder.interfaceOffsets[type.interfaceOffsetsStart + i];
            TypeIndex iTypeIdx = offsetPair.interfaceTypeIndex;
            if (GetInheritingDefinition(iTypeIdx) == vMethod.declaringType) {
                interfaceOffset = offsetPair.offset;
            }
        }
        uint16_t slot;
        if (interfaceOffset != -1) {
            slot = interfaceOffset + vMethod.slot;
        } else if (RecursiveCheckParents(typeIdx, vMethod.declaringType)) {
            // Method is in a parent type
            slot = vMethod.slot;
        } else {
            logger.error("Could not find vMethod %i in parents or interfaces",
                         vMethodIdx);
            SAFE_ABORT();
        }
        VTableIndex vtableIdx = type.vtableStart + slot;
        EncodedMethodIndex encodedIdx = oMethodIdx;
        encodedIdx |= (kIl2CppMetadataUsageMethodDef << 29);
        builder.vtableMethods[vtableIdx] = encodedIdx;
    }
}

namespace {

bool CARangeTokenComparer(Il2CppCustomAttributeTypeRange a,
                          Il2CppCustomAttributeTypeRange b) {
    return a.token < b.token;
}

} // namespace

void SetCustomAttributes(ImageIndex imageIdx,
                         std::span<MergeCustomAttributeTarget> targets) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    Il2CppImageDefinition &image = builder.images[imageIdx];
    image.customAttributeStart = builder.attributesInfo.size();
    image.customAttributeCount = targets.size();

    for (MergeCustomAttributeTarget &caTarget : targets) {
        Il2CppCustomAttributeTypeRange typeRange;
        typeRange.start = builder.attributeTypes.size();
        typeRange.count = caTarget.attributes.size();
        switch (caTarget.targetType) {
        case AttributeTarget::Type:
            typeRange.token = builder.typeDefinitions[caTarget.targetIdx].token;
        case AttributeTarget::Method:
            typeRange.token = builder.methods[caTarget.targetIdx].token;
        case AttributeTarget::Property:
            typeRange.token = builder.properties[caTarget.targetIdx].token;
        case AttributeTarget::Field:
            typeRange.token = builder.fields[caTarget.targetIdx].token;
        case AttributeTarget::Parameter:
            typeRange.token = builder.parameters[caTarget.targetIdx].token;
        case AttributeTarget::Assembly:
            typeRange.token = builder.assemblies[caTarget.targetIdx].token;
        case AttributeTarget::Event:
            typeRange.token = builder.events[caTarget.targetIdx].token;
        }
        for (CustomAttributeIndex caIdx : caTarget.attributes) {
            builder.attributeTypes.push_back(caIdx);
        }
        builder.attributesInfo.push_back(typeRange);
        ModLoader::addedCACacheGenerators.push_back(caTarget.generator);
    }

    // Sort by token for bsearch
    std::sort(builder.attributesInfo.begin() + image.customAttributeStart,
              builder.attributesInfo.begin() + image.customAttributeStart +
                  image.customAttributeCount,
              CARangeTokenComparer);
}

} // end namespace Merge::API