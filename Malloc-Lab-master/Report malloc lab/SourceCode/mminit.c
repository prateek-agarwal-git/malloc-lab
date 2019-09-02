int mm_init(void)
{
	int index;
	//initialize the segregated free list NULL
	for(index=0;index< LIST; index++)
		free_lists[index] = NULL;

	char * heap;
	//cannot allocated the heap
	if((long)(heap = mem_sbrk(4 * WSIZE)) == -1)
		return  -1;
	//padding
	PUT_NOTAG(heap, 0);
	//input the prologue header
	PUT_NOTAG(heap + 1* WSIZE, PACK(DSIZE,1));
	//prologue footer
	PUT_NOTAG(heap + 2* WSIZE, PACK(DSIZE,1));
	//epilogue header
	PUT_NOTAG(heap + 3* WSIZE, PACK(0,1));

	if(extend_heap(INITCHUNKSIZE)==NULL)
		return -1;
    return 0;
}
