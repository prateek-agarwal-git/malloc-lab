
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
#define GETSIZEFROMHEADER(bp) (*(size_t *)((char* )bp-SIZE_T_SIZE))
#define PUTSIZEINFOOTER(bp,size) (*(size_t *)((char* )bp+size-SIZE_T_SIZE) = (size))
#define GETSIZEFROMFOOTER(bp) (*(size_t *)((char* )bp-SIZE_T_SIZE))
#define SETALLOCATEBIT(size) (size|= 0x1)
int mm_init(void);
void * mm_malloc(size_t);
void mm_free(void *);
static char * mm_heap;
static char *mm_head;
void *mm_realloc(void *, size_t);
int mm_init(void)
{	mm_heap = 0;
    mm_heap = 0;
    mm_head =mem_sbrk(MINSIZE);
    size_t minsize = MINSIZE|0x1;
   *(size_t *)(mm_head) = minsize;
   *(size_t *)((char* )mm_head+MINSIZE-SIZE_T_SIZE) = minsize;
   *(char *)((char *)mm_head +SIZE_T_SIZE) = NULL;
    *(char *)((char *)mm_head +SIZE_T_SIZE+ALIGN(sizeof(void *))) = NULL;
    printf("this should be head footer from mm_init %u\n", mm_head+MINSIZE-SIZE_T_SIZE);
    mm_heap = mm_head;
   // printf("previous should be null %u\n", *(char *)((char *)mm_head +SIZE_T_SIZE));
   // printf("next should be null %u\n", *(char *)((char *)mm_head +SIZE_T_SIZE+ALIGN(sizeof(void *))));
  // printf("Getting size from header %u\n", *(size_t *)mm_head);
    printf("Getting size from footer %u\n",  *(size_t *)((char* )mm_head+minsize-SIZE_T_SIZE));
	return 0;
}
void *mm_malloc(size_t payload)
{
    size_t size = ALIGN(payload)+2*SIZE_T_SIZE;
    //long int a = (long int) size- minsize;
	//if (a<0) size  = MINSIZE;
    //seach free list
    void * bp;
	bp = mem_sbrk(size);
    SETALLOCATEBIT(size);
    PUTSIZEINHEADER(bp,size);
    PUTSIZEINFOOTER(bp,size);
    
    //printf("Getting size from header %u\n", *(size_t *)bp);
    //printf("Getting size from footer %u\n",  *(size_t *)((char* )bp+size-SIZE_T_SIZE));
    mm_heap = bp;
    //printf("block from malloc %u\n", bp);


    return (char *)bp+SIZE_T_SIZE;

    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
        //printf("size from free %u\n",*(size_t *) ((char *)ptr - SIZE_T_SIZE ));
        void * bp = ((char *)ptr - SIZE_T_SIZE );
        printf("block from free %u\n", ((char *)ptr - SIZE_T_SIZE ));
        printf("this is head %u\n", mm_head);
        printf("this should be head footer from free %u\n", ((char *)bp - SIZE_T_SIZE));
        if (bp == mm_heap){
            size_t physicalprevsize =*(size_t *) ( (char *)bp -SIZE_T_SIZE);
		    size_t physicalprevbool = physicalprevsize & 0x1;
            printf("this should be 33 = %u \n", physicalprevsize);
            exit(0);





        }



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

int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }*/