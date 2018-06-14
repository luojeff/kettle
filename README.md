# kettle

Final project for MKS66 (Graphics Programming) Pd. 10

### Contributors
Jeffrey Luo\
Edmond Wong

### Description
  A graphics rendering engine that supports MDL lighting commands, mesh generation from OBJ files, and predefined constants.

### Features / Plans
- Add multiple light sources\
```light <light name> <X> <Y> <Z> <R> <G> <B>```
- Create predefined constants\
```constant <constant name> <constant value 1> <constant value 2> ...```
- Create meshes from obj files and attach constants to them to simulate certain textures\
```mesh <constant> <file path to OBJ>```\
- Ambient lightning\
```ambient <r> <g> <b>```\

Note:
OBJ files only support `v` (vertex) and `f` (face) prefixes.\
e.g. Valid entries include ```v 5.6 10.3 1.9``` and ```f 1 3 4```
- Supports non-linear vary modifiers. Utilizes hermite-curve mechanics by generating a trinomial.\
```vary <knob_name> <start_frame> <end_frame> <start_val> <end_val> <start "slope"> <end "slope">```\
The last two arguments represent the "slope" or magnitude of the knob variation at the start and end respectively.
Example usage: ```vary spinny 0 49 0 1 3 1.5```

### Compilation
In terminal, type ```$ make```\
Run by typing ```$ make run```. This will automatically use ```basic.mdl``` as an input.\
If you want to use your own MDL file, type ```$ ./mdl <MDL file>```
