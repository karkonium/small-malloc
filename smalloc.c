#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"

void *mem;
struct block *freelist;
struct block *allocated_list;

/* Return 0 if block with addr value ptr was successfully
 * deleted, 1 otherwise 
 */
int delete_block(struct block **head_ptr, void *ptr) {

	if (*head_ptr == NULL) {
		return 1;
	}

	struct block *curr = *head_ptr;
	struct block *pvr = *head_ptr;

	// find block that has ptr as a addr member
	if (curr->addr == ptr) {
		// if head is to be deleted, must update head value
		*head_ptr = curr->next;
		free(curr);
		return 0;
	} else {
		curr = curr->next;

		while (curr != NULL) {
			if (curr->addr == ptr) {
				pvr->next = curr->next;
				free(curr);
				return 0;
			}
			pvr = curr;
			curr = curr->next;
		}
	}
	return 1;
}

/* Return 0 if block insertion at the head was successful, 
 * 1 otherwise
 */
int insert_head(struct block **head_ptr, void *ptr, unsigned int nbytes) {

	struct block *new = malloc(sizeof(struct block));
	if (new == NULL)
		return 1;

	new->addr = ptr;
	new->size = nbytes;
	new->next = *head_ptr;
	*head_ptr = new;
	return 0;
}

/* Return 0 if ordered block insertion based on ptr was successful, 
 * 1 otherwise
 */
int insert_ordered(struct block **head_ptr, void *ptr, unsigned int nbytes) {

	struct block *curr = *head_ptr;
	struct block *pvr = *head_ptr;

	// if insertion is at the curr
	if (curr == NULL || ptr < curr->addr) {
		return insert_head(head_ptr, ptr, nbytes);
	} else {
		// create new block
		struct block *new = malloc(sizeof(struct block));
		if (new == NULL)
			return 1;
		new->addr = ptr;
		new->size = nbytes;
		new->next = NULL;

		// find where to inset
		curr = curr->next;
		while (curr != NULL && !(ptr < curr->addr)) {
			pvr = curr;
			curr = curr->next;
		}

		// insert into linked list
		new->next = curr;
		pvr->next = new;
	}

	return 0;
}

/* Return the first block that has size of nbytes or more,
 * if no block has enough space return NULL
 */
struct block *findblock_size(struct block *head, unsigned int nbytes) {

	while (head != NULL) {
		if (nbytes <= head->size) {
			return head;
		}
		head = head->next;
	}

	return NULL;
}

/* Return the block that has addr value of ptr,
 * if no block has that addr value return NULL
 */
struct block *findblock_addr(struct block *head, void *ptr) {

	while (head != NULL) {
		if (head->addr == ptr)
			return head;
		head = head->next;
	}

	return NULL;
}

/* Reserves nbytes of space from the memory region created by mem_init.  Returns
 * a pointer to the reserved memory. Returns NULL if memory cannot be allocated 
 */
void *smalloc(unsigned int nbytes) {

	if (nbytes == 0)
		return NULL;

	// round nbytes up to the nearset mutliple of 8
	nbytes = ((nbytes + 7) / 8) * 8;

	// Find if freelist contains memory block with enough space
	struct block *to_split = findblock_size(freelist, nbytes);

	// If not enough space, then return NULL
	if (to_split == NULL)
		return NULL;

	void *ptr = to_split->addr;

	// create new block for allocated_list
	insert_head(&allocated_list, ptr, nbytes);

	// update freelist
	if (to_split->size == nbytes) {
		// if freelist block's size becomes 0 ie excat match for size, delete block
		delete_block(&freelist, ptr);
	} else {
		// else update addr of to_split block
		to_split->addr = to_split->addr + nbytes;
	}

	to_split->size = to_split->size - nbytes;
	
	return ptr;
}

/* Free the reserved space starting at addr.  Returns 0 if successful 
 * -1 if the address cannot be found in the list of allocated blocks 
 */
int sfree(void *addr) {

	// find block with addr value of addr
	struct block *to_remove = findblock_addr(allocated_list, addr);

	// if not successful return -1
	if (to_remove == NULL)
		return -1;

	// keep its size
	int size = to_remove->size;

	// remove block from allocated_list
	delete_block(&allocated_list, addr);

	// insert block into freelist
	insert_ordered(&freelist, addr, size);

	return 0;
}

/* Initialize the memory space used by smalloc,
 * freelist, and allocated_list
 */
void mem_init(int size) {

	mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	// initialize  allocated_list
	allocated_list = NULL;

	// initialize freelist
	freelist = malloc(sizeof(struct block));
	freelist->addr = mem;
	freelist->size = size;
	freelist->next = NULL;
}

/* Free any dynamically used memory in the allocated and free list */
void mem_clean() {

	struct block *lists[2] = {freelist, allocated_list};

	for (int i = 0; i < 2; i++) {
		struct block *curr = lists[i];
		struct block *next = NULL;

		while (curr != NULL) {
			// first extract next value
			next = curr->next;
			// free current block
			free(curr);
			// move on to next block
			curr = next;
		}
	}
}
