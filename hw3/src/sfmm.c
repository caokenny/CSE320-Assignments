/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EINVAL 22
#define ENOMEM 12
void *firstAllocation(size_t size);
void coalescBlocks(sf_header *newHeader, sf_footer *newFooter);
void *multiplePageAllocations(size_t size);

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
    if (size >= PAGE_SZ && seg_free_list[0].head == NULL && seg_free_list[1].head == NULL && seg_free_list[2].head == NULL && seg_free_list[3].head == NULL)
        return multiplePageAllocations(size);
    else if (get_heap_start() == NULL && get_heap_end() == NULL)
        return firstAllocation(size);
    else {
        int checkThisFirst = 0;
        //Check to see which list we should be checking first according to the size given
        if (size <= LIST_1_MAX) checkThisFirst = 0;
        else if (size >= LIST_2_MIN && size <= LIST_2_MAX) checkThisFirst = 1;
        else if (size >= LIST_3_MIN && size <= LIST_3_MAX) checkThisFirst = 2;
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
                    if (footerPointer + 1 != get_heap_end()) {
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
                        *freeHeaderPtr = newFreeHeader;

                        sf_footer freeFooter;
                        sf_footer *freeFooterPointer = (sf_footer*)freeHeaderPtr;
                        freeFooterPointer += ((newFreeHeader.header.block_size << 4) - 8)/8;
                        freeFooter.allocated = newFreeHeader.header.allocated;
                        freeFooter.padded = newFreeHeader.header.padded;
                        freeFooter.requested_size = 0;
                        freeFooter.two_zeroes = 0;
                        freeFooter.block_size = newFreeHeader.header.block_size;
                        *freeFooterPointer = freeFooter;

                        if (seg_free_list[i].head->next == NULL && seg_free_list[i].head->prev == NULL)
                            seg_free_list[i].head = NULL;
                        else {
                            sf_free_header *pointToAList = seg_free_list[i].head->next;
                            while (pointToAList != NULL){
                                if ((pointToAList->header.block_size << 4) == header.block_size << 4){
                                    pointToAList->prev->next = pointToAList->next;
                                    break;
                                } else {
                                    pointToAList = pointToAList->next;
                                }
                            }
                        }

                        int placeIntoThisList = 0;

                        if(remainingUnusedBytes >= LIST_1_MIN && remainingUnusedBytes <= LIST_1_MAX) placeIntoThisList = 0;
                        else if (remainingUnusedBytes >= LIST_2_MIN && remainingUnusedBytes <= LIST_2_MAX) placeIntoThisList = 1;
                        else if (remainingUnusedBytes >= LIST_3_MIN && remainingUnusedBytes <= LIST_3_MAX) placeIntoThisList = 2;
                        else placeIntoThisList = 3;

                        if (seg_free_list[placeIntoThisList].head != NULL) {
                            seg_free_list[placeIntoThisList].head->prev = freeHeaderPtr;
                            freeHeaderPtr->next = seg_free_list[placeIntoThisList].head;
                            seg_free_list[placeIntoThisList].head = freeHeaderPtr;
                            freeHeaderPtr->prev = NULL;
                        } else {
                            seg_free_list[placeIntoThisList].head = freeHeaderPtr;
                        }
                        return (sf_header*)freeHeader + 1;
                    } else {
                        if (freeHeader == seg_free_list[i].head){
                            if (freeHeader->next == NULL) {
                                seg_free_list[i].head = NULL;
                                return (sf_header*)freeHeader + 1;
                            } else {
                                seg_free_list[i].head = freeHeader->next;
                                seg_free_list[i].head->prev->next = NULL;
                                seg_free_list[i].head->prev = NULL;
                                return (sf_header*)freeHeader + 1;
                            }
                        } else {
                            if (freeHeader->next == NULL) {
                                freeHeader->prev->next = NULL;
                                freeHeader->prev = NULL;
                                return (sf_header*)freeHeader + 1;
                            } else {
                                freeHeader->prev->next = freeHeader->next;
                                freeHeader->next->prev = freeHeader->prev;
                                freeHeader->next = NULL;
                                freeHeader->prev = NULL;
                                return (sf_header*)freeHeader + 1;
                            }
                        }
                    }
                }
                else {
                    freeHeader = freeHeader->next;
                }
            }
        }
        int x = 1;
        if (size <= PAGE_SZ) x = 1;
        else if (size <= PAGE_SZ*2) x = 2;
        else if (size <= PAGE_SZ*3) x = 3;
        else x = 4;
        if ((get_heap_end() - get_heap_start() + PAGE_SZ * x) > (PAGE_SZ * 4)) {
            sf_errno = ENOMEM;
            return NULL;
        }
        //Anything that runs after this means we couldn't find a suitable memory block
        sf_header *headPointer = sf_sbrk();
        int i = 1;
        size_t totalBlockSize = PAGE_SZ * i;
        while (1){
            if ((headPointer - 1)->allocated == 0){ //if prev block's allocated bit is 0
                if (((headPointer - 1)->block_size << 4) + (PAGE_SZ * i) < size + 16){ //if prev block's size + PAGE_SZ * i is less than size, allocate more space
                    if (get_heap_end() - get_heap_start() + PAGE_SZ > (PAGE_SZ * 4)) {
                        sf_errno = ENOMEM;
                        return NULL;
                    }
                    sf_sbrk();
                    i++;
                    continue;
                }
                //Now that we have enough space, we coalesce
                //Get header of previous block of original brk
                int removeFromHere = 0;
                if ((headPointer - 1)->block_size << 4 <= LIST_1_MAX) removeFromHere = 0;
                else if ((headPointer - 1)->block_size << 4 >= LIST_2_MIN && (headPointer - 1)->block_size << 4 <= LIST_2_MAX) removeFromHere = 1;
                else if ((headPointer - 1)->block_size << 4 > LIST_3_MIN && (headPointer - 1)->block_size << 4 <= LIST_3_MAX) removeFromHere = 2;
                else removeFromHere = 3;

                if (seg_free_list[removeFromHere].head->next == NULL && seg_free_list[removeFromHere].head->prev == NULL)
                    seg_free_list[removeFromHere].head = NULL;
                else if (seg_free_list[removeFromHere].head->header.block_size << 4 == (headPointer - 1)->block_size << 4) {
                    seg_free_list[removeFromHere].head = seg_free_list[removeFromHere].head->next;
                    seg_free_list[removeFromHere].head->prev->next = NULL;
                    seg_free_list[removeFromHere].head->prev = NULL;
                }
                else {
                    sf_free_header *pointToANode = seg_free_list[removeFromHere].head->next;
                    while (pointToANode != NULL) {
                        if (pointToANode->header.block_size << 4 == (headPointer - 1)->block_size << 4) {
                            pointToANode->prev->next = pointToANode->next;
                            if (pointToANode->next != NULL)
                                pointToANode->next->prev = pointToANode->prev;
                        } else {
                            pointToANode = pointToANode->next;
                        }
                    }
                }

                totalBlockSize = ((headPointer - 1)->block_size << 4) + (PAGE_SZ * i);
                headPointer -= ((headPointer - 1)->block_size << 4)/8; //This should take us to the previous block's header

                break;
            } else {
                if ((PAGE_SZ * i) < size + 16){ //if prev block's size + PAGE_SZ * i is less than size, allocate more space
                    if (get_heap_end() - get_heap_start() + PAGE_SZ > (PAGE_SZ * 4)) {
                        sf_errno = ENOMEM;
                        return NULL;
                    }
                    sf_sbrk();
                    i++;
                    continue;
                }

                totalBlockSize = PAGE_SZ * i;

                break;
            }
        }
        header.allocated = 1; //Set allocate bit to 1
        if ((size % 16) != 0){ //If size isn't a multiple of 16 we need to pad memory to align it
            padding = 16 - (size%16);
            header.padded = 1;
        }else header.padded = 0;
        header.two_zeroes = 0; //Set two zeroes
        header.block_size = 16 + size + padding; //block_size = 16[header + footer] + requested size + padding
        header.block_size = header.block_size >> 4; //Store it and shift right by 4
        header.unused = 0; //Set unused bits

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
        int remainingUnusedBytes = totalBlockSize - (header.block_size<<4); //Remaining free bytes is the Page_Sz - our block_size

        //set correct bits for a free header
        sf_free_header freeHeaderReg;
        freeHeaderReg.header.allocated = 0;
        freeHeaderReg.header.padded = 0;
        freeHeaderReg.header.unused = 0;
        freeHeaderReg.header.two_zeroes = 0;
        freeHeaderReg.header.block_size = (remainingUnusedBytes>>4);
        freeHeaderReg.next = NULL;
        freeHeaderReg.prev = NULL;

        footerPointer += 1;
        sf_free_header* freeHeaderPtr = (sf_free_header*)footerPointer;
        *freeHeaderPtr = freeHeaderReg;

        sf_footer freeFooter;
        sf_footer *freeFooterPointer = (sf_footer*)freeHeaderPtr;
        freeFooterPointer += ((freeHeaderReg.header.block_size << 4) - 8)/8;
        freeFooter.allocated = freeHeaderReg.header.allocated;
        freeFooter.padded = freeHeaderReg.header.padded;
        freeFooter.requested_size = 0;
        freeFooter.two_zeroes = 0;
        freeFooter.block_size = freeHeaderReg.header.block_size;
        *freeFooterPointer = freeFooter;

        if(remainingUnusedBytes >= LIST_1_MIN && remainingUnusedBytes <= LIST_1_MAX) seg_free_list[0].head = freeHeaderPtr;
        else if (remainingUnusedBytes >= LIST_2_MIN && remainingUnusedBytes <= LIST_2_MAX) seg_free_list[1].head = freeHeaderPtr;
        else if (remainingUnusedBytes >= LIST_3_MIN && remainingUnusedBytes <= LIST_3_MAX) seg_free_list[2].head = freeHeaderPtr;
        else seg_free_list[3].head = freeHeaderPtr;

        return headPointer + 1;
    }
	return NULL;
}

