// comments are below the concerned line

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"
team_t team = {
    "Humanists",
    "Parmar Raja Vijay",
    "rajaparmar@cse.iitb.ac.in",
    "Prateek Agarwal",
    "prateekag@cse.iitb.ac.in"
};
//alloc = 1
//free = 0
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
//ALERT : see all the alerts
#define GETHEADER(bp) (*bp) 
#define GETSIZEHEADER(bp) (*(size_t *) bp & ~0x1)
// address of  previous free block. stored in current free block
#define GETPREVFREE(bp)*(void *) (bp+SIZE_T_SIZE)
//get next free block from current free block
#define GETNEXTFREE(bp)* (void *)  (bp+SIZE_T_SIZE+ALIGN(sizeof(void*)))
// this macro is used for checking the last bit of the block used in colaescing
#define GETFOOTER(bp)((bp + GETSIZEHEADER(bp)- SIZE_T_SIZE))
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))

// get two macros are used for colaescing.
#define SETALLOCBITHEADER(bp) ((*(size_t *) bp)|= 0x1)
#define SETALLOCBITFOOTER(bp) ((*(size_t *)GETFOOTER(bp))|=0x1 )
//our heap
static char * mm_heap;
static char *mm_head;
// seglist contains pointer to free blocks according to their size classes. seglist[0] will map to block of 
//size 1, [1] to 2, [2] to 3-4, [3] to 5-8
static char * mm_seglist[36];
// used in our  malloc to check for next free block(we will be using first fit). it returns starting pointer.
void * search_free_block(int,size_t );
//returns nothing. checks contiguous blocks in physical memory( not in our data structure for colaescing
void colaesce(void * bp); 
// not using init as of now. may be required for storing dummy head/ dummy tail
int mm_init(void)
{
	//make a dummy head node for free list
	size_t minsize = MINSIZE;
	mm_head =mem_sbrk(MINSIZE);
	assert(minsize%2==0);
	memcpy(mm_head,&minsize,sizeof(size_t));//header
	memcpy(mm_head+minsize-SIZE_T_SIZE,&minsize, sizeof(size_t));//footer
	memcpy(mm_head + SIZE_T_SIZE, NULL, sizeof(void *));//prev
	memcpy(mm_head + SIZE_T_SIZE+ALIGN(sizeof(void *)),NULL, sizeof(void *));//next
	return 0;
}

void *mm_malloc(size_t payload)
{
	size_t size = ALIGN(payload+2*SIZE_T_SIZE);
	if (size<MINSIZE) size  = MINSIZE;
	void * block_pointer;
	block_pointer =search_free_block(size); 
	if (block_pointer != NULL){
		SETALLOCBITHEADER(block_pointer);
		SETALLOCBITFOOTER(block_pointer);
		 return block_pointer;
	}
	block_pointer = mem_sbrk(size);
	if (block_pointer == (void *) -1) return NULL;
	size = size|0x1;
	memcpy(block_pointer, &size, sizeof(size_t));
	block_pointer = block_pointer + size - SIZE_T_SIZE-1;
	memcpy(block_pointer, &size, sizeof(size_t));
	return block_pointer;
}
void mm_free(void *ptr)
{


}
void colaesce(void *bp){
	size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
	size_t physicalnextsize = *(size_t *)(bp +GETSIZEHEADER(bp));
	size_t physicalprev = physicalprevsize & 0x1;
	size_t physicalnext = physicalnextsize &  0x1;
	//alloc = 1 , free = 0
	if (physicalprev && physicalnext) return;
	else if (physicalprev && !physicalnext){
		//combine with next block
		//alert delete from freelist
		size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1);
		memcpy(bp, &newfreesize, sizeof(size_t));
		memcpy(bp + newfreesize - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
		 //alert: insert into freelist
	/*



	*/ 


	}
	else if (!physicalprev && physicalnext){



		size_t newfreesize = GETSIZEHEADER(bp) +(physicalprevsize &~0x1); 

	// combine with previous block

	}
	else{
	// combine all the three



		size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1)(physicalprevsize &~0x1); 




	}



}
void *mm_realloc(void *ptr, size_t payload)
{
    
    //copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    //if ​ ptr​ is NULL, the effect of the call is equivalent to mm_malloc(size);
	if (ptr == NULL) return mm_malloc(payload);
	if (payload == 0) {
		mm_free(ptr);
		return NULL;

	}
	size_t size = ALIGN(payload+2*SIZE_T_SIZE);
	if (size<MINSIZE) size  = MINSIZE;

	size_t currentsize = GETSIZEHEADER(ptr)-1; 
	if (size == currentsize){
		return ptr;
	}

	if (size < currentsize )){
		size_t remaining = currentsize - size;
		size = size+1;
		size_t newsize = size -1;
		memcpy(ptr,&size, sizeof(size_t );
		memcpy(ptr+newsize- SIZE_T_SIZE, &size, sizeof(size_t));
		size = size-1;;
		//insert into seglist
		if (remaining >= MINSIZE){
			//setting header and footer of the freed block
			memcpy(ptr+newsize,&remaining, sizeof(size_t)  );
			memcpy(ptr +currentsize -SIZE_T_SIZE ,&remaining, sizeof(size_t));
		
			//alert : insert into free list
		}
		return ptr;			
	}
 	size_t physicalnextsize = *(size_t *)(bp +currentsize);
	if ((physicalnextsize%2 == 0) &&(physicalnextsize+currentsize)>= size)){
		size_t remaining = physicalnextsize+currentsize - size;
		size_t blocksize = size;
		size = size+1;
		memcpy(ptr,&size, sizeof(size_t ));
		memcpy(ptr+blocksize- SIZE_T_SIZE, &size, sizeof(size_t));
		if (remaining > MINSIZE){
		assert(remaining%2==0);
		memcpy(ptr+blocksize,&remaining, sizeof(size_t)  );
		memcpy(ptr +blocksize + remaining -SIZE_T_SIZE ,&remaining, sizeof(size_t));
		//alert: insert into free list
		}
		return ptr;


	}	
	else{
		void * newptr = mm_malloc(size);
		if (newptr == NULL){

		return  NULL;//check this or void * -1;
		}
		memcpy(newptr,ptr,currentsize );
		mm_free(ptr);
		return newptr;
	}  
  	return ptr;
}

void * search_free_block(size_t size){
	//implementing first fit
	void * curr =GETNEXTFREE(mm_head);
	while(curr!= NULL){
		if (GETSIZEHEADER(curr)>= size){
					/*
			temp.prev.next = temp.next
			temp.next.prev = temp.prev
		*/
			void * prev = GETPREVFREE(curr);
			void * next = GETNEXTFREE(curr);
			memcpy(prev + SIZE_T_SIZE+ALIGN(sizeof(void *)),next, sizeof(void *));//next
			if (next != NULL) memcpy(next + SIZE_T_SIZE, prev, sizeof(void *));//prev
			return curr;
		}
		curr = GETNEXTFREE(curr);
		
	}
	return NULL;	

}













