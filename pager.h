#ifndef PAGER_H
#define PAGER_H
#define CACHE_SIZE 127
#define PAGE_SIZE 4096

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct{
	char      magic[8];
	uint32_t  page_size;
	uint32_t  root_page;
	uint32_t  verions;
} FileHeader;

typedef struct{
	int key;
	int frame_index;
} HashTable;

typedef struct{
        int      page_num;
	bool     is_dirty;
	bool	   in_use;
	uint32_t pin_count;
        uint8_t  data[PAGE_SIZE];
	int lru_prev; //double linked list (inner)	
	int lru_next; //double linked list (inner)	
} PageFrame;

typedef struct{
	int       fd;
	int	  page_size;
	int       num_pages;
	PageFrame frames[CACHE_SIZE];
	int       lru_head;
	int       lru_previous;
} Pager;


Pager* pager_open(char* const file_dir);


#endif
