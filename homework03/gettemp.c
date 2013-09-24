#include <stdio.h>
#include <stdlib.h>

int main( int argc, char * argv[]){
	FILE * fp;
	int status;
	char output[100];
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
