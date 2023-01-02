#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <sys/mman.h>


typedef struct {
    char* name;
    void (*fun)(state*);
}funDesc;

typedef struct {
    char debug_mode;
    char file_name[128];
    FILE* myfile;
    int fileSize;
    void* fileDate;
    //int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    int displayMode;
} state;

void notImplemented(state* s){
    printf("NOT IMPLEMENTED YET");
}

void toggleDebugMode(state* s) {
    if (isDebug(s)) {
        s->debug_mode = '0';
        printf("Debug flag now off\n");
    } else {
        s->debug_mode = '1';
        printf("Debug flag now on\n");
    }
}
//void setUnitSize(state* s){
//    printf("Please Enter Unit Size\n");
//    char unitSizeStr[10];
//    int unitSize;
//    if(fgets(unitSizeStr, sizeof(unitSizeStr), stdin) != NULL) {
//        sscanf(unitSizeStr, "%d", &unitSize);
//        if (isValidUnitSize(unitSize))
//            s->unit_size = unitSize;
//        else {
//            if (s->displayMode)
//                printf("ERROR: %X is not a valid size, please chose from 0x1, 0x2, 0x4\n", unitSize);
//            else
//                printf("ERROR: %i is not a valid size, please chose from 1, 2, 4\n", unitSize);
//        }
//    }
void closeFile(state* s){
    if(s->myfile != NULL){
        munmap(s->fileDate, s->fileSize);
        fclose(s->myfile);
    }
    s->myfile = NULL;
}

void examine(state* s){
    printf("please enter file name");
    char file_name[100];
    if(fgetc(file_name, sizeof(file_name), stdin) != NULL) {
        if(strcmp(file_name, s->file_name)!=0) {
            closeFile(s);
            FILE *file = fopen(s->file_name, "r+");
            s->file_name = file_name;
            s->myfile = file;
            if (file == NULL)
                printf("ERROR: file %s does not exist. /n", s->file_name);

            fseek(s->myfile, 0, SEEK_END);
            long fileSize = ftell(s->myfile);
            fseek(s->myfile, 0, SEEK_SET);
            void *address = mmap(NULL, fileSize, PROT_READ, 0, s->myfile, 0);
            if (address == MAP_FAILED)
                printf("Error mapping file to memory");
            else
                s->fileDate = address;
        }

    }
}

void quit(state* s){
    if(isDebug(s)){
        printf("QUITING\n");
    }
    exit(0);
}



int main(int argc, char** argv){
    state* myState = calloc(1,sizeof (state));
    funDesc funcs[9] = {{"Toggle Debug Mode", &toggleDebugMode},{"Examine ELF File", &examine}, {"Print Section Names", &notImplemented},
                        {"Print Symbols", &notImplemented},{"Relocation Tables", &notImplemented},{"Quit", &quit}};
    while(1){
        if(myState->debug_mode == '1') {
            if(myState->displayMode)
                printf("Debug: unit_size: %X file_name: %s mem_count: %X\n", myState->unit_size, myState->file_name, myState->mem_count);
            else
                printf("Debug: unit_size: %d file_name: %s mem_count: %zu\n", myState->unit_size, myState->file_name, myState->mem_count);

        }

        printf("Chose a function number:\n");
        for(int i = 0;  i < 9 ; i++)
            printf("%i- %s\n", i, funcs[i].name);

        char input[2048];
        if(fgets(input,  sizeof (input), stdin) != NULL){
            int index;
            sscanf(input ,"%d", &index);
            funcs[index].fun(myState);
        }
    }