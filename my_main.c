/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
  based on the provided values, then
  multiply the current top of the
  origins stack by it.

  box/sphere/torus: create a solid object based on the
  provided values. Store that in a
  temporary matrix, multiply it by the
  current top of the origins stack, then
  call draw_polygons.

  line: create a line based on the provided values. Stores
  that in a temporary matrix, multiply it by the
  current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"
#include "obj_reader.h"
#include "lights.h"


/*======== void first_pass() ==========
  Inputs:
  Returns:

  Checks the op array for any animation commands
  (frames, basename, vary)

  Should set num_frames and basename if the frames
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.
  ====================*/
void first_pass() {
  //in order to use name and num_frames throughout
  //they must be extern variables
  extern int num_frames;
  extern char name[128];

  int frames_found = 0, vary_found = 0, basename_found = 0;  

  int i;
  for(i=0; i<lastop; i++) {
    switch (op[i].opcode) {
    case FRAMES:
      num_frames = op[i].op.frames.num_frames;
      frames_found++;
      break;
    case BASENAME:
      basename_found++;
      strcpy(name, op[i].op.basename.p->name);
      break;
    case VARY:
      vary_found++;
      break;
    }
  }

  // Error and bound checking
  if(!frames_found) {
    printf("Error: No frames found\n");
    exit(0);
  } else if(!vary_found) {
    printf("Warning: No vary found\n");
  }
  
  if(vary_found && !basename_found) {
    printf("Error: Vary found, basename not found\n");
    exit(0);
  }

  if (frames_found && !basename_found) {
    char def_name[] = "default";
    strcpy(name, def_name);    
    printf("Basename undefined, set to \"%s\"\n", def_name);
  }

  if(basename_found > 1) {
    printf("Error: Multiple basenames found!\n");
  } else if (frames_found > 1) {
    printf("Error: Multiple frames found!\n");
  }
}

/*======== struct vary_node ** second_pass() ==========
  Inputs:
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a separate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropirate value.
  ====================*/
struct vary_node **second_pass() {
  struct vary_node **arr_nodes = (struct vary_node **)calloc(num_frames, sizeof(struct vary_node *));

  int sf, ef;
  double sv, ev;

  int i;
  for(i=0; i<lastop; i++) {
    if(op[i].opcode == VARY) {
      sf = op[i].op.vary.start_frame, ef = op[i].op.vary.end_frame;
      sv = op[i].op.vary.start_val, ev = op[i].op.vary.end_val;

      double cv = sv; // current knob value
      double inc = (double)(ev - sv) / (ef - sf);
      struct vary_node *vnode, *new_node;

      int j;
      for(j=sf; j<=ef; j++){	
	if(arr_nodes[j] == NULL) {
	  vnode = (struct vary_node *)malloc(sizeof(struct vary_node));
	  strncpy(vnode->name, op[i].op.vary.p->name, 128);
	  vnode->value = cv;
	  vnode->next = NULL;
	  arr_nodes[j] = vnode;
	} else {
	  vnode = arr_nodes[j];
	  while(vnode->next != NULL)
	    vnode = vnode->next;
	  new_node = (struct vary_node *)malloc(sizeof(struct vary_node));
	  vnode->next = new_node;
	  new_node->next = NULL;
	  strncpy(new_node->name, op[i].op.vary.p->name, 128);
	  new_node->value = cv;
	}
	
	cv += inc;
      }
    }
  }

  return arr_nodes;
}

/*======== void print_knobs() ==========
  Inputs:
  Returns:

  Goes through symtab and display all the knobs and their
  current values
  ====================*/
void print_knobs() {
  int i;

  printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
  for(i=0; i<lastsym; i++) {
    if(symtab[i].type == SYM_VALUE) {
      printf( "%d\t%s\t\t", i, symtab[i].name );
      printf( "SYM_VALUE\t");
      printf( "%6.2f\n", symtab[i].s.value);
    }
  }
}

/*======== void my_main() ==========
  Inputs:
  Returns:

  This is the main engine of the interpreter, it should
  handle most of the commands in mdl.

  If frames is not present in the source (and therefore
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf,
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487
  ====================*/
