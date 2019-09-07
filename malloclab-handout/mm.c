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
#define GETSIZEHEADER(bp) (((size_t)* (char *)bp) & ~0x1)
// address of  previous free block. stored in current free block
#define GETPREVFREE(bp)*  (char  *) ( (char *)bp+SIZE_T_SIZE)
//get next free block from current free block
#define GETNEXTFREE(bp) *(char *) ((char *) bp+SIZE_T_SIZE+ALIGN(sizeof(char *))
// this macro is used for checking the last bit of the block used in colaescing
#define GETFOOTER(bp)((bp + GETSIZEHEADER(bp)- SIZE_T_SIZE))
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))

// get two macros are used for colaescing.
#define SETALLOCBITHEADER(bp) ((*(size_t *) bp)|= 0x1)
#define SETALLOCBITFOOTER(bp) ((*(size_t *)GETFOOTER(bp))|=0x1 )
//our heap
static char * mm_heap;
static char *mm_head;
void * search_free_block(size_t );
void insertfreelist(void * bp);
void * delete_from_free(void * bp); 
int mm_init(void)
{
	//make a dummy head node for free list
	size_t minsize = MINSIZE;
	mm_head =mem_sbrk(MINSIZE);
	mm_heap = mm_head;
	assert(minsize%2==0);
	memcpy(mm_head,&minsize,sizeof(size_t));//header
	memcpy(mm_head+minsize-SIZE_T_SIZE,&minsize, sizeof(size_t));//footer
	memset(mm_head + SIZE_T_SIZE, 0, sizeof(void *));//prev
	memset(mm_head + SIZE_T_SIZE+ALIGN(sizeof(void *)),0, sizeof(void *));//next
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
	mm_heap = block_pointer;
	size = size|0x1;
	memcpy(block_pointer, &size, sizeof(size_t));
	block_pointer = block_pointer + size - SIZE_T_SIZE-1;
	memcpy(block_pointer, &size, sizeof(size_t));
	return block_pointer;
}

void mm_free(void *bp){
	if (bp !=mm_heap ){
	size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
	size_t physicalnextsize = *(size_t *)(bp +GETSIZEHEADER(bp));
	size_t physicalprevbool = physicalprevsize & 0x1;
	size_t physicalnextbool = physicalnextsize &  0x1;
	if (physicalprevbool && physicalnextbool){
 		size_t currsize = GETSIZEHEADER(bp);
		memcpy(bp, &currsize, sizeof(size_t));
		memcpy(bp +currsize - SIZE_T_SIZE, &currsize, sizeof(size_t));
		insertfreelist(bp);
		return;//both are allocated
	}
	else if (physicalprevbool && !physicalnextbool){//next is free
		size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1);
		assert(newfreesize %ALIGNMENT == 0);
		void * x =delete_from_free((char *)bp+physicalnextsize);
		assert(x!= NULL);
		memcpy(bp, &newfreesize, sizeof(size_t));
		memcpy(bp + newfreesize - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
		insertfreelist(bp);
	}
	else if (!physicalprevbool && physicalnextbool){//previous is free
		size_t newfreesize = GETSIZEHEADER(bp) +(physicalprevsize &~0x1); 
		assert(newfreesize %ALIGNMENT == 0);
		void * x =delete_from_free((char *)bp-physicalprevsize);
		assert(x!= NULL);

		memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
		memcpy(bp+GETSIZEHEADER(bp) - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
		insertfreelist(bp-physicalprevsize);
	}
	else{
		size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1)(physicalprevsize &~0x1); 
		assert(newfreesize%ALIGNMENT==0);
		void * x =delete_from_free(bp+physicalnextsize);
		assert(x!= NULL);
		x =searchfreeblock(bp-physicalprevsize);
		assert(x!= NULL);
		memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
		memcpy(bp-physicalprevsize+newfreesize - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
		insertfreelist(bp-physicalprevsize);

	}}
	else{


	}
	return;
}
void *mm_realloc(void *ptr, size_t payload)
{
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
		if (remaining >= MINSIZE){
			//setting header and footer of the freed block
			memcpy(ptr+newsize,&remaining, sizeof(size_t)  );
			memcpy(ptr +currentsize -SIZE_T_SIZE ,&remaining, sizeof(size_t));
			insertfreelist((ptr+newsize));
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
			insertfreelist(ptr + blocksize);
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
void insertfreelist(void * bp){
	
	void *nexthead = GETNEXTFREE(mm_head);
	memcpy(bp+SIZE_T_SIZE+ALIGN(sizeof(void*)),&nexthead,sizeof(void *) );//next of current
	memcpy(mm_head + SIZE_T_SIZE+ALIGN(sizeof(void *)),bp, sizeof(void *));//next of head
	memcpy(bp+SIZE_T_SIZE,mm_head,sizeof(void *) );//prev of current
	return;
}


void * delete_from_free(void * bp){
	void * curr =GETNEXTFREE(mm_head);
	while (curr!= bp){
		curr = GETNEXTFREE(curr);
		if (GETSIZEHEADER(curr)>= size){
			void * prev = GETPREVFREE(curr);
			void * next = GETNEXTFREE(curr);
			memcpy(prev + SIZE_T_SIZE+ALIGN(sizeof(void *)),next, sizeof(void *));//next
			if (next != NULL) memcpy(next + SIZE_T_SIZE, prev, sizeof(void *));//prev
			return curr;
		}
	

	}


} 