void *sf_realloc(void *ptr, size_t size) {
    if (ptr == NULL) abort();
    sf_header *newHeader = ptr;
    sf_footer *newFooter = ptr;
    newHeader -= 1;
    newFooter -= 1;
    newFooter += ((newHeader->block_size << 4) - 8)/8;
    sf_footer *oldFooter = newFooter;
    if (newHeader < (sf_header*) get_heap_start() || (newFooter + 1) > (sf_footer*) get_heap_end()) abort();
    if (newHeader->allocated == 0 || newFooter->allocated == 0) abort();
    if (newFooter->requested_size + 16 != newFooter->block_size << 4){
        if (newFooter->padded != 1 && newHeader->padded != 1) abort();
    }
    if (newHeader->allocated != newFooter->allocated || newHeader->padded != newFooter->padded)
        abort();
    if (size == 0){
        sf_free(ptr);
        return NULL;
    }
    if (size > (newHeader->block_size << 4) - 16) { //Reallocating to a larger size
        sf_header *newAllocatedMemoryHeader = (sf_header*)sf_malloc(size);
        if (newAllocatedMemoryHeader == NULL) return NULL;
        newHeader += 1;
        memcpy(newAllocatedMemoryHeader, newHeader, (((newHeader - 1)->block_size << 4) - 16));
        sf_free(ptr);
        return newAllocatedMemoryHeader;
    } else { //Reallocating to a smaller size
        size_t ogPayloadSize = (newHeader->block_size << 4) - 16;
        int padding = 0;
        if (ogPayloadSize - size >= LIST_1_MIN) { //split the block
            newHeader->allocated = 1;
            if (size % 16 != 0) {
                padding = 16 - (size%16);
                newHeader->padded = 1;
            } else newHeader->padded = 0;
            newHeader->block_size = 16 + size + padding;
            newHeader->block_size = newHeader->block_size >> 4;
            newFooter = (sf_footer*) newHeader;
            newFooter += ((newHeader->block_size << 4) - 8)/8;
            newFooter->allocated = newHeader->allocated;
            newFooter->block_size = newHeader->block_size;
            newFooter->padded = newHeader->padded;
            newFooter->requested_size = size;
            newFooter += 1;
            sf_header *freeHeader = (sf_header*)newFooter;
            freeHeader->block_size = (oldFooter->block_size) - (newHeader->block_size);
            oldFooter->block_size = freeHeader->block_size;
            freeHeader->allocated = 1;
            oldFooter->allocated = 1;
            freeHeader->padded = 1;
            oldFooter->padded = 1;
            sf_free(freeHeader + 1);
            return newHeader + 1;
        } else { //Don't split the block
            newFooter->requested_size = size;
            return newHeader + 1;
        }
    }
	return NULL;
}

