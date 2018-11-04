#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

const int Max_str_len = 1000;
FILE *Log_file = NULL;

struct cam_config
{
	int delay;
	int copy_delay;
	char *cam_adr;
	char *rec_adr;
	char *c_rec_adr;
};

struct all_cams
{
	struct cam_config** massive;
	int maximum;
};

int start_record(struct all_cams *cams);
int copy_record(struct all_cams* cams);
int main_record_cycle(struct all_cams* cams);

struct all_cams* cams_initiator(int max_cams);
void all_cams_destroyer(struct all_cams* cams);
struct cam_config *init_cam_config(void);
void delete_cam(struct cam_config *cur_cam);

// screen -d -m -S record ffmpeg -rtsp_transport tcp -i rtsp://admin:qwerty1234@192.168.1.188:554/ch01.264?ptype=tcp -acodec copy -f segment -segment_time 10 -segment_format avi -reset_timestamps 1 -copyts -flags global_header -strftime 1 /home/alexander/TEMP/%Y-%m-%d_%H-%M-%S.mkv

// host cam-1ka-1 { hardware ethernet 9a:f8:b3:cc:6d:6b; fixed-address 10.55.245.11;}
// host cam-1ka-2 { hardware ethernet 1c:94:10:a7:c4:14; fixed-address 10.55.245.12;}

// work now!
// год-месяц-дата_час-минута-секунда
// char* format = "%Y-%m-%d_%H-%M-%S";
// strftime(buffer, 80, format, timeinfo);
// find /home/voik/vidosiki -mmin +N -exec mv {} \;


int main()
{
	int amount_of_cams = 0;
	char log_file[100] = {};

	printf("How many cams you need?\n");
	scanf("%d", &amount_of_cams);
	if(amount_of_cams <= 0)
	{
		perror("Invalid amount of cams!");
		return -1;
	}

	printf("Enter log adress\n");
	scanf("%s", log_file);
	Log_file = fopen(log_file, "a+");

	struct all_cams* cams = cams_initiator(amount_of_cams);

	if(cams == NULL)
	{
		perror("Invalid initialization of cams!\n");
		return -2;
	}

	int error = main_record_cycle(cams); //NI

	if(cams != NULL)
		all_cams_destroyer(cams);
	
	return 0;
}

