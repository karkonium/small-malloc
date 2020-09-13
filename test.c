/* This is a test case which tests that no blocks which have size 0 remain
 * on the freelist linked list. This is interesting because it rarely ever 
 * happens that the first block is the one that which meets the size requirements
 * excatly. And there is little chance of detecting this problem because it would 
 * just be ignored as the size would never be enough.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"

#define SIZE 100

/* Simple test for smalloc and sfree. */

int main(void) {

    mem_init(SIZE);

    char *ptrs[10];
    int i;

    /* Call smalloc 3 times */

    for (i = 0; i < 3; i++) {
        int num_bytes = (i + 1) * 7;

        ptrs[i] = smalloc(num_bytes);
        write_to_mem(num_bytes, ptrs[i], i);
    }

    printf("List of allocated blocks:\n");
    print_allocated();

    printf("List of free blocks:\n");
    print_free();

    printf("Contents of allocated memory:\n");
    print_mem();

    printf("freeing %p result = %d\n", ptrs[1], sfree(ptrs[1]));

    printf("List of allocated blocks:\n");
    print_allocated();

    printf("List of free blocks:\n");
    print_free();

    printf("Contents of allocated memory:\n");
    print_mem();

    printf("adding a new block of excatly 16 bits\n");
    ptrs[1] = smalloc(16);

    printf("List of allocated blocks:\n");
    print_allocated();

    printf("List of free blocks:\n");
    print_free();

    printf("Contents of allocated memory:\n");
    print_mem();
    mem_clean();
    
    return 0;
}
