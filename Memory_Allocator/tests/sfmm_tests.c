#include <criterion/criterion.h>
#include <errno.h>
#include <signal.h>
#include "sfmm.h"
#include <stdio.h>


int find_list_index_from_size(int sz) {
	if (sz >= LIST_1_MIN && sz <= LIST_1_MAX) return 0;
	else if (sz >= LIST_2_MIN && sz <= LIST_2_MAX) return 1;
	else if (sz >= LIST_3_MIN && sz <= LIST_3_MAX) return 2;
	else return 3;
}

Test(sf_memsuite_student, Malloc_an_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	int *x = sf_malloc(sizeof(int));

	cr_assert_not_null(x);

	*x = 4;

	cr_assert(*x == 4, "sf_malloc failed to give proper space for an int!");

	sf_header *header = (sf_header*)((char*)x - 8);

	/* There should be one block of size 4064 in list 3 */
	free_list *fl = &seg_free_list[find_list_index_from_size(PAGE_SZ - (header->block_size << 4))];

	cr_assert_not_null(fl, "Free list is null");

	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert_null(fl->head->next, "Found more blocks than expected!");
	//printf("%d\n", fl->head->header.block_size);
	cr_assert(fl->head->header.block_size << 4 == 4064);
	cr_assert(fl->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	cr_assert(get_heap_start() + PAGE_SZ == get_heap_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, Malloc_over_four_pages, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	void *x = sf_malloc(PAGE_SZ << 2);

	cr_assert_null(x, "x is not NULL!");
	cr_assert(sf_errno == ENOMEM, "sf_errno is not ENOMEM!");
}

Test(sf_memsuite_student, free_double_free, .init = sf_mem_init, .fini = sf_mem_fini, .signal = SIGABRT) {
	sf_errno = 0;
	void *x = sf_malloc(sizeof(int));
	sf_free(x);
	sf_free(x);
}

Test(sf_memsuite_student, free_no_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *x = */ sf_malloc(sizeof(long));
	void *y = sf_malloc(sizeof(double) * 10);
	/* void *z = */ sf_malloc(sizeof(char));

	sf_free(y);

	free_list *fl = &seg_free_list[find_list_index_from_size(96)];

	cr_assert_not_null(fl->head, "No block in expected free list");
	cr_assert_null(fl->head->next, "Found more blocks than expected!");
	cr_assert(fl->head->header.block_size << 4 == 96);
	cr_assert(fl->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, free_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *w = */ sf_malloc(sizeof(long));
	void *x = sf_malloc(sizeof(double) * 11);
	void *y = sf_malloc(sizeof(char));
	/* void *z = */ sf_malloc(sizeof(int));

	sf_free(y);
	sf_free(x);

	free_list *fl_y = &seg_free_list[find_list_index_from_size(32)];
	free_list *fl_x = &seg_free_list[find_list_index_from_size(144)];

	cr_assert_null(fl_y->head, "Unexpected block in list!");
	cr_assert_not_null(fl_x->head, "No block in expected free list");
	cr_assert_null(fl_x->head->next, "Found more blocks than expected!");
	cr_assert(fl_x->head->header.block_size << 4 == 144);
	cr_assert(fl_x->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	/* void *u = */ sf_malloc(1);          //32
	void *v = sf_malloc(LIST_1_MIN); //48
	void *w = sf_malloc(LIST_2_MIN); //160
	void *x = sf_malloc(LIST_3_MIN); //544
	void *y = sf_malloc(LIST_4_MIN); //2080
	/* void *z = */ sf_malloc(1); // 32

	int allocated_block_size[4] = {48, 160, 544, 2080};

	sf_free(v);
	sf_free(w);
	sf_free(x);
	sf_free(y);

	// First block in each list should be the most recently freed block
	for (int i = 0; i < FREE_LIST_COUNT; i++) {
		sf_free_header *fh = (sf_free_header *)(seg_free_list[i].head);
		cr_assert_not_null(fh, "list %d is NULL!", i);
		cr_assert(fh->header.block_size << 4 == allocated_block_size[i], "Unexpected free block size!");
		cr_assert(fh->header.allocated == 0, "Allocated bit is set!");
	}

	// There should be one free block in each list, 2 blocks in list 3 of size 544 and 1232
	for (int i = 0; i < FREE_LIST_COUNT; i++) {
		sf_free_header *fh = (sf_free_header *)(seg_free_list[i].head);
		if (i != 2)
		    cr_assert_null(fh->next, "More than 1 block in freelist [%d]!", i);
		else {
		    cr_assert_not_null(fh->next, "Less than 2 blocks in freelist [%d]!", i);
		    cr_assert_null(fh->next->next, "More than 2 blocks in freelist [%d]!", i);
		}
	}
}

Test(sf_memsuite_student, realloc_larger_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int));
	/* void *y = */ sf_malloc(10);
	x = sf_realloc(x, sizeof(int) * 10);

	free_list *fl = &seg_free_list[find_list_index_from_size(32)];

	cr_assert_not_null(x, "x is NULL!");
	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.block_size << 4 == 32, "Free Block size not what was expected!");

	sf_header *header = (sf_header*)((char*)x - 8);
	cr_assert(header->block_size << 4 == 64, "Realloc'ed block size not what was expected!");
	cr_assert(header->allocated == 1, "Allocated bit is not set!");
}

Test(sf_memsuite_student, realloc_smaller_block_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int) * 8);
	void *y = sf_realloc(x, sizeof(char));

	cr_assert_not_null(y, "y is NULL!");
	cr_assert(x == y, "Payload addresses are different!");

	sf_header *header = (sf_header*)((char*)y - 8);
	cr_assert(header->allocated == 1, "Allocated bit is not set!");
	cr_assert(header->block_size << 4 == 48, "Block size not what was expected!");

	free_list *fl = &seg_free_list[find_list_index_from_size(4048)];

	// There should be only one free block of size 4048 in list 3
	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.allocated == 0, "Allocated bit is set!");
	cr_assert(fl->head->header.block_size << 4 == 4048, "Free block size not what was expected!");
}

