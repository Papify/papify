/*
 * papify_lib.c
 *
 *  Created on: Apr 29, 2014
 *      Author: aarias
 */

#include "papify.h"

int DEBUG;

int papify(struct n_project_s *project){
	FILE *actor_src;
	FILE *actor_cpy;
	int i;
	char actor_cpy_path[500];
	char c;

	for(i = 0; i<project->actors_nb;i++){
		if(project->actors[i]->papify != 0){//if it's scheduled for adding papi code..
			if(DEBUG) printf("Creating a backup of %s\n", project->actors[i]->actor_path);
			if(backup_actors(project->actors[i]->actor_path)==-1){
				printf("WARNING: Looks like the code for %s has already been manipulated.\nThis might lead to errors in the final output.\nDo you still want to continue? [y/n]\n",project->actors[i]->actor_path);
				scanf("%c", &c);
				getchar();
				if(c == 'n'){
					printf("Skipping this actor\n");
					continue;
				}
			}


			if(DEBUG) printf("Attempting to open %s\n", project->actors[i]->actor_path);
			actor_src =fopen(project->actors[i]->actor_path,"r");
			if (!actor_src) {
				printf("ERROR opening source file, aborting this actor\n");
				continue;
			}

			strcpy(actor_cpy_path, project->actors[i]->actor_path);
			strcat(actor_cpy_path, ".papi.c");
			actor_cpy = fopen(actor_cpy_path,"w"); //this is a temp file that contains the actor code with the papi code added, will later replace the actual actor

			if(DEBUG) printf("Copying initializes in %s\n", project->actors[i]->actor_path);

			if(project->actors[i]->papify==1) papiwrite(actor_src, actor_cpy, project->actors[i], project->events);
			if(project->actors[i]->papify==2) papiwrite_everything(actor_src, actor_cpy, project->actors[i], project->events);


			fclose(actor_src);
			actor_src =fopen(project->actors[i]->actor_path,"w");
			fclose(actor_cpy);
			actor_cpy =fopen(actor_cpy_path,"r");

			clone(actor_cpy,actor_src);

			fclose(actor_src);
			fclose(actor_cpy);

			remove(actor_cpy_path);
		}
	}
	return 0;
}

int create_file(char *somepath, char* papicode){
	FILE *somefile;

	if((somefile = fopen(somepath,"w"))==NULL) return -1;
	else{
		insert_papicode(somefile, papicode);
		fclose(somefile);
		return 0;
	}
}

int backup_actors(char *actor_path){
	char *backup_path;
	backup_path = malloc(strlen(actor_path)+8);
	strcpy(backup_path,actor_path);
	strcat(backup_path,".backup");
	if( access( backup_path, F_OK ) != -1 ){
		if (DEBUG) printf("Previous backup found, not overwriting..\n"); //should ask if user wants to recover the backup before applying code
		return -1;
	}
	FILE *actor_src;
	FILE *actor_cpy;
	actor_src = fopen(actor_path,"r");
	actor_cpy = fopen(backup_path,"w");
	clone(actor_src, actor_cpy);
	fclose(actor_src);
	fclose(actor_cpy);
	return 0;
}

//will set pointer to the first line after the last keyword
int copy_until(FILE *actor_src, FILE* actor_cpy, char *keyword) {
	char word[150];
	char buf[1500];
	int out=0;
	int THREADED = 0;
	while (fgets(buf,1500, actor_src)!=NULL){
		fputs(buf,actor_cpy);
		sscanf(buf,"%s%*[^\n]",word);
		if(strstr(buf, "#define THREAD_ID")!=NULL) THREADED = 1;
		if(strcmp(word,keyword)==0)
			do {
				fgets(buf,1500, actor_src);
				if(strstr(buf, "#define THREAD_ID")!=NULL) THREADED = 1;
				fputs(buf,actor_cpy);
				sscanf(buf,"%s%*[^\n]",word);
				out = strcmp(word,keyword);
			} while (!out);
		if(out) break;
	}
	fseek(actor_src, 0-strlen(buf)-1, SEEK_CUR);
	fseek(actor_cpy, 0-strlen(buf)-1, SEEK_CUR);
	if (THREADED)
		return 1;
	else
		return 0;
}

