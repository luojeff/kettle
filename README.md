# kettle

Final project for MKS66 (Graphics Programming) Pd. 10

### Contributors
Jeffrey Luo\
Edmond Wong

### Description
  A graphics rendering engine that supporst MDL lighting commands, mesh generation from OBJ files, and predefined constants.

### Features / Plans
- Add multiple light sources\
```light <light name> <X> <Y> <Z> <R> <G> <B>```
- Create predefined constants\
```constant <constant name> {<constant values>...}```
- Create meshes from obj files\
```mesh <file path to OBJ>```\
Note:
OBJ files only support 'v' (vertex) and 'f' (face) prefixes.\
e.g. Valid entries include ```v 5.6 10.3 1.9``` and ```f 1 3 4```

### Compilation
In terminal, type ```$ make```\
Run by typing ```$ make run```. This will automatically use ```basic.mdl``` as input.\
If you want to use your own MDL file, type ```$ ./mdl <MDL file>```
