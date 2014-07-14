/*
 * papify_everything.c
 *
 *  Created on: Jul 14, 2014
 *      Author: aarias
 */
#include "papify.h"

int papify_everything(struct project_s *project){
	FILE *actor_src;
	FILE *actor_cpy;
	int i;
	char actor_cpy_path[500];
	char c;

	/*for(i = 0; i<project->actors_nb;i++){

	}*/

	structures_test(project);

	for(i = 0; i<project->actors_nb;i++){
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


		papiwrite_everything(actor_src, actor_cpy, project->actors[i], project->papify_all);


		fclose(actor_src);
		actor_src =fopen(project->actors[i]->actor_path,"w");
		fclose(actor_cpy);
		actor_cpy =fopen(actor_cpy_path,"r");

		clone(actor_cpy,actor_src);

		fclose(actor_src);
		fclose(actor_cpy);

		remove(actor_cpy_path);
	}
	return 0;
}

void papiwrite_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, struct action_s *action){ //action contains the events for every action in every actor
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
	fprintf(actor_cpy,"FILE* papi_output_%s;\n",actor->actor_name);

	fputs("\n", actor_cpy);
	fprintf(actor_cpy,"papi_action_s *Papi_actions_%s;//PAPI\n", actor->actor_name);

	int stat_actions = 1;
	int stat_init = 1;

	while(fgets(buf,1500, actor_src)!=NULL) {
			if(strstr(buf, "// Actions")!=NULL && stat_actions){
				fputs(buf, actor_cpy);
				papiwrite_actions_everything(actor_src,actor_cpy, actor, action, &THREADED);
				stat_actions = 0;
			}
			else if(strstr(buf, "// Initializes")!=NULL && stat_init){
				fputs(buf, actor_cpy);
				papiwrite_init_everything(actor_src,actor_cpy, actor, action, &THREADED);
				stat_init = 0;
			}
			else fputs(buf, actor_cpy);
	}

}

void papiwrite_init_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, struct action_s *action, int* THREADED){
	int k, i;

	if(DEBUG) printf("Generating Initializing code\n");

	copy_until_bracket(actor_src, actor_cpy, "// Initializes");//TODO get rid of 3rd argument here
	fputs("\t//PAPI INITIALIZING CODE\n", actor_cpy);
	fputs("\ti32 papi_local_THREAD_ID;\n", actor_cpy);
	fputs("\tmkdir(\"papi-output\", 0777);\n", actor_cpy);

	fprintf(actor_cpy, "\n\tPapi_actions_%s = malloc(sizeof(papi_action_s));\n",
			actor->actor_name);


	fprintf(actor_cpy,"\tpapi_output_%s = fopen(\"papi-output/papi_output_%s.csv\",\"w\");\n",actor->actor_name, actor->actor_name);

	//only valid for same amount of events on every action!!!
	fprintf(actor_cpy,"\tfprintf(papi_output_%s,\"Actor; Action; ",actor->actor_name);
	for(k=0; k < action->events_nb; k++){
		fprintf(actor_cpy,"%s;", action->events[k]);
	}

	fprintf(actor_cpy,"\\n\");\n");

	//

	fprintf(actor_cpy,"\tfclose(papi_output_%s);\n",actor->actor_name);
//
	fprintf(actor_cpy,"\tPapi_actions_%s->eventCodeSetSize = %d;\n",actor->actor_name, action->events_nb);
	fprintf(actor_cpy,"\tPapi_actions_%s->eventCodeSet = malloc(sizeof(unsigned long)*Papi_actions_%s->eventCodeSetSize);\n",
			actor->actor_name, actor->actor_name);

	for(k=0; k < action->events_nb; k++){
		fprintf(actor_cpy,"\tPapi_actions_%s->eventCodeSet[%d] = %s;\n",actor->actor_name,k,action->events[k]);
	}
	fprintf(actor_cpy,"\tPapi_actions_%s->eventSet = malloc(sizeof(int) * Papi_actions_%s->eventCodeSetSize);\n",actor->actor_name,actor->actor_name);
	fprintf(actor_cpy,"\tPapi_actions_%s->eventSet = PAPI_NULL;\n",actor->actor_name);
	fprintf(actor_cpy,"\tPapi_actions_%s->counterValues = malloc(sizeof(unsigned long) * Papi_actions_%s->eventCodeSetSize);\n",actor->actor_name, actor->actor_name);
//

	fprintf(actor_cpy,"\n\tevent_init();\n");

	if (*THREADED)
		fprintf(actor_cpy,"\tpapi_local_THREAD_ID = THREAD_ID;\n");
	else
		fprintf(actor_cpy, "\tpapi_local_THREAD_ID = -1;\n");

	//initializes events
	fprintf(actor_cpy,"\tevent_create_eventList(&(Papi_actions_%s->eventSet), Papi_actions_%s->eventCodeSetSize, Papi_actions_%s->eventCodeSet, papi_local_THREAD_ID);\n",actor->actor_name,actor->actor_name,actor->actor_name);


	fputs("\t//END OF PAPI CODE\n", actor_cpy);

}

