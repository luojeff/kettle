#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "string.h"
#include "draw.h"
#include "matrix.h"
#include "obj_reader.h"

/*  
    Parses a .obj file as specified in *path* into a vertex
    matrix *mat* and face order matrix *face_ord*
*/
int read_obj_file(char *path, struct mesh *mh) {

  struct matrix *mat, *face_ord, *vert_norms;
  
  mat = mh->points;
  face_ord = mh->face_ords;
  vert_norms = mh->vert_norms;
  
  FILE *fp = fopen(path, "r");
  char *line = NULL;
  size_t read, len = 0;

  if (fp == NULL)
    printf("Error: Cannot read .obj file!\n");

  double d_params[4], d_params2[3];
  int i_params[4];
  i_params[3] = 0;

  int count;
  char type;
  while ((read = getline(&line, &len, fp)) != -1) {    
    count = 0;
    
    char *s;
    s = strtok(line, " ");

    if(strcmp(s, "v") == 0) {

      // vertex code
      while((s = strtok(NULL, " ")) != NULL)
	d_params[count++] = atof(s);      
      if (mat->lastcol == mat->cols)
	grow_matrix(mat, mat->lastcol + 100);
      mat->m[0][mat->lastcol] = d_params[0];
      mat->m[1][mat->lastcol] = d_params[1];
      mat->m[2][mat->lastcol] = d_params[2];
      mat->m[3][mat->lastcol] = d_params[3];
      mat->lastcol++;
    } else if (strcmp(s, "f") == 0) {
      
      // face code
      while((s = strtok(NULL, " ")) != NULL)
	i_params[count++] = atoi(s);
      if (face_ord->lastcol == face_ord->cols)
	grow_matrix(face_ord, face_ord->lastcol + 100);
      face_ord->m[0][face_ord->lastcol] = (double)i_params[0];
      face_ord->m[1][face_ord->lastcol] = (double)i_params[1];
      face_ord->m[2][face_ord->lastcol] = (double)i_params[2];
      face_ord->m[3][face_ord->lastcol] = (double)i_params[3];
      face_ord->lastcol++;
    } else if (strcmp(s, "vn") == 0) {
      
      // vertex normal code
      while((s = strtok(NULL, " ")) != NULL)
	d_params2[count++] = atof(s);
      if (vert_norms->lastcol == vert_norms->cols)
	grow_matrix(vert_norms, vert_norms->lastcol + 100);
      vert_norms->m[0][vert_norms->lastcol] = d_params2[0];
      vert_norms->m[1][vert_norms->lastcol] = d_params2[1];
      vert_norms->m[2][vert_norms->lastcol] = d_params2[2];
      vert_norms->lastcol++;
    } else if (strcmp(s, "g") == 0) {
      // group code
    }
  }

  fclose(fp);
  
  if(line)
    free(line);
}

/*
  Increments pointer until reaching the next character that is not
  whitespace

  Example usages:

  "v -10.2 10.5 20.1" ===> "-10.2 10.5 20.1"
  "f 5 12 13"         ===> "5 12 31"
*/
char *skip_whitespace(char *s) {
  s++;

  while(s[0] == ' ' || s[0] == '\t')
    s++;
}
