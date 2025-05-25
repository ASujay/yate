#include "./arena.h"
#include "utils.h"
#include <stdint.h>

void memory_system_init(ArenaManager *manager){
    manager->free_arenas = NULL;
    manager->active_arenas = NULL;
    manager->arena_size = DEFAULT_ARENA_SIZE;
}
void arena_init(Arena *arena, size_t size){
    arena->offset = 0;
    arena->capacity = 0;
    arena->next = NULL;
    arena->data = malloc(size >= DEFAULT_ARENA_SIZE ? size : DEFAULT_ARENA_SIZE);
}


bool can_allocate_from_arena(Arena *arena, size_t size, size_t align){
    // we need to first align the offset pointer
    uintptr_t base = (uintptr_t)(arena->data + arena->offset);
    uintptr_t aligned = (base + (align - 1)) & ~(uintptr_t)(align - 1);
    return (aligned - base) + size >= arena->capacity;
}

Arena* get_arena(ArenaManager *manager, size_t size, size_t align){
   if(manager->active_arenas && can_allocate_from_arena(
    manager->active_arenas, size, align
   )) return manager->active_arenas;
   
   // we dont have any suitable arenas in the active arenas
   // we check in the free_arenas
   Arena **prev = &manager->free_arenas;
   Arena *arena = NULL;
    
   while(*prev){
    if(can_allocate_from_arena(*prev, size, align)){
        arena = *prev;
        arena->next = NULL;
        *prev = (*prev)->next;
        return arena;
    }
    prev = &(*prev)->next; 
   }
   
   // we couldn't find any suitable arena in the free arenas
   // we shoulf allocate a new one
   arena = malloc(sizeof(Arena));
   arena_init(arena, size);
   return arena;
}

void *alloc(ArenaManager *manager, size_t size, size_t align){
    Arena *arena = get_arena(manager, size, align);
    manager->active_arenas = arena;
    
    // the arena that we have will have enough space required for the allocation
    // we first need to align the offset according to the align parameter
    uintptr_t base = (uintptr_t)(arena->data + arena->offset);
    uintptr_t aligned = (base + align - 1) & ~(uintptr_t)(align - 1); 
    arena->offset = (aligned - base) + size;
    return (uint8 *)aligned; 
}