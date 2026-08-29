#ifndef B_TREE_H
#define B_TREE_H
#define COMMON_HEADER 6
#define LEAF_HEADER 4
#define PAGE_SIZE 4096

#include <stdio.h>
#include <stdint.h>

typedef struct __attribute__((packed)){
	uint8_t node_type;
	uint8_t is_root;
	uint32_t parent_pointer; //page number
} CommonHeader;

typedef struct __attribute__((packed)){

	uint32_t num_cells;

} LeafHeader;

typedef struct __attribute__((packed)){
	uint32_t id;
        char username[32];
	char email[255];
} Row;

#define ROW_SIZE (sizeof(Row))
#define CELL_SIZE (sizeof(uint32_t)+ROW_SIZE)
#define LEAF_NODE_HEADER_SIZE (sizeof(CommonHeader))
#define LEAF_NODE_SPACE_FOR_CELLS (PAGE_SIZE - LEAF_NODE_HEADER_SIZE)
#define LEAF_NODE_MAX_CELLS (LEAF_NODE_SPACE_FOR_CELLS / CELL_SIZE)

#endif
