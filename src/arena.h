#ifndef __ARENA__H__
#define __ARENA__H__

#include "utils.h"
#include <pthread.h>

#define DEFAULT_ARENA_SIZE (1 << 12)    // 4 KB

typedef struct Arena Arena;
typedef struct ArenaManager ArenaManager;


struct Arena {
    size_t offset;
    size_t capacity;
    uint8 *data;
    Arena *next;
};

struct ArenaManager {
    Arena *free_arenas;
    Arena *active_arenas;
    size_t arena_size;
};

void memory_system_init(ArenaManager *manager);
void arena_init(Arena* arena, size_t arena_size);
void *alloc(ArenaManager* manager, size_t size, size_t align);

#endif