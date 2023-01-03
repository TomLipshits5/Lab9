#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>


typedef struct {
    int debug_mode;
    char file_name[128];
    int description;
    size_t fileSize;
    void* fileData;
    Elf32_Ehdr* head;
} state;

typedef struct {
    char* name;
    void (*fun)(state*);
}funDesc;

void notImplemented(state* s){
    printf("NOT IMPLEMENTED YET");
}

void toggleDebugMode(state* s) {
    if (s->debug_mode) {
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    } else {
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
}

void closeFile(state* s){
    if(s->description != 0){
        munmap(s->fileData, s->fileSize);
        close(s->description);
    }
    s->description = 0;
}

char* getDataType(state* s){
    char data = s->head->e_ident[5];
    if (data == ELFDATANONE)
        return "Invalid data encoding";
    else if(data == ELFDATA2LSB){
        return "2's complement, little endian";
    }else if(data == ELFDATA2MSB){
        return "2's complement, big endian";
    }else{
        return "NO DATA";
    }
}

void printInfo(state* s){
    printf("MAGIC NUMBER: %X, %X, %X\n", s->head->e_ident[EI_MAG0], s->head->e_ident[EI_MAG1], s->head->e_ident[EI_MAG2]);
    printf("ENTRY POINT: %X\n", s->head->e_entry);
    printf("DATA TYPE: %s\n", getDataType(s));
    printf("SECTION HEADER OFFSET: %d\n", s->head->e_shoff);
    printf("NUMBER SECTIONS: %d\n", s->head->e_shnum);
    printf("SIZE OF SECTION HEADERS: %d\n", s->head->e_shentsize);
    printf("PROGRAM HEADER OFFSET: %d\n", s->head->e_phoff);
    printf("NUMBER PROGRAMS: %d\n", s->head->e_phnum);
    printf("SIZE OF PROGRAM HEADERS: %d\n", s->head->e_phentsize);
    printf("\n");
}

void examine(state* s){
    printf("please enter file name\n");
    char file_name[100];
    char input[10000];
    fgets(input, sizeof(input), stdin);
    if(sscanf(input, "%s", file_name) > 0) {
        if(strcmp(file_name, s->file_name)!=0) {
            strcpy(s->file_name, file_name);
            if(s->description != 0){
                closeFile(s);
            }
            int description= open(s->file_name, O_RDWR);
            if(description < 0)
                printf("Error: file %s does not exist.\n", s->file_name);


            s->description = description;
            struct stat st;
            if(fstat(description, &st) != 0)
                printf("Error: file %s can't get stat. /n", s->file_name);
            s->fileSize = st.st_size;
            void *address = mmap(0, s->fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, description, 0);
            if (address == MAP_FAILED)
                printf("Error mapping file to memory");
            else{
                s->fileData = address;
                s->head = (Elf32_Ehdr*) address;
            }
        }
        if(strncmp((char*)s->head->e_ident, (char*)ELFMAG, 4) == 0){
            printInfo(s);
        } else{
            printf("Error: provided file is not an ELF file, file name: %s", s->file_name);
            closeFile(s);
        }
    }
}

void quit(state* s){
    if(s->debug_mode){
        printf("QUITING\n");
    }
    free(s);
    exit(0);
}



int main(int argc, char** argv) {
    state *myState = calloc(1, sizeof(state));
    myState->description = 0;
    funDesc funcs[6] = {{"Toggle Debug Mode",   &toggleDebugMode},
                        {"Examine ELF File",    &examine},
                        {"Print Section Names", &notImplemented},
                        {"Print Symbols",       &notImplemented},
                        {"Relocation Tables",   &notImplemented},
                        {"Quit",                &quit}};
    while (1) {
        printf("Chose a function number:\n");
        for (int i = 0; i < 6; i++)
            printf("%i- %s\n", i, funcs[i].name);

        char input[2048];
        if (fgets(input, sizeof(input), stdin) != NULL) {
            int index;
            sscanf(input, "%d", &index);
            funcs[index].fun(myState);
        }
    }
}
