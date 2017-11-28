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
	unsigned long freq;
	int pos;
} FREQ_POS;

typedef struct 
{
	FREQ_POS value;
	bool is_leaf;
	bool is_right;
	int parent_pos;
} HUFFMAN_LEAF;

int encode(unsigned char*, unsigned char*);
void decode(unsigned int, HUFFMAN_MAP*, unsigned char*, unsigned char*);