int isblankline(char *line){
	if (strncmp(line,"\n",1)==0)
		return 1;
	else
		return 0;
}

int copy_until_bracket(FILE *actor_src, FILE* actor_cpy, char *keyline) {
	char buf[1500];
	/*do{
		fgets(buf,1500, actor_src);
		fputs(buf,actor_cpy);
	} while(strncmp(buf,keyline,strlen(keyline))!=0);*/

	do{
		fgets(buf,1500, actor_src);
		fputs(buf,actor_cpy);
	} while(strstr(buf, "{")==NULL);

/*	do {
		fgets(buf,1500, actor_src);
		fputs(buf,actor_cpy);
	} while(!isblankline(buf));*/



	/*fseek(actor_src, 0-strlen(buf), SEEK_CUR);
	fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);*/
	return 0;
}

int copy_immediately_after(FILE *actor_src, FILE* actor_cpy, char *keyline) {
	char buf[1500];
	int THREADED = 0;
	do{
		fgets(buf,1500, actor_src);
		fputs(buf,actor_cpy);
		if(strstr(buf, "#define THREAD_ID")!=NULL) THREADED = 1;
	} while(strncmp(buf,keyline,strlen(keyline))!=0);

	if(THREADED)	return 1;
	else	return 0;
}

void skiplines(FILE* file, char *ref){
	char buf[1500];
	do{
		fgets(buf,1500, file);
	}while(strncmp(buf,ref,strlen(ref))!=0);
}

void find_next_blank(FILE *actor_src, FILE* actor_cpy){
	char buf[1500];
	do{
		fgets(buf,1500, actor_src);
		fputs(buf,actor_cpy);
	}while(strncmp(buf,"\n",strlen("\n"))!=0);
}


int find_end_of_function(FILE *actor_src, FILE* actor_cpy, int *open_brackets){ //if "return" is found, returns immediately with value 1. Otherwise, returns 0 once it reaches the last closing bracket.
	char buf[1500];

	while (*open_brackets != 0) {
		fgets(buf,1500, actor_src);
		fputs(buf, actor_cpy);
		if(strstr(buf, "return")!=NULL) {
			fseek(actor_src, 0-strlen(buf), SEEK_CUR);
			fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);
			return 1;
		}
		if(strstr(buf, "{")!=NULL) (*open_brackets)++;
		if(strstr(buf, "}")!=NULL) (*open_brackets)--;
	}
	fseek(actor_src, 0-strlen(buf), SEEK_CUR);
	fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);

	return 0;
}

int skip_action(FILE *actor_src, FILE* actor_cpy, int *open_brackets){
	char buf[1500];

	if (*open_brackets != 0) {
		while (*open_brackets != 0) {
			fgets(buf,1500, actor_src);
			fputs(buf, actor_cpy);
			if(strstr(buf, "{")!=NULL) (*open_brackets)++;
			if(strstr(buf, "}")!=NULL) (*open_brackets)--;
		}
		fseek(actor_src, 0-strlen(buf), SEEK_CUR);
		fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);
	}
	return 0;
}


char* get_next_action(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, int *open_brackets, int *action_number){
	char buf[1500];
	int i;
	char *someName;
	char *someString;
	*action_number = -1;

	while((strncmp(buf,"// Token functions",strlen("// Token functions"))!=0)) {
		fgets(buf,1500, actor_src);
		fputs(buf, actor_cpy);

		if(strstr(buf, "{")!=NULL){
			(*open_brackets)++;
			if((someString = strtok(buf, "("))!=NULL){
				someName = strrchr(someString, ' ')+1;
				if (DEBUG) printf("\tFound action \"%s\".\n\tChecking if it needs PAPI code..", someName);
				for(i = 0;i<actor->actions_nb;i++){
					if(strcmp(someName,actor->action_names[i])==0) {
						if (DEBUG) printf(" yes, generating.\n");
						*action_number = i;
						return someName;
					}
				}
				if (DEBUG) printf(" no. Skipping to next action.\n");

				skip_action(actor_src, actor_cpy, &(*open_brackets));


			}
		}
	}
	if (DEBUG) printf("No more actions!\n");
	return NULL;
}

