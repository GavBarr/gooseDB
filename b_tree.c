#include "b_tree.h"
#include "pager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


void btree_create(Pager* pager){
	//printf("page_num->%d\n",pager_allocate_page(pager));
	//return;
	void* page = pager_get_page(pager, pager->root_page);//pager_allocate_page(pager);
	BTreeNode* node = (BTreeNode*)page;

	node->common_header.node_type = LEAF;
	node->common_header.is_root = 1;
	node->common_header.parent_pointer = INVALID_PAGE;
	node->leaf_header.num_cells = 0;

	pager_mark_dirty(pager, pager->root_page);
	pager_flush(pager, pager->root_page);

	FileHeader* header = (FileHeader*)pager_get_page(pager, 0);
	header->root_page = pager->root_page;

	pager_mark_dirty(pager, 0);
	pager_flush(pager, 0);

}

int leaf_find_cell(Pager* pager, int page_num, uint32_t key){
	void* page = pager_get_page(pager, pager_num);

	BTreeNode* node = (BTreeNode*)page;
	if (node->common_header.node_type != LEAF){
		return -1;
	}

	//(key, row_data)
	uint8_t* cell_array = (uint8_t*)page + sizeof(BTreeNode);
	for (int i = 0; i < node->leaf_header.num_cells; i++){
		uint8_t* cell = cell_array + (i * CELL_SIZE);
		uint8_t cell_key = *(uint32_t*)cell; //key = 4bytes
		uint8_t cell_row = *(ROW_SIZE*)(cell + sizeof(uint32_t));
		if (key == cell_key){
			return i; //return the correct cell array index
		}
	}		
	return -1;
}


int leaf_insert_cell(Pager* pager, int page_num, uint32_t key, Row* row){
	
	void* page = pager_get_page(pager, page_num);	
	BTreeNode* node = (BTreeNode*)page;
	
	
}


int find_leaf(Pager* pager, int page_num, uint32_t key){
	void* page  = pager_get_page(pager, page_num);

	BTreeNode* node = (BTreeNode*)page;

	if (node->common_header.node_type == LEAF){
		return page_num;
	}

	uint8_t* cell_array = (uint8_t*)page + sizeof(BTreeNode);
	for (int i = 0; i < node->internal_header.num_keys; i++){
		uint8_t* cell = cell_array + (i * INTERNAL_CELL_SIZE);
		uint32_t cell_key = *(uint32_t*)cell; //only grab first 4 bytes
		uint32_t cell_child = *(uint32_t*)(cell + sizeof(uint32_t));

		//left child node else right child node
		if (key < cell_key){	
			return find_leaf(pager, cell_child, key);
		}

	}


	return find_leaf(pager, node->internal_header.right_child_pointer, key);
}

