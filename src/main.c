#include "arena.h"
#include <stdio.h>

int main(int argc, char **argv){
    ArenaManager* manager = malloc(sizeof(ArenaManager));
    memory_system_init(manager);
    int *ptr = alloc(manager, sizeof(int *), sizeof(int *));
    *ptr = 2;
    printf("Memory slot: %p\nValue: %d\n", ptr, *ptr);    
}
