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
/*void printblockdetails(void * bp){
    printf("SIZE FROM HEADER %u\n",GETSIZEFROMHEADER(bp));
    printf("SIZE FROM FOOTER %u\n",*(size_t *)((char *)bp + (GETSIZEFROMHEADER(bp) &~0x1)- SIZE_T_SIZE));
    printf("BLOCK ADDRESS %u\n",bp);
    printf("PREVIOUS POINTER %u\n",*(size_t *)(bp + SIZE_T_SIZE ));
    printf("NEXT POINTER %u\n",*(size_t *)(bp + SIZE_T_SIZE + ALIGN(sizeof(void *))));
    //assert(    );
    //assert((*(size_t *)(bp + SIZE_T_SIZE + ALIGN(sizeof(void *))))%ALIGNMENT==0);
    return;}*/
    /*void printfreeblocks(){
    void * start = mm_head;
    void * curr;
    COPYNEXT(curr,start);
    while (curr!= NULL){

         printf("BLOCK ADDRESS %u\n",curr);
        //printblockdetails(curr);
        COPYNEXT(curr,curr);
    }
    return;

} */
int mm_init(void)
{   
     mm_heap = 0;
    mm_heap = 0;
    mm_head =mem_sbrk(MINSIZE);
    size_t minsize = MINSIZE|0x1;
    PUTSIZEINHEADER(mm_head,minsize);
    PUTSIZEINFOOTER(mm_head,minsize);
    char * X = 0;
   SETNEXT(mm_head, X);
   SETPREVIOUS(mm_head,X);  
    mm_heap = mm_head;
	return 0;
}
void *mm_malloc(size_t payload)
{ 
  
    size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
     long int a = (long int) size- MINSIZE;
	if (a<0) size  = MINSIZE;
    void * bp;
    bp = search_free_block(size);    
     if (bp!= NULL){
     return (char *)bp+SIZE_T_SIZE;}
    bp = mem_sbrk(size);
    if (bp == (void *)-1)	return NULL;
    SETALLOCATEBIT(size);
    //assert(size%2 != 0);
    PUTSIZEINHEADER(bp,size);
    PUTSIZEINFOOTER(bp,size);
    mm_heap = bp;
    return (char *)bp+SIZE_T_SIZE;
}

