#include "b_tree.h"
#include "pager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


void btree_create(Pager* pager){
	//printf("page_num->%d\n",pager_allocate_page(pager));
	void* page = pager_get_page(pager, pager->root_page);//pager_allocate_page(pager);
	BTreeNode* node = (BTreeNode*)page;

	printf("node->common_header.node_type=%d\n",node->common_header.node_type);
	node->common_header.node_type = LEAF;
	node->common_header.is_root = 1;
	node->common_header.parent_pointer = INVALID_PAGE;
	node->leaf_header.num_cells = 0;
	pager_mark_dirty(pager, pager->root_page);
	pager_flush(pager, pager->root_page);
	return;
	FileHeader* header = (FileHeader*)pager_get_page(pager, 0);
	header->root_page = pager->root_page;
	pager_mark_dirty(pager, 0);
	pager_flush(pager, 0);

	printf("page_num->%d\n",pager->root_page); 	
}

