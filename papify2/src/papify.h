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

//NEW SCHOOL
typedef struct n_events_s {
	int events_nb;
	char *event_names[];
} n_events_s;

typedef struct n_actor_s {
	int papify; //if 0: Not papified, if 1: papify some actions, if 2: papify all actions
	char *actor_name;
	char *actor_path;
	int actions_nb; //shouldn't be read if papify is NOT set to 1
	char *action_names[]; //shouldn't be read if papify is NOT set to 1
} n_actor_s;

typedef struct n_project_s {
	char *src_path;
	char *project_path;
	n_events_s *events;
	int actors_nb;
	n_actor_s *actors[];
} n_project_s;

void set_paths(struct n_project_s *project, char *path);
void set_num(struct n_project_s *project, int num);
void set_actor_path(struct n_project_s *project, int num, char *name);
void set_papify_actor(struct n_project_s *project, int n, int opt);

int papify(struct n_project_s *project);
void papiwrite(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, struct n_events_s *events);
void insert_papicode(FILE* actor_cpy, char *ref);
int backup_actors(char *actor_path);
void clone(FILE *actor_src, FILE* actor_cpy);
int copy_until(FILE *actor_src, FILE* actor_cpy, char *keyword);
int copy_after(FILE *actor_src, FILE* actor_cpy, char *keyline);
void skiplines(FILE* file, char *ref);
int isblankline(char *line);
void goto_previous_line();
const int isxcf(const char *filename);
int identify_actors(char *mapping_file_path, struct n_project_s *project);
int get_actors_num(char *mapping_file_path);
char *findMappingFile(char *path);
void free_paths(struct actors_s *actors);
int create_file(char *somepath, char* papicode);

int find_end_of_function(FILE *actor_src, FILE* actor_cpy, int *open_brackets);

char* get_next_action(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, int *open_brackets, int *action_number);

int papify_everything(struct n_project_s *project);
void papiwrite_init_everything(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, struct n_events_s *events, int* THREADED);
void papiwrite_actions_everything(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, struct n_events_s *events, int* THREADED);
#endif /* PAPIFY_H_ */
