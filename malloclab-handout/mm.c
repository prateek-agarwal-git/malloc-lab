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
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define GETSIZE(bp) (*(size_t *) bp & ~1)
#define GETPREVFREE(bp) (bp+sizeof(void*))
#define GETNEXTFREE(bp) (bp+sizeof(size_t)+sizeof(void*))
#define SETALLOCBITHEADER(bp) (*(size_t *) bp  = (*(size_t *) bp| 0x1)  )
#define SETALLOCBITFOOTER(bp) (GETFOOTER|=0x1 )
#define GETFOOTER(bp)(* (size_t *)(bp + GETSIZE(bp)- sizeof(size_t))
static char * mm_heap;
static char * mm_seglist[36];
void * search_free_block(int,size_t ); 
int mm_init(void)
{
	mm_heap= (char *)sbrk(48);
	if (mm_heap == (void *) -1) return -1;
	 return 0;
}
void *mm_malloc(size_t payload)
{
	size_t size = ALIGN(payload)+8;
	size_t minsize = 2*sizeof(size_t)+ 2*sizeof(void *);
	
	if (size<minsize) size  = minsize;
	size_t temp = size>>3;
	int seg_index = 0;
	while (temp!=0){
		temp = (temp>>1);
		seg_index++; 
	}
	void * block_pointer;
	//search free blocks in mm_seglist[]
	block_pointer =search_free_block(seg_index,size); 
	if (block_pointer != NULL){
		SETALLOCBITHEADER(bp);
		SETALLOCBITFOOTER(bp);
		 return block_pointer;
		}

	  
  /*  int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }*/
}

void mm_free(void *ptr)
{


}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
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
			if ((GETSIZE(temp) & ~1) >=size){
				return temp;
			}
			temp = GETNEXTFREE(temp);
		}
		index++;

	}
	return NULL;

}













