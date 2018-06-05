#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

int read_obj_file(char * path, struct matrix * m){
	FILE * fp;
	char * line[255];
	size_t len = 0;
	char * cursor = line;

	fp = fopen(path,"r");
	if(fp == NULL){
		//obj file is missing
		return 0;
	}

	while( getLine(&line, &len, fp) != -1 ){
		if(cursor[1] != ' ' || cursor[1] != '\t'){
			return -1;
		}
		if(*cursor == 'v'){
			char * nptr = cursor + 1;
			char ** endptr;
			int i;
			int * parameters[3];
			for(i = 0; i < 2; i++){
				parameters[i] = strtod(nptr,endptr);
				if !(isspace(endptr)){
					return -1;
				}
				nptr = *endptr;
			}
			parameters[2] = strtod(nptr,endptr);
			/*
			parameters are the points that should be added to the matrix

			I'll bugtest this later
			*/
		}
		else{
			return -1;
		}
	fclose(fp);
	return 1;
}
