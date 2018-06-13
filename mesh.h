#ifndef MESH_H
#define MESH_H

struct mesh {
  struct matrix *points;
  struct matrix *face_ords;
};

void free_mesh(struct mesh *);

#endif
