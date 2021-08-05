#include "merge-api.h"
#include "ModLoader.h"

namespace Merge::API {

void Initialize() { ModLoader::Initialize(); }

std::optional<Il2CppTypeDefinition>
GetTypeDefinition(std::string_view namespaze, std::string_view name) {}

TypeIndex CreateSZArrayType(TypeIndex elementType) {}

TypeIndex CreatePointerType(TypeIndex type) {}

ImageIndex CreateImage(std::string_view name) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;
    char *cname = new char[name.size() + 1];
    strcpy(cname, name.data());

    Il2CppImageDefinition image;
    image.nameIndex = builder.AppendString(cname);
    image.assemblyIndex = -1;
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

    ModLoader::addedCodeGenModules.emplace(idx, cname);

    return idx;
}

AssemblyIndex CreateAssembly(ImageIndex image, std::string_view name) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

    Il2CppAssemblyDefinition assembly;
    assembly.aname.nameIndex = builder.AppendString(name.data());
    assembly.imageIndex = image;
    assembly.referencedAssemblyStart = -1;
    assembly.referencedAssemblyCount = 0;
    // TODO: token
    assembly.token = 0;

    AssemblyIndex idx = builder.assemblies.size();
    builder.assemblies.push_back(assembly);
    return idx;
}

TypeDefinitionIndex CreateTypes(AssemblyIndex assembly,
                                std::span<MergeTypeDefinition> types) {
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
        typeDef.interfacesStart = -1;
        typeDef.vtableStart = -1;
        typeDef.interfaceOffsetsStart = -1;
        typeDef.method_count = 0;
        typeDef.property_count = 0;
        typeDef.field_count = 0;
        typeDef.event_count = 0;
        typeDef.nested_type_count = 0;
        typeDef.vtable_count = 0;
        typeDef.interfaces_count = 0;
        typeDef.interface_offsets_count = 0;
        typeDef.bitfield = 0;
        typeDef.bitfield |= type.valueType & 1;
        if (type.typeEnum == IL2CPP_TYPE_ENUM)
            typeDef.bitfield |= 2;

        builder.typeDefinitions.push_back(typeDef);
    }

    return startIdx;
}

MethodIndex CreateMethods(TypeDefinitionIndex type,
                          std::span<MergeMethodDefinition> methods) {
    MetadataBuilder &builder = ModLoader::metadataBuilder;

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
        // TODO: token
        methodDef.token = 0;
        methodDef.flags = method.flags;
        // TODO: vtable
        methodDef.slot = 0;
        methodDef.parameterCount = method.parameters.size();
        builder.methods.push_back(methodDef);
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

} // end namespace Merge::API