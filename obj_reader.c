#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "matrix.h"
#include "obj_reader.h"

int read_obj_file(char *path, struct matrix *mat) {
  FILE *fp;
  size_t len = 0;
  
  char line[255];
  char *cursor = line;  

  fp = fopen(path,"r");
  
  if(fp == NULL){
    //obj file is missing
    return 0;
  }

  while( getline(&line, &len, fp) != -1 ){
    if(cursor[1] != ' ' || cursor[1] != '\t'){
      return -1;
    }
    
    if(*cursor == 'v'){
      
      char *nptr = cursor + 1;-
      char **endptr;
      
      int i;
      double parameters[3];
      for(i = 0; i < 2; i++){
	parameters[i] = strtod(nptr, endptr);
	if(!isspace(endptr))
	  return -1;
	
	nptr = *endptr;
      }
      
      parameters[2] = strtod(nptr, endptr);

      mat->m[0][mat->lastcol] = parameters[0];
      mat->m[1][mat->lastcol] = parameters[1];
      mat->m[2][mat->lastcol] = parameters[2];
      
    } else {
      return -1;
    }
  }
    
  fclose(fp);
  
  return 1;
}
