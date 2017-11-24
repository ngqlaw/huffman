#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

typedef struct 
{
	unsigned int len;
	unsigned int symbol;
	int source;
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

int read(FILE*, unsigned int*, unsigned char*, FREQ_POS*);
void encode(unsigned int, FREQ_POS*, HUFFMAN_MAP*);
void decode(unsigned int, HUFFMAN_MAP*, unsigned char*, unsigned char*);
