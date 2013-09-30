#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "gpio-utils.h"
#include <string.h>
#include "i2c-dev.h"
#include "i2cbusses.h"



int main( int argc, char * argv[]){
	FILE * fp;
	system("i2cset -y 1 0x4a 2 0x1a");
	system("i2cset -y 1 0x49 2 0x1a");
	system("i2cset -y 1 0x4a 3 0x1a");
	system("i2cset -y 1 0x49 3 0x1a");
	
	struct pollfd fdset[2];
	memset((void*)fdset, 0, sizeof(fdset));

	int status;
	char output[100];

	gpio_export(30);
	gpio_set_dir(30, "in");
	gpio_set_edge(30, "both");
	
	gpio_export(60);
	gpio_set_dir(60, "in");
	gpio_set_edge(60, "both");
	
	int alert_fd [2];
	alert_fd[0] = gpio_fd_open(30, O_RDONLY);
	alert_fd[1] = gpio_fd_open(60, O_RDONLY);

	while(1){
		fdset[0].fd = alert_fd[0];
		fdset[0].events = POLLPRI;
		fdset[1].fd = alert_fd[1];
		fdset[1].events = POLLPRI;
		int rc = poll(fdset, 2, -1);
		if (fdset[0].revents & POLLPRI){
			char buf[10];
			buf[0] = '\0';
			(void)read(fdset[0].fd, buf, 1);
			if(buf[0]==0){
				break;
			}
		}
		if (fdset[1].revents & POLLPRI){
			char buf[10];
			buf[0] = '\0';
			(void)read(fdset[1].fd, buf, 1);
			if(buf[0]==0){
				break;
			}
		}

	}
	system("i2cset -y 1 0x4a 1 0x60");
	fp = popen("i2cget -y 1 0x4a 0 w", "r");

	if (fp==NULL){
		fprintf(stderr, "Failed to get temperature");
		exit(-1);
	}
	
	fgets(output, sizeof(output)-1, fp);
	int bytes = strtol(output, NULL, 0);
	float temp = (bytes & 0xFF) + (bytes >> 8) / 255.0f;
	temp = temp * 9/5+32;
	printf("%3.4f\n", temp);


	system("i2cset -y 1 0x49 1 0x60");
	fp = popen("i2cget -y 1 0x49 0 w", "r");

	if (fp==NULL){
		fprintf(stderr, "Failed to get temperature");
		exit(-1);
	}

	fgets(output, sizeof(output)-1, fp);
	bytes = strtol(output, NULL, 0);
	temp = (bytes & 0xFF) + (bytes >> 8) / 255.0f;
	temp = temp * 9/5 +32;
	printf("%3.4f\n", temp);
	
	return 0;
}