void papiwrite_init(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, n_events_s *events, int* THREADED){
	int k, i;

	if(DEBUG) printf("Generating Initializing code\n");

	copy_until_bracket(actor_src, actor_cpy, "// Initializes");//TODO get rid of 3rd argument here
	fputs("\t//PAPI INITIALIZING CODE\n", actor_cpy);
	fputs("\ti32 papi_local_THREAD_ID;\n", actor_cpy);
	fputs("\tmkdir(\"papi-output\", 0777);\n", actor_cpy);

	fprintf(actor_cpy, "\n\tPapi_actions_%s = malloc(sizeof(papi_action_s) * %d);\n",
			actor->actor_name, actor->actions_nb);


	fprintf(actor_cpy,"\tpapi_output_%s = fopen(\"papi-output/papi_output_%s.csv\",\"w\");\n",actor->actor_name, actor->actor_name);
	for(i = 0; i < actor->actions_nb; i++){
		fprintf(actor_cpy,"\tPapi_actions_%s[%d].action_id = malloc(strlen(\"%s\")+1);\n"
				"\tPapi_actions_%s[%d].action_id = \"%s\";\n",
				actor->actor_name,i,actor->action_names[i],actor->actor_name, i,actor->action_names[i]);
		//old//fprintf(actor_cpy,"\tpapi_output_%s_%s = fopen(\"papi-output/papi_output_%s-%s.csv\",\"w\");\n",actor->actor_name, actor->actions[i]->action_name,actor->actor_name, actor->actions[i]->action_name);
	}

	//only valid for same amount of events on every action!!!
	fprintf(actor_cpy,"\tfprintf(papi_output_%s,\"Actor; Action; ",actor->actor_name);
	for(k=0; k < events->events_nb; k++){
		fprintf(actor_cpy,"%s;", events->event_names[k]);
	}

	fseek(actor_cpy, -1, SEEK_CUR);
	fprintf(actor_cpy,"\\n\");\n");
	//

	fprintf(actor_cpy,"\tfclose(papi_output_%s);\n",actor->actor_name);

	for(i=0;i<actor->actions_nb;i++){
		fprintf(actor_cpy,"\tPapi_actions_%s[%d].eventCodeSetSize = %d;\n"
				"\tPapi_actions_%s[%d].eventCodeSet = malloc(sizeof(unsigned long)*Papi_actions_%s[%d].eventCodeSetSize);\n",
				actor->actor_name, i, events->events_nb, actor->actor_name, i, actor->actor_name, i);

		for(k=0; k < events->events_nb; k++){
			fprintf(actor_cpy,"\tPapi_actions_%s[%d].eventCodeSet[%d] = %s;\n",actor->actor_name,i,k,events->event_names[k]);
		}
		fprintf(actor_cpy,"\tPapi_actions_%s[%d].eventSet = malloc(sizeof(int) * Papi_actions_%s[%d].eventCodeSetSize);\n",actor->actor_name,i,actor->actor_name, i);
		fprintf(actor_cpy,"\tPapi_actions_%s[%d].eventSet = PAPI_NULL;\n",actor->actor_name,i);
		fprintf(actor_cpy,"\tPapi_actions_%s[%d].counterValues = malloc(sizeof(unsigned long) * Papi_actions_%s[%d].eventCodeSetSize);\n",actor->actor_name, i,actor->actor_name, i);

		//restore this?//fprintf(actor_cpy,"\tfprintf(papi_output_%s_%s,\"\\\"Thread\\\", \\\"Action\\\", ",actor->actor_name, actor->actions[i]->action_name);
		/*for(k=0; k < actor->actions[i]->events_nb; k++){
			fprintf(actor_cpy,"\\\"%s\\\",", actor->actions[i]->events[k]);
		}
		fseek(actor_cpy, -1, SEEK_CUR);
		fprintf(actor_cpy,"\\n\");\n");*/
	}

	fprintf(actor_cpy,"\n\tevent_init();\n");

	if (*THREADED)
		fprintf(actor_cpy,"\tpapi_local_THREAD_ID = THREAD_ID;\n");
	else
		fprintf(actor_cpy, "\tpapi_local_THREAD_ID = -1;\n");

	//initializes events
	for(i=0;i<actor->actions_nb;i++){
		fprintf(actor_cpy,"\tprintf(\"Creating eventlist for action %s in actor %s\\n\"); //PAPI DEBUG\n",actor->action_names[i], actor->actor_name); //PAPI DEBUG
		fprintf(actor_cpy,"\tevent_create_eventList(&(Papi_actions_%s[%d].eventSet), Papi_actions_%s[%d].eventCodeSetSize, Papi_actions_%s[%d].eventCodeSet, papi_local_THREAD_ID);\n",actor->actor_name,i,actor->actor_name,i,actor->actor_name,i);
	}

	fputs("\t//END OF PAPI CODE\n", actor_cpy);

}

