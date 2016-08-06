#include <stdbool.h>
#include <stdio.h> 
#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

using namespace std;

int MAX_SIZE = 10000;
int MIN_SIZE = 10;
int MAP_SIZE = 4096;
int current_mapblock;
int poses[100];
int alltxt[100];
int num_occur = 0;
int linenes[100];
string f1;

typedef struct opt_file{
    char* f;
    char* name_file;

    int isopen;
    int fd; 
    int file_size;
    
    opt_file() {
        f = NULL; 
        name_file = (char*)malloc(257);
        isopen = 0; 
        fd = -1; 
        file_size = 0;
        }
}opt_file;

opt_file Meow;
int SetBlock(int);

bool ChooseFile(){
    chdir("/home/dislau/Documents/OS_LABS/Labs/src/Kursach/");
    Meow.fd = open(Meow.name_file, O_RDWR);
    perror("open");
    if(Meow.fd == -1){
        return false;
    }
    Meow.f = (char*)mmap(NULL, MAP_SIZE,PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, 0);
    
    struct stat info;
    fstat(Meow.fd, &info);
    
    if(info.st_size < MIN_SIZE){
        printf("Try to open another file(<MIN_SIZE)\n");
        close(Meow.fd);
        return false;
    }
    else if(info.st_size > MAX_SIZE){
        printf("Try to open another file(>MAX_SIZE\n");
        close(Meow.fd);
        return false;
    }
    Meow.file_size = info.st_size;
    Meow.isopen = true;
    printf("SUCCESS OPEN FILE\n");
    return true;
}

void Info(){
    if(Meow.fd == -1){
        printf("File isn't opened (Info)\n");
         return;
    }
    int count = 0;
    Meow.f = (char*)mmap(NULL, -lseek(Meow.fd, 0, SEEK_SET) +lseek(Meow.fd, 0, SEEK_END), PROT_READ | PROT_WRITE, MAP_SHARED, Meow.fd, 0);
    for(int i = 0; i < (int)strlen((char*)Meow.f); i++){
        if(Meow.f[i]=='\n'){
            count++;
        }
    }
    printf("Info about your file: %s\n", Meow.name_file);
    printf("Number of symbols: %d\n", (int)(-lseek(Meow.fd, 0, SEEK_SET)+lseek(Meow.fd, 0, SEEK_END)));
    printf("Number of lines: %d\n", count);
}

void GetLine(int line){
    if(Meow.isopen == false){
        printf("File isn't opened (GetLine)\n");
        return;
    }
    int temp = 1, pos = 0, page_counts = 1;
    
    while(temp != line){
        if(Meow.f[pos] == '\n'){
            ++temp;
        }
        ++pos;
        if(pos>= MAP_SIZE){
            munmap((void*)Meow.f, MAP_SIZE);
            Meow.f = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, MAP_SIZE * page_counts);
            ++page_counts;
            pos = 0;
        }
    }
     while(Meow.f[pos] != '\n'){
         printf("%c", Meow.f[pos]);
         ++pos;
         if(pos>= MAP_SIZE){
            munmap((void*)Meow.f, MAP_SIZE);
            Meow.f = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, MAP_SIZE * page_counts);
            ++page_counts;
            pos = 0;
        }
        if(pos>=(int)strlen((char*)Meow.f)){
            break;
        }
    }
    printf("\n");
    munmap((void*)Meow.f, MAP_SIZE);
    Meow.f = (char*)mmap(NULL, MAP_SIZE,PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, 0);
    
}

void NaiveSearch(char* substr, int registr){
    if(Meow.fd == -1){
        printf("File isn't opened (Search)\n");
        return;
    }
    num_occur = 0;
    int n = Meow.file_size;
    int m = strlen(substr);
    int pos = 0;
    int lines = 1;
    SetBlock(0);
    for(int i = 0; i <= n-m+1;i++){
        for(int j = 0; j <= m; j++){
            if(!SetBlock(i+j)){
                printf("Error of setting block (NaiveSearch)\n");
                return;
            }
            if(registr == 1){
            if(Meow.f[(i+j)%MAP_SIZE]!=substr[j])
                break;
                
            }
            else if(registr == 0){
                if((tolower(Meow.f[(i+j)%MAP_SIZE])!=substr[j]) && (toupper(Meow.f[(i+j)%MAP_SIZE])!=substr[j]))
                    break;
            }
                if(j == m-1){
                    alltxt[num_occur] = i;
                    poses[num_occur] = pos+1;
                    linenes[num_occur] = lines;
                    num_occur++;
                    
                }
        }
        pos++;
        if(Meow.f[i%MAP_SIZE] == '\n'){
            lines++;
            pos = 0;
            
        }
    }
}

