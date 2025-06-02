#include "stack.h"
#include <stdio.h>
#include "arena.h"


int main(void){
    int arr[] = {1, 2, 3, 4, 5, 6};
    Stack_T stk = Stack_new();
    for(int i = 0; i < 6; i++){
        Stack_push(stk, &arr[i]);
    }

    while(!Stack_empty(stk)){
        void *val = Stack_pop(stk);
        printf("%d\n", *((int *)val));
    }
}
