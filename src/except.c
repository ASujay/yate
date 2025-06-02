#include "except.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

Except_Frame *Except_stack = NULL;

void Except_raise(const Except_T *exception, const char *file, int line){
    Except_Frame *p = Except_stack;
    assert(exception);
    if(p == NULL){
        // announce an uncaught exception
        fprintf(stderr, "Uncaught exception");
        if(exception->reason)
            fprintf(stderr, "%s", exception->reason);
        else 
            fprintf(stderr, " at 0x%p", exception);
        if(file && line > 0)
            fprintf(stderr, " raised at %s:%d\n", file, line);
        fprintf(stderr, "aborting...\n");
        fflush(stderr);
        abort();
    }

    p->exception = exception;
    p->file = file;
    p->line = line;
    Except_stack = Except_stack->prev;
    longjmp(p->env, Except_raised);
}