void papiwrite_actions(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, n_events_s *events, int* THREADED){
	char buf[1500];
	int k, i;
	char *actionName;

	if(DEBUG) printf("Generating actions code\n");
	int open_brackets = 0;
	int action_number = 3;
	/*if (copy_immediately_after(actor_src, actor_cpy, "// Actions")==1) {
		*THREADED = 1;
		if(DEBUG) printf("Actor is threaded!\n");
	}*/
	for(i=0;i<actor->actions_nb;i++){
		if((actionName = get_next_action(actor_src, actor_cpy, actor, &open_brackets, &action_number))==NULL) {
			printf("WARNING: One or more actions were not found in the actor\n");
			break;
		}
		find_next_blank(actor_src, actor_cpy);
		if (*THREADED)
			fprintf(actor_cpy, "\tint papi_local_THREAD_ID;\n"
				"\tpapi_local_THREAD_ID = THREAD_ID;\n");
		else
			fprintf(actor_cpy, "\tint papi_local_THREAD_ID;\n"
				"\tpapi_local_THREAD_ID = -1;\n");
		fprintf(actor_cpy, "\tevent_start(&(Papi_actions_%s[%d].eventSet), papi_local_THREAD_ID); //PAPI\n", actor->actor_name, action_number);

		find_end_of_function(actor_src, actor_cpy, &open_brackets);
		fprintf(actor_cpy, "\t//PAPI\n");

		if (*THREADED)
			fprintf(actor_cpy, "\tpapi_local_THREAD_ID = THREAD_ID;\n");

		fprintf(actor_cpy, "\tevent_stop(&(Papi_actions_%s[%d].eventSet), "
				"Papi_actions_%s[%d].eventCodeSetSize, "
				"Papi_actions_%s[%d].counterValues, "
				"papi_local_THREAD_ID);\n",
				actor->actor_name, action_number, actor->actor_name, action_number, actor->actor_name, action_number);


		//screen output..
		/*fprintf(actor_cpy,"\tprintf(\"Thread = %%d\tAction = %%s\"\n");
		for(k=0;k<actor->actions[action_number]->events_nb;k++){
			fprintf(actor_cpy,"\t\t\"\\t%s = %%lu\"\n",actor->actions[action_number]->events[k]);
		}

		fseek(actor_cpy, -2, SEEK_CUR);
		fputs("\\n\"\n",actor_cpy);

		fseek(actor_cpy, -1, SEEK_CUR);
		fprintf(actor_cpy,",\n\t\tpapi_local_THREAD_ID, Papi_actions[%d].action_id,\n", action_number);

		for(k=0;k<actor->actions[action_number]->events_nb;k++){
			fprintf(actor_cpy,"\t\tPapi_actions[%d].counterValues[%d], \n",i,k);
		}*/


		//file (csv) output..
		fprintf(actor_cpy,"\tpapi_output_%s = fopen(\"papi-output/papi_output_%s.csv\",\"a+\");\n",actor->actor_name, actor->actor_name);
		fprintf(actor_cpy,"\tfprintf(papi_output_%s,\"\\\"%%s\\\";\\\"%%s\\\";",actor->actor_name);
		for(k=0;k<events->events_nb;k++){
			fprintf(actor_cpy,"\\\"%%lu\\\";");
		}

		fseek(actor_cpy, -1, SEEK_CUR);
		fputs("\\n\"\n",actor_cpy);

		fseek(actor_cpy, -1, SEEK_CUR);
		fprintf(actor_cpy,",\n\t\t\"%s\", Papi_actions_%s[%d].action_id,\n", actor->actor_name, actor->actor_name, action_number);


		for(k=0;k<events->events_nb;k++){
			fprintf(actor_cpy,"\t\tPapi_actions_%s[%d].counterValues[%d], \n",actor->actor_name,i,k);
		}

		fseek(actor_cpy, -3, SEEK_CUR);
		fprintf(actor_cpy,");\n\t//PAPI\n");

		fprintf(actor_cpy,"\tfclose(papi_output_%s);\n",actor->actor_name);

		if (open_brackets != 0) {
			while (open_brackets != 0) {
				fgets(buf,1500, actor_src);
				fputs(buf, actor_cpy);
				if(strstr(buf, "{")!=NULL) open_brackets++;
				if(strstr(buf, "}")!=NULL) open_brackets--;
			}
			fseek(actor_src, 0-strlen(buf), SEEK_CUR);
			fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);
		}
	}
}