void mm_free(void *ptr)
{        void * bp = ((char *)ptr - SIZE_T_SIZE );
        size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
        size_t bpsize = GETSIZEFROMHEADER(bp);
        bpsize = bpsize&~0x1;
        
        void * temp = -1;
        if (bp == mm_heap){
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
		    size_t prevoccupied = physicalprevsize & 0x1; 
            if (prevoccupied){
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
                insertfreelist(bp,mm_head);
                return;
            }
            else{
                size_t bpsize = *(size_t *) (bp); 
                bpsize = bpsize&~0x1;
                size_t newsize = bpsize + physicalprevsize;
                void * prevblock = (char *) bp - physicalprevsize;
                COPYPREVIOUS(temp,prevblock);
                delete_from_free_list(prevblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                insertfreelist(prevblock,mm_head);
                mm_heap = prevblock;
                return;}
        }
        else{//it is not the last block
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
		    size_t prevoccupied = physicalprevsize & 0x1;
             void * nextblock = (char *)bp + bpsize;
            size_t physicalnextsize = GETSIZEFROMHEADER(nextblock);
            size_t nextoccupied = physicalnextsize& 0x1;
            if (prevoccupied&&nextoccupied){
                PUTSIZEINHEADER(bp,bpsize); 
                PUTSIZEINFOOTER(bp,bpsize);
                insertfreelist(bp,mm_head);                
                return;}
            
            else if (prevoccupied && !nextoccupied){
                size_t newsize = bpsize + physicalnextsize;
                COPYPREVIOUS(temp,nextblock);
                delete_from_free_list(nextblock);
                PUTSIZEINHEADER(bp,newsize);
                PUTSIZEINFOOTER(bp,newsize);
                insertfreelist(bp,mm_head);
                if (nextblock == mm_heap){
                    mm_heap = bp;
                }
                return;
                }  
            else if (!prevoccupied&&nextoccupied){
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize =physicalprevsize+ bpsize;
                COPYPREVIOUS(temp,prevblock);
                delete_from_free_list(prevblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock,newsize);
                insertfreelist(prevblock, mm_head);
                return;
                
            }
            else{
                void * prevblock = (char *) bp - physicalprevsize;
                size_t newsize = physicalnextsize+ physicalprevsize+bpsize;
                COPYPREVIOUS(temp,prevblock);
                delete_from_free_list(prevblock);
                COPYPREVIOUS(temp,nextblock);
                delete_from_free_list(nextblock);
                PUTSIZEINHEADER(prevblock,newsize);
                PUTSIZEINFOOTER(prevblock, newsize);
                insertfreelist(prevblock,mm_head);
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
           
    }
    else{
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
    if (next != NULL){
        SETNEXT(prev,next);
        SETPREVIOUS(next,prev);
    }
    else        SETNEXT(prev, NULL);
    return ;
}
void * search_free_block(size_t size){
    void * curr;
    COPYNEXT(curr, mm_head);
    void * temp3 = 0;
    while (curr!= NULL){
        size_t freeblocksize  = GETSIZEFROMHEADER(curr);
        long int cmp = (long int) freeblocksize - size;
        if (cmp >= 0){
           COPYPREVIOUS(temp3,curr);
          void * xyz = 0;
          COPYPREVIOUS(xyz,curr);
            delete_from_free_list(curr);
            size_t minsize = MINSIZE;
            size_t remaining = freeblocksize - size;
            long int c = (long int)remaining - minsize;
            if (c >= 0){
                SETALLOCATEBIT(size);
                PUTSIZEINHEADER(curr,size);
                PUTSIZEINFOOTER(curr,size);
                 void * splittedfreeblock;
                splittedfreeblock = (char *) curr +(size&~0x1);
                PUTSIZEINHEADER(splittedfreeblock,remaining);
                PUTSIZEINFOOTER(splittedfreeblock, remaining);
                insertfreelist(splittedfreeblock,xyz);
                if (curr == mm_heap){
                    mm_heap = splittedfreeblock;
                }  
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
 * 
 * 
 * void *mm_realloc(void *ptr, size_t payload)
{
 


	if (memcmp(&size,&currentsize, sizeof(size_t) )<0){
		
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
  	return ptr;} mm_heap take care
 */
void *mm_realloc(void *ptr, size_t payload)
{
//if you reallocate it to a new place, free the previous one
    

    if (ptr == NULL) return mm_malloc(payload);
    if (payload == 0) {
		mm_free(ptr);
		return NULL;}
     void * bp = ((char *)ptr - SIZE_T_SIZE );
     size_t oldblocksize = GETSIZEFROMHEADER(bp);
     size_t requestedsize = ALIGN(payload+2*SIZE_T_SIZE);
     long int realloc1 = (long int) requestedsize-oldblocksize;
     long int  splittingrequired = (long int) oldblocksize-requestedsize-MINSIZE;
    if ((bp == mm_heap)&& realloc1 >=0){
        //lastblock and size required ismore than available
        long int extendsize =(long int) realloc-SIZE_T_SIZE;
        if (extendsize >0){
        void * t = mem_sbrk(extendsize);
        requestedsize = (requestedsize|0x1);
         PUTSIZEINHEADER(bp, requestedsize);
        PUTSIZEINFOOTER(bp,requestedsize);
        }
        return ptr;
    }
    else if ((bp == mm_heap)&& realloc1 <0){
        // lastblock and size required
        if (splittingrequired >0){
            //last block splitting

        }
        //cannot reduce heapsize
        //split free block
        return ptr;
    }
    else if (realloc1 >=0) {
        // check whether next physical block is free. 
        /*if yes:
            check whether next physical block is mm_heap:
            set (mm_ heap to current  pointer - SIZE_T_SIZE)
               if yes: colaesce these blocks and see if we can split it again
                       set header and footer if we can split: change mm-heap to this new block
                no: ???*/
    //copy its data to new place and mm_free and mm_malloc and insert this data into new malloc
    else{//reuqested size is less than the current block size:
        /*reset the sizes in header and footer and split a new block;
*/
    }


        /*//it is not the last block and reallocation is required
        




        No:
        if both requested and old both are equal , return ptr
        //if next block is free, and its next block is not free
        //

      

 are equal 
*/
  


    }
    //now it was not the 
    /*
     long int checkmin= (long int) size -minsize;
     void * newptr;
     size_t minsize = MINSIZE;
     
     void * newptr;
     size_t minsize = MINSIZE;
     size_t requestedsize = ALIGN(payload+2*SIZE_T_SIZE);
    long int checkmin= (long int) size -minsize;
    if (checkmin <0) requestedsize = minsize;
     
     long int realloc = (long int) requestedsize-oldblocksize;
    
    if (realloc <=0){
        long int  c = (long int) oldblocksize-requestedsize-MINSIZE;
        if (c <0){
            return *ptr;
        }
        else{//we have to split it
            size_t remainingfree = oldblocksize- requestedsize;
            //        mm_heap
        //  size_t blockoffset = requestedsize;
            SETALLOCATEBIT(requestedsize);
            PUTSIZEINHEADER(bp, requestedsize);
            PUTSIZEINFOOTER(bp,requestedsize);
            void * splittedfreeblock = 0;
            splittedfreeblock = (char *) bp + (requestedsize&~0x1);
            PUTSIZEINHEADER(splittedfreeblock,remainingfree);
            PUTSIZEINFOOTER(splittedfreeblock, remainingfree);
            insertfreelist(splittedfreeblock, mm_head);
    }
/* our code newver
 void * splittedfreeblock;
                splittedfreeblock = (char *) curr +(size&~0x1);
                PUTSIZEINHEADER(splittedfreeblock,remaining);
                PUTSIZEINFOOTER(splittedfreebremaininglock, remaining);
                insertfreelist(splittedfreeblock,xyz);
                if (curr == mm_heap){
                    mm_heap = splittedfreeblock;
                }  
                return curr;      
b
*/
    


        /*size_t remaining = currentsize - size;
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
		return ptr;*/
    }
   
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
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