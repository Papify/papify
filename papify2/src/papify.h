/*
 * papify.h
 *
 *  Created on: Apr 29, 2014
 *      Author: aarias
 */
#ifndef PAPIFY_H_
#define PAPIFY_H_

#include <stdio.h>
#include "roxml.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "papicode.h"
#include "eventLib.h"


extern int DEBUG;

// USAR LAS NUEVAS ESTRUCTURAS!!!!
/*typedef struct events_s {
	char *event_name;
} events_s;*/

typedef struct action_s {
	int events_nb;
	char *action_name;
	char *events[];
} action_s;

typedef struct actor_s {
	char *actor_name;
	char *actor_path;
	int actions_nb; //shouldn't be read if papify is not set to 1
	action_s *actions[]; //shouldn't be read if papify is not set to 1
} actor_s;

typedef struct project_s {
	int actors_nb;
	char *src_path;
	char *project_path;
	int *papify; //array same size of actors, if 1, actor will be papified
	action_s *papify_all;
	actor_s *actors[];
} project_s;

/*
typedef struct actors_s {
	int num;
	char *src_path;
	char *project_path;
	int *papify; //array same size of actors, if 1, actor will be papified
	char *actor_path[];
} actors_s;*/


void set_paths(struct project_s *project, char *path);
void set_num(struct project_s *project, int num);
void set_actor_path(struct project_s *project, int num, char *name);
void set_papify_actor(struct project_s *project, int n, int opt);

int papify(struct project_s *project);
void papiwrite(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor);
void insert_papicode(FILE* actor_cpy, char *ref);
int backup_actors(char *actor_path);
void clone(FILE *actor_src, FILE* actor_cpy);
int copy_until(FILE *actor_src, FILE* actor_cpy, char *keyword);
int copy_after(FILE *actor_src, FILE* actor_cpy, char *keyline);
void skiplines(FILE* file, char *ref);
int isblankline(char *line);
void goto_previous_line();
const int isxcf(const char *filename);
int identify_actors(char *mapping_file_path, struct project_s *project);
int get_actors_num(char *mapping_file_path);
char *findMappingFile(char *path);
void free_paths(struct actors_s *actors);
int create_file(char *somepath, char* papicode);

int find_end_of_function(FILE *actor_src, FILE* actor_cpy, int *open_brackets);

char* get_next_action(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, int *open_brackets, int *action_number);

int papify_everything(struct project_s *project);
void papiwrite_init_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, struct action_s *action, int* THREADED);
void papiwrite_actions_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, struct action_s *action, int* THREADED);
#endif /* PAPIFY_H_ */
