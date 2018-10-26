
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdint.h>

const int Command_len = 100;

int main_cycle(void);
int set_copy(const char* file_from, const char* file_to);

void prinf_help(void);

int main()
{
	int error = main_cycle();

	return 0;
}

int main_cycle(void)
{
	unsigned long int delay = 0;
	char cur_command[100] = {};
	char adress_1[200] = {};
	char adress_2[200] = {};
	int i = 0;
	int error = 0;
	char shure = 0;

	printf(">>> initialization start (enter any string)...\n");	
	printf("> ");
	scanf("%100[^\n]",cur_command);
	printf("\n");
	printf(">>> init complete (help for help)\n");

	while(strcmp(cur_command,"exit") != 0)
	{
		i = 0;
		while(i != Command_len)
			cur_command[i++] = '\0';

		printf("> ");
		scanf("%c %99[^\n]",cur_command ,cur_command);
		printf("\n");

		printf(">>> \"%s\"\n", cur_command); // TEST
		
		// Commands here

		if(strcmp(cur_command,"set delay") == 0)
		{
			printf(">>> Enter delay (in seconds)\n> ");
			scanf("%ld", &delay);
			printf("\n>>> Delay set to %ld\n", delay);
		}
		else if(strcmp(cur_command, "set copy") == 0)
		{
			if(delay > 0)
			{
				printf(">>> Enter filepath for copy\n> ");
				scanf("%s", adress_1);
				printf("\n>>> Enter filepath for paste\n> ");
				scanf("%s", adress_2);
				printf("\n>>> Are you write ewerything correct?(Y/n)\n");
				scanf("%c", &shure); //err here
				printf("\n");


				if(shure == 'Y')
					error = set_copy(&adress_1, &adress_2);
				else
					printf(">>> Command cancel!\n\n");
			}
			else
				printf(">>> Error! Delay time <= 0! Can't set copy for this!\n");
		}
		else if(strcmp(cur_command,"get info") == 0)
		{
			// info imliment
		}
		else if(strcmp(cur_command,"help") == 0)
		{
			prinf_help();
		}
		else if(strcmp(cur_command,"exit") == 0);
		else
			printf(">>> Unknown command try help, for help\n");

	}

	return 0;
}

int set_copy(const char* file_from, const char* file_to)
{
	printf(">>> Copy \"%s\", paste \"%s\"\n", file_from, file_to);

	return 0;
}

void prinf_help(void)
{
	//printf(">>> \n");
	printf(">>> ------------ help start ---------------\n");
	printf(">>> \n");
	printf(">>> \"set delay\" - set delay set the delay between recording (in seconds, pls don't do it low) \n");
	printf(">>> \"set copy\" - set copy_path from file1 to file2 with setted delay\n");
	printf(">>> \"get info\" - print all info about recording (current delay, active cameras...)\n");
	printf(">>> \"exit\" - exit from programm (close all recording!)\n");
	printf(">>> \n");
	printf(">>> ------------- help end ----------------\n");

}








