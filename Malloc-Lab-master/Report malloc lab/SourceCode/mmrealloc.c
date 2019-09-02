// helper function void * memcpy(void * destination, const void * source, size_t num);

void *mm_realloc(void *ptr, size_t size)
{
	if(size == 0 )
		return NULL;
    void * oldptr = ptr;
    size_t newsize =size;	//size of the new block
    int remain;	//the remain size after allocation
    int extend;	//size of heap extension 
    int blockbuff;

    //align block size
    if( size <= DSIZE)
    {
    	newsize = 2*DSIZE;
    }
    else
	{
    	newsize =ALIGN(size + DSIZE);
	}
	//add overhead requirment for block size
	newsize += REALLOC_BUFFER;
	//calculate the block buffer
	blockbuff = GET_SIZE(HDRP(ptr)) - newsize;
	//not enough space
	if(blockbuff < 0)
	{
		//check if the next block is free or the epilogue block
		if(GET_ALLOC(HDRP(NEXT_BLKP(ptr)))==0 || GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0)
		{
			//calculate the space missing
			remain = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - newsize;
			//not enough space
			if(remain < 0)
			{
				extend = MAX ( -remain, CHUNKSIZE);
				//can not extend the heap
				if(extend_heap(extend) == NULL)
					return NULL;
				remain += extend;
			}

			delete_node(NEXT_BLKP(ptr));

			//do not split block, update the info of the current block
			PUT_NOTAG(HDRP(ptr), PACK(newsize + remain,1));
			PUT_NOTAG(FTRP(ptr), PACK(newsize + remain,1));

		}
		else
		{
			//enough space to allocated into the block
			oldptr = mm_malloc(newsize -DSIZE);
			memcpy( oldptr, ptr, MIN(size,newsize));
			mm_free(ptr);
		}
		blockbuff = GET_SIZE(HDRP(oldptr)) - newsize;
	}

	if(blockbuff < 2 * REALLOC_BUFFER)
	{
		SET_RATAG(HDRP(NEXT_BLKP(oldptr)));
	}
	//return the reallocation block
	return oldptr;
}
