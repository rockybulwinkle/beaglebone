#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "gpio-utils.h"
#include <string.h>
#include "i2c-dev.h"
#include "i2cbusses.h"


#define UP_GPIO 3
#define DOWN_GPIO 2
#define LEFT_GPIO 49
#define RIGHT_GPIO 15

#define TIMEOUT -1 
#define BICOLOR

#define MAX_SIZE_X -1 //max board sizes, -1 will mean infinite
#define MAX_SIZE_Y -1

//game state
typedef struct game_state{ 
	int cursor_x; //cursor's x and y
	int cursor_y; 
	int current_size_x; // current size in x (array is dynamic)
	int current_size_y; //current size in y
	char ** grid; //pointer to grid
}game_state;

int get_next_input(); //abstraction for input method
int get_next_input_beagle(int * gpio_fd ); //abstraction for input method
void handle_next_input(game_state * state, int input); //handles next input
void display_state(game_state * state); //abstraction for displaying
void display_state_matrix(game_state * state, int file); //abstraction for displaying
game_state* setup_display(int start_size_x, int start_size_y); //sets up the gamestate
//int free_display(game_state);

int main(){
	int gpio_fd[4];
	gpio_export(LEFT_GPIO);
	gpio_set_dir(LEFT_GPIO, "in");
	gpio_set_edge(LEFT_GPIO, "falling");
	gpio_fd[0] = gpio_fd_open(LEFT_GPIO, O_RDONLY);

	gpio_export(UP_GPIO);
	gpio_set_dir(UP_GPIO, "in");
	gpio_set_edge(UP_GPIO, "falling");
	gpio_fd[1] = gpio_fd_open(UP_GPIO, O_RDONLY);
	
	gpio_export(RIGHT_GPIO);
	gpio_set_dir(RIGHT_GPIO, "in");
	gpio_set_edge(RIGHT_GPIO, "falling");
	gpio_fd[2] = gpio_fd_open(RIGHT_GPIO, O_RDONLY);

	gpio_export(DOWN_GPIO);
	gpio_set_dir(DOWN_GPIO, "in");
	gpio_set_edge(DOWN_GPIO, "falling");
	gpio_fd[3] = gpio_fd_open(DOWN_GPIO, O_RDONLY);
	
	int file;
	char filename[20];
	int i2cbus = lookup_i2c_bus("1");
	printf("i2cbus = %d\n", i2cbus);
	int address = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", address);
	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	printf("file = %d\n", file);
	i2c_smbus_write_byte(file, 0x21);//start osc (p10)
	i2c_smbus_write_byte(file, 0x81);//disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7);//full brightness (p15)

	game_state * state = setup_display(5,5);
	while(1){
	//	int next_direction = get_next_input_beagle(gpio_fd);
	//	handle_next_input(state,next_direction);
	//	display_state(state);
		display_state_matrix(state, file);
	}
	return 0;
}

int get_next_input_beagle(int *gpio_fd ){
	int value[4] = {0,0,0,0};
	struct pollfd fdset[4];
	memset((void*)fdset, 0, sizeof(fdset));
	int i;
	for(i = 0; i < 4; i++){
		fdset[i].fd = gpio_fd[i];
		fdset[i].events = POLLPRI;
	}

	int rc = poll(fdset, 4, TIMEOUT);
	printf("rc:%d\n", rc);
	if (rc < 0){
		return -1;
	}
	for (i = 0; i < 4; i++){
		if (fdset[i].revents & POLLPRI){
			char buf[10];
			buf[0] = '\0';
			(void)read(fdset[i].fd, buf, 1);
			if(buf[0]==value[i]){
				return i;
			}
		}
	}
	return -1;
	
	
}
int get_next_input(){
	char buf[1000]; //yeah I know security hole
	scanf("%s", buf);
	switch(buf[0]){
		case 97: return 0;
		case 119: return 1;
		case 100: return 2;
		case 115: return 3;
	}
	return -1;
}

void handle_next_input(game_state * state, int input){
	switch(input){
		case 0: //left arrow
			state->cursor_x -= 1;
			break;
		case 1:	//up arrow
			state->cursor_y -= 1;
			break;
		case 2: //right arrow
			state->cursor_x += 1;
			break;
		case 3: //down arrow
			state->cursor_y += 1;
			break;
		default: break;
	}
	
	if (state->cursor_x < 0){
		state->cursor_x = 0;
	}
	if (state->cursor_y < 0){
		state->cursor_y = 0;
	}
	if ((state->cursor_x >= MAX_SIZE_X) && (MAX_SIZE_X != -1)){
		state->cursor_x = MAX_SIZE_X;
	}
	if ((state->cursor_y >= MAX_SIZE_Y) && (MAX_SIZE_Y != -1)){
		state->cursor_y = MAX_SIZE_Y;
	}
	int x,y;
	if (state->cursor_x >= state->current_size_x){
		state->current_size_x = state->cursor_x + 1; //2, one for the difference between size and index, the other for increasing the size
		state->grid = realloc(state->grid, sizeof(char*)*state->current_size_x);
		if (state->grid == NULL){
			printf("FAIL\n");
			exit(0);
		}
		state->grid[state->current_size_x-1] = malloc(sizeof(char*)*state->current_size_y);
		if (state->grid[state->current_size_x-1] == NULL){
			printf("FAIL\n");
			exit(0);
		}
		
		for (y = 0; y < state->current_size_y; y++){
			printf("y:%d, x:%d\n",y,state->cursor_x);
			fflush(stdout);
			state->grid[state->cursor_x][y] = ' ';
		}
	}
	
	if (state->cursor_y >= state->current_size_y){
		state->current_size_y = state->cursor_y + 1;
		for (x = 0; x < state->current_size_x; x++){
			state->grid[x] = realloc(state->grid[x], sizeof(char *) * state->current_size_y);
			state->grid[x][state->current_size_y-1] = ' ';
		}
	}
	state->grid[state->cursor_x][state->cursor_y]='x';
}

game_state* setup_display(int start_size_x, int start_size_y){
	game_state * state = malloc(sizeof(game_state)); //allocate space for a game_state
	state->cursor_x = 0; //initiate cursors
	state->cursor_y = 0;
	state->current_size_x = start_size_x; //initiate the current size
	state->current_size_y = start_size_y;
	state->grid = malloc(sizeof(char*) * state->current_size_x); //initiate the grid for the current size
	int x,y; 
	for (x = 0; x < state->current_size_x; x++){
		state->grid[x] = malloc(sizeof(char*) * state->current_size_y); //allocate each row
		for (y = 0; y < state->current_size_y; y++){
			state->grid[x][y] = ' '; //set each cell in each row to ' '
		}
	}
	return state;
}


void display_state(game_state * state){
	int x,y;
	
	system("clear");
	printf("    "); //print empty space so the y labels look right
	for (x = 0; x < state->current_size_x; x++){ //print out the x labels
		printf("%3d",x);
	}
	printf("\n"); //new line...
	
	for (y = 0; y < state->current_size_y; y++){
		printf("%3d:", y);  //print out the y labels
		for (x = 0; x < state->current_size_x; x++){ //print out the row
			printf("  %c", state->grid[x][y]);
		}
		printf("\n"); //new line at end of row
	}
	
}


void display_state_matrix(game_state * state, int file){
	static __u16 bytes[] = {65535,65535,65535, 65535, 65535, 65535, 65535, 0};
	int i;
//	for (i = 0; i < 8; i += 1){
//		bytes[i] = 0xFFFF;
//	}
	i2c_smbus_write_i2c_block_data(file, 0x00, 16, (__u8 *) bytes);
}





