void papiwrite(FILE *actor_src, FILE* actor_cpy, struct n_actor_s *actor, struct n_events_s *events){
	int i, k;
	char *actionName;
	int THREADED = 0;
	char buf[1500];

	//Writing includes
	if(DEBUG) printf("Generating includes and variables in %s\n", actor->actor_path);
	if (copy_until(actor_src, actor_cpy, "#include")==1){
		THREADED = 1;
		if(DEBUG) printf("Actor is threaded!\n");
	}
	fputs("#include \"eventLib.h\" //PAPI\n\n", actor_cpy);
	fprintf(actor_cpy,"FILE* papi_output_%s; //PAPI\n",actor->actor_name);

	fputs("\n", actor_cpy);
	fprintf(actor_cpy,"papi_action_s *Papi_actions_%s;//PAPI\n", actor->actor_name);

	int stat_actions = 1;
	int stat_init = 1;

	while(fgets(buf,1500, actor_src)!=NULL) {
			if(strstr(buf, "// Actions")!=NULL && stat_actions){
				fputs(buf, actor_cpy);
				papiwrite_actions(actor_src,actor_cpy, actor, events, &THREADED);
				stat_actions = 0;
			}
			else if(strstr(buf, "// Initializes")!=NULL && stat_init){
				fputs(buf, actor_cpy);
				papiwrite_init(actor_src,actor_cpy, actor, events, &THREADED);
				stat_init = 0;
			}
			else fputs(buf, actor_cpy);
	}


	//Writing in action

	//Writing initializes:

	//clone(actor_src, actor_cpy);

}



void insert_papicode(FILE* actor_cpy, char *ref){
	char buf[1500];
	FILE *papiref;
	if((papiref =fopen("libs/papicode","r"))==NULL){
		printf("Error: could not generate file.\n");
		return;
	}
	skiplines(papiref,ref);
	fgets(buf,1500, papiref);
		while (strncmp(buf,"//endofcode",11)!=0){
			fputs(buf, actor_cpy);
			fgets(buf,1500, papiref);
		}
	fputs("\n", actor_cpy);
	fclose(papiref);
}

