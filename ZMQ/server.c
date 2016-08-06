#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "zmq.h"

char play_field[10][10];
char* recieve;
bool open[10][10];
int towin = 0;

const char SUCCESS = 11;
const char CHEATS = 'b';
const char FOX = 'L';
const char START_GAME = 's';
const char CHOOSE = 'c';
const char EDGE = '*';
const char OPENED = 'o';
const int FOXIES = 8;
const char WIN = 'w';

char field(){
    recieve = (char*)malloc(100);
    srand(time(0));
    int i, j, k, count = 0;
    for(k = 0; k < 100; k++){
        play_field[k/10][k%10] = EDGE;
        open[k/10][k%10] = false;
    }
    for(k = 0; k < 10; k++){
        do{
            i = rand()%10;
            j = rand()%10;
        }while(play_field[i][j]!=EDGE);
        play_field[i][j] = FOX;
        count++;
        if(count == FOXIES){
            break;
        }
    }
    for(i = 0; i < 100; i++)
        recieve[i] = play_field[i/10][i%10];
    return SUCCESS;
}

char open_edge(int str, int strk){
    char res = 0;
    int i;
    if(open[str][strk] == true){
        return OPENED;
    }
    else {
    open[str][strk] = true;
    if (play_field[str][strk] == 'L'){
            towin++;
            if(towin == FOXIES){
                printf("End of game\n");
		printf("\n");
                return WIN;
            }
            else{
                return FOX;
            }
        }
    else {
        for(i = 0; i < 10; i++){
            if (play_field[str][i] == 'L'){
                res++;
            }
            if (play_field[i][strk] == 'L'){
                res++;
            }
            if ((str+i) < 10 && (strk+i) < 10){
                if (play_field[str+i][strk+i] == 'L'){
                    res++;
                }
            }
            if ((str-i) >= 0 && (strk+i) < 10){
                if (play_field[str-i][strk+i] == 'L'){
                    res++;
                }
            }
            if ((str-i) >= 0 && (strk-i) >= 0){
                if (play_field[str-i][strk-i] == 'L'){
                    res++;
                }
            }
            if ((str+i) < 10 && (strk-i) >= 0){
                if (play_field[str+i][strk-i] == 'L'){
                    res++;
                }
            }
          }
          return res;
        }
    }
}

int main(){
    void* context = zmq_ctx_new();
    void* respond = zmq_socket(context, ZMQ_REP);
    zmq_bind(respond, "tcp://127.0.0.1:4040");
    printf("Starting server...\n");
    
    for(;;){
        int x, y;
        char action, ans; 
        
        zmq_msg_t request;
        zmq_msg_init(&request);
        zmq_msg_recv(&request, respond, 0);
        
        char* data = (char*)zmq_msg_data(&request);
        memcpy(&action, data, sizeof(char));
        zmq_msg_close(&request);
        zmq_msg_t reply;
        
        if(action == START_GAME){
            zmq_msg_init_size(&reply, 100);
            ans = field();
            printf("Game started...\n");
            memcpy(zmq_msg_data(&reply), recieve, 100);
            free(recieve);
        }
        else if(action == CHOOSE){
            memcpy(&x, data + sizeof(char), sizeof(int));
            memcpy(&y, data + sizeof(char) + sizeof(int), sizeof(int));
            zmq_msg_init_size(&reply, sizeof(char));
            ans = open_edge(x, y);
            memcpy(&reply, &ans, sizeof(char));
        }
        else if(action == CHEATS){
            zmq_msg_init_size(&reply, sizeof(int));
            int time = rand()%30;
            printf("Server: you can't put anything in %d seconds, don't use cheats!\n", time);
            memcpy(&reply, &time, sizeof(int));
        }
        zmq_msg_send(&reply, respond, 0);
        zmq_close(&reply);
    }
    return 0;
}
