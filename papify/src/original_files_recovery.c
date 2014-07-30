/*
 * original_files_recovery.c
 *
 *  Created on: Jul 30, 2014
 *      Author: aarias
 */


#include "papify.h"

const int isbackup(const char *filename) {
    //char *e = tolower(strrchr (filename, '.'));
	char *e = strrchr (filename, '.');
	if (e == NULL) return 0; //si el fichero no tiene extensión..
	else if (strcmp(e,".papibackup") == 0)
    	return 1;
    else
    	return 0;
}

//TODO recover cmakelists and delete eventLib.h and .c
void original_file_recovery(char* project_path){
	if (project_path == NULL) {
		printf("You must specify the path to the C generated code folder. e.g.: papify -p /some/path/\n");
		exit(0);
	}


	struct dirent *pDirent;
	DIR *pDir;
	char* orig_file_name, *orig_file_path, *backup_file_path;

	char * path_to_src = malloc(strlen(project_path)+strlen("/src"));
	strcpy(path_to_src, project_path);
	strcat(path_to_src,"/src/");
	pDir = opendir (path_to_src);
	if (pDir == NULL) {
		printf ("Cannot open directory '%s'\n", path_to_src);
		exit(0);
	}

	while ((pDirent = readdir(pDir)) != NULL) {
		if(isbackup(pDirent->d_name)) {
			orig_file_path = (char*) malloc(strlen(path_to_src)+strlen(pDirent->d_name)+5);
			backup_file_path = (char*) malloc(strlen(path_to_src)+strlen(pDirent->d_name)+5);

			strcpy(backup_file_path, project_path);
			strcat(backup_file_path, "/src/");
			strcat(backup_file_path, pDirent->d_name);

			strcpy(orig_file_path, project_path);
			orig_file_name = strtok (pDirent->d_name, ".");
			strcat(orig_file_path, "/src/");
			strcat(orig_file_path, orig_file_name);
			strcat(orig_file_path, ".c");

			if( access(orig_file_path, F_OK ) != -1 ) {
				printf("Recovering %s.c\n", orig_file_name);
				remove(orig_file_path);
				rename(backup_file_path, orig_file_path);
			}

			free(orig_file_path);
			free(backup_file_path);
		}
	}
	closedir (pDir);

}