void copy_upto_end_of_function(FILE *actor_src, FILE* actor_cpy, int *open_brackets){
	char buf[1500];
	while (*open_brackets != 0) {
		fgets(buf,1500, actor_src);
		fputs(buf, actor_cpy);
		if(strstr(buf, "{")!=NULL) (*open_brackets)++;
		if(strstr(buf, "}")!=NULL) (*open_brackets)--;
	}
	fseek(actor_src, 0-strlen(buf), SEEK_CUR);
	fseek(actor_cpy, 0-strlen(buf), SEEK_CUR);
}

char* get_next_action_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, int *open_brackets, int *action_number){
	char buf[1500];
	char *someName;
	char *result;
	char *someString;
	*action_number = -1;

	while((strncmp(buf,"////////////////////////////////////////////////////////////////////////////////",strlen("////////////////////////////////////////////////////////////////////////////////"))!=0)) {
		fgets(buf,1500, actor_src);
		fputs(buf, actor_cpy);

		if(strstr(buf, "{")!=NULL){
			(*open_brackets)++;
			if((someString = strtok(buf, "("))!=NULL){
				someName = strrchr(someString, ' ')+1;
				result = malloc(sizeof(char)*strlen(someName)+1);
				strcpy(result, someName);
				if (strstr(result, "isSchedulable_")==NULL) {
					if (DEBUG) printf("\tFound action \"%s\".\n", result);
					return result;
				}
				else copy_upto_end_of_function(actor_src, actor_cpy, &(*open_brackets));
			}
		}
	}
	if (DEBUG) printf("No more actions!\n");
	return NULL;
}

void papiwrite_actions_everything(FILE *actor_src, FILE* actor_cpy, struct actor_s *actor, struct action_s *action, int* THREADED){
	char buf[1500];
	int k, i;
	char *actionName;

	if(DEBUG) printf("Generating actions code\n");
	int open_brackets = 0;
	int action_number = 3;

	while((actionName = get_next_action_everything(actor_src, actor_cpy, actor, &open_brackets, &action_number)) != NULL) {

		find_next_blank(actor_src, actor_cpy);
		if (*THREADED)
			fprintf(actor_cpy, "\tint papi_local_THREAD_ID;\n"
				"\tpapi_local_THREAD_ID = THREAD_ID;\n");
		else
			fprintf(actor_cpy, "\tint papi_local_THREAD_ID;\n"
				"\tpapi_local_THREAD_ID = -1;\n");
		fprintf(actor_cpy, "\tevent_start(&(Papi_actions_%s->eventSet), papi_local_THREAD_ID); //PAPI\n", actor->actor_name);

		find_end_of_function(actor_src, actor_cpy, &open_brackets);
		fprintf(actor_cpy, "\t//PAPI\n");

		if (*THREADED)
			fprintf(actor_cpy, "\tpapi_local_THREAD_ID = THREAD_ID;\n");

		fprintf(actor_cpy, "\tevent_stop(&(Papi_actions_%s->eventSet), "
				"Papi_actions_%s->eventCodeSetSize, "
				"Papi_actions_%s->counterValues, "
				"papi_local_THREAD_ID);\n",
				actor->actor_name, actor->actor_name, actor->actor_name);



		//file (csv) output..
		fprintf(actor_cpy,"\tpapi_output_%s = fopen(\"papi-output/papi_output_%s.csv\",\"a+\");\n",actor->actor_name, actor->actor_name);
		fprintf(actor_cpy,"\tfprintf(papi_output_%s,\"\\\"%%s\\\";\\\"%%s\\\";",actor->actor_name);

		for(k=0;k<action->events_nb;k++){
			fprintf(actor_cpy,"\\\"%%lu\\\";");
		}

		fseek(actor_cpy, -1, SEEK_CUR);
		fputs("\\n\"\n",actor_cpy);

		fseek(actor_cpy, -1, SEEK_CUR);
		fprintf(actor_cpy,",\n\t\t\"%s\", \"%s\",\n", actor->actor_name, actionName);


		for(k=0;k<action->events_nb;k++){
			fprintf(actor_cpy,"\t\tPapi_actions_%s->counterValues[%d], \n",actor->actor_name,k);
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