void Search(char* substr, int registr){
    NaiveSearch(substr, registr);
    if(num_occur > 0)
    for (int i = 0; i < num_occur;i++){
        printf("Occurance: line %d, pos %d, alltxtpos %d\n", linenes[i], poses[i], alltxt[i]);
    }
    else {
        printf("No occurance\n");
    }
}

int ReplaceFrag(char* frag, char* replfrag){
    if(Meow.fd == -1){
        printf("File isn't opened (Replace)\n");
        return -1;
    }
    int oldpos = 0;
    int n = strlen(frag);
    int m = strlen(replfrag);
    
    string newstr;
    
    NaiveSearch(frag, 1);
    if(num_occur == 0){
        printf("Can't replace, cause of no occurance\n");
        return -1;
    }
    
    int res = 0;
    
    if(num_occur > 1){
        res = Meow.file_size - n*num_occur + m*num_occur;
    }
    else if(num_occur == 1){
        res = Meow.file_size - n + m;
    }
    
    if(res < MIN_SIZE){
            printf("Sorry, i can't replace (< min)\n");
            return -1;
        }
    if(res > MAX_SIZE){
            printf("Sorry, i can't replace (> max)\n");
            return -1;
        }
    
    printf("Old: %d\n", Meow.file_size);
    printf("New: %d\n", res);
    
    munmap(&f1, MAP_SIZE);
    f1 = (char*)mmap(NULL, MAP_SIZE,PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, 0);
    for(int i = 0; i < num_occur; i++){
            newstr.append(f1, oldpos, alltxt[i]-oldpos);
            newstr+= replfrag;
            oldpos = alltxt[i]+n;
            if(oldpos>MAP_SIZE){
                if(!SetBlock(oldpos)){
                    printf("Error of setting block\n");
                    return -1;
                }
            }
    }
    
    for (int i = oldpos; i <= Meow.file_size; i++){
        newstr.append(f1, i, 1);
        if(i>MAP_SIZE){
                if(!SetBlock(i)){
                    printf("Error of setting block\n");
                    return -1;
                }
            }
    }
    
    munmap((void*)Meow.f, MAP_SIZE);
    if(ftruncate(Meow.fd, res) < 0)
    perror("ftruncate: ");
    Meow.f = (char*)mmap(NULL, res, PROT_READ|PROT_WRITE, MAP_SHARED, Meow.fd, 0);
    strcpy(Meow.f,newstr.c_str());
    munmap(&f1, MAP_SIZE);
    if(msync(Meow.f, res, MS_SYNC) < 0){
        perror("msync");
    }
    else {
        printf("msync comlete \n");
            }
    munmap((void*)Meow.f, res);
    printf("Success replace!\n");
    Meow.file_size = res;
    return 0;
}

int SetBlock(int pos){
    if(Meow.fd == -1){
        printf("File isn't opened (SetBlock)\n");
        return 0;
    }
    if(pos < 0 || pos > Meow.file_size){
        return 0;
    }
    if(pos / MAP_SIZE != current_mapblock){
        current_mapblock = pos / MAP_SIZE;
        munmap((void*)Meow.f, MAP_SIZE);
        munmap(&f1, MAP_SIZE);
        Meow.f = (char*)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, Meow.fd, MAP_SIZE * current_mapblock);
        f1 = (char*)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, Meow.fd, MAP_SIZE * current_mapblock);
    }
    return 1;
}

void help(){
    printf("This is mini-guide about this program:\n");
    printf("\n");
    printf("stat - count of lines and symbols of current file\n");
    printf("choose <file_name> - will open file with <file_name>\n");
    printf("replace <fragment> <repl_fragment> - will replace <fragment> by <repl_fragment>\n");
    printf("getline <nubmer> - print line of file with <nubmer>\n");
    printf("set_min <size> - set minimum file size(bytes)\n");
    printf("set_max <size> - set maximum file size(bytes)\n");
    printf("help - print guide\n");
    printf("search <substr> <mode> - searching substring, mode 0 - insensitive, 1 - case sensitive\n");
    printf("exit - turn off this program (only in interactive mode)\n");
}

