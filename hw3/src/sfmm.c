/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>

#define EINVAL 22
#define ENOMEM 12

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {
    if (size <= 0 || size > 16384){ //if invalid size set sf_errno to EINVAL and return NULL
        sf_errno = EINVAL;
        return NULL;
    }
    int padding = 0;
    if (size <= LIST_1_MAX){ //if size is less than list 1 max check first list for free block
        if (seg_free_list[0].head == NULL){
            sf_header *heapPointer = sf_sbrk();
            sf_header header;
            header.allocated = 1;
            if ((size % 8) != 0){
                padding = 8 - (size%8);
                header.padded = 1;
            }else header.padded = 0;
            header.two_zeroes = 0;
            header.block_size = 16 + size + padding;
            header.unused = 0;
            if (header.block_size < LIST_1_MIN){
                header.block_size = LIST_1_MIN;
                header.padded = 1;
            }
            sf_footer footer;
            sf_footer *footerPointer = get_heap_start();
            footer.allocated = header.allocated;
            footer.padded = header.padded;
            footer.two_zeroes = 0;
            footer.block_size = header.block_size;
            footer.requested_size = size;
            *heapPointer = header;
            footerPointer += footer.block_size - 8;
            *footerPointer = footer;
            sf_blockprint(get_heap_start());
            return get_heap_start() + 8;
        }
    }
    //else if(size <= LIST_2_MAX){//if size is less than list 2 max check second list for free block

    //}
    //else if (size <= LIST_3_MAX){//if size is less than list 3 max check third list for free block

    //}
    //else {//if size is greater than list 3 max check last list for free block

    //}

	return NULL;
}

void *sf_realloc(void *ptr, size_t size) {
	return NULL;
}

void sf_free(void *ptr) {
	return;
}
