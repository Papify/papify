#include "papify.h"


void read_upto_end_of_function(FILE *actor_src, int *open_brackets){
	char buf[1500];
	while (*open_brackets != 0) {
		fgets(buf,1500, actor_src);
		if(strstr(buf, "{")!=NULL) (*open_brackets)++;
		if(strstr(buf, "}")!=NULL) (*open_brackets)--;
	}
	fseek(actor_src, 0-strlen(buf), SEEK_CUR);
}

int read_immediately_after(FILE *actor_src, char *keyline) {
	char buf[1500];
	int THREADED = 0;
	do{
		fgets(buf,1500, actor_src);
		if(strstr(buf, "#define THREAD_ID")!=NULL) THREADED = 1;
	} while(strncmp(buf,keyline,strlen(keyline))!=0);

	return 0;
}


char* read_next_action(FILE *actor_src, int *open_brackets){
	char buf[1500];
	char *someName;
	char *someString;

	while((strncmp(buf,"////////////////////////////////////////////////////////////////////////////////",strlen("////////////////////////////////////////////////////////////////////////////////"))!=0)) {
		fgets(buf,1500, actor_src);

		if(strstr(buf, "{")!=NULL){
			(*open_brackets)++;
			if((someString = strtok(buf, "("))!=NULL){
				someName = strrchr(someString, ' ')+1;
				if (DEBUG) printf("\t\t\t\tFound action \"%s\".\n", someName);
				if (strstr(buf, "isSchedulable_")==NULL) return someName;
				else read_upto_end_of_function(actor_src, &(*open_brackets));
				}
			}
		}
	if (DEBUG) printf("\t\t\t\tNo more actions!\n");
	return NULL;
}

void put_actions(FILE* actor_src, node_t* someInstanceNode, node_t* someActionNode, node_t* someEventNode) {
	char *actionName;
	int open_brackets = 0;

	read_immediately_after(actor_src, "// Actions");

	while((actionName = read_next_action(actor_src, &open_brackets)) != NULL) {
		someActionNode = roxml_add_node(someInstanceNode, 0, ROXML_ELM_NODE, "Action", NULL);
		roxml_add_node(someActionNode, 0, ROXML_ATTR_NODE, "id", actionName);
		read_upto_end_of_function(actor_src, &open_brackets);
		someEventNode = roxml_add_node(someActionNode, 0, ROXML_ELM_NODE, "event", NULL);
		roxml_add_node(someEventNode, 0, ROXML_ATTR_NODE, "id", "PAPI_TOT_INS");
		someEventNode = roxml_add_node(someActionNode, 0, ROXML_ELM_NODE, "event", NULL);
		roxml_add_node(someEventNode, 0, ROXML_ATTR_NODE, "id", "PAPI_L1_DCM");
	}
}

int xcf_creator(int argc, char *project_path) {
	FILE* actor_src;
	int j, k, n,size_of_config, size_of_partitioning, size_of_partition, size_of_papi, len, actor_num, number_of_actions, number_of_events;
	int num = 0;
	node_t *configuration, *partitioning, *partition, *instance, *papi, *actor_instance, *action, *event;
	char *actor_path;

	char *mapping_file_path;

	if ((mapping_file_path = findMappingFile(project_path)) == NULL) {
		printf("Unable to locate a mapping file, make sure you are using the root path to the RVC CAL generated code with the C backend\n");
		exit(1);
	}

	int actors_num = get_actors_num(mapping_file_path);

	configuration = roxml_load_doc(mapping_file_path); //a: LIBROXML node http://www.libroxml.net/public-api.html
	if (configuration == NULL) {
		printf("I/O error when reading mapping file.\n");
		exit(1);
	}
	node_t *papi_root_node = roxml_add_node(configuration, 0, ROXML_ELM_NODE, "Papi", NULL);
	node_t *papi_instance_node, *papi_action_node, *papi_event_node;

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
			papi_instance_node = roxml_add_node(papi_root_node, 0, ROXML_ELM_NODE, "Instance", NULL);
			roxml_add_node(papi_instance_node, 0, ROXML_ATTR_NODE, "id", roxml_get_content(roxml_get_attr(instance, "id", 0), NULL, 0, &len));

			//this goes for every action in the actor:
			actor_path = malloc(strlen(project_path)+strlen(roxml_get_content(roxml_get_attr(instance, "id", 0), NULL, 0, &len))+10);
			strcpy(actor_path,project_path);
			actor_path = strcat(actor_path,"/src/");
			actor_path = strcat(actor_path,roxml_get_content(roxml_get_attr(instance, "id", 0), NULL, 0, &len));
			actor_path = strcat(actor_path,".c");
			printf("opening %s\n", actor_path);
			actor_src = fopen(actor_path,"r");
			put_actions(actor_src, papi_instance_node, papi_action_node, papi_event_node);
			fclose(actor_src);
		}
	}

	char *mapping_file_cpy = malloc(strlen(mapping_file_path)+strlen(".papi.xcf")+1);

	strcpy(mapping_file_cpy, mapping_file_path);
	strcat(mapping_file_cpy, ".papi.xcf");



	//tmp = roxml_add_node(tmp, 0, ROXML_ELM_NODE, "price", "24");
	roxml_commit_changes(configuration, mapping_file_cpy, NULL, 1);
	//roxml_close(configuration);


	// release the last allocated buffer even if no pointer is maintained by the user
	roxml_release(RELEASE_LAST);
	// here no memory leak can occur.
	roxml_close(configuration);


	return 0;
}



