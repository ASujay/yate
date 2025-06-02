#include "assert.h"
#include "except.h"
#include <assert.h>

const Except_T Assert_Failed = {"Assertion failed"};

// the parenthesis around assert in the function name supresses the macro expansion
void (assert)(int e){
    assert(e);
}
