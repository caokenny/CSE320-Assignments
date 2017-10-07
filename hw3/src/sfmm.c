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
                    if ((size % 16) != 0){
                        padding = 16 - (size%16);
                        header.padded = 1;
                    }else header.padded = 0;
                    header.two_zeroes = 0;
                    header.block_size = 16 + size + padding;
                    header.block_size = header.block_size >> 4;
                    header.unused = 0;
                    int remainingUnusedBytes = (freeHeader->header.block_size << 4) - (header.block_size<<4);

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
    sf_header *newHeader = ptr; //header pointer = ptr address
    sf_footer *newFooter = ptr; //footer pointer = ptr address
    newHeader -= 1; //decrement to get header address
    newFooter -= 1; //decrement to get header address
    newFooter += ((newHeader->block_size<<4) - 8)/8; //go to footer address
    if (newHeader == NULL) abort(); //if pointer is null abort
    //if header starts before heap abort, if block ends after heap abort
    if (newHeader < (sf_header*)get_heap_start() || (newFooter + 1) > (sf_footer*)get_heap_end()) abort();
    //if header or footer allocated bit is 0 abort
    if (newHeader->allocated == 0 || newFooter->allocated == 0) abort();
    if (newFooter->requested_size + 16 != newFooter->block_size){
        if (newFooter->padded != 1 && newHeader->padded != 1) abort();
    }
    //if header and footer padded/allocated bits are inconsistent abort
    if (newHeader->allocated != newFooter->allocated || newHeader->padded != newFooter->padded)
        abort();
    //set header and footer allocated bits to 0
    newHeader->allocated = 0;
    newFooter->allocated = 0;
    //if next block is free coalesc
    if ((newFooter + 1)->allocated == 0) coalescBlocks(newHeader, newFooter);
    size_t freeBlockSize = newHeader->block_size << 4;
    int placeIntoThisList = 0;
    if (freeBlockSize < LIST_1_MAX) placeIntoThisList = 0;
    else if (freeBlockSize > LIST_1_MAX && freeBlockSize < LIST_2_MAX) placeIntoThisList = 1;
    else if (freeBlockSize > LIST_2_MAX && freeBlockSize < LIST_3_MAX) placeIntoThisList = 2;
    else placeIntoThisList = 3;

    sf_free_header *freeHeader = (sf_free_header*)newHeader;

    if (seg_free_list[placeIntoThisList].head == NULL) seg_free_list[placeIntoThisList].head = freeHeader;
	return;
}

void coalescBlocks(sf_header *newHeader, sf_footer *newFooter){
    size_t newBlockSize = (newFooter + 1)->block_size << 4; //Go into header of next block, grab block_size
    newBlockSize += newHeader->block_size; //add next header block size to current block size
    newHeader->block_size = newBlockSize >> 4;
    newFooter = (sf_footer*)newHeader;
    newFooter += ((newHeader->block_size<<4) - 8)/8;
    newFooter->block_size = newBlockSize >> 4;
    int removeFromThisList = 0;
    size_t coalescBlockSize = newHeader->block_size << 4;
    if (coalescBlockSize < LIST_1_MAX) removeFromThisList = 0;
    else if (coalescBlockSize > LIST_1_MAX && coalescBlockSize < LIST_2_MAX) removeFromThisList = 1;
    else if (coalescBlockSize > LIST_2_MAX && coalescBlockSize < LIST_3_MAX) removeFromThisList = 2;
    else removeFromThisList = 3;

    if (seg_free_list[removeFromThisList].head->next == NULL) seg_free_list[removeFromThisList].head = NULL;
    return;
}

void *firstAllocation(size_t size){
    int padding = 0;
    sf_header *headPointer = sf_sbrk(); //Get previous brk
    sf_header header;

    header.allocated = 1; //Set allocate bit to 1
    if ((size % 16) != 0){ //If size isn't a multiple of 16 we need to pad memory to align it
        padding = 16 - (size%16);
        header.padded = 1;
    }else header.padded = 0;
    header.two_zeroes = 0; //Set two zeroes
    header.block_size = 16 + size + padding; //block_size = 16[header + footer] + requested size + padding
    header.block_size = header.block_size >> 4; //Store it and shift right by 4
    header.unused = 0; //Set unused bits
    /*if ((header.block_size << 4) < LIST_1_MIN){
        header.block_size = LIST_1_MIN >> 4;
        header.padded = 1;
    }*/

    sf_footer footer;
    sf_footer *footerPointer = (sf_footer*)headPointer; //set footer pointer to point to the address headPointer is pointing to
    footer.allocated = header.allocated; //Set allocated bit of footer to 1
    footer.padded = header.padded; //Set the padding of footer to same as header
    footer.two_zeroes = 0; //Set two zeroes
    footer.block_size = header.block_size; //Footer block_size == Header block_size
    footer.requested_size = size; //set requested size
    *headPointer = header; //dereference headPointer and make its contents = header.
    footerPointer += ((header.block_size<<4) - 8)/8; //Get the memory address to correctly place the footer
    *footerPointer = footer; //dereference footerPointer and make its contects = footer.
    int remainingUnusedBytes = PAGE_SZ - (header.block_size<<4); //Remaining free bytes is the Page_Sz - our block_size

    //set correct bits for a free header
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