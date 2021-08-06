#pragma once

#include "il2cpp-class-internals.h"
#include "il2cpp-metadata.h"
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace Merge::API {

struct MergeParameterDefinition {
    std::string name;
    TypeIndex type;
};

struct MergeFieldDefinition {
    std::string name;
    TypeIndex type;
};

struct MergePropertyDefinition {
    std::string name;
    MethodIndex get;
    MethodIndex set;
    uint32_t attrs;
};

struct MergeMethodDefinition {
    std::string name;
    std::vector<MergeParameterDefinition> parameters;
    TypeIndex returnType;
    Il2CppMethodPointer methodPointer;
    InvokerMethod invoker;
    uint16_t flags;
    uint16_t iflags;
};

struct MergeTypeDefinition {
    std::string namespaze;
    std::string name;
    TypeIndex parent;
    uint16_t attrs;
    Il2CppTypeEnum typeEnum;
    bool valueType;
    std::vector<TypeIndex> interfaces;
};

/// Initialize Merge and its API. Call this before anything else.
void Initialize();

/**
 * Finds an existing type definition in the metadata.
 *
 * @param namespaze The namespace of the type.
 * @param name The name of the type.
 * @return The index of the type definition of it was found, otherwise -1
 */
TypeDefinitionIndex FindTypeDefinitionIndex(std::string_view namespaze,
                                            std::string_view name);

/**
 * Gets an existing type definition from the metadata at the specified index.
 *
 * @param idx The index of the type definition.
 * @return A copy of the type definition.
 */
Il2CppTypeDefinition GetTypeDefinition(TypeDefinitionIndex idx);

/**
 * Finds an existing method definition in the metadata.
 *
 * @param type The type to look in.
 * @param name The name of the method.
 * @param paramCount The number of parameters the method has.
 * @return The index of the method definition of it was found, otherwise -1
 */
MethodIndex FindMethodDefinitionIndex(TypeDefinitionIndex typeIdx,
                                      std::string_view name,
                                      uint16_t paramCount);

/**
 * Gets an existing method definition from the metadata at the specified index.
 *
 * @param idx The index of the method definition.
 * @return A copy of the method definition.
 */
Il2CppMethodDefinition GetMethodDefinition(MethodIndex idx);

/**
 * Creates a zero indexed, single dimentional array type from a given element
 * type.
 *
 * @param elementType The index of the element type.
 * @return The index of the array type.
 */
TypeIndex CreateSZArrayType(TypeIndex elementType);

/**
 * Creates a pointer type of the given type.
 *
 * @param type The type to create a pointer type of.
 * @return The index of the pointer type.
 */
TypeIndex CreatePointerType(TypeIndex type);

/**
 * Creates an assembly and adds it to the metadata.
 *
 * You must link an image to this assembly with `CreateImage`
 *
 * @param name The name if the assembly to create.
 * @return Index of the assembly created.
 */
AssemblyIndex CreateAssembly(std::string_view name);

/**
 * Creates an image and links it to the assembly.
 *
 * @param assembly The assembly to link to.
 * @param name The name of the image to create.
 * @return Index of the image created.
 */
ImageIndex CreateImage(AssemblyIndex assembly, std::string_view name);

/**
 * Creates type definitions in the assembly.
 *
 * @param assembly The assembly to create type defintions in.
 * @param types Descriptions of the types to create.
 * @return Index of first type definition created.
 */
TypeDefinitionIndex CreateTypes(ImageIndex image,
                                std::span<MergeTypeDefinition> types);

/**
 * Creates methods in a type.
 *
 * @param image The image of the type.
 * @param type The type to create the methods in.
 * @param methods Descriptions of the methods to create.
 * @return Index of the first method created.
 */
MethodIndex CreateMethods(ImageIndex image, TypeDefinitionIndex type,
                          std::span<MergeMethodDefinition> methods);

/**
 * Creates fields in a type.
 *
 * @param type The type to create the fields in.
 * @param fields Descriptions of the fields to create.
 * @return Index of the first field created.
 */
FieldIndex CreateFields(TypeDefinitionIndex type,
                        std::span<MergeFieldDefinition> fields);

/**
 * Creates properties in a type.
 *
 * @param type The type to create the properties in.
 * @param properties Descriptions of the properties to create.
 * @return Index of the first property created.
 */
PropertyIndex CreateProperties(TypeDefinitionIndex type,
                               std::span<MergePropertyDefinition> properties);

/**
 * Sets the declaring type of a method.
 *
 * @param method The index of a method to set the declaring type of.
 * @param type The type to set the declaying type to.
 */
void SetMethodDeclaringType(MethodIndex method, TypeIndex type);

using OverridesMap = std::unordered_map<MethodIndex, MethodIndex>;

/**
 * Overrides virtual methods of the parent type and implementing interfaces.
 *
 * This will populate `type`'s vtable;
 *
 * @param type The type doing the overriding.
 * @param overrides Mapping of the methods in `type` to the methods you want to
 * override
 */
void SetMethodOverrides(TypeDefinitionIndex type,
                        const OverridesMap &overrides);

} // end namespace Merge::API