void sf_free(void *ptr) {
    if (ptr == NULL) abort(); //if pointer is null abort
    sf_header *newHeader = ptr; //header pointer = ptr address
    sf_footer *newFooter = ptr; //footer pointer = ptr address
    newHeader -= 1; //decrement to get header address
    newFooter -= 1; //decrement to get header address
    newFooter += ((newHeader->block_size<<4) - 8)/8; //go to footer address
    //if header starts before heap abort, if block ends after heap abort
    if (newHeader < (sf_header*)get_heap_start() || (newFooter + 1) > (sf_footer*)get_heap_end()) abort();
    //if header or footer allocated bit is 0 abort
    if (newHeader->allocated == 0 || newFooter->allocated == 0) abort();
    if (newFooter->requested_size + 16 != newFooter->block_size << 4){
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
    newHeader->padded = 0;
    newFooter->padded = 0;
    size_t freeBlockSize = newHeader->block_size << 4;
    int placeIntoThisList = 0;
    if (freeBlockSize <= LIST_1_MAX) placeIntoThisList = 0;
    else if (freeBlockSize >= LIST_2_MIN && freeBlockSize <= LIST_2_MAX) placeIntoThisList = 1;
    else if (freeBlockSize >= LIST_3_MIN && freeBlockSize <= LIST_3_MAX) placeIntoThisList = 2;
    else placeIntoThisList = 3;

    sf_free_header *freeHeader = (sf_free_header*)newHeader;

    if (seg_free_list[placeIntoThisList].head == NULL) seg_free_list[placeIntoThisList].head = freeHeader;
    else{
        sf_free_header *freeHeaderHolder = seg_free_list[placeIntoThisList].head;
        seg_free_list[placeIntoThisList].head = freeHeader;
        freeHeader->next = freeHeaderHolder;
        freeHeader->prev = NULL;
        freeHeaderHolder->prev = freeHeader;
    }
	return;
}

void coalescBlocks(sf_header *newHeader, sf_footer *newFooter){
    size_t newBlockSize = (newFooter + 1)->block_size << 4; //Go into header of next block, grab block_size
    size_t oldFreeBlockSize = (newFooter +1)->block_size << 4;
    newBlockSize += newHeader->block_size<<4; //add next header block size to current block size
    newHeader->block_size = newBlockSize >> 4;
    newFooter = (sf_footer*)newHeader;
    newFooter += ((newHeader->block_size<<4) - 8)/8;
    newFooter->block_size = newHeader->block_size;
    int removeFromThisList = 0;
    if (oldFreeBlockSize <= LIST_1_MAX) removeFromThisList = 0;
    else if (oldFreeBlockSize >= LIST_2_MIN && oldFreeBlockSize <= LIST_2_MAX) removeFromThisList = 1;
    else if (oldFreeBlockSize >= LIST_3_MIN && oldFreeBlockSize <= LIST_3_MAX) removeFromThisList = 2;
    else removeFromThisList = 3;

    if (seg_free_list[removeFromThisList].head->next == NULL) seg_free_list[removeFromThisList].head = NULL;
    else if (seg_free_list[removeFromThisList].head->header.block_size << 4 == oldFreeBlockSize) {
        seg_free_list[removeFromThisList].head = seg_free_list[removeFromThisList].head->next;
        seg_free_list[removeFromThisList].head->prev->next = NULL;
        seg_free_list[removeFromThisList].head->prev = NULL;
    }
    else {
        sf_free_header *freeHeaderPointer = seg_free_list[removeFromThisList].head->next;
        while (freeHeaderPointer != NULL) {
            if (freeHeaderPointer->header.block_size << 4 == oldFreeBlockSize){
                if (freeHeaderPointer->next == NULL) {
                    freeHeaderPointer->prev->next = NULL;
                    freeHeaderPointer->prev = NULL;
                }
                else {
                    freeHeaderPointer->prev->next = freeHeaderPointer->next;
                    freeHeaderPointer->next->prev = freeHeaderPointer->prev;
                    freeHeaderPointer->next = NULL;
                    freeHeaderPointer->prev = NULL;
                }
                break;
            } else {
                freeHeaderPointer = freeHeaderPointer->next;
            }
        }
    }
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
    if (footerPointer + 1 != get_heap_end()) {
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
        *freeHeaderPtr = freeHeader;

        sf_footer freeFooter;
        sf_footer *freeFooterPointer = (sf_footer*)freeHeaderPtr;
        freeFooterPointer += ((freeHeader.header.block_size << 4) - 8)/8;
        freeFooter.allocated = freeHeader.header.allocated;
        freeFooter.padded = freeHeader.header.padded;
        freeFooter.requested_size = 0;
        freeFooter.two_zeroes = 0;
        freeFooter.block_size = freeHeader.header.block_size;
        *freeFooterPointer = freeFooter;

        if(remainingUnusedBytes >= LIST_1_MIN && remainingUnusedBytes <= LIST_1_MAX) seg_free_list[0].head = freeHeaderPtr;
        else if (remainingUnusedBytes >= LIST_2_MIN && remainingUnusedBytes <= LIST_2_MAX) seg_free_list[1].head = freeHeaderPtr;
        else if (remainingUnusedBytes > LIST_3_MIN && remainingUnusedBytes <= LIST_3_MAX) seg_free_list[2].head = freeHeaderPtr;
        else seg_free_list[3].head = freeHeaderPtr;
    }

    return headPointer + 1;
}

void *multiplePageAllocations(size_t size){
    int padding = 0;
    sf_header *headPointer = sf_sbrk();
    for (int i = 0; i < size/PAGE_SZ; i++)
        sf_sbrk(); //now we should have enough heap space to work with
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
    if (footerPointer + 1 != get_heap_end()) {
        int remainingUnusedBytes = (PAGE_SZ * ((size + PAGE_SZ)/PAGE_SZ)) - (header.block_size<<4); //Remaining free bytes is the Page_Sz - our block_size

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
        *freeHeaderPtr = freeHeader;

        sf_footer freeFooter;
        sf_footer *freeFooterPointer = (sf_footer*)freeHeaderPtr;
        freeFooterPointer += ((freeHeader.header.block_size << 4) - 8)/8;
        freeFooter.allocated = freeHeader.header.allocated;
        freeFooter.padded = freeHeader.header.padded;
        freeFooter.requested_size = 0;
        freeFooter.two_zeroes = 0;
        freeFooter.block_size = freeHeader.header.block_size;
        *freeFooterPointer = freeFooter;

        if(remainingUnusedBytes >= LIST_1_MIN && remainingUnusedBytes <= LIST_1_MAX) seg_free_list[0].head = freeHeaderPtr;
        else if (remainingUnusedBytes >= LIST_2_MIN && remainingUnusedBytes <= LIST_2_MAX) seg_free_list[1].head = freeHeaderPtr;
        else if (remainingUnusedBytes >= LIST_3_MIN && remainingUnusedBytes <= LIST_3_MAX) seg_free_list[2].head = freeHeaderPtr;
        else seg_free_list[3].head = freeHeaderPtr;
    }

    return headPointer + 1;
}
