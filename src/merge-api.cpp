#include "merge-api.h"
#include "Logger.h"
#include "ModLoader.h"
#include "beatsaber-hook/shared/utils/utils.h"

namespace Merge::API {

void Initialize() { ModLoader::Initialize(); }

std::optional<TypeDefinitionIndex>
FindTypeDefinitionIndex(std::string_view namespaze, std::string_view name) {
    return ModLoader::metadataBuilder.FindTypeDefinition(namespaze.data(),
                                                         name.data());
}

Il2CppTypeDefinition GetTypeDefinition(TypeDefinitionIndex idx) {
    return ModLoader::metadataBuilder.typeDefinitions[idx];
}

std::optional<MethodIndex>
FindMethodDefinitionIndex(TypeDefinitionIndex typeIdx, std::string_view name,
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

    return std::nullopt;
}

Il2CppMethodDefinition GetMethodDefinition(MethodIndex idx) {
    return ModLoader::metadataBuilder.methods[idx];
}

TypeIndex CreateSZArrayType(TypeIndex elementType) {
    // TODO: Check to see if type already exists
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
    // TODO: Check to see if type already exists
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
    // TODO: token
    assembly.token = 0;

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
    // TODO: token
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

} // namespace

TypeDefinitionIndex CreateTypes(ImageIndex image,
                                std::span<MergeTypeDefinition> types) {
    auto logger = MLogger::GetLogger().WithContext("Merge::API::CreateTypes");
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    TypeDefinitionIndex startIdx = builder.typeDefinitions.size();
    for (auto &type : types) {
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
        builder.interfaces.insert(builder.interfaces.end(),
                                  type.interfaces.begin(),
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
                MLogger::GetLogger().debug(
                    "Placing method with slot %i at slot %i", method.slot, i);
                // vtable should be populated later with a call to
                // SetMethodOverrides
                builder.vtableMethods.push_back(-1);
            }
        }
        typeDef.vtable_count =
            builder.vtableMethods.size() - typeDef.vtableStart;

        logger.debug(
            "Adding type %s.%s, interfaces count: %hi, vtable size: %hi",
            type.namespaze.c_str(), type.name.c_str(), typeDef.interfaces_count,
            typeDef.vtable_count);
        builder.typeDefinitions.push_back(typeDef);
    }

    builder.images[image].typeStart = startIdx;
    builder.images[image].typeCount = types.size();

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
            Il2CppParameterDefinition paramDef;
            paramDef.nameIndex = builder.AppendString(param.name.c_str());
            paramDef.typeIndex = param.type;
            // TODO: token
            paramDef.token = 0;
        }
        // TODO: generics
        methodDef.genericContainerIndex = -1;
        int32_t rid = moduleBuilder.GetNextMethodRID();
        // The upper 8 bits of the token seem to be used for something other
        // than the RID, but I don't know what it is and I don't see it being
        // used.
        methodDef.token = rid;
        methodDef.flags = method.flags;
        methodDef.slot = -1;
        methodDef.parameterCount = method.parameters.size();
        builder.methods.push_back(methodDef);
        int32_t invokerIdx = ModLoader::GetInvokersCount();
        ModLoader::addedInvokers.push_back(method.invoker);
        logger.debug("Adding method %s, RID: %i", method.name, rid);
        moduleBuilder.AppendMethod(method.methodPointer, invokerIdx);
    }

    return startIdx;
}

FieldIndex CreateFields(TypeDefinitionIndex type,
                        std::span<MergeFieldDefinition> fields) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    FieldIndex startIdx = builder.fields.size();
    for (auto &field : fields) {
        Il2CppFieldDefinition fieldDef;
        fieldDef.nameIndex = builder.AppendString(field.name.c_str());
        fieldDef.typeIndex = field.type;
        // TODO: token
        fieldDef.token = 0;
        builder.fields.push_back(fieldDef);
    }

    return startIdx;
}

PropertyIndex CreateProperties(TypeDefinitionIndex type,
                               std::span<MergePropertyDefinition> properties) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    PropertyIndex startIdx = builder.properties.size();
    for (auto &prop : properties) {
        Il2CppPropertyDefinition propDef;
        propDef.nameIndex = builder.AppendString(prop.name.c_str());
        propDef.get = prop.get;
        propDef.set = prop.set;
        propDef.attrs = prop.attrs;
        // TODO: token
        propDef.token = 0;
        builder.properties.push_back(propDef);
    }

    return startIdx;
}

void SetMethodDeclaringType(MethodIndex method, TypeIndex type) {
    ModLoader::metadataBuilder.methods[method].declaringType = type;
}

void SetMethodOverrides(TypeDefinitionIndex typeIdx,
                        const OverridesMap &overrides) {
    auto logger =
        MLogger::GetLogger().WithContext("Merge::API::SetMethodOverrides");
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    Il2CppTypeDefinition &type = builder.typeDefinitions[typeIdx];
    for (auto &[oMethodIdx, vMethodIdx] : overrides) {
        auto &vMethod = builder.methods[vMethodIdx];
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
        // TODO: Check in parents
        if (interfaceOffset == -1) {
            logger.error("Could not find method idx %i in interfaces",
                         vMethodIdx);
        }
        uint16_t slot = interfaceOffset + vMethod.slot;
        builder.methods[oMethodIdx].slot = slot;
        VTableIndex vtableIdx = type.vtableStart + slot;
        EncodedMethodIndex encodedIdx = oMethodIdx;
        encodedIdx |= (kIl2CppMetadataUsageMethodDef << 29);
        builder.vtableMethods[vtableIdx] = encodedIdx;
    }
}

} // end namespace Merge::API