/*
 * papify.c
 * hello gitHub!! :)
 *  Created on: Apr 9, 2014
 *      Author: aarias
 */


/*
 * PAPIFY will add selected PAPI code to your RVC-CAL generated program in C
 */

#include "papify.h"
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
	project_s *project = malloc(sizeof(project_s)+sizeof(actor_s*) * actors_num);

	set_paths(project, project_path);
	set_num(project, actors_num);

	identify_actors(mapping_file_path, project);

	//structures_test(project);

	papify(project);

	//Copying new CMakeLists.txt
	char *somepath = malloc(strlen(project->project_path)+strlen("/libs/CMakeLists.txt")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/CMakeLists.txt");
	if(create_file(somepath, l_cmakelists)!=0) printf("Could not create CMakeLists.txt.. :(\n", somepath);
	free(somepath);

	//Copying eventLib
	somepath = malloc(strlen(project->project_path)+strlen("/libs/orcc/src/eventLib.c")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/orcc/src/eventLib.c");
	if(create_file(somepath, l_eventc)!=0) printf("Could not create %s.. maybe the folder doesn't exist?\n", somepath);
	free(somepath);

	somepath = malloc(strlen(project->project_path)+strlen("/libs/orcc/include/eventLib.h")+2);
	strcpy(somepath,project->project_path);
	strcat(somepath,"/libs/orcc/include/eventLib.h");
	if(create_file(somepath, l_eventh)!=0) printf("Could not create %s.. maybe the folder doesn't exist?\n", somepath);
	free(somepath);


	printf("Papi code successfully added, to recover your original files run blah blah.\nThis program asumes PAPI is already properly installed in the system\n");
	//free_paths(actors);
	free(project);
	return 0;
}
