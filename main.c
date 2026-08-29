#include "pager.h"
#include <string.h>

int main(void){
	printf("test");
	Pager* pager = pager_open("/home/gavinbarrett/gooseDB/db_file");

	if (pager == NULL) return -1;
	printf("pager->root_page%u\n",pager->root_page);
	printf("pager->fd=%d\n",pager->fd);
	printf("pager->page_size=%d\n",pager->page_size);
	printf("pager->num_pages=%d\n",pager->num_pages);
	pager_close(pager);
	return 0;
}
