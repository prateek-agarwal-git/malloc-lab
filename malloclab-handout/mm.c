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
#define GETHEADER(bp) (*bp) 
#define GETSIZEHEADER(bp)((*(size_t *)bp) &= ~0x1)
#define GETPREVFREE(bp) *(char  *) ( (char *)bp+SIZE_T_SIZE)
#define GETNEXTFREE(bp) *(char *) ((char *) bp+SIZE_T_SIZE+ALIGN(sizeof(char *)))
#define GETFOOTER(bp)((bp + GETSIZEHEADER(bp)- SIZE_T_SIZE))
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))
static char * mm_heap;
static char *mm_head;
void * search_free_list(size_t );
void insertfreelist(void *, void *);
int mm_init(void);
void * mm_malloc(size_t);
void mm_free(void *);
void *mm_realloc(void *, size_t);
void  delete_from_freelist(void * bp);
int mm_init(void)
{
	//make a dummy head node for free list
	mem_init();
	size_t minsize = MINSIZE+1;
	mm_head =mem_sbrk(MINSIZE);
	mm_heap = mm_head;
	//assert(minsize%2==0);
	memcpy(mm_head,&minsize,sizeof(size_t));//header
	memcpy(mm_head+MINSIZE-SIZE_T_SIZE,&minsize, sizeof(size_t));//footer
	memset(mm_head + SIZE_T_SIZE, 0, sizeof(void *));//pre
	memset(mm_head + SIZE_T_SIZE+ALIGN(sizeof(void *)),0, sizeof(void *));//next
	return 0;
}

