static void *extend_heap(size_t size)
{
	size_t tempsize=size;
	void * ptr= mem_sbrk(tempsize);

	//not enough space
	if(ptr == (void * ) -1)
		return NULL;

	//set header and footer infomation
	//header
	PUT_NOTAG(HDRP(ptr),PACK(tempsize,0));
	//footer
	PUT_NOTAG(FTRP(ptr),PACK(tempsize,0));
	PUT_NOTAG(HDRP(NEXT_BLKP(ptr)),PACK(0,1));
	//insert free node
	insert_node(ptr,tempsize);

	return coalesce(ptr);
}