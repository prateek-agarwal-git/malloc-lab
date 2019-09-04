#include <stdio.h>
#include <stddef.h>
int main(){
	//printf("%d\n",sizeof(size_t)+ sizeof(void *));
	int i = 24;
	i|=1;
	printf("%d\n", i);

	return 0;
}
