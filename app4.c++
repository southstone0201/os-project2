#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int job;
	int delay;
	char name[4];
	int wait = 0;
	int response = 0;
	int flag = 2;
	int priority;
	if(argc < 4) {
		printf("\nInsufficient Arguments\n");
		return 1;
	}
	
	job = atoi(argv[1]);
	delay = atoi(argv[2]);
	strcpy(name, argv[3]);
	priority = atoi(argv[4]);

	sleep(delay);
	printf("\nProcess %s : I will use CPU by %ds.\n", name, job);
	job *= 10;
	int i = syscall(338, name, job, priority);
	sleep(10);
	printf("sleep 10s\n");

	while(job) {
		if (!syscall(338, name, job, priority)){
			if(flag == 2) {
				response = wait;
				flag = 0;
				job --;
			}
			else{
				job--;
			}
			
		}
		else {
			wait++;
		}
		usleep(100000);
	}

	syscall(338, name, 0, priority);
	printf("\nProcess %s : Finish! My response time is %ds and My total wait time is %ds.\n", name, (response)/10, (wait + 5)/10);
	return 0;
}