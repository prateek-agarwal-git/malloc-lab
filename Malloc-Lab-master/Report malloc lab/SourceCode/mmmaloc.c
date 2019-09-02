// helper function place
static void * place(void * ptr, size_t asize)
{
	size_t size = GET_SIZE(HDRP(ptr));
	size_t remain = size - asize;

	delete_node(ptr);

	if(remain <= DSIZE*2)
	{
		//do not split
		PUT(HDRP(ptr), PACK(size,1));
		PUT(FTRP(ptr), PACK(size,1));
	}

	else if(asize >= 100)
	{
		//split block
		PUT(HDRP(ptr), PACK(remain,0));
		PUT(FTRP(ptr), PACK(remain,0));
		//put the allocated block at the end of the free block
		PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(asize,1));
		PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(asize,1));
		//insert the remainder free block to segregated free list
		insert_node(ptr,remain);
		return NEXT_BLKP(ptr);
	}
	//put the allocated block at the beginning of the free block
	else
	{
		//split block
		PUT(HDRP(ptr), PACK(asize,1));
		PUT(FTRP(ptr), PACK(asize,1));
		PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(remain,0));
		PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(remain,0));
		insert_node(NEXT_BLKP(ptr),remain);
	}
	return ptr;
}


// malloc function 
void *mm_malloc(size_t size)
{
    if(size==0)
    	return NULL;

    size_t asize ; //adjust size
    size_t extend; //extend heap if neccessary
    void * ptr = NULL;

    //align block size
    if( size <= DSIZE)
    {
    	asize = 2*DSIZE;
    }
    else
	{
    	asize =ALIGN(size + DSIZE);
	}

	int index=0;
	size_t search =asize;
	//traverse the segregated free list
	while(index < LIST)
	{
		//find the appropriate free list
		if((index == LIST -1) || (search <= 1 && free_lists[index] != NULL))
		{
			ptr = free_lists[index];
			//ignore the block with reallcation bit and find the smallest different size block
			while(ptr !=NULL  && ((asize > GET_SIZE(HDRP(ptr)) || GET_TAG(ptr))))
			{
				ptr = PRED(ptr);
			}
			//can find the free block
			if(ptr != NULL)
				break;
		}
		search = search >>1;
		index ++;
	}
	//expand the heap to allocate
	if(ptr == NULL)
	{
		extend = MAX(asize,CHUNKSIZE);
		//cannot extend the heap
		ptr = extend_heap(extend);
		if(ptr == NULL)
			return NULL;
	}
	//place and divide block to the memory
	ptr = place(ptr,asize);

	//return pointer to the allocated block
	return ptr;
}