void my_main() {

  int i;
  struct matrix *tmp;
  struct matrix *face_order;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  g.red = 0;
  g.green = 0;
  g.blue = 0;
  double step_3d = 30;
  double theta;
  double knob_value, xval, yval, zval;

  //variables for constants
  SYMTAB * constant_dictionary[255];
  int constant_dictionary_size = 0;
  //

  //Lighting values here for easy access    
  color ambient;
  double view[3];
  double areflect[3];
  double dreflect[3];
  double sreflect[3];

  // Supports up to MAX_LIGHTS light sources
  double light[MAX_LIGHTS][2][3];

  // Default ambient light if none specified:
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  // Default location for light if none specified:
  // (x, y, z) = (  1,   1,   1)
  // (r, g, b) = (255, 255, 255)  
  light[0][LOCATION][0] = 1;
  light[0][LOCATION][1] = 1;
  light[0][LOCATION][2] = 1;

  light[0][COLOR][RED] = 255;
  light[0][COLOR][GREEN] = 255;
  light[0][COLOR][BLUE] = 255;

  // View vector doesn't change
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;
  
  areflect[RED] = 0.1;
  areflect[GREEN] = 0.1;
  areflect[BLUE] = 0.1;

  dreflect[RED] = 0.5;
  dreflect[GREEN] = 0.5;
  dreflect[BLUE] = 0.5;

  sreflect[RED] = 0.5;
  sreflect[GREEN] = 0.5;
  sreflect[BLUE] = 0.5;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen(t);
  clear_zbuffer(zb);

  first_pass();  
  struct vary_node **vary_nodes = second_pass();
  
  int a;
  for(a=0; a<num_frames; a++) {

    int light_count = 0;
    SYMTAB *curr_sym;
    struct vary_node *curr_node = vary_nodes[a];

    // Set symbol table
    while(curr_node != NULL) {
      curr_sym = lookup_symbol(curr_node->name);
      set_value(curr_sym, curr_node->value);
      curr_node = curr_node->next;
    }
    
    knob_value = 1.0;
    for(i=0; i<lastop; i++) {
      
      switch (op[i].opcode) {
      case SPHERE:
	/* printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f", */
	/* 	 op[i].op.sphere.d[0],op[i].op.sphere.d[1], */
	/* 	 op[i].op.sphere.d[2], */
	/* 	 op[i].op.sphere.r); */
	if (op[i].op.sphere.constants != NULL)
	  {
	    //printf("\tconstants: %s",op[i].op.sphere.constants->name);
	  }
	if (op[i].op.sphere.cs != NULL)
	  {
	    //printf("\tcs: %s",op[i].op.sphere.cs->name);
	  }
	add_sphere(tmp, op[i].op.sphere.d[0],
		   op[i].op.sphere.d[1],
		   op[i].op.sphere.d[2],
		   op[i].op.sphere.r, step_3d);
	matrix_mult( peek(systems), tmp );
	draw_polygons(tmp, t, zb, view, light, ambient,
		      areflect, dreflect, sreflect, light_count);
	tmp->lastcol = 0;
	break;
      case TORUS:
	/* printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f", */
	/* 	 op[i].op.torus.d[0],op[i].op.torus.d[1], */
	/* 	 op[i].op.torus.d[2], */
	/* 	 op[i].op.torus.r0,op[i].op.torus.r1); */
	if (op[i].op.torus.constants != NULL)
	  {
	    //printf("\tconstants: %s",op[i].op.torus.constants->name);
	  }
	if (op[i].op.torus.cs != NULL)
	  {
	    //printf("\tcs: %s",op[i].op.torus.cs->name);
	  }
	add_torus(tmp,
		  op[i].op.torus.d[0],
		  op[i].op.torus.d[1],
		  op[i].op.torus.d[2],
		  op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
	matrix_mult( peek(systems), tmp );
	draw_polygons(tmp, t, zb, view, light, ambient,
		      areflect, dreflect, sreflect, light_count);
	tmp->lastcol = 0;
	break;
      case BOX:
	/* printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f", */
	/* 	 op[i].op.box.d0[0],op[i].op.box.d0[1], */
	/* 	 op[i].op.box.d0[2], */
	/* 	 op[i].op.box.d1[0],op[i].op.box.d1[1], */
	/* 	 op[i].op.box.d1[2]); */
	if (op[i].op.box.constants != NULL)
	  {
	    //printf("\tconstants: %s",op[i].op.box.constants->name);
	  }
	if (op[i].op.box.cs != NULL)
	  {
	    //printf("\tcs: %s",op[i].op.box.cs->name);
	  }
	add_box(tmp,
		op[i].op.box.d0[0],op[i].op.box.d0[1],
		op[i].op.box.d0[2],
		op[i].op.box.d1[0],op[i].op.box.d1[1],
		op[i].op.box.d1[2]);
	matrix_mult(peek(systems), tmp);
	draw_polygons(tmp, t, zb, view, light, ambient,
		      areflect, dreflect, sreflect, light_count);
	tmp->lastcol = 0;
	break;
      case MESH:
	add_mesh(tmp, op[i].op.mesh.name);
	matrix_mult(peek(systems), tmp);
	draw_polygons(tmp, t, zb, view, light, ambient,
		      areflect, dreflect, sreflect, light_count);
	tmp->lastcol = 0;	
	break;	  
      case LINE:
	/* printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",*/
	/* 	 op[i].op.line.p0[0],op[i].op.line.p0[1], */
	/* 	 op[i].op.line.p0[1], */
	/* 	 op[i].op.line.p1[0],op[i].op.line.p1[1], */
	/* 	 op[i].op.line.p1[1]); */
	if (op[i].op.line.constants != NULL)
	  {
	    //printf("\n\tConstants: %s",op[i].op.line.constants->name);
	  }
	if (op[i].op.line.cs0 != NULL)
	  {
	    //printf("\n\tCS0: %s",op[i].op.line.cs0->name);
	  }
	if (op[i].op.line.cs1 != NULL)
	  {
	    //printf("\n\tCS1: %s",op[i].op.line.cs1->name);
	  }
	add_edge(tmp,
		 op[i].op.line.p0[0],op[i].op.line.p0[1],
		 op[i].op.line.p0[2],
		 op[i].op.line.p1[0],op[i].op.line.p1[1],
		 op[i].op.line.p1[2]);
	matrix_mult( peek(systems), tmp );
	draw_lines(tmp, t, zb, g);
	tmp->lastcol = 0;
	break;
      case MOVE:
	xval = op[i].op.move.d[0];
	yval = op[i].op.move.d[1];
	zval = op[i].op.move.d[2];
	printf("Move: %6.2f %6.2f %6.2f",
	       xval, yval, zval);
	
	if (op[i].op.move.p != NULL) {
	  printf("\tknob: %s",op[i].op.move.p->name);
	  SYMTAB *curr_sym = lookup_symbol(op[i].op.move.p->name);
	  knob_value = curr_sym->s.value;
	  xval *= knob_value;
	  yval *= knob_value;
	  zval *= knob_value;
	}
	tmp = make_translate( xval, yval, zval );
	matrix_mult(peek(systems), tmp);
	copy_matrix(tmp, peek(systems));
	tmp->lastcol = 0;
	break;
      case SCALE:
	xval = op[i].op.scale.d[0];
	yval = op[i].op.scale.d[1];
	zval = op[i].op.scale.d[2];
	printf("Scale: %6.2f %6.2f %6.2f",
	       xval, yval, zval);
	if (op[i].op.scale.p != NULL) {
	  printf("\tknob: %s",op[i].op.scale.p->name);
	  SYMTAB *curr_sym = lookup_symbol(op[i].op.scale.p->name);
	  knob_value = curr_sym->s.value;
	  xval *= knob_value;
	  yval *= knob_value;
	  zval *= knob_value;
	}
	tmp = make_scale( xval, yval, zval );
	matrix_mult(peek(systems), tmp);
	copy_matrix(tmp, peek(systems));
	tmp->lastcol = 0;
	break;
      case ROTATE:
	xval = op[i].op.rotate.axis;
	theta = op[i].op.rotate.degrees;
	printf("Rotate: axis: %6.2f degrees: %6.2f",
	       xval, theta);
	if (op[i].op.rotate.p != NULL) {
	  printf("\tknob: %s",op[i].op.rotate.p->name);
	  SYMTAB *curr_sym = lookup_symbol(op[i].op.rotate.p->name);
	  knob_value = curr_sym->s.value;
	  theta *= knob_value;
	}	
	theta *= (M_PI / 180);
	if (op[i].op.rotate.axis == 0 )
	  tmp = make_rotX( theta );
	else if (op[i].op.rotate.axis == 1 )
	  tmp = make_rotY( theta );
	else
	  tmp = make_rotZ( theta );

	matrix_mult(peek(systems), tmp);
	copy_matrix(tmp, peek(systems));
	tmp->lastcol = 0;
	break;
      case AMBIENT:
	ambient.red = op[i].op.ambient.c[0];
	ambient.green = op[i].op.ambient.c[1];
	ambient.blue = op[i].op.ambient.c[2];
	break;
      case LIGHT:
	if(light_count < MAX_LIGHTS) {
	  struct light *lgt = op[i].op.light.p->s.l;
	  (light[light_count])[LOCATION][0] = lgt->l[0];
	  (light[light_count])[LOCATION][1] = lgt->l[1];
	  (light[light_count])[LOCATION][2] = lgt->l[2];
	  
	  (light[light_count])[COLOR][RED] = lgt->c[0];
	  (light[light_count])[COLOR][GREEN] = lgt->c[1];
	  (light[light_count])[COLOR][BLUE] = lgt->c[2];
	  
	  light_count++;
	}
	break;
      case CONSTANTS:
	; // don't remove this semicolon
      char * name = op[i].op.constants.p->name;
      printf("name: %s\n",name);

      struct constants *cons;

      printf("part0\n");
      int ii;
      for(ii = 0; ii < constant_dictionary_size; ii++){
        if(strcmp(name,constant_dictionary[ii]->name) == 0){
            //if constant is already defined
            cons = constant_dictionary[ii]->s.c;
            break;
        }
      }
      printf("part1\n");
      if(ii >= constant_dictionary_size){
          //if constant is not defined
		  printf("part2\n");
          cons = op[i].op.constants.p->s.c;
          printf("part3\n");
          constant_dictionary[constant_dictionary_size] = op[i].op.constants.p;
          printf("part4\n");
          constant_dictionary_size++;
      }

	  cons->r;
	  cons->g;
	  cons->b;
	
	  view[0] = cons->r[0];
	  view[1] = cons->g[0];
	  view[2] = cons->b[0];

	  areflect[RED]   = cons->r[1];
	  areflect[GREEN] = cons->g[1];
	  areflect[BLUE]  = cons->b[1];
	
	  dreflect[RED]   = cons->r[2];
	  dreflect[GREEN] = cons->g[2];
	  dreflect[BLUE]  = cons->b[2];

	  sreflect[RED]   = cons->r[3];
	  sreflect[GREEN] = cons->g[3];
	  sreflect[BLUE]  = cons->b[3];
	
	break;
      case PUSH:
	//printf("Push");
	push(systems);
	break;
      case POP:
	//printf("Pop");
	pop(systems);
	break;
      case SAVE:
	//printf("Save: %s",op[i].op.save.p->name);
	save_extension(t, op[i].op.save.p->name);
	break;
      case DISPLAY:
	//printf("Display");
	display(t);
	break;
      } //end opcode switch      

      printf("\n");
    }//end operation loop
    
    // Saving images into directory
    char rel_file_path[128];
    mkdir(DIRECTORY_NAME, 0744);
    sprintf(rel_file_path, "%s/%s%03d", DIRECTORY_NAME, name, a);
    save_extension(t, rel_file_path);

    // Reset screen and z-buffer
    clear_zbuffer(zb);
    clear_screen(t);
    
    // Reset stack
    free_stack(systems);
    systems = new_stack();

    // Reset temp matrix
    free(tmp);
    tmp = new_matrix(4, 1000);
  }

  make_animation(name); // Auto-create GIF

  printf("Finished!\n");
}
