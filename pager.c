#include "pager.h"
#include <fcntl.h>   // For flags like O_RDONLY, O_WRONLY, O_CREAT
#include <unistd.h>  // For system calls like close(), read(), write()
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


/*
   typedef struct{
          int key;
          int frame_index;
  } HashTable;
                   
  typedef struct{
          int      page_num;
          bool     is_dirty;
          bool       in_use;
          uint32_t pin_count;
          uint8_t  data[PAGE_SIZE];
          int lru_prev; //double linked list (inner)      
          int lru_next; //double linked list (inner)      
  } PageFrame;
         
  typedef struct{
          int       fd;
          int       page_size;
          int       num_pages;
          PageFrame frames[CACHE_SIZE];
          int       lru_head;
          int       lru_previous;
  } Pager;
*/

static int header_check(int fd);

Pager* pager_open(char* const file_dir){
	Pager* pager = malloc(sizeof(Pager));
	
	int fd = open(file_dir, O_RDWR | O_CREAT,0644);
	if (fd == -1){
		perror("open");
		printf("failed to get fd");
		return NULL;
	}

	struct stat file_info;

	if (fstat(fd, &file_info) != 0){
		printf("failed to get fstat()");
		return NULL;
	}
	printf("file_info.st_size->%lu\n",file_info.st_size);
	if (file_info.st_size > 0){
		if(header_check(fd) == -1){
			printf("header check fail\n");
			return NULL;
		}
		pager->fd = fd;
		pager->num_pages = ((file_info.st_size + PAGE_SIZE - 1) / PAGE_SIZE); //we skip the header or first "page" of the num of pages
		pager->page_size = PAGE_SIZE;
		pager->lru_head = -1;
		pager->lru_previous = -1;
	}
	

	return pager;
}

static int header_check(int fd){
	FileHeader header[PAGE_SIZE + 1]; // +1 for null terminator

	ssize_t bytes_read = read(fd, header, PAGE_SIZE);
	if (memcmp(header,"GOOSESDB",8) != 0) return -1;
	
	return 0;
}
