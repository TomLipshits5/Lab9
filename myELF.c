#include <stdlib.h>
#include <stdio.h>
#include "string.h"


typedef struct {
    char* name;
    void (*fun)(state*);
}funDesc;






void notImplemented(state* s){
    printf("NOT IMPLEMENTED YET");
}




int main(int argc, char** argv){
    state* myState = calloc(1,sizeof (state));
    funDesc funcs[9] = {{"Toggle Debug Mode", &toggleDebugMode},{"Set File Name", &setFileName}, {"Set Unit Size", &setUnitSize},
                        {"Load Into Memory", &loadToMemory},{"Toggle Display Mode", &toggleDisplayMode},{"Memory Display", &memoryDisplay},
                        {"Save Into File", &saveIntoFile}, {"Memory Modify", &memoryModify},{"Quit", &quit}};
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