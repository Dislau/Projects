#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "zmq.h"

char play_field[10][10];
bool open[10][10];
int towin;
void* req = NULL;

const char SUCCESS = 11;
const char FOX = 'L';
const char CHEATS = 'b';
const char START_GAME = 's';
const char CHOOSE = 'c';
const char EDGE = '*';
const char OPENED = 'o';
const int FOXIES = 8;
const char WIN = 'w';

void start(){
    zmq_msg_t request;
    zmq_msg_init_size(&request, sizeof(char));
    
    memcpy(zmq_msg_data(&request), &START_GAME, sizeof(char));
    zmq_msg_send(&request, req, 0);
    zmq_close(&request);
    
    zmq_msg_t reply;
    zmq_msg_init_size(&reply, 100);
    zmq_msg_recv(&reply, req, 0);
    char data[100];
    strcpy(data, (char*)zmq_msg_data(&reply));
    int i;
    for(i = 0; i < 100; i++)
        play_field[i/10][i%10] = data[i];
    
}

void open_edges(int x, int y){
    zmq_msg_t request;
    zmq_msg_init_size(&request, sizeof(char) + sizeof(int)*2);
    
    memcpy(zmq_msg_data(&request), &CHOOSE, sizeof(char));
    memcpy(zmq_msg_data(&request) + sizeof(char), &x, sizeof(int));
    memcpy(zmq_msg_data(&request) + sizeof(char) + sizeof(int), &y, sizeof(int));
    
    zmq_msg_send(&request, req, 0);
    zmq_close(&request);
    
    zmq_msg_t reply;
    zmq_msg_init_size(&reply, sizeof(char));
    zmq_msg_recv(&reply, req, 0);
    char* data = (char*) zmq_msg_data(&reply);
    zmq_close(&reply);
    
    if(*data == OPENED){
        printf("You opened this edge early\n");
    }
    else if(*data == FOX){
        play_field[x][y] = FOX;
        towin++;
        open[x][y] = true;
    }
    else if(*data ==  WIN){
        towin++;
        printf("YOU WIN!!!\n");
        printf("GOOD JOB, COMRAD! YOU ARE BEST OF THE BEST!!!\n");
    }
    else {
        play_field[x][y] = *data;
        open[x][y] = true;
    }
}

void help(){
    printf("Your aim is capture 8 foxes on map 10x10\n");
    printf("After your each movement you'll see map with your perform action\n");
    printf("Your commands:\n");
    printf("start - will start your game\n");
    printf("choose x y - will open edge (x - line, y - column)\n");
    printf("stat - info about the number of moves and captured foxes\n");
    printf("help - mini-guide\n");
    printf("rules - rules of game\n");
    printf("exit - turn off this program\n");
    printf("L - it's a fox\n");
    printf("* - it's closed edge\n");
    printf("Number - count of foxies on the same vertical, horizontal, both diagonals\n");
}

void rules(){
   printf ("On the field (10x10) random anonymous way for a player are placed eight fox\n");
   printf ( "The player outputs its position by entering the coordinates\n" );
   printf ( "In return, he receives the amount of fox, which is located in the same vertical, horizontal and diagonal to the specified cell\n" );
   printf ( "If the player's location coincided with the position of the fox, it is considered to be found\n" );
   printf ( "The game continues until all the fox will be found\n" );
}

void statis(int count){
    printf("Count of your movement: %d\n", count);
    printf("Foxes you found: %d\n", towin);
}

void close_edges(){
    int i, j;
    for(i = 0; i < 10; i++)
        for(j = 0; j < 10; j++)
            open[i][j] = false;
}

void print_map(){
    int i, j;
    printf("   0  1  2  3  4  5  6  7  8  9\n");
    for(i = 0; i < 10; i++){
        printf("%d ", i);
        for(j = 0; j < 10; j++){
            if(play_field[i][j] == FOX && open[i][j] == true)
            printf(" %c ", play_field[i][j]);
            else if(play_field[i][j] == FOX && open[i][j] == false){
                printf(" * ");
            }
            else if(play_field[i][j] == EDGE){
                printf(" %c ", play_field[i][j]);
            }
            else printf(" %d ", (int)play_field[i][j]);
        }
        printf("\n");
    }
}

void print_cheat(){
    int i, j;
    printf("   0  1  2  3  4  5  6  7  8  9\n");
    for(i = 0; i < 10; i++){
        printf("%d ", i);
        for(j = 0; j < 10; j++){
            if((play_field[i][j] != FOX) && (play_field[i][j] != EDGE)){
                printf(" %d ", (int)play_field[i][j]);
            }
            else printf(" %c ", play_field[i][j]);
        }
        printf("\n");
    }
}

void cheater(){
    zmq_msg_t request;
    zmq_msg_init_size(&request, sizeof(char));
    
    memcpy(zmq_msg_data(&request), &CHEATS, sizeof(char));
    zmq_msg_send(&request, req, 0);
    zmq_close(&request);
    
    zmq_msg_t reply;
    zmq_msg_init_size(&reply, sizeof(int));
    zmq_msg_recv(&reply, req, 0);
    int time;
    memcpy(&time, zmq_msg_data(&reply), sizeof(int));
    printf("Sorry, but you can't put anything in %d seconds, because of cheats\n", time);
    sleep(time);
    printf("Okay, temp. ban is gone\n");
    
}

int main(void){
    int xod = 0;
    int x, y;
    char res;
    void* context = zmq_ctx_new();
    req = zmq_socket(context, ZMQ_REQ);
    zmq_connect(req, "tcp://localhost:4040");
    char command[20];
    printf("Welcome to the Hunt for Fox! Now you'll see mini-guide about this game\n");
    help();
    printf("\n");
    for(;;){
        printf("Enter your command: ");
        scanf("%s", command);
        printf("\n");
        if(strcmp(command, "start") == 0){
            towin = 0;
            close_edges();
            start();
            print_map();
        }
        else if(strcmp(command, "choose") == 0){
            scanf("%d %d", &x, &y);
            system("clear");
            if((x < 0 || x > 9) || (y < 0 || y > 9)){
                printf("Wrong input of X and Y\n");
            }
            else {
                xod++;
                open_edges(x, y);
                print_map();
                
                if(towin == FOXIES){
                    printf("Your results: \n");
                    statis(xod);
                    printf("\n");
                } 
            }
        }
        else if(strcmp(command, "help") == 0){
            help();
            printf("\n");
        }
        else if(strcmp(command, "stat") == 0){
            statis(xod);
            printf("\n");
        }
        else if(strcmp(command, "exit") == 0){
            break;
        }
        else if(strcmp(command, "cheat") == 0){
            print_cheat();
            printf("\n");
            cheater();
        }
        else if(strcmp(command, "rules") == 0){
        	rules();
        	printf("\n");
		}
    }
    printf("Turn off the game\n");
    printf("You found all foxes for %d movements\n", xod);
    zmq_close(req);
    zmq_ctx_destroy(context);
    return 0;
}