void Interactivemode(){
    int line, registr;
    char replfrag[256];
    char command[20];
    char substr[256];
    current_mapblock = -1;
    printf("Welcome to interactive mode!\n");
    help();
    while(true){
        printf("\n");
        printf("->");
        scanf("%s", command);
        if(strcmp(command, "exit") == 0){
            munmap((void*)Meow.f, MAP_SIZE);
            munmap(&f1, MAP_SIZE);
            close(Meow.fd);
            break;
        }
        else if(strcmp(command, "set_min") == 0){
            scanf("%d", &MIN_SIZE);
        }
        else if(strcmp(command, "set_max") == 0){
            scanf("%d", &MAX_SIZE);
        }
        else if(strcmp(command, "choose") == 0){
            scanf("%s", Meow.name_file);
            ChooseFile();
        }
        else if(strcmp(command, "stat") == 0){
            if(Meow.fd == -1){
                printf("Open file pls\n");
                continue;
            }
            Info();
        }
        else if(strcmp(command, "getline") == 0){
            if(Meow.fd == -1){
                printf("Open file pls\n");
                continue;
            }
            scanf("%d", &line);
            GetLine(line);
        }
        else if(strcmp(command, "search") == 0){
            if(Meow.fd == -1){
                printf("Open file pls\n");
                continue;
            }
            scanf("%s", substr);
            scanf("%d", &registr);
            Search(substr, registr);
        }
        else if(strcmp(command, "help") == 0){
            system("clear");
            help();
        }
        else if(strcmp(command, "replace") == 0){
            scanf("%s", substr);
            scanf("%s", replfrag);
            SetBlock(0);
            if(ReplaceFrag(substr, replfrag) < 0)
                continue;
            
            SetBlock(0);
        }
        else {
            printf("Wrong command\n");
        }
    }
}

void Commandmode(int argc, char** argv){
    printf("Hey, user, it's a command mode\n\n");
    int line, registr;
    char replfrag[256];
    char substr[256];
    current_mapblock = -1;
    int n = 1;
    while(n < argc){
        if ( strcmp ( argv[n], "-choose" ) == 0 && ( argc - n ) >= 1 ){
            printf("->%s\n", argv[n]);
            strcpy(Meow.name_file, argv[n+1]);
            ChooseFile();
            n+=2;
        }
        else if ( strcmp ( argv[n], "-stat" ) == 0 ){
            printf("->%s\n", argv[n]);
            if(Meow.isopen == false){
                printf("File isn't opened\n");
                ++n;
                continue;
            }
            Info();
            ++n;
        }
        else if ( strcmp ( argv[n], "-getline" ) == 0 && ( argc - n ) >= 1 ){
            printf("->%s\n", argv[n]);
            if(Meow.isopen == false){
                printf("File isn't opened\n");
                n+=2;
                continue;
            }
            line = atoi(argv[n+1]);
            GetLine(line);
            n+=2;
        }
        else if ( strcmp ( argv[n], "-search" ) == 0 && ( argc - n ) >= 2 ){
            printf("->%s\n", argv[n]);
            if(Meow.isopen == false){
                printf("File isn't opened\n");
                n+=3;
                continue;
            }
            strcpy(substr, argv[n+1]);
            registr = atoi(argv[n+2]);
            Search(substr, registr);
            n+=3;
        }
        else if ( strcmp ( argv[n], "-set_min" ) == 0 ){
            printf("->%s\n", argv[n]);
            MIN_SIZE = atoi(argv[n+1]);
            ++n;
        }
        else if ( strcmp ( argv[n], "-set_max" ) == 0 ){
            printf("->%s\n", argv[n]);
            MAX_SIZE = atoi(argv[n+1]);
            ++n;
        }
         else if ( strcmp ( argv[n], "-help" ) == 0 ){
             help();
             ++n;
         }
          else if ( strcmp ( argv[n], "-replace" ) == 0 && ( argc - n ) >= 2 ){
              printf("->%s\n", argv[n]);
              if(Meow.isopen == false){
                printf("File isn't opened\n");
                n+=3;
                continue;
              }
              strcpy(substr, argv[n+1]);
              strcpy(replfrag, argv[n+2]);
              SetBlock(0);
              if(ReplaceFrag(substr, replfrag) < 0){
                n+=3;
                continue;
              }
              n+=3;
              SetBlock(0);
          }
    }
}

int main(int argc, char** argv){
    
    if (argc == 1){
        Interactivemode();
    }
    else if(argc > 1){
        Commandmode(argc, argv);
    }
    return 0;
}
