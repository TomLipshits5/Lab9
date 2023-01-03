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

char* getShrType(int d) {
    switch (d){
        case 0:
            return "NULL";
            break;
        case 1:
            return "PROGBITS";
            break;
        case 2:
            return "SYMTAB";
            break;
        case 3:
            return "STRTAB";
            break;
        case 4:
            return "RELA";
            break;
        case 5:
            return "HASH";
            break;
        case 6:
            return "DYNAMIC";
            break;
        case 7:
            return "NOTE";
            break;
        case 8:
            return "NOBITS";
            break;
        case 9:
            return "REL";
            break;
        case 10:
            return "SHLIB";
            break;
        case 11:
            return "DYNSYM";
            break;
        case 14:
            return "INIT_ARRAY";
            break;
        case 15:
            return "SHT_NULL";
            break;
        case 16:
            return "SHT_NULL";
            break;
        case 17:
            return "SHT_NULL";
            break;
        case 18:
            return "SHT_NULL";
            break;
        case 19:
            return "SHT_NULL";
            break;
    }
    return "SHT_NULL";
}

void printSectionNames(state* s){
    int offset = s->head->e_shoff;
    int sectionNum = s->head->e_shnum;
    Elf32_Shdr* stringTable = s->fileData + offset + (s->head->e_shstrndx * s->head->e_shentsize);
    printf("[index] sh_name    sh_addr   sh_offset    sh_size   sh_type\n");
    for (int i = 0; i < sectionNum; ++i) {
        Elf32_Shdr* sectionHeader = s->fileData + offset + (i * s->head->e_shentsize);
        char* name = (char*)(s->fileData + stringTable->sh_offset + sectionHeader->sh_name);
        printf("[%2d] %-15s %-10x %-10x %-10x %-15s\n", i, name, sectionHeader->sh_addr, sectionHeader->sh_offset, sectionHeader->sh_size,
               getShrType(sectionHeader->sh_type));

    }
}

void printSymbolNames(state* s){
    int offset = s->head->e_shoff;
    int sectionNum = s->head->e_shnum;
    Elf32_Shdr*  stringTable = s->fileData + offset + (s->head->e_shstrndx * s->head->e_shentsize);
    Elf32_Shdr* symTableHeader;
    Elf32_Shdr* strtHeader;
    printf("[index] val  sec_idx     sec_name         symbol_name\n");
    int symbolIndex = 0;
    for (int i = 0; i < sectionNum; ++i) {
        Elf32_Shdr* sectionHeader = s->fileData + offset + (i * s->head->e_shentsize);
        char* name = (char*)(s->fileData + stringTable->sh_offset + sectionHeader->sh_name);
        if(strcmp(name, ".symtab") == 0){
            symTableHeader = sectionHeader;
        }else if(strcmp(name, ".strtab") == 0){
            strtHeader = sectionHeader;
        }
    }
    for (int i = 0; i < symTableHeader->sh_size / sizeof(Elf32_Sym); ++i) {
        Elf32_Sym* sym = s->fileData + symTableHeader->sh_offset + (i * sizeof(Elf32_Sym));
        char* sectionName = "";
        if(sym->st_shndx == 0xFFF1){
            sectionName = "ABS";
        }else if(sym->st_shndx == 0x0){
            sectionName = "UND";
        }else{
            Elf32_Shdr* section = s->fileData + offset + (sym->st_shndx * s->head->e_shentsize);
            sectionName = (char*)(s->fileData + stringTable->sh_offset + section->sh_name);
        }
        char* symName = (char*)(s->fileData + strtHeader->sh_offset + sym->st_name);
        printf("[%2d] %-10x %-10d %-15s %-15s \n", symbolIndex, sym->st_value, sym->st_shndx, sectionName, symName);
        symbolIndex++;
    }
}

void quit(state* s){
    if(s->debug_mode){
        printf("QUITING\n");
    }
    free(s);
    exit(0);
}



void relocationTable(state* s){
    int offset = s->head->e_shoff;
    int sectionNum = s->head->e_shnum;
    Elf32_Shdr*  stringTable = s->fileData + offset + (s->head->e_shstrndx * s->head->e_shentsize);
    Elf32_Shdr* dynSymTable;
    Elf32_Shdr* dynStrTable;
    for (int i = 0; i < sectionNum; ++i) {
        Elf32_Shdr* sectionHeader = s->fileData + offset + (i * s->head->e_shentsize);
        char* name = (char*)(s->fileData + stringTable->sh_offset + sectionHeader->sh_name);
        if(sectionHeader->sh_type == SHT_DYNSYM){
            dynSymTable = sectionHeader;
        }else if(strcmp(name, ".dynstr") == 0){
            dynStrTable = sectionHeader;
        }
    }
    printf("Offset  Info  Type  Sym.Value  Sym.Name\n");
    for (int i = 0; i < sectionNum; ++i) {
        Elf32_Shdr* sectionHeader = s->fileData + offset + (i * s->head->e_shentsize);
        if(sectionHeader->sh_type == SHT_REL){
            for (int j = 0; j < sectionHeader->sh_size / sizeof(Elf32_Rel); ++j) {
                Elf32_Rel* rel = s->fileData + sectionHeader->sh_offset + (j * sizeof(Elf32_Rel));
                int symbolInfo = ELF32_R_SYM(rel->r_info);
                Elf32_Sym* sym = s->fileData + dynSymTable->sh_offset + symbolInfo;
                char* symName = (char*)(s->fileData + dynStrTable->sh_offset + sym->st_name);
                printf("%-10x  %-10x  %-15u  %-10x  %-15s\n", rel->r_offset, rel->r_info, ELF32_R_TYPE(rel->r_info), sym->st_value, symName);
            }
        }
    }
}



int main(int argc, char** argv) {
    state *myState = calloc(1, sizeof(state));
    myState->description = 0;
    funDesc funcs[6] = {{"Toggle Debug Mode",   &toggleDebugMode},
                        {"Examine ELF File",    &examine},
                        {"Print Section Names", &printSectionNames},
                        {"Print Symbols",       &printSymbolNames},
                        {"Relocation Tables",   &relocationTable},
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
