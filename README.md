# kettle

Final project for MKS66 (Graphics Programming) Pd. 10

### Contributors
Jeffrey Luo\
Edmond Wong

### Description
  A graphics rendering engine that supports MDL lighting commands (predefined constants and light sources), mesh generation from OBJ files, and new vary modifiers.

### Features
- Add multiple light sources\
```light <light name> <X> <Y> <Z> <R> <G> <B>```
- Set ambient lightning\
```ambient <r> <g> <b>```
- Create predefined constants\
```constant <constant name> <R-amb> <R-diff> <R-spec> <G-amb> <G-diff> <G-spec> <B-amb> <B-diff> <B-spec>```\
Define a constant: ```constants shiny 0.2 0.4 0.85 0.2 0.4 0.85 0.2 0.4 0.85```\
Then attach them later:
```
mesh shiny :teapot.obj
sphere shiny 0 0 0 100
```
- Create meshes from obj files\
```mesh <constant> :<file path to OBJ>```
Note:
OBJ files only support `v` (vertex) and `f` (face) prefixes.\
e.g. Valid entries include ```v 5.6 10.3 1.9``` and ```f 1 3 4```
- Create non-linear vary modifiers. Approximates using a trinomial obtained from a hermite curve matrix.\
```vary <knob_name> <start_frame> <end_frame> <start_val> <end_val> <start "slope"> <end "slope">```\
The last two arguments represent the "slope" or magnitude of the knob variation at the start and end respectively.
Example usage: ```vary spinny 0 49 0 1 3 1.5```\
If you leave them out, a linear increment will be used.

### Compilation
In the terminal, type ```$ make```\
Run by typing ```$ make run```. This will automatically use ```pumpkin.mdl``` as an input.\
If you wish to use your own MDL file, type ```$ ./mdl <MDL file>```
