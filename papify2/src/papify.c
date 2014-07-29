/*
 * papify.c
 *
 *  Created on: Apr 9, 2014
 *      Author: aarias
 */


/*
 * PAPIFY will add selected PAPI code to your RVC-CAL generated program in C
 */

#include "papify.h"
#include "eventLib_h.xxd"
#include "eventLib_c.xxd"
int DEBUG = 0;

void printhelp(void) {
	printf("here goes the help\n");
}

int main (int argc, char **argv) {
	int xcf_flag = 0;
	char *project_path = NULL;
	int index;
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "vxp:")) != -1) {
		switch (c)
		{
			case 'v':
				DEBUG = 1;
				//verbose (debug) mode
				break;
			case 'x':
				xcf_flag = 1;
				break;
			case 'p':
				project_path = optarg;
				break;
			case '?':
				if (isprint (optopt)) {
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
					printhelp();
				}
				else {
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
					printhelp();
				}
				return 1;
			default:
				abort();
		}

	}

	if(xcf_flag) xcf_creator(argc, project_path, argv[0]);
	else papify_main(argc, project_path, argv[0]);

	return 0;
}




int papify_main(int argc, char *project_path, char *exec)
{
	if (project_path == NULL) {
		printf("You must specify the path to the C generated code folder. e.g.: %s -p /some/path/\n", exec);
		return 0;
	}
	char *mapping_file_path;

	if ((mapping_file_path = findMappingFile(project_path)) == NULL) {
		printf("Unable to locate a mapping file, make sure you are using the root path to the RVC CAL generated code with the C backend\n");
		exit(1);
	}

	int actors_num = get_actors_num(mapping_file_path);
	n_project_s *project = malloc(sizeof(n_project_s)+sizeof(n_actor_s*) * actors_num);

	set_paths(project, project_path);
	set_num(project, actors_num);

	if(identify_actors(mapping_file_path, project) == 1) papify_everything(project);
	else papify(project);


	//Generating CMakesLists.txt
	char *somepath = malloc(strlen(project->project_path)+strlen("/libs/orcc-native/CMakeLists.txt")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/orcc-native/CMakeLists.txt");
	if(editCMakelists(somepath)) perror("Could not create CMakeLists.txt.. :(\n");
	free(somepath);

	//Copying eventLib
	somepath = malloc(strlen(project->project_path)+strlen("/libs/orcc-native/src/eventLib.c")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/orcc-native/src/eventLib.c");
	if(create_file(somepath, eventLib_c)!=0) perror("Could not create eventLib.c.. maybe the folder doesn't exist?\n");
	free(somepath);

	somepath = malloc(strlen(project->project_path)+strlen("/libs/orcc-native/include/eventLib.h")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/orcc-native/include/eventLib.h");
	if(create_file(somepath, eventLib_h)!=0) perror("Could not create eventLib.h.. maybe the folder doesn't exist?\n");
	free(somepath);


	printf("Papi code successfully added, to recover your original files run blah blah.\nThis program asumes PAPI is already properly installed in the system\n");
	//free_paths(actors);
	free(project);
	return 0;
}
