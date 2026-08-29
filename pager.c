#include "pager.h"
#include <complex.h>
#include <fcntl.h>   // For flags like O_RDONLY, O_WRONLY, O_CREAT
#include <unistd.h>  // For system calls like close(), read(), write()
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

HashTable* hash_table;

static void move_lru_head(Pager* pager, int frame_index);
static uint32_t lookup_has_entry(HashTable* hashtable, int key);
static int insert_hash_entry(HashTable* hash_table, int key, int value);
static int remove_hash_entry(HashTable* hash_table, int slot);
static int header_check(Pager* pager, int fd);
static int find_free_frame(Pager* pager);
static int evict_page_frame(Pager* pager);

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
		perror("fstat");
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
		pager->lru_tail = -1;

	}

	for (int p = 0; p < CACHE_SIZE; p++){
		pager->frames[p].page_num = -1;
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


/**
 * @brief Marking a page as dirty
 *
 * Marking the designated page as dirty, so we can later write the page during
 * the pager_flush() function. 
 *
 * @param Pager* pager - the global pager.
 * @param int page_num - page number that needs to be marked as dirty.
 * @return void, nothing to return.
 */
void pager_mark_dirty(Pager* pager, int page_num){
	if (page_num < 0) return;
        int frame_index = lookup_has_entry(hash_table, page_num);

        if (frame_index != -1){
                pager->frames[frame_index].is_dirty = true;
        }

        //linear scan if not in hashtable
        for(int i = 0; i < CACHE_SIZE; i++){
                if (pager->frames[i].page_num == page_num){
                        pager->frames[i].is_dirty = true;
			printf("marked is_dirty!\n");
                }
        }


}

void pager_flush(Pager* pager, int page_num){
	if (page_num < 0) return;
        int frame_index = lookup_has_entry(hash_table, page_num);

        if (frame_index == -1){
		//linear scan if not in hashtable
	        for(int i = 0; i < CACHE_SIZE; i++){
        	        if (pager->frames[i].page_num == page_num){
                	        frame_index = i;
				break;
                	}
        	}
        }

	if (frame_index == -1){
		printf("frame_index%d\n",frame_index);
	}

	off_t offset = (off_t)(page_num * pager->page_size);
	
	printf("frame_index%d\n",frame_index);
	pager->frames[frame_index].pin_count++;
	ssize_t bytes_written = pwrite(pager->fd, pager->frames[frame_index].data, pager->page_size, offset);
	if (bytes_written <=0 ){
		perror("pwrite");
	}

	pager->frames[frame_index].pin_count--;

	pager->frames[frame_index].is_dirty=false;

	if (bytes_written <= 0)
	{	
		perror("write");
		printf("failed to flush page: %d", page_num);
	}
}


void pager_close(Pager* pager){

	for (int i = 0; i < CACHE_SIZE; i++){
		if (pager->frames[i].is_dirty){
			pager_flush(pager, pager->frames[i].page_num);
		}
	}
	
	close(pager->fd);
	free(pager);

}

static uint32_t hash(int key, int table_size){

	return key % table_size;
}

static int evict_page_frame(Pager* pager){
	int frame_index = pager->lru_tail;
	int i = 0;

	while(1){
		if (i > CACHE_SIZE)break;
		
		if (pager->frames[frame_index].pin_count == 0){
		
			return frame_index;
		}
		frame_index = pager->frames[frame_index].lru_prev;
		i++;
	}


	return -1;
}
//HEAD
// ↓
//Frame 2 → Frame 0 → Frame 3
//                       ↑
//                      TAIL
//Frame 2:
//    prev = -1
//    next = 0

//Frame 0:
//    prev = 2
//    next = 3

//Frame 3:
//    prev = 0
//    next = -1
static void move_lru_head(Pager* pager, int frame_index){
	pager->frames[frame_index].lru_prev = -1; //set prev -1, this is the new head
	pager->frames[frame_index].lru_next = pager->lru_head;

	// check for empty list
	if (pager->lru_head != -1){
		pager->frames[pager->lru_head].lru_prev = frame_index;

	}else{
		pager->lru_tail = frame_index;
	}

	pager->lru_head = frame_index;


}


int pager_allocate_page(Pager* pager){
	
	int last_page = pager->num_pages; //-1 is so that we are zero based 
		
	ftruncate(pager->fd, (last_page + 1) * pager->page_size);

	return last_page + 1;
}

void* pager_get_page(Pager* pager, int page_num){
	if (page_num < 0) return NULL;
	int frame_index = lookup_has_entry(hash_table, page_num);

	if (frame_index != -1){
		//set lru_prev and lru_next as necessary, lru_head included.
		move_lru_head(pager, frame_index);

		return pager->frames[frame_index].data;
	}

	//linear scan if not in hashtable
	for(int i = 0; i < CACHE_SIZE; i++){
		if (pager->frames[i].page_num == page_num){
			move_lru_head(pager, frame_index);
			return pager->frames[i].data;
		}
	}
	
	//find free frame
	frame_index = find_free_frame(pager);

	if (frame_index == -1){
		printf("free frame not found!\n");
		frame_index = evict_page_frame(pager);
		if (frame_index == -1){
			return NULL;
		}
	}

	pager->frames[frame_index].pin_count++;
	ssize_t data_read = pread(pager->fd, pager->frames[frame_index].data ,pager->page_size,page_num * pager->page_size);
	pager->frames[frame_index].pin_count--;

	if (data_read == -1){
		printf("data read has 0 bytes!\nn");

	}
	
	printf("page_num_hash->%d\n",page_num);
	insert_hash_entry(hash_table, frame_index, page_num);
	move_lru_head(pager, frame_index);
	pager->frames[frame_index].page_num = page_num;	
	
	return pager->frames[frame_index].data;
	//return NULL;
}


static int find_free_frame(Pager* pager){
	for (int i = 0; i < CACHE_SIZE; i++){
		if (pager->frames[i].page_num == -1){
			return i;
		}
	}
	return -1; //return -1 if none are free
}

static int header_check(Pager* pager, int fd){
	FileHeader* header = malloc(PAGE_SIZE + 1); // +1 for null terminator

	ssize_t bytes_read = pread(fd, header, PAGE_SIZE, 0);
	if (memcmp(header,"GOOSESDB",8) != 0) return -1;

	pager->root_page = 1;//header->root_page;	
	free(header);

	return 0;
}