void clone(FILE *actor_src, FILE* actor_cpy) {
	char buf[1500];
	while (fgets(buf,1500, actor_src)!=NULL){
		fputs(buf,actor_cpy);
	}
}
/*
void goto_previous_line(FILE *actor_src){
	do{
		fseek(actor_src, -2, SEEK_CUR);
	} while(fgetc(actor_src)!='\n');
}
*
void free_paths(struct actors_s *actors){
	int i;
	for(i=0;i < actors->num; i++)
		free(actors->actor_path[i]);
	free(actors->src_path);
	free(actors->project_path);
}
*/
void set_paths(struct n_project_s *project, char *path) {
	project->src_path=malloc(sizeof(char*)*25);//malloc(strlen(path)+strlen("/src/")+2);
	strcpy(project->src_path,path);
	strcat(project->src_path,"/src/");
	project->project_path=malloc(strlen(path)+1);
	strcpy(project->project_path,path);
}



void set_num(struct n_project_s *project, int num){
	project->actors_nb = num;
}

void set_actor_path(struct n_project_s *project, int num, char *name){
	int size = strlen(name)+strlen(project->src_path)+strlen(".c")+4;
	project->actors[num]->actor_path = malloc(size);
	project->actors[num]->actor_name = malloc(strlen(name)+1);
	strcpy(project->actors[num]->actor_name, name);
	strcpy(project->actors[num]->actor_path, project->src_path);
	strcat(project->actors[num]->actor_path, name);
	strcat(project->actors[num]->actor_path, ".c");
	if( access( project->actors[num]->actor_path, F_OK ) == -1 )
		printf("WARNING: File %s.c not found!\n", name);
}

void set_papify_actor(struct n_project_s *project, int n, int opt){
	project->actors[n]->papify = opt;
}

const int isxcf(const char *filename) {
    //char *e = tolower(strrchr (filename, '.'));
	char *e = strrchr (filename, '.');
	if (e == NULL) return 0; //si el fichero no tiene extensión..
	else if (strcmp(e,".xcf") == 0)
    	return 1;
    else
    	return 0;
}

char *findMappingFile(char *path) {

    struct dirent *pDirent;
    DIR *pDir;
    int success = 0;
    char * path_to_src = malloc(strlen(path)+strlen("/src"));
    strcpy(path_to_src, path);
    strcat(path_to_src,"/src/");
    pDir = opendir (path_to_src);
    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", path_to_src);
        return NULL;
    }
    if(DEBUG) {
    	printf ("Looking for mapping file");
    	fflush(stdout);
    }

    while ((pDirent = readdir(pDir)) != NULL) {
    	//usleep(30000);
    	if(DEBUG) printf (".");
    	if(DEBUG) fflush(stdout);
        if(isxcf(pDirent->d_name)) {
        	success = 1;
        	if(DEBUG) printf (" found! [%s]\n", pDirent->d_name);
        	break;
        }
    }
    closedir (pDir);
    //TODO TEMP FIX: +500 SOLVE THIS!
    char *result = malloc(strlen(pDirent->d_name)+strlen(path_to_src)+500); //+1 for the zero-terminator CHECK ERRORS MALLOC
    strcpy(result, path_to_src);
    free(path_to_src);
    strcat(result, "/");
    strcat(result, pDirent->d_name);
    if (success) return result;
    else return NULL;
}

int get_actors_num(char *mapping_file_path){
	int j, k, size_of_partitioning, size_of_partition;
	int num = 0;
	node_t *configuration, *partitioning, *partition;
	if (DEBUG) printf("Attemping to read %s\n", mapping_file_path);
	configuration = roxml_load_doc(mapping_file_path); //a: LIBROXML node http://www.libroxml.net/public-api.html
	if (configuration == NULL) {
		printf("I/O error when reading mapping file.\n");
		exit(1);
	}

	partitioning = roxml_get_chld(configuration, NULL, 0); //a: This function returns a given chld of a node etheir by name, or the nth child. (node, child name, number of the child to get)
	size_of_partitioning = roxml_get_chld_nb(partitioning); //a: This function return the number of chlidren for a given node
	for (j = 0; j < size_of_partitioning; j++) {
		partition = roxml_get_chld(partitioning, NULL, j);
		size_of_partition = roxml_get_chld_nb(partition);
		for (k = 0; k < size_of_partition; k++) {
			num++;
		}
	}

	return num;
}

