#ifndef MESH_H
#define MESH_H

struct mesh {
  struct matrix *points;  
  struct matrix *face_ords;
  struct matrix *vert_norms;
};

void free_mesh(struct mesh *);

#endif
