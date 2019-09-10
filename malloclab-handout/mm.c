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
#define IS_ALIGNED(p)  ((((unsigned int)(p)) % ALIGNMENT) == 0)

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define MINSIZE (2*SIZE_T_SIZE +2* ALIGN(sizeof(void *)))
#define PUTSIZEINHEADER(bp,size) ((((size_t *)(bp)))[0] = (size))
#define GETSIZEFROMHEADER(bp) (*(size_t *)((char* )bp))
#define PUTSIZEINFOOTER(bp,size) (((size_t *)((char* )bp+(size&~0x1)-SIZE_T_SIZE))[0] = (size))
//#define GETSIZEFROMFOOTER(bp) (*(size_t *)((char* )bp-SIZE_T_SIZE))
#define SETALLOCATEBIT(size) (size|= 0x1)
#define COPYNEXT(dest,src) ((dest)= *(size_t *)  ((char *)src+SIZE_T_SIZE + ALIGN(sizeof(void *)))) 
#define COPYPREVIOUS(dest,src) ((dest)=  *(size_t *)((char *)src+SIZE_T_SIZE ))
//may change to double star in next two
#define SETNEXT(dest,src) (*(size_t *)((char *)(dest) +SIZE_T_SIZE + ALIGN(sizeof(void *))) =(src))
#define SETPREVIOUS(dest,src) (*(size_t *)((char *)(dest) +SIZE_T_SIZE ) = (src))
int mm_init(void);
void * mm_malloc(size_t);
void mm_free(void *);
static char * mm_heap;
static char *mm_head;
static int colaescingkitnibaar;
void *mm_realloc(void *, size_t);
void insertfreelist(void *bp, void * start);
void delete_from_free_list(void *bp);
void * search_free_block(size_t); 
int numfreelist(void){
    void * curr = 0;
    int i = 0;
    COPYNEXT(curr,mm_head);
    while (curr!= NULL){
        i++;
        void * temp = 0;
        COPYNEXT(temp,curr);
        curr = temp;
    }
    return i;
}
void printblockdetails(void * bp){
    printf("SIZE FROM HEADER %u\n",GETSIZEFROMHEADER(bp));
    printf("SIZE FROM FOOTER %u\n",*(size_t *)((char *)bp + (GETSIZEFROMHEADER(bp) &~0x1)- SIZE_T_SIZE));
    printf("BLOCK ADDRESS %u\n",bp);
    printf("PREVIOUS POINTER %u\n",*(size_t *)(bp + SIZE_T_SIZE ));
    printf("NEXT POINTER %u\n",*(size_t *)(bp + SIZE_T_SIZE + ALIGN(sizeof(void *))));
    //assert(    );
    //assert((*(size_t *)(bp + SIZE_T_SIZE + ALIGN(sizeof(void *))))%ALIGNMENT==0);
    return;}
void printfreeblocks(){
    void * start = mm_head;
    void * curr;
    COPYNEXT(curr,start);
    while (curr!= NULL){

         printf("BLOCK ADDRESS %u\n",curr);
        //printblockdetails(curr);
        COPYNEXT(curr,curr);
    }
    return;

} 
static int index1malloc;
static int index2free;
static char * dummy;
int mm_init(void)

{    colaescingkitnibaar = 0;
    index1malloc= 0;
    index2free= 0;
    mm_heap = 0;
    mm_heap = 0;
    mm_head =mem_sbrk(MINSIZE);
    dummy = mm_head;
    size_t minsize = MINSIZE|0x1;
    PUTSIZEINHEADER(mm_head,minsize);
    PUTSIZEINFOOTER(mm_head,minsize);
   //
      char * X = 0;
   SETNEXT(mm_head, X);
   SETPREVIOUS(mm_head,X);  
   printblockdetails(mm_head);

   // exit(0);
    void * tempnext= -1;
   COPYNEXT(tempnext, mm_head);
    void * tempprev= -1;
    COPYPREVIOUS(tempprev, mm_head);
    mm_heap = mm_head;
	return 0;
}
void *mm_malloc(size_t payload)
{   ///assert(size%2 != 0);
    index1malloc ++;
    size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
    printf("This should be  8208 = %d", size);
    long int a = (long int) size- MINSIZE;
	if (a<0) size  = MINSIZE;
    void * bp;
    bp = search_free_block(size);
    
     if (bp!= NULL){
         printf("\n\nThis is  a 2 \n\n");
        printblockdetails(bp);

     return (char *)bp+SIZE_T_SIZE;}
    bp = mem_sbrk(size);
    if (bp == (void *)-1)	return NULL;
    SETALLOCATEBIT(size);
    assert(size%2 != 0);
    PUTSIZEINHEADER(bp,size);
    PUTSIZEINFOOTER(bp,size);
    mm_heap = bp;
    //printf("block number %d  from malloc \n",index1malloc++);
    //\printblockdetails(bp);
    return (char *)bp+SIZE_T_SIZE;
}

