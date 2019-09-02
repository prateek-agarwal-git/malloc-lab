static void insert_node(void * ptr, size_t size)
{
	int index;
	void *next = ptr;
	void *before = NULL;

	for(index=0;index < LIST -1; index++ )
	{
		if(size > 1)
		{
			size = size >> 1;
		}
		else break;
	}
	next = free_lists[index];
	//traverse the free list to find a position to input the node
	while( next !=NULL && size < GET_SIZE(HDRP(next)))
	{
		before = next;
		next = PRED(next);
	}
	if(next != NULL)
	{
		//insert between the list
		if(before!= NULL)
		{
			SET_PTR(PRED_PTR(ptr),next);
			SET_PTR(SUCC_PTR(next), ptr);
			SET_PTR(PRED_PTR(before), ptr);
			SET_PTR(SUCC_PTR(ptr), before);
		}
		//insert at the begining of the list
		else
		{
			SET_PTR(PRED_PTR(ptr), next);
			SET_PTR(SUCC_PTR(next), ptr);
			SET_PTR(SUCC_PTR(ptr), NULL);
			//update the root of the free list
			free_lists[index]= ptr;
		}
	}
	//at the end of the list
	else
	{
		//at the end of the list
		if(before!=NULL)
		{
			SET_PTR(PRED_PTR(ptr),NULL);
			SET_PTR(SUCC_PTR(ptr), before);
			SET_PTR(PRED_PTR(before),ptr);
		}
		//the list is empty initially at that index
		else
		{
			SET_PTR(PRED_PTR(ptr),NULL);
			SET_PTR(SUCC_PTR(ptr),NULL);
			//update the root of free list at the index
			free_lists[index]=ptr;
		}
	}
	return;
	
}