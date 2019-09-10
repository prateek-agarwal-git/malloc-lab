#include <stdio.h>
#include <stdlib.h>
int main(){
	void * a;
	int * b;
	int F[20];
	int * a;
	printf("\n");
	printf("%lu", sizeof(a));
	printf("\n");
	return 0;
}

#define SETNEXT(dest,src) (*(char **)((char *)(dest)+SIZE_T_SIZE+ALIGN(sizeof(void *))) =(src))
#define SETPREVIOUS(dest,src) (*(char **)((char *)(dest)+SIZE_T_SIZE) =*( (src))