Test(sf_memsuite_student, realloc_smaller_block_free_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(double) * 8);
	void *y = sf_realloc(x, sizeof(int));

	cr_assert_not_null(y, "y is NULL!");

	sf_header *header = (sf_header*)((char*)y - 8);
	cr_assert(header->block_size << 4 == 32, "Realloc'ed block size not what was expected!");
	cr_assert(header->allocated == 1, "Allocated bit is not set!");


	// After realloc'ing x, we can return a block of size 48 to the freelist.
	// This block will coalesce with the block of size 4016.
	free_list *fl = &seg_free_list[find_list_index_from_size(4064)];

	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.allocated == 0, "Allocated bit is set!");
	cr_assert(fl->head->header.block_size << 4 == 4064, "Free block size not what was expected!");
}


//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################

Test(sf_memsuite_student, malloc_multiple_pages, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(4000);
	void *y = sf_malloc(5000);

	sf_header *header = (sf_header*)((char*)x-8);
	cr_assert(header->block_size << 4 == 4016, "Unexpected block size!");
	header = (sf_header*)((char*)y-8);
	cr_assert(header->block_size << 4 == 5024, "Unexpected block size!");

	free_list *fl = &seg_free_list[find_list_index_from_size(3248)];

	cr_assert(fl->head->header.block_size << 4 == 3248);

	sf_free(x);

	fl = &seg_free_list[find_list_index_from_size(4016)];

	cr_assert(fl->head->header.block_size << 4 == 4016);

	sf_free(y);

	fl = &seg_free_list[find_list_index_from_size(8272)];

	cr_assert(fl->head->header.block_size << 4 == 8272);
}

Test(sf_memsuite_student, free_with_realloc, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(double) * 100);

	sf_header *header = (sf_header*)((char*)x - 8);
	cr_assert(header->block_size << 4 == 816, "Unexpected block size!");

	free_list *fl = &seg_free_list[find_list_index_from_size(3280)];
	cr_assert(fl->head->header.block_size << 4 == 3280);

	x = sf_realloc(x, 0);

	cr_assert_null(x, "x is not NULL!");

	cr_assert(header->allocated == 0, "Allocated bit is not 0");

	fl = &seg_free_list[find_list_index_from_size(4096)];

	cr_assert(fl->head->header.block_size << 4 == 4096);
}

