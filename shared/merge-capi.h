#pragma once

#include "il2cpp-class-internals.h"
#include "il2cpp-metadata.h"

struct MergeParameterDefinition {
    char *name;
    TypeIndex type;
};

struct MergeFieldDefinition {
    char *name;
    TypeIndex type;
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
};

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

void merge_initialize();
Il2CppTypeDefinition *merge_get_type_definition(char *namespaze, char *name);
TypeIndex merge_create_sz_array_type(TypeIndex elementType);
TypeIndex merge_create_pointer_type(TypeIndex type);
ImageIndex merge_create_image(char *name);
AssemblyIndex merge_create_assembly(ImageIndex image, char *name);
TypeDefinitionIndex merge_create_types(ImageIndex image,
                                       MergeTypeDefinition *types,
                                       int32_t typesCount);
MethodIndex merge_create_methods(ImageIndex image, TypeDefinitionIndex type,
                                 MergeMethodDefinition *methods,
                                 int32_t methodsCount);
FieldIndex merge_create_fields(TypeDefinitionIndex type,
                               MergeFieldDefinition *fields,
                               int32_t fieldsCount);
PropertyIndex merge_create_properties(TypeDefinitionIndex type,
                                      MergePropertyDefinition *properties,
                                      int32_t propertiesCount);
void merge_set_method_declaring_type(MethodIndex method, TypeIndex type);

#if defined(__cplusplus)
}
#endif // __cplusplus