int find_actor(char* name,  struct n_project_s *project) {
	int i;
	for (i=0;i<project->actors_nb;i++){
		if(strcmp(project->actors[i]->actor_name, name)==0)
			return i;
	}
	return -1;
}

int identify_actors(char *mapping_file_path, struct n_project_s *project) {
	int i, j, k, n,size_of_config, size_of_partitioning, size_of_partition, size_of_papi, len, actor_num, number_of_actions, number_of_events;
	int num = 0;
	node_t *configuration, *partitioning, *partition, *instance, *papi, *actor_instance, *action, *events, *event_instance, *actors, *action_instance;

	//*actors->actor_path = malloc((actors->num) * sizeof (char *)+1);


	configuration = roxml_load_doc(mapping_file_path); //a: LIBROXML node http://www.libroxml.net/public-api.html
	if (configuration == NULL) {
		printf("I/O error when reading mapping file.\n");
		exit(1);
	}
	size_of_config = roxml_get_chld_nb(configuration); //a: This function return the number of children for a given node

	if(DEBUG)printf("Found %d configuration\n", size_of_config);


	partitioning = roxml_get_chld(configuration, NULL, 0); //a: This function returns a given child of a node either by name, or the nth child. (node, child name, number of the child to get)
	size_of_partitioning = roxml_get_chld_nb(partitioning); //a: This function return the number of children for a given node
	if(DEBUG)printf("	Found %d partitionings\n", size_of_partitioning);
	for (j = 0; j < size_of_partitioning; j++) {
		partition = roxml_get_chld(partitioning, NULL, j);
		if(DEBUG)printf("		Found partition, id = '%s'\n", roxml_get_content(roxml_get_attr(partition, "id", 0), NULL, 0, &len));
		size_of_partition = roxml_get_chld_nb(partition);
		for (k = 0; k < size_of_partition; k++) {
			instance = roxml_get_chld(partition, NULL, k);
			if(DEBUG)printf("			Found instance, id = '%s'\n", roxml_get_content(roxml_get_attr(instance, "id", 0), NULL, 0, &len));
			project->actors[num] = malloc(sizeof(n_actor_s));//!!!
			set_actor_path(project, num++, roxml_get_content(roxml_get_attr(instance, "id", 0), NULL, 0, &len));
		}
	}

	if(DEBUG) printf("\tReading Papify config\n");
	papi = roxml_get_chld(configuration, NULL, 1); //a: This function returns a given child of a node either by name, or the nth child. (node, child name, number of the child to get)
	size_of_papi = roxml_get_chld_nb(papi); //a: This function return the number of children for a given node

	int mode_papify_all;

	switch(size_of_papi){
	case 1:
			mode_papify_all = 2;
			break;
	case 2:
			mode_papify_all = 0;
			break;
	case -1:
			printf("Papify config not present.");
			exit(0);
	}


	for (i=0;i<project->actors_nb;i++){
		set_papify_actor(project, i, mode_papify_all);
	}


	//Reading events
	events = roxml_get_chld(papi, NULL, 0);
	int events_nb = roxml_get_chld_nb(events);
	project->events=malloc(sizeof(n_events_s)+sizeof(char*)*events_nb);
	project->events->events_nb = events_nb;

	if(DEBUG) printf("\t\tFound %d PAPI events:\n", project->events->events_nb);
	for (i = 0; i < events_nb; i++) {
		event_instance = roxml_get_chld(events, NULL, i);
		project->events->event_names[i] = malloc(sizeof(strlen(roxml_get_content(roxml_get_attr(event_instance, "id", 0), NULL, 0, &len)))+1);
		strcpy(project->events->event_names[i],roxml_get_content(roxml_get_attr(event_instance, "id", 0), NULL, 0, &len));
		if(DEBUG) printf("\t\t\tEvent id = '%s'\n", roxml_get_content(roxml_get_attr(event_instance, "id", 0), NULL, 0, &len));
	}

	if(mode_papify_all) return 1; //no need to read further

	//Reading actors
	actors = roxml_get_chld(papi, NULL, 1);
	int actors_nb = roxml_get_chld_nb(actors);
	int actions_nb;
	if(DEBUG) printf("\t\tFound %d actors to be modified:\n", actors_nb);

	for (i = 0; i < actors_nb; i++) {
		actor_instance = roxml_get_chld(actors, NULL, i);
		actor_num = find_actor(roxml_get_content(roxml_get_attr(actor_instance, "id", 0), NULL, 0, &len), project);
		if (actor_num != -1) {
			set_papify_actor(project, actor_num, 1);
			if(DEBUG) printf("\t\t\tActor id = '%s'\n", roxml_get_content(roxml_get_attr(actor_instance, "id", 0), NULL, 0, &len));
		}
		else{
			printf("ERROR: No such actor '%s'\n",roxml_get_content(roxml_get_attr(actor_instance, "id", 0), NULL, 0, &len));
			continue;
		}
		//Reading actions
		actions_nb = roxml_get_chld_nb(actor_instance);
		project->actors[actor_num]->actions_nb = actions_nb;
		if(actions_nb !=0){
			if(DEBUG) printf("\t\t\tFound %d actions:\n", actions_nb);
		}
		else
			if(DEBUG) printf("\t\t\tFor this actor, all actions will be modified\n", actions_nb);

		project->actors[actor_num]=realloc(project->actors[actor_num], sizeof(n_actor_s)+sizeof(char*)*actions_nb);
		if(actions_nb==0) set_papify_actor(project, actor_num, 2);
		for (j = 0; j < actions_nb; j++) {
			action_instance = roxml_get_chld(actor_instance, NULL, j);
			project->actors[actor_num]->action_names[j] = malloc(sizeof(strlen(roxml_get_content(roxml_get_attr(action_instance, "id", 0), NULL, 0, &len)))+1);
			strcpy(project->actors[actor_num]->action_names[j],roxml_get_content(roxml_get_attr(action_instance, "id", 0), NULL, 0, &len));
			if(DEBUG) printf("\t\t\t\t'%s'\n", project->actors[actor_num]->action_names[j]);
		}
	}

	// release the last allocated buffer even if no pointer is maintained by the user
	roxml_release(RELEASE_LAST);
	// here no memory leak can occur.
	roxml_close(configuration);

	return 0;
}


