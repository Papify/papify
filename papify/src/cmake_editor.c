/*
 * cmake_editor.c
 *
 *  Created on: Jul 10, 2014
 *      Author: aarias
 */
#include "papify.h"

int editCMakelists(char *somepath){
	FILE* cmake_file;
	FILE* cmake_copy;
	char *cmake_copy_path = malloc (strlen(somepath)+strlen(".papi.txt")+1);
	char buf[1500];

	make_backup(somepath);

	cmake_file = fopen(somepath,"r");

	strcpy(cmake_copy_path, somepath);
	strcat(cmake_copy_path, ".papi.txt");
	cmake_copy = fopen(cmake_copy_path,"w"); //this is a temporal file that contains the actor code with the papi code added, will later replace the actual actor

	int stat_src = 1;
	int stat_hdr = 1;
	int stat_dir = 1;
	int stat_lnk = 1;
	int stat_beg = 1;
	int do_not_touch = 0;

	while(fgets(buf,1500, cmake_file)!=NULL) {
		if(strstr(buf, "# CMakeLists modified by Papify")!=NULL && stat_src){
			if (DEBUG) printf("Seems like libs/orcc-native/CMakeLists.txt already has Papify included, not touching it..\n");
			do_not_touch = 1;
			break;
		}
		if(stat_beg){
			cmake_add_beginning(cmake_copy);
			fputs(buf, cmake_copy);
			stat_beg = 0;
		}
		if(strstr(buf, "set(orcc_native_sources")!=NULL && stat_src){
			cmake_addat_closingparenthesis(cmake_file,cmake_copy,"src/eventLib.c");
			stat_src = 0;
		}
		else if(strstr(buf, "set(orcc_native_headers")!=NULL && stat_hdr){
			cmake_addat_closingparenthesis(cmake_file,cmake_copy,"include/eventLib.h");
			stat_hdr = 0;
		}
		else if(strstr(buf, "include_directories(")!=NULL && stat_dir){
			cmake_addat_closingparenthesis(cmake_file,cmake_copy,"${PAPI_INCLUDE_DIR}");
			stat_dir = 0;
		}
		else if(strstr(buf, "target_link_libraries(")!=NULL && stat_lnk){
			cmake_addat_closingparenthesis(cmake_file,cmake_copy,"${papi_LIBRARY}");
			stat_lnk = 0;
		}
		else fputs(buf, cmake_copy);
	}

	if(do_not_touch){
		fclose(cmake_file);
		fclose(cmake_copy);

		remove(cmake_copy_path);
	}
	else {
	fclose(cmake_file);
	cmake_file =fopen(somepath,"w");
	fclose(cmake_copy);
	cmake_copy =fopen(cmake_copy_path,"r");

	clone(cmake_copy,cmake_file);

	fclose(cmake_file);
	fclose(cmake_copy);

	remove(cmake_copy_path);
	}

	return 0;
}

void cmake_add_beginning(FILE* cmake_copy){
	fprintf(cmake_copy,
			"# CMakeLists modified by Papify\n"
			"########################################\n"
			"# find papi library\n"
			"find_library(papi_LIBRARY papi)\n\n"
			"# set papi include directory\n"
			"set(PAPI_INCLUDE_DIR /usr/local/include)\n\n"
			"########################################\n");
}

int cmake_addat_closingparenthesis(FILE *cmake_file, FILE* cmake_copy, char* string){ //if "return" is found, returns immediately with value 1. Otherwise, returns 0 once it reaches the last closing bracket.
	char buf[1500];
	int open_brackets = 0;

	do{//go to rpevious line..
		fseek(cmake_file, -2, SEEK_CUR);
	} while(fgetc(cmake_file)!='\n');

	do {
		fgets(buf,1500, cmake_file);
		fputs(buf, cmake_copy);
		if(strstr(buf, "(")!=NULL) open_brackets++;
		if(strstr(buf, ")")!=NULL) open_brackets--;
	}while  (open_brackets != 0);

	fseek(cmake_copy, 0-2, SEEK_CUR);
	fprintf(cmake_copy," %s) # -papify\n",string);

	return 0;
}

int make_backup(char *path){
	char *backup_path;
	backup_path = malloc(strlen(path)+8);
	strcpy(backup_path,path);
	strcat(backup_path,".backup");
	if( access( backup_path, F_OK ) != -1 ){
		if (DEBUG) printf("Previous backup found, not overwriting..\n"); //should ask if user wants to recover the backup before applying code
		return -1;
	}
	FILE *cmake_file;
	FILE *cmake_copy;
	cmake_file = fopen(path,"r");
	cmake_copy = fopen(backup_path,"w");
	clone(cmake_file, cmake_copy);
	fclose(cmake_file);
	fclose(cmake_copy);
	return 0;
}

