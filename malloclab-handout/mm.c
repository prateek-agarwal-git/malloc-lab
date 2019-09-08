
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
#define GETSIZEHEADER(bp) (*(unsigned int  *)bp)
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))
#define IS_ALIGNED(p)  ((((unsigned int)(p)) % ALIGNMENT) == 0)
static char * mm_heap;
static char *mm_head;
void * search_free_list(size_t );
void insertfreelist(void *, void *);
int mm_init(void);
void * mm_malloc(size_t);
void mm_free(void *);
void *mm_realloc(void *, size_t);
void  delete_from_freelist(void * bp);
void  setnextfree(void ** , void * );
void  setprevfree(void ** , void * );
void printfreelist(){
	void * curr ;
	setnextfree(&curr,mm_head);
	while (curr!= NULL){
//		printf("free size%u", GETSIZEHEADER(curr));
		void * temp;
		setnextfree(&temp,curr);
		curr = temp;

	}
	return;
		//printf("is curr null %u\n",curr);

}
int mm_init(void)
{
	mem_init();
	size_t minsize = MINSIZE+1;
	mm_head =mem_sbrk(MINSIZE);
	//mm_heap = mm_head+MINSIZE-1;
	memcpy(mm_head,&minsize,sizeof(size_t));//header
	memcpy(mm_head+MINSIZE-SIZE_T_SIZE,&minsize, sizeof(size_t));//footer
	memset(mm_head + SIZE_T_SIZE, 0, sizeof(void *));//pre
	memset(mm_head + SIZE_T_SIZE+ALIGN(sizeof(void *)),0, sizeof(void *));//next
	return 0;
}

