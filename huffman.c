#include "huffman.h"

/*读取文件内容,生成源码表、频次表以及最大长度,单个源码长度8bits*/
int read(file, length, source, freq)
FILE* file;
unsigned int* length;
unsigned char* source;
FREQ_POS* freq;
{
	unsigned char line[128];
  unsigned int i,j,len=0;

	while(fgets(line, 127, file) != NULL){
		for (i = 0; i < 127; ++i)
		{
			if (line[i] == 0)break;
			for (j = 0; j < len; ++j)
			{
				if (source[j] == line[i])
				{
					++(freq[j].freq);
					break;
				}
			}
			if (j < len)continue;
			source[len] = line[i];
			freq[len].freq = 1;
			freq[len].pos = (int)len;
			++len;
		}
	}
	if (ferror(file))
	{
		printf("fail to read file!");
		return -1;
	}
	*length = len;
	return 0;
}

void sort(value, values, len)
FREQ_POS* values;
FREQ_POS value;
unsigned int len;
{
	FREQ_POS temp;
	unsigned int i;
	for (i = 0; i < len; ++i)
	{
		if (value.freq > values[i].freq)break;
	}
	for (; i < len; ++i)
	{
		temp = values[i];
		values[i] = value;
		value = temp;
	}
	values[i] = value;
}

unsigned int get_num(unsigned int len)
{
	unsigned int temp = 0;
	while(1){
		temp += len;
		if (len == 1)break;
		if ( (len % 2) != 0 )
			len = len / 2 + 1;
		else
			len = len / 2;
	}
	return temp;
}

/*按位反序*/
unsigned int reverse_bits(value, len)
unsigned int value, len;
{
	unsigned int i, temp = 0;
	for (i = 0; i < len; ++i)
	{
		temp += ((value >> i) & 0x01) << (len - i - 1);
	}
	return temp;
}

/*生成Huffman编码*/
void generate_huffman_code(leaf, leafs, len, map)
HUFFMAN_LEAF leaf;
HUFFMAN_LEAF* leafs;
unsigned int len;
HUFFMAN_MAP* map;
{
	unsigned int i, j, k = 0, symbol = 0, next_pos = leaf.parent_pos;
	if (leaf.is_right) 
		j = 0;
	else 
		j = 1;
	while(1){
		for (i = 0; i < len; ++i)
		{
			if (leafs[i].value.pos == next_pos)
			{
				symbol = (symbol << 1) + j;
				++k;
				if (leafs[i].is_right) 
					j = 0;
				else 
					j = 1;
				next_pos = leafs[i].parent_pos;
				break;
			}
		}
		if(i == len)break;
	}
	map->len = k;
	map->symbol = reverse_bits(symbol, k);
}

void encode(length, freq, map)
unsigned int length;
FREQ_POS* freq;
HUFFMAN_MAP* map;
{
	unsigned int num, i, j, k;
	int gen_pos = length;
	FREQ_POS temp_freq;
	FREQ_POS* sort_freq;
	HUFFMAN_LEAF* leafs;

	sort_freq = malloc((length + 1) * sizeof(FREQ_POS));
	/*定义最长Huffman树*/
	num = get_num(length);
	printf("max leafs:%d\n", num);
	leafs = malloc((num + 1) * sizeof(HUFFMAN_LEAF));
	/*按照频率排序(低频在高位)*/
	for (i = 0; i < length; ++i)
	{
		sort(freq[i], sort_freq, i);
	}
	/*构造Huffman树*/
	i = length - 1;
	j = 0;
	while(i > 0){
		/*Huffman树左节点*/
		leafs[j].value = sort_freq[i];
		leafs[j].parent_pos = gen_pos;
		leafs[j].is_right = false;
		if(sort_freq[i].pos < length)
			leafs[j].is_leaf = true;
		else
			leafs[j].is_leaf = false;
		/*Huffman树右节点*/
		leafs[j + 1].value = sort_freq[i - 1];
		leafs[j + 1].parent_pos = gen_pos;
		leafs[j].is_right = true;
		if(sort_freq[i - 1].pos < length)
			leafs[j + 1].is_leaf = true;
		else
			leafs[j + 1].is_leaf = false;
		/*新构造的频次映射点存放在频次映射列表最后*/
		temp_freq.freq = sort_freq[i].freq + sort_freq[i - 1].freq;
		temp_freq.pos = gen_pos;
		freq[gen_pos] = temp_freq;
		++gen_pos;
		/*给新构造的频次映射点排序*/
		sort(temp_freq, sort_freq, i - 1);

		j+=2;
		--i;
	}
	/*根节点*/
	leafs[j].value = sort_freq[i];
	leafs[j].parent_pos = gen_pos;
	leafs[j].is_right = false;
	if(sort_freq[i].pos < length)
		leafs[j].is_leaf = true;
	else
		leafs[j].is_leaf = false;
	/*生成Huffman表*/
	k = j + 1;
	printf("tree leafs:%d %d\n", k, gen_pos);
	j = 0;
	for (i = 0; i < k; ++i)
	{
		/*生成一个映射*/
		if (leafs[i].is_leaf)
		{
			map[j].source = leafs[i].value.pos;
			generate_huffman_code(leafs[i], leafs, k, map + j);
			++j;
		}
	}
	printf("huffman table length:%d\n", j);
	free(sort_freq);
	free(leafs);
}

void decode(length, map, input, output)
unsigned int length;
HUFFMAN_MAP *map;
unsigned char *input, *output;
{

}

int main(int argc,char *argv[])
{
	FILE* file;
	unsigned int i, length;
	unsigned char source[256];
	FREQ_POS freq[512];
	HUFFMAN_MAP* map;

	if( (file = fopen(argv[1], "r")) == NULL ){
		return -1;
	}
	if( read(file, &length, source, freq) != 0 ){
		return -2;
	}
	printf("length:%d\n", length);
	/*Huffman索引*/
	map = malloc((length + 1) * sizeof(HUFFMAN_MAP));
	encode(length, freq, map);
	printf("huaffman table done!\n");
	/*打印Huffman表*/
	for (i = 0; i < length; ++i)
	{
		printf("%c:%d %d\n", source[map[i].source], map[i].len, map[i].symbol);
	}
	free(map);
	return 0;
}
