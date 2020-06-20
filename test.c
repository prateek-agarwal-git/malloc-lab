#include <stdio.h>
#include <stddef.h>
int main(){
	//printf("%d\n",sizeof(size_t)+ sizeof(void *));
	int j=1;
	int* i=&j;
	
	printf("%d\n",*&*i);

	return 0;
}
