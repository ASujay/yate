#include "mem.h"
#include "except.h"
#include "utils.h"
#include "assert.h"

const Except_T Mem_Failed = { "Allocation Failed" };

#ifdef PROD_IMPL

void *Mem_alloc(uint64 nbytes, const char *file, int line){
    void *ptr;
    
    assert(nbytes > 0);
    ptr = malloc(nbytes);
    if(ptr == NULL){
        if(file == NULL)
            RAISE(Mem_Failed);
        else
            Except_raise(&Mem_Failed, file, line);
    }
    return ptr;
}

void *Mem_calloc(uint64 count, uint64 nbytes, 
        const char *file, int line){
    void *ptr;
    assert(count > 0);
    assert(nbytes > 0);
    ptr = calloc(count, nbytes);
    if(ptr == NULL){
        if(file == NULL)
            RAISE(Mem_Failed);
        else
            Except_raise(&Mem_Failed, file, line);
    }
    return ptr;
}

void Mem_free(void *ptr, const char *file, int line){
    if(ptr)
        free(ptr);
}

void *Mem_resize(void *ptr, uint64 nbytes, 
        const char *file, int line){
    assert(ptr);
    assert(nbytes > 0);
    ptr = realloc(ptr, nbytes);
    if(ptr == NULL){
        if(file == NULL)
            RAISE(Mem_Failed);
        else
            Except_raise(&Mem_Failed, file, line);
    }
    return ptr;
}

#endif

#define CHECK_IMPL
#ifdef CHECK_IMPL
// The function exported byt the checking implementation of the Mem interface 
// catch the access error of the ptr supplied and report them as checked runtime errors.
// Mem_free and Mem_resize can detect access errors if Mem_alloc, Mem_calloc, and 
// Mem_resize never return the same address twice and if they remember all of the
// address they do return and which ones refer to allocated memory.

static struct descriptor {
    struct descriptor *free;            // This is the link to the freelist of descriptors
    struct descriptor *link;            // This is a list of descriptors for blocks that hash to the same index in htab
    const void *ptr;                    // Address of the block, which is allocated elsewhere
    size_t size;                        // size of the block
    const char *file;                   // file and line are allocation coordinates
    int line;                           // allocation coordinates mean where the block was allocated
} *htab[2048];

// checking types
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


// checking macros
#define hash(p, t) (((uint64)(p)>>3) & \
        (sizeof (t)/sizeof((t)[0])-1))

#define NDESCRIPTORS 512

#define NALLOC ((4096 + sizeof(union align) - 1)/ \
        (sizeof(union align)))*(sizeof(union align))

// data
// checking data
// These descriptors form a list of free blocks; the head of this list is the dummy descriptor.
// The list is threaded through the free field of the descriptors.
// This list is circular: freelist is the last descriptor on the list and its free
// fields points to the first descriptor.
// the following syntax is a common idiom of creating a circular linked list
// this is called self initialization
// this only works if the first field of the struct descriptor which is free to point to itself
// freelist->free = &freelist;
static struct descriptor freelist = { &freelist };


// checking functions
static struct descriptor *find(const void *ptr){
    struct descriptor *bp = htab[hash(ptr, htab)];
    while(bp && bp->ptr != ptr)
        bp = bp->link;
    return bp;
}

void Mem_free(void *ptr, const char *file, int line){
    if(ptr){
        struct descriptor *bp;
        // we check the memory alignment
        if(((uint64)ptr)%(sizeof(union align)) != 0 
            || (bp = find(ptr)) == NULL || bp->free)
            Except_raise(&Assert_Failed, file, line);
        bp->free = freelist.free;
        freelist.free = bp;
    }
}

void *Mem_resize(void *ptr, uint64 nbytes,
        const char *file, int line){
    struct descriptor *bp;
    void *newptr;

    assert(ptr);
    assert(nbytes > 0);
    if(((uint64)ptr)%(sizeof(union align)) != 0 
            || (bp = find(ptr)) == NULL || bp->free)
            Except_raise(&Assert_Failed, file, line);
    newptr = Mem_alloc(nbytes, file, line);
    memcpy(newptr, ptr, 
            nbytes < bp->size ? nbytes : bp->size);
    Mem_free(ptr, file, line);
    return newptr;
}

void *Mem_calloc(uint64 count, uint64 nbytes, const char *file,
        int line){
    void *ptr;

    assert(count > 0);
    assert(nbytes > 0);
    ptr = Mem_alloc(count*nbytes, file, line);
    memset(ptr, '\0', count*nbytes);
    return ptr;
}


// Here we get one descriptor out of the 512 descriptors that was allocated
static struct descriptor *dalloc(void *ptr, uint64 size,
        const char *file, int line){
    static struct descriptor *avail;
    static int nleft;
    if(nleft <= 0){
        avail = malloc(NDESCRIPTORS*sizeof(*avail));
        if(avail == NULL)
            return NULL;
        nleft = NDESCRIPTORS;
    }
    avail->ptr = ptr;
    avail->size = size;
    avail->file = file;
    avail->line = line;
    avail->free = avail->link = NULL;
    nleft--;
    return avail++;
}

// Mem_alloc allocates a block of memoru using the first-fit algorithm, one of many
// memory allocation algorithms.
// It searches freelist for the first free block that is large enough to satisfy the 
// request and divides that block to fill the request.
// If freelist doesn't contain a suitable block Mem_alloc calls malloc to allocate a chunk 
// of memory that's larger than nbytes, add this chunk onto the free list and tries again.
// Since the new chunk is larger that nbytes, it is used to fill the request the second time around.
void *Mem_alloc(uint64 nbytes, const char *file, int line){
    struct descriptor *bp;
    void *ptr;

    assert(nbytes > 0);
    nbytes = ((nbytes + sizeof(union align) - 1) / 
            (sizeof(union align))) * (sizeof(union align));
    for(bp = freelist.free; bp; bp = bp->free){
        // we find th suitable descriptor in the freelist
        if(bp->size > nbytes){
            bp->size -= nbytes;
            ptr = (char *)bp->ptr + bp->size; // moving the current pointer in the block
            if((bp = dalloc(ptr, nbytes, file, line)) != NULL){
                uint64 h = hash(ptr, htab);
                bp->link = htab[h];
                htab[h] = bp;
                return ptr;
            } else {
                Except_raise(&Mem_Failed, file, line);
            }
        }
        if(bp == &freelist){
            struct descriptor *newptr;
            if((ptr = malloc(nbytes + NALLOC)) == NULL ||
                        (newptr = dalloc(ptr, nbytes + NALLOC, __FILE__, __LINE__)) == NULL)
                    Except_raise(&Mem_Failed, __FILE__,  __LINE__);
            newptr->free = freelist.free;
            freelist.free = newptr;
        }
    }
    assert(0);
    return NULL;
}

#endif

