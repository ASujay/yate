#ifndef __ARENA_H__
#define __ARENA_H__

#include "except.h"
#include "utils.h"

#define T Arena_T
typedef struct T *T;

extern const Except_T Arena_NewFailed;
extern const Except_T Arena_Failed;

// exported functions
extern T Arena_new(void);
extern void Arena_dispose(T *ap);

extern void *Arena_alloc(T arena, uint64 nbytes,
        const char *file, int line);
extern void *Arena_calloc(T arena, uint64 count, uint64 nbytes,
        const char *file, int line);
extern void Arena_free(T arena);


#undef T
#endif
