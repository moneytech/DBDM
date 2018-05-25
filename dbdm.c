#include "dbdm.h"

/* DYNAMIC MEMORY ALLOCATION IMPLEMENTATION
*  Dynamic memory allocation functions such as m/re/c-alloc and free are not
* implemented on the Z8
*  architechture.
*  This .c file is an implementation of a buffer-based (which functions as a
* heap) dynamic memory
* management system.
*  For proof of concept, a buffer of 128 bytes has been allocated. In theory,
* the algorithms would
*  apply equally well
*  to larger buffers, assuming that the memNode_t* struct contains unsigned
* integers, instead of
*  chars (chars will limit memory address differences to be
* 255*sizeof(memNode_t)).
*  Freeing memory is equally supported. This contains algorithms for coalescing
*  free memory blocks, to reduce fragmentation of the buffer.
*/

#define DBUFFER_SIZE 128 // Total size in bytes of the memory buffer

// Initialize a 128 byte data buffer
static memNode_t dBuffer[DBUFFER_SIZE / sizeof(memNode_t)];

// buffer contains a pointer to the tail of the allocated data inside the buffer
static memNode_t* dB_tail;

// Initializer function for memory mangement. Sets the tail to the first
// position of the heap.
// Next is set to NULL, indicating that no free blocks are in the free block
// list.
// This resembles a linked list implementation - we implement the "linking" by
// saving the
// memory address difference between two free blocks.
// Linked list is not implemented, to avoid a pointer inside the memNode_t
// structs, thus keeping the
// allocation block size to 2 bytes.
void dbdm_init() {
    dB_tail = dBuffer;
    dB_tail->size = (DBUFFER_SIZE / sizeof(memNode_t)) - 1;
    dB_tail->next = 0;
}

// Function taking requested byte size as argument, and returns a pointer to a
// position in memory
// where nBytes is allowed to be written
void* dbdm_malloc(char nBytes) {
    memNode_t* curr_alloc;
    memNode_t* before;

    char nNodes; // number of memNodes that the users requested memory
                 // allocation fits inside

    // In case user have requested an amt. of bytes not divisible by the size of
    // the allocation
    // blocks (memNodes),
    // we calculate the required allocation size in sizes of memNodes
    // Below we calculate ceil((nBytes + sizeof(memNode_t)) / sizeof(memNode_t))
    // to round up the
    // amt. of nodes required
    nNodes = (nBytes + sizeof(memNode_t) - 1) / sizeof(memNode_t);

    // 1. Traverse through the free block list until an exact match is found
    curr_alloc = dB_tail;
    while (curr_alloc->size != nNodes && curr_alloc->next != 0) {
        before = curr_alloc;
        curr_alloc = curr_alloc - curr_alloc->next;
    }

    // If an exact match was found
    if (curr_alloc->size == nNodes) {

        // Set the before pointers' next free allocation, to match the free
        // block which
        // the matched block referred to, to clear the now allocated memory
        // block from the free
        // list.
        // To get the correct interval, we add the interval towards and away
        // from the block that has
        // just been found.
        if (curr_alloc->next = 0) {
            before->next = 0;
        } else {
            before->next += curr_alloc->next;
        }
        return curr_alloc + 1; // return a pointer to the memory allocated
                               // memory - one beyond the header which
                               // describes the allocation
    }

    // 2. Allocate new memory from the data buffer tail, if data buffer tail is
    // big enough
    if (dB_tail->size >= nNodes) {
        curr_alloc = dB_tail;
        dB_tail += nNodes + 1; // Increment tail corresponding to the new
                               // allocation and its header

        // change the info in the tail to match the allocation that has been
        // decided upon
        dB_tail->size =
          curr_alloc->size - nNodes - 1; // minus 1 to account for the header
        dB_tail->next =
          curr_alloc->next == 0 ? 0 : (curr_alloc->next + nNodes + 1);

        // Edit the current allocation header according to the new allocation
        curr_alloc->size = nNodes; // minus one to account for the inclusion of
                                   // the header in nNodes
        return curr_alloc + 1;     // Return a pointer to the allocated memory
    }

    // 3. If no free memory is available at the tail, search for a free block
    // that is
    //    big enough to be split. This is the "last resort" allocation, which
    //    will increase
    //    fragmentation of the buffer, if split blocks are unavailable for
    //    coaliscion.

    // Check if the free block list is not empty
    if (dB_tail->next == 0) {
        return 0;
    };

    // Iterate through the free block list, to find a free block of size greater
    // or equal to nNodes
    curr_alloc = dB_tail;
    before = curr_alloc;
    while (!(curr_alloc->size >= nNodes) && curr_alloc->next != 0) {
        before = curr_alloc;
        curr_alloc = curr_alloc - curr_alloc->next;
    }

    // Test if an applicable memory location has been found
    if (curr_alloc->size >= nNodes) {
        // calculate the position of the new "before" that is created by the
        // split
        before->next =
          before->next - nNodes - 1; // Set the previous' node in the free list
        // to point to the position after the newly allocated data

        // change before to be the new "before", created by the split (which
        // lies in behind the
        // allocated data)
        before = before - before->next;

        if (curr_alloc->next == 0) { // If the allocation is last in the free
                                     // list, set before to be last in free list
            before->next = 0;
        } else {
            before->next = before->next - nNodes -
                           1; // Set the previous' node in the free list
            // to point to the position after the newly allocated data
        }
        before->size = curr_alloc->size - nNodes - 1;

        // Edit the current allocation header according to the new allocation
        curr_alloc->size = nNodes;

        // Return a pointer to the allocated memory
        return curr_alloc + 1;
    }

    // Memory could not be allocated, return NULL
    return 0;
}