void *mm_malloc(size_t payload)
{
	size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
	if (size<MINSIZE) size  = MINSIZE;
	void * block_pointer;
	block_pointer =search_free_list(size); 
	if (block_pointer!= NULL) return block_pointer;	
	block_pointer = mem_sbrk(size);
	if (block_pointer == (void *) -1){
		assert(3==4);
		 return NULL;}
	mm_heap = block_pointer;
	size = size|0x1;
	memcpy(block_pointer, &size, sizeof(size_t));
	memcpy(block_pointer+size-SIZE_T_SIZE-1, &size, sizeof(size_t));
	return block_pointer;
}
void mm_free(void *bp){
	if (bp==mm_heap){
		size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
		size_t physicalprevbool = physicalprevsize & 0x1;
		if (physicalprevbool){
 			size_t currsize = GETSIZEHEADER(bp);
			printf("%u", currsize);
			memcpy(bp, &currsize, sizeof(size_t));
			memcpy(bp +currsize - SIZE_T_SIZE, &currsize, sizeof(size_t));
			printf("blockpointer= %lu\n, currsize = %lu\n, size_t_size = %lu\n, ",bp, currsize, SIZE_T_SIZE );
			insertfreelist(bp,mm_head);
			return;//both are allocated
		}
		else{

			size_t newfreesize = GETSIZEHEADER(bp) +(physicalprevsize &~0x1); 
			assert(newfreesize %ALIGNMENT == 0);
			delete_from_freelist((char *)bp-physicalprevsize);
			memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
			memcpy(bp+GETSIZEHEADER(bp) - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
			insertfreelist(bp-physicalprevsize,mm_head);
			return;
		}
	}
	else{
		
		size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
		size_t physicalnextsize = *(size_t *)(bp +GETSIZEHEADER(bp));
		size_t 	physicalprevbool = physicalprevsize & 0x1;
		size_t physicalnextbool = physicalnextsize &  0x1;
		if (physicalprevbool && physicalnextbool){
 			size_t currsize = GETSIZEHEADER(bp);
			memcpy(bp, &currsize, sizeof(size_t));
			memcpy(bp +currsize - SIZE_T_SIZE, &currsize, sizeof(size_t));
			insertfreelist(bp,mm_head);
			return;//both are allocated
		}
		else if (physicalprevbool && !physicalnextbool){//next is free
			size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1);
			assert(newfreesize %ALIGNMENT == 0);
			delete_from_freelist((char *)bp+physicalnextsize);
			memcpy(bp, &newfreesize, sizeof(size_t));
			memcpy(bp + newfreesize - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
			insertfreelist(bp, mm_head);
		}
		else if (!physicalprevbool && physicalnextbool){//previous is free
			size_t newfreesize = GETSIZEHEADER(bp) +(physicalprevsize &~0x1); 
			assert(newfreesize %ALIGNMENT == 0);
			delete_from_freelist((char *)bp-physicalprevsize);
			memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
			memcpy(bp+GETSIZEHEADER(bp) - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
			insertfreelist(bp-physicalprevsize, mm_head);
		}
		else{
			size_t newfreesize = GETSIZEHEADER(bp) +(physicalnextsize &~0x1)+(physicalprevsize &~0x1); 
			assert(newfreesize%ALIGNMENT==0);
			delete_from_freelist(bp+physicalnextsize);
			delete_from_freelist(bp-physicalprevsize);
			memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
			memcpy(bp-physicalprevsize+newfreesize - SIZE_T_SIZE, &newfreesize, sizeof(size_t));
			insertfreelist(bp-physicalprevsize, mm_head);
		}	
	}
	return;
}
void *mm_realloc(void *ptr, size_t payload)
{
    	if (ptr == NULL) return mm_malloc(payload);
	if (payload == 0) {
		mm_free(ptr);
		return NULL;}
	size_t size = ALIGN(payload+2*SIZE_T_SIZE);
	if (size<MINSIZE) size  = MINSIZE;
	size_t currentsize = GETSIZEHEADER(ptr); 
	if (size == currentsize) return ptr;
	if (size < currentsize ){
		size_t remaining = currentsize - size;
		size = size+1;
		size_t newsize = size -1;
		memcpy(ptr,&size, sizeof(size_t));
		memcpy(ptr+newsize- SIZE_T_SIZE, &size, sizeof(size_t));
		size = size-1;;
		if (remaining >= MINSIZE){
			memcpy(ptr+newsize,&remaining, sizeof(size_t)  );
			memcpy(ptr +currentsize -SIZE_T_SIZE ,&remaining, sizeof(size_t));
			insertfreelist((ptr+newsize), mm_head);
		}
		return ptr;
	}
 	size_t physicalnextsize = *(size_t *)((char *)ptr +currentsize);
	if ((physicalnextsize%2 == 0) &&((physicalnextsize+currentsize)>= size)){
		size_t remaining = physicalnextsize+currentsize - size;
		size_t blocksize = size;
		size = size+1;
		memcpy(ptr,&size, sizeof(size_t ));
		memcpy(ptr+blocksize- SIZE_T_SIZE, &size, sizeof(size_t));
		if (remaining > MINSIZE){
			assert(remaining%2==0);
			memcpy(ptr+blocksize,&remaining, sizeof(size_t)  );
			memcpy(ptr +blocksize + remaining -SIZE_T_SIZE ,&remaining, sizeof(size_t));
			insertfreelist(ptr + blocksize,mm_head);
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
void * search_free_list(size_t size){
	void * curr =GETNEXTFREE(mm_head);
	while(curr!= NULL){
		if (GETSIZEHEADER(curr)>= size){
			size_t freeblocksize = GETSIZEHEADER(curr);

			if (freeblocksize - size >= MINSIZE){
				void * prev = GETPREVFREE(curr);
				void * next = GETNEXTFREE(curr);
				size = size|0x1;								
				memcpy(curr, &size, sizeof(size_t));
				memcpy(curr+ size -1 - SIZE_T_SIZE, &size,sizeof(size_t));
				size_t remaining = size- 1 - freeblocksize;
				assert(remaining%ALIGNMENT == 0);
				memcpy(curr+size-1, &remaining, sizeof(size_t));
				memcpy(curr+ freeblocksize - SIZE_T_SIZE, &remaining,sizeof(size_t));
				insertfreelist(curr+size-1,prev);				
				return curr;
				}
			else{
				size = freeblocksize|0x1;
				memcpy(curr, &size, sizeof(size_t));
				memcpy(curr+ size -1 - SIZE_T_SIZE, &size,sizeof(size_t));
				return curr;

			}
		}
		curr = GETNEXTFREE(curr);
	}
	return NULL;	
}
void insertfreelist(void * bp, void *prev){
	void *nexttohead = GETNEXTFREE(prev);
	if (nexttohead != NULL){
		memcpy(bp + SIZE_T_SIZE, &prev, sizeof(void *) );
		memcpy(bp + SIZE_T_SIZE+ALIGN(sizeof(void *)),&nexttohead, sizeof(void *) );
		memcpy(prev+SIZE_T_SIZE + ALIGN(sizeof(void *)),&bp, sizeof(void *));
		void * previousnexttohead = GETNEXTFREE(bp);
		memcpy(previousnexttohead + SIZE_T_SIZE, &bp, sizeof(void *));
		return;
	}
	else{
		memcpy(bp+SIZE_T_SIZE,&prev,sizeof(void *) );
		memset(bp+SIZE_T_SIZE+ ALIGN(sizeof(void *)),0,sizeof(void *) );
		memcpy(prev+SIZE_T_SIZE+ALIGN(sizeof(void *)),&bp, sizeof(void *));
		return;
	}
}


void  delete_from_freelist(void * bp){
	void * next =GETNEXTFREE(bp);
	void * prev = GETPREVFREE(bp);
	memcpy((char *)prev + SIZE_T_SIZE + ALIGN(sizeof(void *)), &next, sizeof (void *));
	memcpy((char *)next + SIZE_T_SIZE , &prev, sizeof (void *));
	return;
} 



	/*else{
		update size with freeblocksize;
		allocate the whole block and return pointer;
		}*/

//searches the free list when malloc is called.
//steps: 1) search a block with the given size

//2) if found: 
/*  check size of the free block.
new headers and footers and allocate bit should be set to 1.
// now some free space is remaining. if it is greater than min size:
//set header and footer of the remaining block and 
*/
// else: remove the block from free list
//
//
//
//
