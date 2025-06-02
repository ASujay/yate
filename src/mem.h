#ifndef __MEM_H__
#define __MEM_H__

#include "except.h"
#include "utils.h"
#include "assert.h"

// exported exceptions
extern const Except_T Mem_Failed;

// exported functions
// Mem_alloc allocates a block of atleat nbytes and returnsa pointer to
// the first byte. The block is aligned on an addressing boundary.
// The contents of the block are uninitialized.
// It is a checked runtime error for nbytes to be non-positive.
extern void *Mem_alloc(uint64 nbytes, 
        const char *file, int line);

// Mem_calloc allocate a block large enough to hold an array of count elements 
// each of size nbytes and returns pointer to the first element.
// It is a checked runtime error for nbytes to be non-positive.
extern void *Mem_calloc(uint64 count, uint64 nbytes, 
        const char *file, int line);

// This takes a pointer to the block to be deallocated. If ptr is non-null,
// Mem_free deallocates that block; if ptr is null, Mem_free has no effect.
// The FREE macro also takes a pointer to a block, call Mem_free to deallocate
// the block, and sets the ptr to the null pointer.
// There are two implementations that export the Mem interface. 
// The checking implementation implements checked runtime errors to help catch
// access errors.
// It is a checked runtime error to pass Mem_free a nonnull ptr that was not 
// returned by a previous call to Mem_alloc, Mem_calloc, or Mem_resize.
// Or a ptr that has already been passed to Mem_free or Mem_resize.
// In the production implementation, however, these access errors are unchecked 
// runtime errors.
extern void Mem_free(void *ptr, const char *file, int line);


extern void *Mem_resize(void *ptr, uint64 nbytes, 
        const char *file, int line);

// exported macros
#define ALLOC(nbytes) \
    Mem_alloc((nbytes), __FILE__, __LINE__)

#define CALLOC(count, nbytes) \
    Mem_calloc((count), (nbytes), __FILE__, __LINE__)

#define RESIZE(ptr, nbytes) ((ptr) = Mem_resize(ptr), \
        (nbytes), __FILE__, __LINE__)

#define FREE(ptr) ((void)(Mem_free((ptr), \
                __FILE__, __LINE__), (ptr) = 0))

// Common allocation idiom
// Wrong: 
// struct T *p;
// p = Mem_alloc(sizeof(struct T));       
// the above is not good as the no of bytes allocated is dependent on the type of p
// if type of p is changed then we must change the argument to Mem_alloc as well.
//
// Correct:
// p = Mem_alloc(sizeof(*p))


#define NEW(p) ((p) = ALLOC((uint64)sizeof(*(p))))
#define NEW0(p) ((p) = CALLOC(1, (uint64)sizeof(*(p)))) 

#endif
