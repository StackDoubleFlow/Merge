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
    // See section "Flags for Params (22.1.12)" in il2cpp-tabledefs.h
    uint16_t attrs;
};

struct MergeFieldDefinition {
    std::string name;
    TypeIndex type;
    // See section "Field Attributes (21.1.5)" in il2cpp-tabledefs.h
    uint16_t attrs;
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

enum struct AttributeTarget {
    Type,
    Method,
    Property,
    Field,
    Parameter,
    Assembly,
    Event
};

struct MergeCustomAttributeTarget {
    AttributeTarget targetType;
    uint32_t targetIdx;
    CustomAttributesCacheGenerator generator;
    std::vector<TypeIndex> attributes;
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
 * Creates a zero indexed, single dimensional array type from a given element
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
 * Creates type definitions in the image.
 *
 * @param image The image to create type defintions in.
 * @param types Descriptions of the types to create.
 * @return Index of first type definition created.
 */
TypeDefinitionIndex CreateTypes(ImageIndex image,
                                std::span<MergeTypeDefinition> types);

/**
 * Appends types to the last image you used CreateTypes with.
 *
 * @param types Descriptions of the types to create.
 * @return Index of first type definition appended to the image.
 */
TypeDefinitionIndex AppendTypes(std::span<MergeTypeDefinition> types);

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
FieldIndex CreateFields(ImageIndex image, TypeDefinitionIndex type,
                        std::span<MergeFieldDefinition> fields);

/**
 * Creates properties in a type.
 *
 * @param type The type to create the properties in.
 * @param properties Descriptions of the properties to create.
 * @return Index of the first property created.
 */
PropertyIndex CreateProperties(ImageIndex image, TypeDefinitionIndex type,
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
 * This will populate `type`'s vtable. Crashes if the overriding method is not a
 * member of `type`, the virtual method you are trying to override is not a
 * member of the parents or interfaces, or the method you are
 * trying to override does not have a slot in the vtable.
 *
 * This can be called multiple times for a type. Because of this, it does not
 * check if any slots in the vtable go unpopulated.
 *
 * @param type The type doing the overriding.
 * @param overrides Mapping of the methods in `type` to the methods you want to
 * override
 */
void SetMethodOverrides(TypeDefinitionIndex type,
                        const OverridesMap &overrides);

/**
 * Set all the custom attributes in an image.
 *
 * This should only be called once per image.
 *
 * @param image The image containing the targets.
 * @param targets Each target to set the custom attributes of.
 */
void SetCustomAttributes(ImageIndex image,
                         std::span<MergeCustomAttributeTarget> targets);

/**
 * Offsets the size of a type definition. This can be useful for custom c++
 * fields or anything extra that il2cpp cannot calculate itself.
 *
 * It can also subtract the size but that sounds like it can go horribly
 * wrongly.
 *
 * @param type The type defintion to modify.
 * @param sizeOffset The amount of bytes to offset the size by.
 */
void OffsetSize(TypeDefinitionIndex type, int32_t sizeOffset);

} // end namespace Merge::API