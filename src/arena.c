#include "./arena.h"
#include "except.h"
#include "utils.h"
#include <assert.h>
#include <string.h>

#define THRESHOLD 10

#define T Arena_T

struct T {
    T prev;              // Points to the head of the chunk   
    uint8 *avail;        // points to the first available byte
    uint8 *limit;        // points to the just after the last avaiable byte

};

union align {
    int i;
    long l;
    long *lp;
    void *p;
    void (*fp)(void);
    float f;
    double d;
    long double ld;
};

union header {
    struct T b;
    union align a;
};

static T freechunks;
static int nfree;

const Except_T Arena_NewFailed = {"Arena Creation Failed"};
const Except_T Arena_Failed = {"Arena Allocation Failed"};

T Arena_new(void){
    T arena = malloc(sizeof(*arena));
    if(arena == NULL)
        RAISE(Arena_NewFailed);
    arena->prev = NULL;
    arena->limit = arena->avail = NULL;
    return arena;
}

void Arena_dispose(Arena_T *ap){
    assert(ap && *ap);
    Arena_free(*ap);    // This deallocates the memory block
    free(*ap);          // This deallocates the Arena_T object
    *ap = NULL;
}

void *Arena_alloc(T arena, uint64 nbytes,
        const char *file, int line){

    assert(arena);
    assert(nbytes > 0);

    nbytes = ((nbytes + sizeof(union align) - 1)/
            (sizeof(union align)))*(sizeof(union align));
    while(nbytes > arena->limit - arena->avail){
        T ptr;
        uint8 *limit;
        //
        *ptr = *arena;
        arena->avail = (uint8 *)((union header *)ptr + 1);
        arena->limit = limit;
        arena->prev = ptr;
    }
    arena->avail  += nbytes;
    return arena->avail - nbytes;
}

void *Arena_calloc(T arena, uint64 count, uint64 nbytes,
        const char *file, int line){
    void *ptr;
    
    assert(count > 0);
    ptr = Arena_alloc(arena, count*nbytes, file, line);
    memset(ptr, '\0', count*nbytes);
    return ptr;
}

void Arena_free(T arena){
    assert(arena);
    while(arena->prev){
        struct T tmp = *arena->prev;
        if(nfree < THRESHOLD){
            arena->prev->prev = freechunks;
            freechunks = arena->prev;
            nfree++;
            freechunks->limit = arena->limit;
        } else
            free(arena->prev);
        *arena = tmp;
    }
    assert(arena->limit == NULL);
    assert(arena->avail == NULL);
}

#undef T
