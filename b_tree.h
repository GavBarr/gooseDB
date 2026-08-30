#ifndef B_TREE_H
#define B_TREE_H
#define COMMON_HEADER 6
#define LEAF_HEADER 4
#define PAGE_SIZE 4096
#define LEAF 1
#define INTERNAL 0
#define INVALID_PAGE -1

#include "pager.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* internal cell = 8bytes, (key, child_page_num)
 * leaf cell = sizeof(Row) + sizeof(uint32_t), (key, row_data)
 */


typedef struct __attribute__((packed)){
	uint8_t node_type;
	uint8_t is_root;
	uint32_t parent_pointer; //page number
} CommonHeader;

typedef struct __attribute__((packed)){

	uint32_t num_cells;

} LeafHeader;

typedef struct __attribute__((packed)){
	uint32_t num_keys;
	uint32_t right_child_pointer;
} InternalHeader;

typedef struct __attribute__((packed)){
        CommonHeader common_header;
	union{
		LeafHeader leaf_header;
		InternalHeader internal_header;
	};
} BTreeNode;

typedef struct __attribute__((packed)){
	uint32_t id;
        char username[32];
	char email[255];
} Row;

void btree_create(Pager* pager);
int find_leaf(Pager* pager, int page_num, uint32_t key);
int leaf_find_cell(Pager* pager, int page_num, uint32_t key);
int leaf_insert_cell(Pager* pager, int page_num, uint32_t key, Row* row);

#define ROW_SIZE (sizeof(Row))
#define CELL_SIZE (sizeof(uint32_t)+ROW_SIZE)
#define INTERNAL_CELL_SIZE (sizeof(uint32_t)+sizeof(uint32_t))
#define LEAF_NODE_HEADER_SIZE (sizeof(BTreeNode))//(sizeof(COMMON_HEADER))
#define LEAF_NODE_SPACE_FOR_CELLS (PAGE_SIZE - LEAF_NODE_HEADER_SIZE)
#define LEAF_NODE_MAX_CELLS (LEAF_NODE_SPACE_FOR_CELLS / CELL_SIZE)

#endif
