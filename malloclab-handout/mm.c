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
#define GETSIZEHEADER(bp) (*(size_t *) bp & ~1)
// get previous free block. stored in current free block
#define GETPREVFREE(bp) (bp+sizeof(void*))
//get next free block from current free block
#define GETNEXTFREE(bp) (bp+SIZE_T_SIZE+sizeof(void*))
// this macro is used for checking the last bit of the block used in colaescing
#define GETFOOTER(bp)((bp + GETSIZEHEADER(bp)- SIZE_T_SIZE))
#define MINSIZE (2*SIZE_T_SIZE + ALIGN(2*sizeof(void *)))

// get two macros are used for colaescing.
#define SETALLOCBITHEADER(bp) ((*(size_t *) bp)|= 0x1)
#define SETALLOCBITFOOTER(bp) ((*(size_t *)GETFOOTER(bp))|=0x1 )
//our heap
static char * mm_heap;
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
	 return 0;
}

void *mm_malloc(size_t payload)
{
	size_t size = ALIGN(payload+2*SIZE_T_SIZE);
	if (size<MINSIZE) size  = MINSIZE;
	size_t temp = size;
	int seg_index = 0;
	while (temp!=0){
		temp = (temp>>1);
		seg_index++; 
	}
	void * block_pointer;
	
	block_pointer =search_free_block(seg_index,size); 
	if (block_pointer != NULL){
		SETALLOCBITHEADER(block_pointer);
		SETALLOCBITFOOTER(block_pointer);
		 return block_pointer;
	}
	block_pointer = mem_sbrk(size);
	if (block_pointer == (void *) -1) return NULL;
	size = size|0x1;
	memcpy(block_pointer, &size, sizeof(size_t));
	block_pointer = block_pointer + size - SIZE_T_SIZE;
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
		//alert delete from seglist
		/*
			temp.prev.next = temp.next
			temp.next.prev = temp.prev
		*/
		size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1);
		memcpy(newblock, &remaining, sizeof(size_t));
		 


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
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

void * search_free_block(int index, size_t size){
	while(index<36){
		char * temp = mm_seglist[index];
		while (temp!= 0){
			//multiple getsizeheader

			if (GETSIZEHEADER(temp) >=size){
				//alert delete from seglist
				/*
					temp.prev.next = temp.next
					temp.next.prev = temp.prev
				*/
				size_t remaining = GETSIZEHEADER(temp) - size;
				if (remaining > MINSIZE){
					void * newblock = temp+size;
					memcpy(newblock, &remaining, sizeof(size_t));
					void* footernewblock = newblock + remaining - SIZE_T_SIZE;
					memcpy(footernewblock, &remaining, sizeof(size_t));

					//alert: insert into seglist
					/*
						newblock.prev = NULL
						newblock.next = seglist[index]
						seglist[index] = newblock
					

					*/
				}
				return temp;
			}
			temp = GETNEXTFREE(temp);
		}
		index++;

	}
	return NULL;

}