void *mm_malloc(size_t payload)
{	//printfreelist();
	size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
	if (size<MINSIZE) size  = MINSIZE;
	void * block_pointer;
	block_pointer =search_free_list(size);
 	//assert(block_pointer != NULL);
	if (block_pointer!= NULL) return block_pointer;	
	block_pointer = mem_sbrk(size);
	if (block_pointer == (void *) -1){
		assert(3==4);
		 return NULL;}
	assert(mm_head!=block_pointer); 
	mm_heap = block_pointer;
	size = size|0x1;
	memcpy(block_pointer, &size, sizeof(size_t));
	size_t step = size&~0x1;
	memcpy(block_pointer+step-SIZE_T_SIZE, &size, sizeof(size_t));
	return block_pointer;
}
void mm_free(void *bp){
	void * temp;
	if (bp==mm_heap){
		size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
		size_t physicalprevbool = physicalprevsize & 0x1;
		if (physicalprevbool){
 			size_t  currsize = GETSIZEHEADER(bp);
			currsize= currsize &~0x1;
			memcpy(bp, &currsize, sizeof(size_t));
			memcpy(bp +(currsize - SIZE_T_SIZE), &currsize, sizeof(size_t));
			insertfreelist(bp,mm_head);
			printfreelist();
			return;
		}
		else{
			size_t newfreesize = (GETSIZEHEADER(bp)&~0x1) +(physicalprevsize &~0x1); 
			assert(newfreesize %ALIGNMENT == 0);
			delete_from_freelist((char *)bp-physicalprevsize);
			memcpy(bp -physicalprevsize, &newfreesize, sizeof(size_t));
			assert(IS_ALIGNED(bp +GETSIZEHEADER(bp)));
			size_t temp =GETSIZEHEADER(bp)&~0x1; 
			memcpy(bp+ temp- SIZE_T_SIZE, &newfreesize, sizeof(size_t));
			void * l =bp-physicalprevsize;
			insertfreelist(l,mm_head);
			return;
		}
	}
	else{	size_t physicalprevsize =*(size_t *) ( bp - SIZE_T_SIZE);
		size_t physicalnextsize = *(size_t *)(bp +(GETSIZEHEADER(bp)&~0x1));
		size_t 	physicalprevbool = physicalprevsize & 0x1;
		size_t physicalnextbool = physicalnextsize &  0x1;
		if (physicalprevbool && physicalnextbool){
 			size_t currsize = GETSIZEHEADER(bp);
			memcpy(bp, &currsize, sizeof(size_t));
			memcpy(bp +currsize - SIZE_T_SIZE, &currsize, sizeof(size_t));
			insertfreelist(bp,mm_head);
			return;
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
		size = size|0x1;
		size_t newsize = size&~0x1;
		memcpy(ptr,&size, sizeof(size_t));
		memcpy(ptr+newsize- SIZE_T_SIZE, &size, sizeof(size_t));
		size = size&~0x1;
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
		size = size|0x1;
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
	else{	void * newptr = mm_malloc(size);
		if (newptr == NULL){
		return  NULL;
		}
		memcpy(newptr,ptr,currentsize );
		mm_free(ptr);
		return newptr;
	}  
  	return ptr;}
void * search_free_list(size_t size){
	void * curr ;
	setnextfree(&curr,mm_head);
	int i = 0;
	//assert((size>=MINSIZE) &&(size%ALIGNMENT == 0));
	while(curr!= NULL){
		printf(" \nwhich free blocks(2056)  %u what size(3040-3056) %u\n ",GETSIZEHEADER(curr), size );

		if (memcmp(curr, &size,sizeof(size_t))>=0){
			printf("hi what is up\n");
			size_t freeblocksize = GETSIZEHEADER(curr);
			assert(1==2);
			//if (freeblocksize < size){
			//	return NULL;
			//}
			if (freeblocksize - size >= MINSIZE){
				void * prev;
				void * next;
				setnextfree(&next,curr);
				setprevfree(&prev,free);
				size = size|0x1;
				size_t step = size &~0x1;								
				memcpy(curr, &size, sizeof(size_t));
				memcpy(curr+ step - SIZE_T_SIZE, &size,sizeof(size_t));
				size_t remaining =  freeblocksize - step;
				assert(remaining%ALIGNMENT == 0);
				memcpy(curr+step, &remaining, sizeof(size_t));
				memcpy(curr+ freeblocksize - SIZE_T_SIZE, &remaining,sizeof(size_t));
				insertfreelist(curr+step,prev);				
				return curr;
				}
			else{
				size = freeblocksize|0x1;
				size_t step = freeblocksize &~0x1;
				memcpy(curr, &size, sizeof(size_t));
				memcpy(curr+ step - SIZE_T_SIZE, &size,sizeof(size_t));
				return curr;
			}
		}
		void * temp;
		setnextfree(&temp,curr);
		curr = temp; 
		i++;
	}
	return NULL;	
}
void  setnextfree(void ** dest, void * src){
	size_t sourcesize = GETSIZEHEADER(src);
	sourcesize = sourcesize &~0x1;
	memcpy(dest,src+SIZE_T_SIZE+ALIGN(sizeof(void *)),sizeof(void *));// store next of prev in a temp variable
	//printf("inside setnextfree %u ",*dest);	
	return;
}
void  setprevfree(void ** dest, void * src){
	size_t sourcesize = GETSIZEHEADER(src);
	sourcesize = sourcesize &~0x1;
	memcpy(dest,src+SIZE_T_SIZE,sizeof(void *));// store next of prev in a temp variable	
	return;
}

void insertfreelist(void * bp, void *prev){
	void *nexttohead;
	size_t headsize = GETSIZEHEADER(prev);
	setnextfree(&nexttohead,prev);
	if (nexttohead != NULL){
		memcpy(bp + SIZE_T_SIZE,&prev, sizeof(void *) );
		memcpy(bp + SIZE_T_SIZE+ALIGN(sizeof(void *)),&nexttohead, sizeof(void *) );
		memcpy(prev+SIZE_T_SIZE + ALIGN(sizeof(void *)),&bp, sizeof(void *));
		void * previousnexttohead;
		setnextfree(&previousnexttohead, bp);
		memcpy(previousnexttohead + SIZE_T_SIZE, &bp, sizeof(void *));
		return;
	}
	else{
		//printf("\nbp should not be zero %u\n", bp);
		//printf("before memcpy %u\n", prev+SIZE_T_SIZE+ALIGN(sizeof(void *)));
  		memcpy(prev+SIZE_T_SIZE+ALIGN(sizeof(void *)),&bp, sizeof(void *));
		//printf("checking memcpy? %u\n", *(void **)(prev+SIZE_T_SIZE + ALIGN(sizeof(void *))));
		//void * temp2;
		//setnextfree(&temp2,prev);
		//printf("\nwhat here bp?? %u\n",temp2 );
		memcpy(bp+SIZE_T_SIZE,&prev,sizeof(void *) );
		memset(bp+SIZE_T_SIZE+ ALIGN(sizeof(void *)),0,sizeof(void *) );
		return;
	}
}


void  delete_from_freelist(void * bp){
	void * next;
	void * prev;
	setnextfree(&next, bp);
	setprevfree(&prev,bp);
	if (next!= NULL){
		memcpy(prev + SIZE_T_SIZE + ALIGN(sizeof(void *)), &next, sizeof (void *));
		memcpy(next + SIZE_T_SIZE , &prev, sizeof (void *));
	}
	else memset(prev + SIZE_T_SIZE + ALIGN(sizeof(void *)), 0, sizeof (void *));

	
	return;
} 
