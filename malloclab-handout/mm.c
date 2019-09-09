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
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))
#define PUTSIZEINHEADER(bp,size) ((((size_t *)(bp)))[0] = (size))
#define GETSIZEFROMHEADER(bp) (*(size_t *)((char* )bp))
#define PUTSIZEINFOOTER(bp,size) (((size_t *)((char* )bp+(size&~0x1)-SIZE_T_SIZE))[0] = (size))
//#define GETSIZEFROMFOOTER(bp) (*(size_t *)((char* )bp-SIZE_T_SIZE))
#define SETALLOCATEBIT(size) (size|= 0x1)
#define COPYNEXT(dest,src) ((dest)= *(char **)  ((char *)src+SIZE_T_SIZE + ALIGN(sizeof(void *)))) 
#define COPYPREVIOUS(dest,src) ((dest)=  *(char **)((char *)src+SIZE_T_SIZE ))
#define SETNEXT(dest,src) (*(char *)((char *)(dest)+SIZE_T_SIZE+ALIGN(sizeof(void *))) = *(char **)(src))
#define SETPREVIOUS(dest,src) (*(char *)((char *)(dest)+SIZE_T_SIZE) =*(char **) (src))
int mm_init(void);
void * mm_malloc(size_t);
void mm_free(void *);
static char * mm_heap;
static char *mm_head;
void *mm_realloc(void *, size_t);
void insertfreelist(void *bp, void * start);
void delete_from_free_list(void *bp);
void * search_free_block(size_t);
void printblockdetails(void * bp){
    printf("SIZE FROM HEADER %u\n",GETSIZEFROMHEADER(bp));
    printf("SIZE FROM FOOTER %u\n",*(size_t *)((char *)bp + (GETSIZEFROMHEADER(bp) &~0x1)- SIZE_T_SIZE));
    printf("BLOCK ADDRESS %u\n",bp);
    printf("PREVIOUS POINTER %u\n",*(char **)(bp + SIZE_T_SIZE ));
    printf("NEXT POINTER %u\n",*(char **)(bp + SIZE_T_SIZE + ALIGN(sizeof(void *))));
    
    return;

}
static int index1;
int mm_init(void)
{	mm_heap = 0;
    mm_heap = 0;
    mm_head =mem_sbrk(MINSIZE);
    size_t minsize = MINSIZE|0x1;
    //PUTSIZEINHEADER(mm_head, MINSIZE);
    //PUTSIZEINFOOTER(mm_head, MINSIZE);
   *(size_t *)(mm_head) = minsize;
   *(size_t *)((char* )mm_head+MINSIZE-SIZE_T_SIZE) = minsize;
  // SETNEXT(mm_head, NULL);
   //SETPREVIOUS(mm_head,NULL);
   *(char *)((char *)mm_head +SIZE_T_SIZE) = NULL;
   *(char *)((char *)mm_head +SIZE_T_SIZE+ALIGN(sizeof(void *))) = NULL;
    void * tempnext= -1;
   COPYNEXT(tempnext, mm_head);
    void * tempprev= -1;
    COPYPREVIOUS(tempprev, mm_head);
    printf("calling from init \n");
    printblockdetails(mm_head);
   // printf("previous to dummy should be null = %u\n",tempprev);
   //printf("next to dummy should be null = %u\n",tempnext);
    //printf("this should be head footer from mm_init %u\n", mm_head+MINSIZE-SIZE_T_SIZE);
    mm_heap = mm_head;
   // printf("previous should be null %u\n", *(char *)((char *)mm_head +SIZE_T_SIZE));
   // printf("next should be null %u\n", *(char *)((char *)mm_head +SIZE_T_SIZE+ALIGN(sizeof(void *))));
  //printf("Getting size from header %u\n", GETSIZEFROMHEADER(mm_head));
 //printf("Getting size from footer %u\n", (*(size_t *)((char* )mm_head+minsize -SIZE_T_SIZE)));
	return 0;
}
void *mm_malloc(size_t payload)
{
    size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
    long int a = (long int) size- MINSIZE;
	if (a<0) size  = MINSIZE;
    //search free list here.
    bp = search_free_block(size);
    void * bp;
	bp = mem_sbrk(size);
    if (bp == (void *)-1)	return NULL;
    SETALLOCATEBIT(size);
    PUTSIZEINHEADER(bp,size);
    PUTSIZEINFOOTER(bp,size);
    printf("calling from %d block pointer\n", index1++);
    printblockdetails(bp);
    //printf("Getting size from header %u\n", *(size_t *)bp);
    //printf("Getting size from footer %u\n",  *(size_t *)((char* )bp+size-SIZE_T_SIZE));
    mm_heap = bp;
    //printf("block from malloc %u\n", bp);


    return (char *)bp+SIZE_T_SIZE;

    
}