void dbdm_free(void* memory) {
    memNode_t* released =
      (memNode_t*)memory - 1; // Get the header of the released memory
    memNode_t* beforeBefore; // needed when coalescing with memory blocks before
                             // the currently
                             // released memory
    memNode_t* before;
    memNode_t* after;

    before = dB_tail;
    beforeBefore = 0;

    // Iterate through the free list until the end is found or when detecting
    // that the next free
    // block lies infront of the currently released block
    // By this, we find the position in the free list where our free block
    // should reside
    while (before->next != 0 && (before - before->next) > released) {
        beforeBefore = before;
        before -= before->next;
    }

    // Insert the released memory into the free list, if the currently released
    // memory does not
    // reside at the end of the free list
    if (before->next != 0) {
        after =
          before -
          before->next; //  Save the current next free block (a block which
                        //  will be infront of the released memory)
        before->next = before - released;  // Calculate the memory adress
                                           // difference between before and
                                           // released
        released->next = released - after; // Calculate the memory address
                                           // difference between released and
                                           // after
    } else { // Released memory must reside in the end of the free list
        before->next = before - released;
        released->next = 0;
        after = 0;
    }

    // COALESCING

    // Initially, we check if we can coalesce the memory with the block after
    // (infront of) it.
    // This requires that there is no allocated memory between the released
    // block and the free block
    // after it, and that the released block is not last in list

    if (after != 0 && ((after + after->size + 1) == released)) {
        before->next = before - after;
        after->size += released->size + 1;
    }

    // Check if the released memory can be coalesced with the block that lies
    // before it.
    // This requires that there is no allocated memory between the released
    // block and the free block
    // before it.
    if (released + released->size + 1 == before) {
        // If released memory lies adjecant to the tail, move the tail backwards
        if (before = dB_tail) {
            dB_tail->size += released->size + 1;
            dB_tail->next = released->next;
            dB_tail -= released->size - 1;
        } else {
            beforeBefore->next += released->size + 1;
            released->size += before->size + 1;
        }
    }
}
