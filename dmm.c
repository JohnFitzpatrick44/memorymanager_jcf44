#include <stdio.h>  // needed for size_t
#include <stdlib.h>
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;

/*  Method that splits a freelist entry into one chunk at ptr
 *  with size numbytes, and another after that with the rest of the bytes
 */
void split(metadata_t *ptr, size_t numbytes) {
  metadata_t* newHead = (void*) ptr + ALIGN(numbytes) + METADATA_T_ALIGNED;  // Makes new head, leaving enough room for numbytes
  newHead->size = ptr->size - ALIGN(numbytes) - METADATA_T_ALIGNED;          // Allocates it the rest of ptr's space

  newHead->next = ptr->next;    // Places it in list
  newHead->prev = ptr;
  if(ptr->next != NULL) (ptr->next)->prev = newHead;
  ptr->next = newHead;

  ptr->size = ALIGN(numbytes);
}
 
void* dmalloc(size_t numbytes) {
  /* initialize through sbrk call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }
  assert(numbytes > 0);

  metadata_t* traverser = freelist;   // Goes through freelist to find open block
  while(traverser != NULL) {          // If no block found, return NULL
    if(traverser->size >= ALIGN(numbytes)) {
      if(traverser->size > ALIGN(numbytes) +  METADATA_T_ALIGNED) {   // If room for another block (+ room for metadata), split
        split(traverser, numbytes);
      }
      if(traverser->prev != NULL) (traverser->prev)->next = traverser->next;  // Remove block from list
      if(traverser->next != NULL) (traverser->next)->prev = traverser->prev;

      if(traverser == freelist) freelist = traverser->next;   // If head of freelist, move freelist pointer

      return (void*) traverser + METADATA_T_ALIGNED;          // Return pointer to data after metadata
    }
    traverser = traverser->next;    // Moves pointer through list
  }
  return NULL;
}

void dfree(void* ptr) {
  metadata_t* released = (void*) ptr - METADATA_T_ALIGNED;  // Grabs metadata from pointer
  if(released < freelist) {
    released->prev = NULL;
    released->next = freelist;
    freelist->prev = released;
    freelist = released;
  } else {
    metadata_t* traverser = freelist;
    while(traverser != NULL) {
      if (traverser->next == NULL) {    // If freed pointer is at end of list - special case
          traverser->next = released;
          released->prev = traverser;
          released->next = NULL;
          break;
      } else if(traverser->next > released) {   // Place in list found, after traverser
        if(released == traverser) return;
        released->next = traverser->next;
        (traverser->next)->prev = released;
        released->prev = traverser;
        traverser->next = released;
        break;
      }
      traverser = traverser->next;
    }
  }

  // Coalescing happends every time free is called
  bool prevBlockFree = false;   // Checks for free blocks immediately next and previous of released
  bool nextBlockFree = false;
  if(released->prev != NULL) prevBlockFree = ((void*) released == (void*) released->prev + (released->prev)->size + METADATA_T_ALIGNED);
  if(released->next != NULL) nextBlockFree = ((void*) released + released->size + METADATA_T_ALIGNED == (void*) released->next);

  if(prevBlockFree && nextBlockFree) {    // Case where released is between two free blocks
    if((released->next)->next != NULL) ((released->next)->next)->prev = released->prev;
    (released->prev)->size += (released->size + (released->next)->size + 2*METADATA_T_ALIGNED);
    (released->prev)->next = (released->next)->next;
    (released->next)->next = NULL;    // Remove from list, expand first entry, set others to NULL/0
    (released->next)->prev = NULL;
    (released->next)->size = 0;
    released->next = NULL;
    released->prev = NULL;
    released->size = 0;
  } else if(prevBlockFree) {  // Previous block free
    (released->prev)->size += (released->size + METADATA_T_ALIGNED);
    if(released->next != NULL) (released->next)->prev = released->prev;
    (released->prev)->next = released->next;
    released->next = NULL;
    released->prev = NULL;
    released->size = 0;
  } else if(nextBlockFree) {  // Next block free
    released->size += ((released->next)->size + METADATA_T_ALIGNED);
    if((released->next)->next != NULL) ((released->next)->next)->prev = released;
    metadata_t* tempPointer = released->next->next;
    if(released->next!=NULL) {
      (released->next)->next = NULL;
    (released->next)->prev = NULL;
    released->next->size = 0;}
    released->next = tempPointer;
  }
}




bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) sbrk(max_bytes); 
  /* Q: Why casting is used? i.e., why (void*)-1? */
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
