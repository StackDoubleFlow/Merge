# Merge

Merge was initially intended to be a mod loader designed to load mods created in C# in the worst way possible, but now it's useful for other purposes.

## Usage

This library is released to [QPM](https://github.com/sc2ad/QuestPackageManager).
Simply add and restore the dependency like so:

```bash
qpm dependency add "merge"
qpm restore
```

## Platform Support

Merge is intended to be used for IL2CPP 24.5 (Unity 2019.4.21-2019.4.x) on AArch64. Functionalily on other platforms is not guaranteed. 

## Example

```c++
#include "merge-api.h"

using namespace Merge::API;

void CreateExampleClass() {
    TypeDefinitionIndex objectIdx = FindTypeDefinitionIndex("System", "Object");
    CRASH_UNLESS(objectIdx >= 0);

    AssemblyIndex assembly = CreateAssembly("NewAssembly");
    ImageIndex image = CreateImage(assembly, "NewAssembly.dll");

    MergeTypeDefinition newTypeDef;
    newTypeDef.name = "NewClass";
    newTypeDef.namespaze = "ExampleNamespace";
    newTypeDef.parent = GetTypeDefinition(objectIdx)->byvalTypeIndex;
    newTypeDef.attrs = 0;
    newTypeDef.typeEnum = IL2CPP_TYPE_CLASS;
    newTypeDef.valueType = false;
    newTypeDef.interfaces = {};
    std::vector<MergeTypeDefinition> types = {newTypeDef};
    TypeDefinitionIndex newTypeIdx = CreateTypes(image, std::span(types));
}
```

## License

Merge is licensed under the [MIT License](./LICENSE).

<sub><sub><sub>Work In Progress : )</sub></sub></sub>
