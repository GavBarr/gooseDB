#include "pager.h"
#include <fcntl.h>   // For flags like O_RDONLY, O_WRONLY, O_CREAT
#include <unistd.h>  // For system calls like close(), read(), write()
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>


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
HashTable* hash_table;


static uint32_t lookup_has_entry(HashTable* hashtable, int key);
static int insert_hash_entry(HashTable* hash_table, int key, int value);
static int remove_hash_entry(HashTable* hash_table, int slot);
static int header_check(Pager* pager, int fd);

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
		if(header_check(pager, fd) == -1){
			printf("header check fail\n");
			return NULL;
		}
		pager->fd = fd;
		pager->num_pages = ((file_info.st_size + PAGE_SIZE - 1) / PAGE_SIZE); //we skip the header or first "page" of the num of pages
		pager->page_size = PAGE_SIZE;
		pager->lru_head = -1;
		pager->lru_previous = -1;
	}

	for (int p = 0; p < CACHE_SIZE; p++){
		pager->frames[p].in_use = 0;
		pager->frames[p].is_dirty = 0;
	}
	
	//create HashTable
	hash_table = malloc(257);
	hash_table->size = 257;
	hash_table->buckets = malloc(sizeof(HashTableEntry*) * hash_table->size);

		
	return pager;
}

static int remove_hash_entry(HashTable* hash_table, int slot){
	if (hash_table->buckets[slot] != NULL){
		free(hash_table->buckets[slot]);
	}else{
		return -1;
	}

	return 0;
}

static uint32_t lookup_has_entry(HashTable* hashtable, int key){
	int slot = key % hashtable->size;

	
	HashTableEntry* entry = hashtable->buckets[slot];
	if (entry != NULL){
		if (entry->key == key)
		{
			return entry->value;
		}
		entry = (HashTableEntry*)entry->next; //traverse forward
	}

	return -1;

}

static int insert_hash_entry(HashTable* hash_table, int key, int value){
	int slot = key % hash_table->size;

	HashTableEntry* entry = malloc(sizeof(HashTableEntry));
	if (entry == NULL) return -1;

	entry->key = key;
	entry->value = value;

	entry->next = hash_table->buckets[slot];
	hash_table->buckets[slot] = entry;



	return 0;
}


static uint32_t hash(int key, int table_size){

	return key % table_size;
}


void* pager_get_page(Pager* pager, int page_num){
	if (page_num < 0) return NULL;
	int frame_index = lookup_has_entry(hash_table, page_num);

	if (frame_index != -1){
		return pager->frames[frame_index].data;
	}

	//linear scan if not in hashtable
	for(int i = 0; i < CACHE_SIZE; i++){
		if (pager->frames[i].page_num == page_num){
			return pager->frames[i].data;
		}
	}

	return NULL;

}

static int header_check(Pager* pager, int fd){
	FileHeader* header = malloc(PAGE_SIZE + 1); // +1 for null terminator

	ssize_t bytes_read = pread(fd, header, PAGE_SIZE, 0);
	if (memcmp(header,"GOOSESDB",8) != 0) return -1;

	pager->root_page = header->root_page;	


	return 0;
}
