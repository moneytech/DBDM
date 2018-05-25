#include "dbdm.h"
#include <stdio.h>

#define nallocs 10
#define nallocsize 10

typedef struct { char* allocs[nallocs]; } memtest_t;

void main() {
    memtest_t memTest;
    char i, j;
    char* inspector;
    dbdm_init();

    // Set all of the test allocation pointer to null. This facilitates
    // debugging:
    for (i = 0; i < nallocs; i++) {
        memTest.allocs[i] = NULL;
    }

    // Allocate request 10x10 bytes = 100 bytes. 10 allocations require 10
    // headers, which is 20 bytes.
    // Total allocation size is 120 bytes, using pretty much the entire buffer
    printf("\nMemory addresses; none should be null: \n");
    for (i = 0; i < nallocs; i++) {
        memTest.allocs[i] = dbdm_malloc(nallocsize);
        printf("%d ,", memTest.allocs[i]);
    }


    // write the number 123 to all allocations
    // We will use this sequence to verify that blocks are not getting
    // overwritten
    for (i = 0; i < nallocs; i++) {
        for (j = 0; j < nallocsize; j++) {
            memTest.allocs[i][j] = 123;
        }
        for (j = 0; j < nallocsize; j++) {
        }
    }

    // print it all
    printf("\nContent of all allocations after inserting test data: \n");
    for (i = 0; i < nallocs; i++) {
        if (memTest.allocs[i] != NULL) {
            printf("Alloc %d: ", i);
            for (j = 0; j < nallocsize; j++) {
                printf("%d ", memTest.allocs[i][j]);
            }
            printf("\n");
        }
    }

    printf("\n");

    // Free a single block of memory
    dbdm_free(memTest.allocs[3]);

    // Free some more memory that is adjescant to each other, but not to the
    // memory just released
    dbdm_free(memTest.allocs[5]);
    dbdm_free(memTest.allocs[6]);
    dbdm_free(memTest.allocs[7]);

    // Request some memory of the same size as initial free block - it should
    // return a pointer to the first free'd location
    // since the request is an exact match, and the first free block cannot be
    // coalesced with anything
    inspector = dbdm_malloc(nallocsize);
    for (j = 0; j < nallocsize; j++) {
        inspector[j] = 8;
    }

    // Request memory that is thrice the size of nallocsize (Since the tail does
    // not contain any more
    // space, this is only possible if coalescing works). This should return a
    // pointer to the same place that memTest.allocs[5] freed
    // Fill it with all 9's
    inspector = dbdm_malloc(3 * nallocsize);
    for (j = 0; j < 3 * nallocsize; j++) {
        inspector[j] = 9;
    }

    // print the contents of all allocations
    printf(
      "Content of all allocations after freeing memory, and rewriting to it: "
      "\n");
    for (i = 0; i < 4; i++) {
        printf("Alloc %d: ", i);
        for (j = 0; j < nallocsize; j++) {
            printf("%d ", memTest.allocs[i][j]);
        }
        printf("\n");
    }
    printf("Alloc 4: ");
    for (j = 0; j < nallocsize; j++) {
        printf("%d ", memTest.allocs[4][j]);
    }
    printf("\n");
    printf("Large 3*nallocisze alloc: ", i);
    for (j = 0; j < 3 * nallocsize; j++) {
        printf("%d ", inspector[j]);
    }
    printf("\n");
    for (i = 0; i < 2; i++) {
        printf("Alloc %d: ", i + 8);
        for (j = 0; j < nallocsize; j++) {
            printf("%d ", memTest.allocs[8 + i][j]);
        }
        printf("\n");
    }
    printf("\n");

    // Request 10 more bytes - should return null pointer, since the buffer
    // close to full
    inspector = dbdm_malloc(nallocsize);
    printf("Requesting more memory returns a null pointer: %d", inspector);
}