void mm_free(void *ptr)
{
        void * bp = ((char *)ptr - SIZE_T_SIZE );
        size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
        printf("this should be 33 = %u\n", physicalprevsize);
        size_t bpsize = GETSIZEFROMHEADER(bp);
        bpsize = bpsize&~0x1;
        if (bp == mm_heap){
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
		    size_t prevoccupied = physicalprevsize & 0x1; 
            if (prevoccupied){
                
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
                return;
            }
            else{
                size_t bpsize = *(size_t *) (bp); 
                bpsize = bpsize&~0x1;
                size_t newsize = bpsize + physicalprevsize;
                assert(newsize %ALIGNMENT == 0);
                void * prevblock = (char *) bp - physicalprevsize;
                //delete_from_free_list(prevblock);
                //exit(0);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                //insertfreelist(prevblock,mm_head);
                return;}
        }
        else{//it is not the last block
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
            printf("Physical previous size is %u\n", physicalprevsize);
            //exit(0);
		    size_t prevoccupied = physicalprevsize & 0x1;
             void * nextblock = (char *)bp + bpsize;
           // void * prevblock = (char *) bp - physicalprevsize;
            size_t physicalnextsize = GETSIZEFROMHEADER(nextblock);
            size_t nextoccupied = physicalnextsize& 0x1;
            if (prevoccupied&&nextoccupied){//bothprevand next are occupied free this block insert it and return
                bpsize &= ~0x1;
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
                //insertfreelist(bp,mm_head);
                return;}
            
            else if (prevoccupied && !nextoccupied){//next is free
                size_t newsize = bpsize + physicalnextsize;
                //delete_from_free_list(nextblock);
                PUTSIZEINHEADER(bp,newsize);
                PUTSIZEINFOOTER(bp,newsize);
               // insertfreelist(bp,mm_head);
                }  
            else if (!prevoccupied&&nextoccupied){//previous is free
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize =physicalprevsize+ bpsize;
                //delete_from_free_list(prevblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock,newsize);
                //insertfreelist(prevblock, mm_head);
            }
            else{//both are free
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize = physicalnextsize+ physicalprevsize+bpsize;
                //delete_from_free_list(prevblock);
                //delete_from_free_list(nextblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                //insertfreelist(prevblock,mm_head);
                return;
            }    
        
        }
              return;
        }
void insertfreelist(void * bp, void *head){
    void *nexttohead;
    COPYNEXT(nexttohead,head);
        if(nexttohead==NULL){// it is the first node of free list 
        SETNEXT(head,bp);
        SETPREVIOUS(bp,head);
        SETNEXT(bp,NULL);        
    }
    else{
        SETPREVIOUS(bp,head);
        SETNEXT(bp, nexttohead);
        SETNEXT(head,bp);
        SETPREVIOUS(nexttohead, bp);
        //return;        

    }
        return;
}
void delete_from_free_list(void *bp){
    void * next;
    void * prev;
    COPYNEXT(next,bp);
    COPYPREVIOUS(prev,bp);
    if (next != NULL){
        SETNEXT(prev,next);
     SETPREVIOUS(next,prev);
    }
    else{
        *(char *)((char *)prev +SIZE_T_SIZE+ALIGN(sizeof(void *))) = NULL;
        //SETNEXT(prev, NULL_)
        
    }
    return ;
}
void * search_free_block(size_t size){
    void * curr;
    COPYNEXT(curr, mm_head);
    while (curr!= NULL){
        size_t freeblocksize  = GETSIZEFROMHEADER(curr);
        assert(curr%ALIGNMENT != 0);
        long int cmp = (long int) freeblocksize - size;
        if (cmp >= 0){
            size_t minsize = MINSIZE;
            size_t remaining = freeblocksize - size;
            long int c = (long int)remaining - minsize;
            if (c >= 0){
                void * prev;
				void * next;
                SE




            }


        }



    }




}
/* search _ free_block
void * curr ;
	setnextfree(&curr,mm_head);
	int i = 0;
	//assert((size>=MINSIZE) &&(size%ALIGNMENT == 0));
	while(curr!= NULL){
		printf(" \nwhich free blocks(2056)  %u what size(3040-3056) %u\n ",GETSIZEHEADER(curr), size );

		if (memcmp(curr, &size,sizeof(size_t))>=0){
		
			if (memcmp(&temp2, &minsize, sizeof(size_t))>=0){
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
				return cvoid * curr ;
	setnextfree(&curr,mm_head);
	int i = 0;
	//assert((size>=MINSIZE) &&(size%ALIGNMENT == 0));
	while(curr!= NULL){
		printf(" \nwhich free blocks(2056)  %u what size(3040-3056) %u\n ",GETSIZEHEADER(curr), size );

		if (memcmp(curr, &size,sizeof(size_t))>=0){
			printf("hi what is up\n");
			size_t freeblocksize = GETSIZEHEADER(curr);
			//assert(1==2);
			//if (freeblocksize < size){
			//	return NULL;
			//}
			size_t minsize = MINSIZE;
			size_t temp2 = freeblocksize - size; 
			
			if (memcmp(&temp2, &minsize, sizeof(size_t))>=0){
				void urr;
			}
		}
		void * temp;
		setnextfree(&temp,curr);
		curr = temp; 
		i++;
	}
	return NULL;	
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
/*void printfreelist(){
	void * curr ;
	COPYNEXT(curr,mm_head);
	while (curr!= NULL){
        printf("free size%u", GETSIZEFROMHEADER(curr));
		void * temp;
		COPYNEXT(temp,curr);
		curr = temp;

	}
	return;
		//printf("is curr null %u\n",curr);

}
 
*/
/*
 * mm_free - Freeing a block does nothing.
 */