#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

typedef struct 
{
	unsigned char len;
	unsigned int symbol;
} HUFFMAN_MAP;

typedef struct 
{
	unsigned int freq;
	int pos;
} FREQ_POS;

typedef struct 
{
	FREQ_POS value;
	bool is_leaf;
	bool is_right;
	int parent_pos;
} HUFFMAN_LEAF;

typedef struct 
{
	unsigned char source;
	HUFFMAN_MAP map;
} HUFFMAN_MAP_D;

int encode(unsigned char*, unsigned char*);
int decode(unsigned char*, unsigned char*);
