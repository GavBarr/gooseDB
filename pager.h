#ifndef PAGER_H
#define PAGER_H
#define CACHE_SIZE 127
#define PAGE_SIZE 4096

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed)){
	uint32_t  key; //page in our case
	uint32_t  value; //pageframe index in our case
	void* next;
	void* prev;

} HashTableEntry;

typedef struct __attribute__((packed)){
	char      magic[8];
	uint32_t  page_size;
	uint32_t  root_page;
	uint32_t  verions;
} FileHeader;

typedef struct __attribute__((packed)){
	HashTableEntry** buckets;
	uint32_t size;
} HashTable;

typedef struct __attribute__((packed)){
        int      page_num;
	bool     is_dirty;
	bool	   in_use;
	uint32_t pin_count;
        uint8_t  data[PAGE_SIZE]; //uint8_t because we want 1 byte per index
	int lru_prev; //frame_index - double linked list (inner)	
	int lru_next; //frame_index - double linked list (inner)	
} PageFrame;

typedef struct __attribute__((packed)){
	int       fd;
	int	  page_size;
	int       num_pages;
	uint32_t  root_page;
	PageFrame frames[CACHE_SIZE];
	HashTable frameHashTable[CACHE_SIZE];
	int       lru_head;
	int       lru_tail;
} Pager;


Pager* pager_open(char* const file_dir);
void* pager_get_page(Pager* pager, int page_num);
void   pager_mark_dirty(Pager* pager, int page_num);
void   pager_flush(Pager* pager, int page_num);     // write one page back to disk
void   pager_close(Pager* pager);                   // flush all dirty pages, close fd
int pager_allocate_page(Pager* pager);  // grow the file by one page, return its number

#endif