int main_record_cycle(struct all_cams* cams)
{
	pid_t parent_pid = getpid();
	bool is_going = true;
	bool is_record_start = false;
	char command = 0;
	pid_t child_pid = fork();
	time_t t = time(NULL);
	struct tm* aTm = localtime(&t);
	char command_str[1000] = {};
	snprintf(command_str, sizeof(command_str), "%s/logs.txt", cams->massive[0]->c_rec_adr); // TEST
	FILE *local_log = fopen(command_str, "a+");
	
	while(is_going == true)
	{
		if(child_pid != 0) 	// roditel
		{
			printf("> Wait your commands (one symbol)! (k = killall, a = show active records)\n");
			
			scanf("%c", &command);

			if(command == 'k')
			{
				// last copy
				fprintf(Log_file, "\"%d-%d %d-%d-%d\" - end of prog working\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec); // log printf
				fclose(Log_file);
				all_cams_destroyer(cams);
				system("killall screen");
				return 0;
			}
			else if(command == 'a')
				system("screen -list");

			printf("\n");
		}
		else				// rebionok
		{
			if(is_record_start == false)
			{
				//fprintf(local_log, "\"%d-%d %d-%d-%d\" - record start\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
				start_record(cams);
				is_record_start = true;
				sleep(cams->massive[0]->copy_delay); //
			}

			//  copy here
			sleep(cams->massive[0]->copy_delay); //
			//fprintf(local_log, "\"%d-%d %d-%d-%d\" - move start\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
			copy_record(cams);
			//fprintf(local_log, "\"%d-%d %d-%d-%d\" - move end\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
		}
	}

	fclose(local_log);
	return 0;
}

// %Y-%m-%d_%H-%M-%S
int copy_record(struct all_cams* cams)
{
	time_t t = time(NULL);
	struct tm* aTm = localtime(&t);
	int max_cams = cams->maximum;
	int cam_num = 0;
	char command_str[1000] = {};

	while(cam_num != max_cams)
	{
		//fprintf(Log_file, "\"%d-%d %d-%d-%d\" - move start for cam %d\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, cam_num);
		snprintf(command_str, sizeof(command_str), "./move %s +%d %s", cams->massive[cam_num]->rec_adr, (cams->massive[cam_num]->delay / 60) + 1, cams->massive[cam_num]->c_rec_adr);
		system(command_str);
		//fprintf(Log_file, "\"%d-%d %d-%d-%d\" - move end for cam %d\n", aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, cam_num);
		snprintf(command_str, sizeof(command_str), "./clear_old %s +60", cams->massive[cam_num]->c_rec_adr);
		system(command_str);
		sleep(3); // Test sleep
		cam_num++;
	}
}

int start_record(struct all_cams *cams)
{
	int i = 0;
	char command_str[1000] = {};

	while(i != cams->maximum)
	{
		snprintf(command_str, sizeof(command_str), "screen -d -m -S record ffmpeg -rtsp_transport tcp -i rtsp:/%s/ch01.264?ptype=tcp -acodec copy -f segment -segment_time %d -segment_format avi -reset_timestamps 1 -copyts -flags global_header -strftime 1 %s%s",cams->massive[i]->cam_adr, cams->massive[i]->delay,  cams->massive[i]->rec_adr, "/%Y-%m-%d_%H-%M-%S.avi");
		printf("### <%s> \n",command_str);
		system("screen -list");

		system(command_str);
		i++;
	}
}



struct all_cams* cams_initiator(int max_cams)
{
	struct all_cams* cams = calloc(1, sizeof(struct all_cams));

	if(cams == NULL)
		return NULL;

	cams->massive = calloc(max_cams, sizeof(struct all_cams));

	int i = 0;
	int max_i = max_cams;
	char accept = 0;
	cams->maximum = max_i;

	while(i != max_i)
	{
		cams->massive[i] = init_cam_config();

		if(cams->massive[i] == NULL)
		{
			printf("Error in init!!\n");
			all_cams_destroyer(cams);
			return NULL;
		}

		//while(accept != 'Y')
		//{
		//	accept = 0;
		printf("Please enter \"delay copy_delay ip_of_cam(<login>:<password>@<ip>:<port>) adress_for_load adress_for_copy\"\n>");
		scanf("%d %d %s %s %s", &cams->massive[i]->delay, &cams->massive[i]->copy_delay, cams->massive[i]->cam_adr, cams->massive[i]->rec_adr, cams->massive[i]->c_rec_adr);
		printf("\n# Camera %d\n# delay:%d\n# copy delay:%d\n# camera adress:%s\n# record adress:%s\n# adress for copy:%s\n\n", i, cams->massive[i]->delay, cams->massive[i]->copy_delay, cams->massive[i]->cam_adr, cams->massive[i]->rec_adr, cams->massive[i]->c_rec_adr);
		printf("\n>");
		//printf("> Are camera config correct? (Y/n)");
		//	accept = getchar();

		//	if(accept == 'n')
		//		return NULL;
		//}

		i++;
	}

	return cams;
}

struct cam_config *init_cam_config(void)
{
	struct cam_config *cur_cam = calloc(1, sizeof(struct cam_config));
	cur_cam->cam_adr = calloc(Max_str_len, sizeof(char));
	cur_cam->rec_adr = calloc(Max_str_len, sizeof(char));
	cur_cam->c_rec_adr = calloc(Max_str_len, sizeof(char));

	if(cur_cam == NULL || cur_cam->cam_adr == NULL || cur_cam->c_rec_adr == NULL || cur_cam->rec_adr == NULL)
	{
		perror("NULL in init!");
		return NULL;
	}

	return cur_cam;
}

void all_cams_destroyer(struct all_cams* cams)
{
	int max_i = cams->maximum;
	int i = 0;
	
	while(i != max_i)
	{
		if(cams->massive[i] != NULL)
			delete_cam(cams->massive[i]);
		i++;
	}

	free(cams->massive);
	free(cams);
}

void delete_cam(struct cam_config *cur_cam)
{
	free(cur_cam->cam_adr);
	free(cur_cam->rec_adr);
	free(cur_cam->c_rec_adr);
	free(cur_cam);
}
