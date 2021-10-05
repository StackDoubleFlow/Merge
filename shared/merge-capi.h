#pragma once

#include "il2cpp-class-internals.h"
#include "il2cpp-metadata.h"

struct MergeParameterDefinition {
    char *name;
    TypeIndex type;
    uint16_t attrs;
};

struct MergeFieldDefinition {
    char *name;
    TypeIndex type;
    uint16_t attrs;
};

struct MergePropertyDefinition {
    char *name;
    MethodIndex get;
    MethodIndex set;
    uint32_t attrs;
};

struct MergeMethodDefinition {
    char *name;
    MergeParameterDefinition *parameters;
    int32_t parametersCount;
    TypeIndex returnType;
    Il2CppMethodPointer methodPointer;
    InvokerMethod invoker;
    uint16_t flags;
    uint16_t iflags;
};

struct MergeTypeDefinition {
    char *namespaze;
    char *name;
    TypeIndex parent;
    uint16_t attrs;
    Il2CppTypeEnum typeEnum;
    bool valueType;
    TypeIndex *interfaces;
    int32_t interfacesCount;
};

enum MergeCATargetType {
    MERGE_CA_TARGET_TYPE,
    MERGE_CA_TARGET_METHOD,
    MERGE_CA_TARGET_PROPERTY,
    MERGE_CA_TARGET_FIELD,
    MERGE_CA_TARGET_PARAMETER,
    MERGE_CA_TARGET_ASSEMBLY,
    MERGE_CA_TARGET_EVENT
};

struct MergeCustomAttributeTarget {
    MergeCATargetType targetType;
    uint32_t targetIdx;
    CustomAttributesCacheGenerator generator;
    TypeIndex *attributes;
    int32_t attributesCount;
};

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

void merge_initialize();
TypeDefinitionIndex merge_find_type_definition_index(char *namespaze,
                                                     char *name);
Il2CppTypeDefinition *merge_get_type_definition(TypeDefinitionIndex idx);
MethodIndex merge_find_method_definition_index(TypeDefinitionIndex typeIdx,
                                               char *name, uint16_t paramCount);
Il2CppMethodDefinition merge_get_method_definition(MethodIndex idx);
TypeIndex merge_create_sz_array_type(TypeIndex elementType);
TypeIndex merge_create_pointer_type(TypeIndex type);
AssemblyIndex merge_create_assembly(char *name);
ImageIndex merge_create_image(AssemblyIndex assembly, char *name);
TypeDefinitionIndex merge_create_types(ImageIndex image,
                                       MergeTypeDefinition *types,
                                       int32_t typesCount);
MethodIndex merge_create_methods(ImageIndex image, TypeDefinitionIndex type,
                                 MergeMethodDefinition *methods,
                                 int32_t methodsCount);
FieldIndex merge_create_fields(ImageIndex image, TypeDefinitionIndex type,
                               MergeFieldDefinition *fields,
                               int32_t fieldsCount);
PropertyIndex merge_create_properties(ImageIndex image,
                                      TypeDefinitionIndex type,
                                      MergePropertyDefinition *properties,
                                      int32_t propertiesCount);
void merge_set_method_declaring_type(MethodIndex method, TypeIndex type);
void merge_set_method_overrides(TypeDefinitionIndex type,
                                uint32_t overridingMethodsCount,
                                MethodIndex *overridingMethods,
                                MethodIndex *virtualMethods);
void merge_set_custom_attributes(ImageIndex image,
                                 MergeCustomAttributeTarget *targets,
                                 uint32_t targetsCount);
void merge_offset_size(TypeDefinitionIndex type, int32_t sizeOffset);

#if defined(__cplusplus)
}
#endif // __cplusplus
