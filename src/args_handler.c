#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"./structures.h"

void process_args(int argc, char **argv, ARGS_OPTIONS *options)
{
	int i;

	for(i=0; i < argc; i++){
		if(!strcmp("-d", argv[i])){
			options->debug = 1;
		} else if(!strcmp("-i", argv[i])){
			options->interface_name = argv[++i];
		} else if(!strcmp("--dry", argv[i])){
			options->dry = 1;
		} else if(!strcmp("--no-debug-files", argv[i])){
			options->no_dbg_files = 1;
		} else if(!strcmp("--arp-check", argv[i])){
			options->arp_check = 1;
		} else if(!strcmp("--release", argv[i])){
			options->release = 1;
		}
	}
	if(NULL == options->interface_name){
		printf("Required option -i\nRead README file\n");
		exit(-1);
	}

	if(options->debug){
		printf("Debug: %d\nDry: %d\nNo dbg files: %d\n"
			"Arp check: %d\nInterface name: %s\n",
			options->debug, options->dry, options->no_dbg_files,
			options->arp_check, options->interface_name);
	}
}
