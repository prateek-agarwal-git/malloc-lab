// helper function coalesce
 static void * coalesce(void * ptr)
{
	//check if the prevrious block is allocated
	size_t prev_all =GET_ALLOC(HDRP(PREV_BLKP(ptr)));
	//check if the next block is allocated
	size_t next_all =GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	size_t size =GET_SIZE(HDRP(ptr));

	//if the previous is reallocated, do not coalesce
	if(GET_TAG(HDRP(PREV_BLKP(ptr))) == 1)
		prev_all = 1;

	//cannot coalesce with previous and the next block
	if(prev_all == 1 && next_all ==1)
		return ptr;

	//can coalesce with the next block
	if(prev_all == 1 && next_all == 0)
	{
		delete_node(ptr);
		delete_node(NEXT_BLKP(ptr));
		//the new size of the coalesce free block
		size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
		//update the info at the header and the footer of the new free block at the pointer
		PUT(HDRP(ptr), PACK(size,0));
		PUT(FTRP(ptr), PACK(size,0));
	}
	//coalesce with the previous block
	else if(prev_all == 0 && next_all == 1)
	{
		delete_node(ptr);
		delete_node(PREV_BLKP(ptr));
		size+= GET_SIZE(HDRP(PREV_BLKP(ptr)));
		ptr= PREV_BLKP(ptr);
		PUT(HDRP(ptr), PACK(size,0));
		PUT(FTRP(ptr), PACK(size,0));
	}
	//coalesce with both previous and next block
	else if (prev_all ==0 && next_all ==0)
	{
		delete_node(ptr);
		delete_node(PREV_BLKP(ptr));
		delete_node(NEXT_BLKP(ptr));

		size+= GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));

		ptr = PREV_BLKP(ptr);
		PUT(HDRP(ptr), PACK(size,0));
		PUT(FTRP(ptr), PACK(size,0));	
	}
	//insert the new free list to the segregated free list
	insert_node(ptr,size);
	return ptr;
}

// mmfree
void mm_free(void *ptr)
{
	size_t size= GET_SIZE(HDRP(ptr));
	//remove the reallcoted tag
	REMOVE_RATAG(HDRP(NEXT_BLKP(ptr)));
	//input the new info in to the block
	PUT(HDRP(ptr), PACK(size,0));
	PUT(FTRP(ptr), PACK(size,0));
	//insert the node to the segregated free list
	insert_node(ptr,size);
	//expand the free block if possible
	coalesce(ptr);

}