void mm_free(void *ptr)
{        void * bp = ((char *)ptr - SIZE_T_SIZE );
        printf("\nnumberof items in free list = %d\n", numfreelist());
        size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
        //printf("this should be 33 = %u\n", physicalprevsize);
        size_t bpsize = GETSIZEFROMHEADER(bp);
        //assert(bpsize%ALIGNMENT!= 0);
        bpsize = bpsize&~0x1;
        //assert(bpsize%ALIGNMENT== 0);
        void * temp = -1;
        if (bp == mm_heap){
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
		    size_t prevoccupied = physicalprevsize & 0x1; 
            if (prevoccupied){
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
                assert(bpsize%ALIGNMENT == 0);
                insertfreelist(bp,mm_head);
                return;
            }
            else{
                size_t bpsize = *(size_t *) (bp); 
                bpsize = bpsize&~0x1;
                printf("this should be (4112) = %u\n", bpsize);
                size_t newsize = bpsize + physicalprevsize;
                assert(newsize %ALIGNMENT == 0);
                printf("this should be (8224) = %u\n", newsize);
                void * prevblock = (char *) bp - physicalprevsize;
                //assert(newsize%ALIGNMENT == 0);
                //could be here
                //assert(prevblock!= mm_head);
                COPYPREVIOUS(temp,prevblock);
                //printf("\n  (dummy) == (temp) %u = %u\n",dummy,temp);
                assert (temp!= NULL);
               // temp = -1;

                delete_from_free_list(prevblock);
                //size_t prevoccupied1 = 
                //exit(0);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                //printf("printing first colaescing, expected value: 8224 should be the size in header and footer\n");
                //printf("Next should be Null, previous should point to dummy");
                //printblockdetails(prevblock);
                insertfreelist(prevblock,mm_head);
                mm_heap = prevblock;
                //printf("this should be 1 %d", numfreelist());
                return;}
        }
        else{//it is not the last block
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
            //printf("Physical previous size is %u\n", physicalprevsize);
            //exit(0);
            assert(bpsize%ALIGNMENT == 0);
		    size_t prevoccupied = physicalprevsize & 0x1;
             void * nextblock = (char *)bp + bpsize;
           // void * prevblock = (char *) bp - physicalprevsize;
            size_t physicalnextsize = GETSIZEFROMHEADER(nextblock);
            size_t nextoccupied = physicalnextsize& 0x1;
            if (prevoccupied&&nextoccupied){//bothprevand next are occupied free this block insert it and return
               
                 
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
               // COPYPREVIOUS(temp,prevblock);

                insertfreelist(bp,mm_head);
                
                return;}
            
            else if (prevoccupied && !nextoccupied){//next is free
                
                //colaescingkitnibaar++;
                size_t newsize = bpsize + physicalnextsize;
                assert(newsize%ALIGNMENT==0);
                COPYPREVIOUS(temp,nextblock);
                printblockdetails(nextblock);
                delete_from_free_list(nextblock);
                PUTSIZEINHEADER(bp,newsize);
                PUTSIZEINFOOTER(bp,newsize);
                insertfreelist(bp,mm_head);
                if (nextblock == mm_heap){
                    mm_heap = bp;
                }
                return;
                }  
            else if (!prevoccupied&&nextoccupied){//previous is free
            colaescingkitnibaar++;
                assert(physicalprevsize%ALIGNMENT==0);
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize =physicalprevsize+ bpsize;
                COPYPREVIOUS(temp,prevblock);
                assert (temp!= NULL);
                delete_from_free_list(prevblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock,newsize);
                insertfreelist(prevblock, mm_head);
                
            }
            else{//both are free
            //colaescingkitnibaar++;
                assert(physicalprevsize%ALIGNMENT==0);
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize = physicalnextsize+ physicalprevsize+bpsize;
                assert(newsize%ALIGNMENT==0);

                //printf("%d\n",trace->ops[op_index].index );
                COPYPREVIOUS(temp,prevblock);
               // assert (temp!= NULL);
                delete_from_free_list(prevblock);
                COPYPREVIOUS(temp,nextblock);
                //assert (temp!= NULL);
                delete_from_free_list(nextblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                insertfreelist(prevblock,mm_head);
                assert(GETSIZEFROMHEADER(prevblock)%2 == 0);
                if (nextblock == mm_heap){
                    mm_heap = prevblock;
                }
                return;
            }          
        }
        return;
        }
void insertfreelist(void * bp, void *head){
    void *nexttohead;
    nexttohead = *(size_t *) *((char *)head +SIZE_T_SIZE + ALIGN(sizeof(void *))) ;
    COPYNEXT(nexttohead,head);
    
     if(nexttohead==NULL){// it is the first node of free list 
        

        SETNEXT(head,bp);
        SETPREVIOUS(bp,head);
        SETNEXT(bp, NULL);
        //printf("\nhead is (bp prev) %u\n", head);
        //printblockdetails(bp);
           
    }
    else{
        //assert(IS_ALIGNED(bp));
        //assert(IS_ALIGNED(head));
        //assert(IS_ALIGNED(nexttohead));
        SETPREVIOUS(bp,head);
        SETNEXT(bp, nexttohead);
        SETNEXT(head,bp);
        SETPREVIOUS(nexttohead, bp);
    }
        return;
}
void delete_from_free_list(void *bp){
    void * next;
    void * prev;
    COPYNEXT(next,bp);
    COPYPREVIOUS(prev,bp);
    //assert (prev!= NULL);
    if (next != NULL){
        SETNEXT(prev,next);
        SETPREVIOUS(next,prev);
    }
    else{
        //*(char *)((char *)prev +SIZE_T_SIZE+ALIGN(sizeof(void *))) = NULL;
        SETNEXT(prev, NULL);
        
    }
    return ;
}
void * search_free_block(size_t size){
    void * curr;
    COPYNEXT(curr, mm_head);
    void * temp3 = 0;
    while (curr!= NULL){
       //printblockdetails(curr);
        size_t freeblocksize  = GETSIZEFROMHEADER(curr);
     
       // printf("f");
        //assert(freeblocksize%ALIGNMENT == 0);
        long int cmp = (long int) freeblocksize - size;
        if (cmp >= 0){
           // assert(curr!= NULL);
           COPYPREVIOUS(temp3,curr);
                assert (temp3!= NULL);
           assert(curr!= mm_head);
            delete_from_free_list(curr);
            size_t minsize = MINSIZE;
            size_t remaining = freeblocksize - size;
            long int c = (long int)remaining - minsize;
            if (c >= 0){
                
                //curr has free block. sset the size set the allocate bi in header and footer
                
                
                SETALLOCATEBIT(size);
                PUTSIZEINHEADER(curr,size);
                PUTSIZEINFOOTER(curr,size);
                assert(remaining %ALIGNMENT== 0);
                void * splittedfreeblock;
                splittedfreeblock = (char *) curr +(size&~0x1);
                //now curr is pointing to the newly made free block
                PUTSIZEINHEADER(splittedfreeblock,remaining);
                PUTSIZEINFOOTER(splittedfreeblock, remaining);
                insertfreelist(splittedfreeblock,mm_head);
                //printblockdetails(splittedfreeblock);
                if (curr == mm_heap){
                    mm_heap = splittedfreeblock;
                }
                //exit(0);
               //let us see we can place it in situ    
                return curr;        
            }
            else{
                size = freeblocksize;
                SETALLOCATEBIT(size);
                PUTSIZEINHEADER(curr,size);
                PUTSIZEINFOOTER(curr,size);
                return curr;
            }
        }

        void * temp;
		COPYNEXT(temp, curr);
		curr = temp; 

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



/*

void * prev;
				void * next;
                COPYPREVIOUS(prev, curr);
                COPYNEXT(next,curr);
                SETNEXT(prev,next);
                if (next!= NULL)         SETPREVIOUS(next,prev);
*/