void n_structures_test(struct n_project_s* project){
	int j,k,n;
	char papi[4];
	int max_width, value_to_print;
	max_width = 8;
	value_to_print = 1000;

	printf("/////////////////////////////////////////////\nPrinting all content in n_project_s structure:\n\n");
	printf("Project path: %s\n", project->project_path);
	printf("Project src path: %s\n", project->src_path);
	printf("Number of actors: %d\n", project->actors_nb);
	printf("Number of events: %d\n", project->events->events_nb);
	printf("\n");

	printf("-Events:\n");
	for(j=0; j<project->events->events_nb;j++){
		printf("\t%s\n", project->events->event_names[j]);
	}
	printf("\n");

	for(j=0; j<project->actors_nb;j++){
		printf("-Actor:\tActor name: %s\n\tActor path: %s\n", project->actors[j]->actor_name, project->actors[j]->actor_path);
		if(project->actors[j]->papify==0) strcpy(papi,"no");
		else strcpy(papi,"yes");
		printf("\tPapify mode: %d\n",project->actors[j]->papify);
		printf("\tPAPI code will be included: %s\n",papi);
		if(project->actors[j]->papify==2) printf("\t-Actions: ALL\n");
		else if(project->actors[j]->papify==1) {
			printf("\tNumber of actions: %d\n", project->actors[j]->actions_nb);
			printf("\t-Actions:\n");
			for(k=0; k<project->actors[j]->actions_nb;k++){
				printf("\t\t%s\n", project->actors[j]->action_names[k]);
			}
		}
		printf("\n");
	}
	printf("/////////////////////////////////////////////\n");
}
