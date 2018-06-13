#ifndef OBJ_READER_H
#define OBJ_READER_H

/*
reads from an obj file at <path> and adds points to
<matrix>
returns 0 if failed to find file
returns -1 if what's written in the obj file is incorrect
returns 1 otherwise
*/
int read_obj_file(char *, struct matrix *, struct matrix *);
char *skip_whitespace(char *);

#endif
