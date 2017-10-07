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
void *firstAllocation(size_t size);
void coalescBlocks(sf_header *newHeader, sf_footer *newFooter);

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
    if (size <= 0 || size > (PAGE_SZ*4)){ //if invalid size set sf_errno to EINVAL and return NULL
        sf_errno = EINVAL;
        return NULL;
    }
    if ((size + 16) > (PAGE_SZ*4)){
        sf_errno = ENOMEM;
        return NULL;
    }
    //If all lists point to NULL then this is our first allocation
    if (seg_free_list[0].head == NULL && seg_free_list[1].head == NULL && seg_free_list[2].head == NULL && seg_free_list[3].head == NULL)
        return firstAllocation(size);
    else {
        int checkThisFirst = 0;
        //Check to see which list we should be checking first according to the size given
        if (size < LIST_1_MAX) checkThisFirst = 0;
        else if (size > LIST_1_MAX && size < LIST_2_MIN) checkThisFirst = 1;
        else if (size > LIST_2_MAX && size < LIST_3_MIN) checkThisFirst = 2;
        else checkThisFirst = 3;
        //freeHeader points to the head of seg_free_list[checkThisFirst]
        sf_free_header *freeHeader;
        sf_header header;
        int padding = 0;
        for (int i = checkThisFirst; i < 4; i++){
            freeHeader = seg_free_list[i].head;
            while (freeHeader != NULL){
                if ((freeHeader->header.block_size << 4) >= (size + 16)){ //If the header we're checking has enough space, allocate it
                    //allocate memory
                    header = freeHeader->header;
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
                    int remainingUnusedBytes = (freeHeader->header.block_size << 4) - header.block_size;
                    sf_footer footer;
                    sf_footer *footerPointer = (sf_footer*) freeHeader;
                    footer.allocated = header.allocated;
                    footer.padded = header.padded;
                    footer.two_zeroes = 0;
                    footer.block_size = header.block_size;
                    footer.requested_size = size;
                    freeHeader->header = header;
                    footerPointer += ((header.block_size<<4) - 8)/8;
                    *footerPointer = footer;

                    sf_free_header newFreeHeader;
                    newFreeHeader.header.allocated = 0;
                    newFreeHeader.header.padded = 0;
                    newFreeHeader.header.unused = 0;
                    newFreeHeader.header.two_zeroes = 0;
                    newFreeHeader.header.block_size = (remainingUnusedBytes>>4);
                    newFreeHeader.next = NULL;
                    newFreeHeader.prev = NULL;

                    footerPointer += 1;
                    sf_free_header* freeHeaderPtr = (sf_free_header*)footerPointer;
                    //freeHeaderPtr += 1;
                    *freeHeaderPtr = newFreeHeader;

                    if(remainingUnusedBytes > LIST_1_MIN && remainingUnusedBytes < LIST_1_MAX) seg_free_list[0].head = freeHeaderPtr;
                    else if (remainingUnusedBytes > LIST_1_MAX && remainingUnusedBytes < LIST_2_MAX) seg_free_list[1].head = freeHeaderPtr;
                    else if (remainingUnusedBytes > LIST_2_MAX && remainingUnusedBytes < LIST_3_MAX) seg_free_list[2].head = freeHeaderPtr;
                    else seg_free_list[3].head = freeHeaderPtr;
                    return (sf_header*)freeHeader + 1;
                }
                else {
                    freeHeader = freeHeader->next;
                }
            }
        }
    }
	return NULL;
}

void *sf_realloc(void *ptr, size_t size) {
	return NULL;
}

void sf_free(void *ptr) {
    sf_header *newHeader = ptr;
    sf_footer *newFooter = ptr;
    newHeader -= 1;
    newFooter += ((newHeader->block_size<<4) - 8)/8;
    printf("%d\n", (newHeader->block_size));
    if (newHeader == NULL) abort();
    if (newHeader < (sf_header*)get_heap_start() || (newFooter + 1) > (sf_footer*)get_heap_end()) abort();
    if (newHeader->allocated == 0 || newFooter->allocated == 0) abort();
    if (newFooter->requested_size + 16 != newFooter->block_size){
        if (newFooter->padded != 1) abort();
    }
    if (newHeader->allocated != newFooter->allocated || newHeader->padded != newFooter->padded)
        abort();
    newHeader->allocated = 0;
    newFooter->allocated = 0;
    if ((newFooter + 1)->allocated == 0) coalescBlocks(newHeader, newFooter);
	return;
}

void coalescBlocks(sf_header *newHeader, sf_footer *newFooter){
    return;
}

void *firstAllocation(size_t size){
    int padding = 0;
    sf_header *headPointer = sf_sbrk();
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
    sf_footer *footerPointer = (sf_footer*)headPointer;
    footer.allocated = header.allocated;
    footer.padded = header.padded;
    footer.two_zeroes = 0;
    footer.block_size = header.block_size;
    footer.requested_size = size;
    *headPointer = header;
    footerPointer += ((header.block_size<<4) - 8)/8;
    *footerPointer = footer;
    int remainingUnusedBytes = PAGE_SZ - header.block_size;

    sf_free_header freeHeader;
    freeHeader.header.allocated = 0;
    freeHeader.header.padded = 0;
    freeHeader.header.unused = 0;
    freeHeader.header.two_zeroes = 0;
    freeHeader.header.block_size = (remainingUnusedBytes>>4);
    freeHeader.next = NULL;
    freeHeader.prev = NULL;

    footerPointer += 1;
    sf_free_header* freeHeaderPtr = (sf_free_header*)footerPointer;
    //freeHeaderPtr += 1;
    *freeHeaderPtr = freeHeader;

    if(remainingUnusedBytes > LIST_1_MIN && remainingUnusedBytes < LIST_1_MAX) seg_free_list[0].head = freeHeaderPtr;
    else if (remainingUnusedBytes > LIST_1_MAX && remainingUnusedBytes < LIST_2_MAX) seg_free_list[1].head = freeHeaderPtr;
    else if (remainingUnusedBytes > LIST_2_MAX && remainingUnusedBytes < LIST_3_MAX) seg_free_list[2].head = freeHeaderPtr;
    else seg_free_list[3].head = freeHeaderPtr;

    return headPointer + 1;
}