Test(sf_memsuite_student, more_than_one_node_in_seg_free_list, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *a = sf_malloc(100);
	void *b = sf_malloc(500);
	void *c = sf_malloc(1000);
	void *d = sf_malloc(2000);
	void *k = sf_malloc(4000);

	sf_header *header = (sf_header*)((char*)a - 8);
	cr_assert(header->block_size << 4 == 128, "Unexpected block size!");

	header = (sf_header*)((char*)b - 8);
	cr_assert(header->block_size << 4 == 528, "Unexpected block size!");

	header = (sf_header*)((char*)c - 8);
	cr_assert(header->block_size << 4 == 1024, "Unexpected block size!");

	header = (sf_header*)((char*)d - 8);
	cr_assert(header->block_size << 4 == 2016, "Unexpected block size!");

	header = (sf_header*)((char*)k - 8);
	cr_assert(header->block_size << 4 == 4016, "Unexpected block size!");

	free_list *fl = &seg_free_list[find_list_index_from_size(480)];

	cr_assert(fl->head->header.block_size << 4 == 480);

	sf_free(a);

	sf_free(b);

	sf_free(c);

	sf_free(d);

	sf_free(k);

	fl = &seg_free_list[find_list_index_from_size(4496)];

	cr_assert(fl->head->header.block_size << 4 == 4496, "Unexpected free list block size!");

	fl = &seg_free_list[2];

	cr_assert(fl->head->header.block_size << 4 == 2016, "Unexpected free list block size!");

	cr_assert(fl->head->next->header.block_size << 4 == 1024);

	cr_assert(fl->head->next->next->header.block_size << 4 == 528);

	fl = &seg_free_list[find_list_index_from_size(128)];

	cr_assert(fl->head->header.block_size << 4 == 128);
}

Test(sf_memsuite_student, multiple_pages_malloc_on_first_allocate, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *k = sf_malloc(16000);

	sf_header *header = (sf_header*)((char*)k - 8);

	cr_assert(header->block_size << 4 == 16016, "Unexpected block size!");
	cr_assert(header->allocated == 1, "Allocated bit isn't 1");

	free_list *fl = &seg_free_list[find_list_index_from_size(368)];

	cr_assert(fl->head->header.block_size << 4 == 368, "Unexpected free list block size!");

	sf_free(k);

	fl = &seg_free_list[find_list_index_from_size(16384)];

	cr_assert(fl->head->header.block_size << 4 == 16384, "Unexpected free list block size!");
}

Test(sf_memsuite_student, multiple_giant_allocations, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *a = sf_malloc(4000);
	void *b = sf_malloc(8000);
	void *c = sf_malloc(16000);
	void *d = sf_malloc(32);

	sf_header *header = (sf_header*)((char*)a - 8);
	cr_assert(header->block_size << 4 == 4016, "Unexpected block size!");

	header = (sf_header*)((char*)b - 8);
	cr_assert(header->block_size << 4 == 8016, "Unexpected block size!");

	header = (sf_header*)c;
	cr_assert_null(header, "c is not NULL!");

	header = (sf_header*)((char*)d - 8);
	cr_assert(header->block_size << 4 == 48, "Unexpected block size!");

	free_list *fl = &seg_free_list[find_list_index_from_size(208)];

	cr_assert(fl->head->header.block_size << 4 == 208, "Unexpected free list block size!");

	sf_free(d);
	sf_free(b);
	sf_free(a);

	fl = &seg_free_list[find_list_index_from_size(PAGE_SZ*3)];

	cr_assert(fl->head->header.block_size << 4 == PAGE_SZ*3, "Unexpected free list block size!");
}

Test(sf_memsuite_student, use_up_first_page_allocate_more_than_page_again, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(LIST_3_MIN); //544
    void *y = sf_malloc(3536); //Use up rest of the page

    sf_header *header = (sf_header*)((char*)x-8);
    cr_assert(header->block_size << 4 == 544, "Unexpected block size!");

    header = (sf_header*)((char*)y-8);
    cr_assert(header->block_size << 4 == 3552, "Unexpected block size!");

    free_list *fl;

    for (int i = 0; i < 4; i++) {
    	fl = &seg_free_list[i];
    	cr_assert_null(fl->head, "Free list isn't NULL!");
    }

    sf_free(x); //free x

    fl = &seg_free_list[find_list_index_from_size(544)];
    cr_assert(fl->head->header.block_size << 4 == 544, "Unexpected free list block size!");

    void *k = sf_malloc(4096); //request for a page of memory

    header = (sf_header*)((char*)k-8);
    cr_assert(header->block_size << 4 == 4112, "Unexpected block size!");

    fl = &seg_free_list[find_list_index_from_size(4080)];
    cr_assert(fl->head->header.block_size << 4 == 4080, "Unexpected free list block size!");

    sf_free(y);

    sf_free(k);

    fl = &seg_free_list[find_list_index_from_size(8192)];
    cr_assert(fl->head->header.block_size << 4 == 8192, "Unexpected free list block size!");
}