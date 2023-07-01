#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"./structures.h"

ARGS_OPTIONS options = {0, 0, 0, 0, NULL};

void process_args(unsigned int args, char **argv){
	for(unsigned int i=0; i < args; i++){
		if(!strcmp("-d", argv[i])){
			options.debug = 1;
		} else if(!strcmp("-i", argv[i])){
			options.interface_name = argv[++i];
		} else if(!strcmp("--dry", argv[i])){
			options.dry = 1;
		} else if(!strcmp("--arp-check", argv[i])){
			options.arp_check = 1;
		} else if(!strcmp("--release", argv[i])){
			options.release = 1;
		}
	}
	if(NULL == options.interface_name){
		printf("Required option -i\nRead README file\n");
		exit(-1);
	}

	if(options.debug){
		printf("Debug: %d\nDry: %d\n"
			"Arp check: %d\nInterface name: %s\n",
			options.debug, options.dry,
			options.arp_check, options.interface_name);
	}
}
