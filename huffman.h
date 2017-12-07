#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

typedef struct 
{
	unsigned char len;
	unsigned char *symbol;
} HUFFMAN_MAP;

typedef struct huffman_tree
{
	unsigned char is_leaf;
	struct huffman_tree *parent;
	union
	{
		unsigned char symbol;
		unsigned long freq;
	};
	union
	{
		struct
		{
			struct huffman_tree *left_point, *right_point;
		};
		unsigned char is_right;
	};
} HUFFMAN_TREE_NODE;

int encode(unsigned char*, unsigned char*);
int decode(unsigned char*, unsigned char*);
