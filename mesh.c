#include <stdlib.h>

#include "mesh.h"

void free_mesh(struct mesh *mesh_contents) {
  free(mesh_contents->points);
  free(mesh_contents->face_ords);
  free(mesh_contents);
}
