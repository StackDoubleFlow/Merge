#include "merge-capi.h"
#include "merge-api.h"

void merge_initialize();

TypeDefinitionIndex merge_find_type_definition_index(char *namespaze,
                                                     char *name) {
    return Merge::API::FindTypeDefinitionIndex(namespaze, name);
}

Il2CppTypeDefinition merge_get_type_definition(TypeDefinitionIndex idx) {
    return Merge::API::GetTypeDefinition(idx);
}

MethodIndex merge_find_method_definition_index(TypeDefinitionIndex typeIdx,
                                               char *name,
                                               uint16_t paramCount) {
    return Merge::API::FindMethodDefinitionIndex(typeIdx, name, paramCount);
}

Il2CppMethodDefinition merge_get_method_definition(MethodIndex idx) {
    return Merge::API::GetMethodDefinition(idx);
}

TypeIndex merge_create_sz_array_type(TypeIndex elementType) {
    return Merge::API::CreateSZArrayType(elementType);
}

TypeIndex merge_create_pointer_type(TypeIndex type) {
    return Merge::API::CreatePointerType(type);
}

AssemblyIndex merge_create_assembly(char *name) {
    return Merge::API::CreateAssembly(name);
}

ImageIndex merge_create_image(AssemblyIndex assembly, char *name) {
    return Merge::API::CreateImage(assembly, name);
}

TypeDefinitionIndex merge_create_types(ImageIndex image,
                                       MergeTypeDefinition *types,
                                       int32_t typesCount) {
    std::vector<Merge::API::MergeTypeDefinition> vec;
    for (int32_t i = 0; i < typesCount; i++) {
        MergeTypeDefinition &cTypeDef = types[i];
        Merge::API::MergeTypeDefinition typeDef;
        typeDef.namespaze = cTypeDef.namespaze;
        typeDef.name = cTypeDef.name;
        typeDef.parent = cTypeDef.parent;
        typeDef.attrs = cTypeDef.attrs;
        typeDef.typeEnum = cTypeDef.typeEnum;
        typeDef.valueType = cTypeDef.valueType;
        typeDef.interfaces =
            std::vector(cTypeDef.interfaces,
                        cTypeDef.interfaces + cTypeDef.interfacesCount);
        vec.push_back(typeDef);
    }
    return Merge::API::CreateTypes(image, std::span(vec));
}

MethodIndex merge_create_methods(ImageIndex image, TypeDefinitionIndex type,
                                 MergeMethodDefinition *methods,
                                 int32_t methodsCount) {
    std::vector<Merge::API::MergeMethodDefinition> vec;
    for (int32_t i = 0; i < methodsCount; i++) {
        MergeMethodDefinition &cMethodDef = methods[i];
        Merge::API::MergeMethodDefinition methodDef;
        methodDef.name = cMethodDef.name;
        std::vector<Merge::API::MergeParameterDefinition> params;
        for (int32_t j = 0; j < cMethodDef.parametersCount; j++) {
            MergeParameterDefinition &cParamDef = cMethodDef.parameters[i];
            Merge::API::MergeParameterDefinition paramDef;
            paramDef.name = cParamDef.name;
            paramDef.type = cParamDef.type;
            params.push_back(paramDef);
        }
        methodDef.parameters = params;
        methodDef.returnType = cMethodDef.returnType;
        methodDef.methodPointer = cMethodDef.methodPointer;
        methodDef.invoker = cMethodDef.invoker;
        methodDef.flags = cMethodDef.flags;
        methodDef.iflags = cMethodDef.iflags;
        vec.push_back(methodDef);
    }
    return Merge::API::CreateMethods(image, type, std::span(vec));
}

FieldIndex merge_create_fields(TypeDefinitionIndex type,
                               MergeFieldDefinition *fields,
                               int32_t fieldsCount) {
    std::vector<Merge::API::MergeFieldDefinition> vec;
    for (int32_t i = 0; i < fieldsCount; i++) {
        MergeFieldDefinition &cFieldDef = fields[i];
        Merge::API::MergeFieldDefinition fieldDef;
        fieldDef.name = cFieldDef.name;
        fieldDef.name = cFieldDef.type;
        vec.push_back(fieldDef);
    }
    return Merge::API::CreateFields(type, std::span(vec));
}

PropertyIndex merge_create_properties(TypeDefinitionIndex type,
                                      MergePropertyDefinition *properties,
                                      int32_t propertiesCount) {
    std::vector<Merge::API::MergePropertyDefinition> vec;
    for (int32_t i = 0; i < propertiesCount; i++) {
        MergePropertyDefinition &cPropDef = properties[i];
        Merge::API::MergePropertyDefinition propDef;
        propDef.name = cPropDef.name;
        propDef.get = cPropDef.get;
        propDef.set = cPropDef.set;
        propDef.attrs = cPropDef.attrs;
        vec.push_back(propDef);
    }
    return Merge::API::CreateProperties(type, std::span(vec));
}

void merge_set_method_declaring_type(MethodIndex method, TypeIndex type) {
    Merge::API::SetMethodDeclaringType(method, type);
}

void merge_set_method_overrides(TypeDefinitionIndex type,
                                uint32_t overridingMethodsCount,
                                MethodIndex *overridingMethods,
                                MethodIndex *virtualMethods) {
    std::unordered_map<MethodIndex, MethodIndex> map;
    for (uint32_t i = 0; i < overridingMethodsCount; i++) {
        map.insert(std::make_pair(overridingMethods[i], virtualMethods[i]));
    }
    Merge::API::SetMethodOverrides(type, map);
}