#include "huffman.h"

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

/*读取文件内容,生成源码频次表,单个源码长度8bits*/
unsigned int read(filename, length, freq)
char* filename;
unsigned int* length;
FREQ_POS* freq;
{
	FILE* file;
	unsigned char cache;
	FREQ_POS temp_freq[256];
  unsigned int i,total;
  int res;

  if( (file = fopen(filename, "rb")) == NULL ){
		return 0;
	}
  /*初始化*/
  for (i = 0; i < 256; ++i)
  {
  	temp_freq[i].freq = 0;
  	temp_freq[i].pos = i;
  	freq[i].freq = 0;
  	freq[i].pos = 0;
  }
  total = 0;
	while( (res = fgetc(file)) != EOF ){
		cache = (unsigned char)res;
		++(temp_freq[cache].freq);
		++total;
	}
	if (ferror(file))
	{
		printf("fail to read file!\n");
		return 0;
	}
	fclose(file);
	/*初始化排序频次, 按照频率排序(低频在高位)*/
	*length = 0;
	for (i = 0; i < 256; ++i)
	{
		if (temp_freq[i].freq > 0)
		{
			sort(temp_freq[i], freq, *length);
			++(*length);
		}
	}
	return total;
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
		temp |= ((value >> i) & 0x01) << (len - i - 1);
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

void encode_table(length, freq, map)
unsigned int length;
FREQ_POS* freq;
HUFFMAN_MAP* map;
{
	unsigned int num, i, j, k;
	int gen_pos = -1;
	FREQ_POS temp_freq;
	HUFFMAN_LEAF* leafs;

	/*定义最长Huffman树*/
	num = get_num(length);
	leafs = malloc((num + 1) * sizeof(HUFFMAN_LEAF));
	/*构造Huffman树*/
	i = length - 1;
	j = 0;
	while(i > 0){
		/*Huffman树左节点*/
		leafs[j].value.freq = freq[i].freq;
		leafs[j].value.pos = freq[i].pos;
		leafs[j].parent_pos = gen_pos;
		leafs[j].is_right = false;
		if(freq[i].pos > 0)
			leafs[j].is_leaf = true;
		else
			leafs[j].is_leaf = false;
		/*Huffman树右节点*/
		leafs[j + 1].value.freq = freq[i - 1].freq;
		leafs[j + 1].value.pos = freq[i - 1].pos;
		leafs[j + 1].parent_pos = gen_pos;
		leafs[j + 1].is_right = true;
		if(freq[i - 1].pos > 0)
			leafs[j + 1].is_leaf = true;
		else
			leafs[j + 1].is_leaf = false;
		/*给新构造的频次映射点排序*/
		temp_freq.freq = freq[i].freq + freq[i - 1].freq;
		temp_freq.pos = gen_pos;
		sort(temp_freq, freq, i - 1);

		--gen_pos;
		j+=2;
		--i;
	}
	/*根节点*/
	leafs[j].value.freq = freq[i].freq;
	leafs[j].value.pos = freq[i].pos;
	leafs[j].parent_pos = gen_pos;
	leafs[j].is_right = false;
	if(freq[i].pos > 0)
		leafs[j].is_leaf = true;
	else
		leafs[j].is_leaf = false;
	/*生成Huffman表*/
	k = j + 1;
	j = 0;
	for (i = 0; i < 256; ++i)
	{
		map[i].len = 0;
		map[i].symbol = 0;
	}
	for (i = 0; i < k; ++i)
	{
		/*生成一个映射*/
		if (leafs[i].is_leaf)
		{
			generate_huffman_code(leafs[i], leafs, k, map + leafs[i].value.pos);
			++j;
		}
	}
	free(leafs);
}

int encode_file(infilename, total, length, map, outfilename)
char *infilename, *outfilename;
unsigned int total, length;
HUFFMAN_MAP* map;
{
	FILE *infile, *outfile;
	unsigned char source, cache[128], symbol, mask, p;
  unsigned int i, j, k, temp;
  unsigned char *map_cache;
  int res;

  if( (infile = fopen(infilename, "rb")) == NULL ){
		return -1;
	}
	if( (outfile = fopen(outfilename, "wb")) == NULL ){
		return -1;
	}
	/*写入长度*/
	temp = htonl(length);
	fwrite(&temp, sizeof(unsigned int), 1, outfile);
	temp = htonl(total);
	fwrite(&temp, sizeof(unsigned int), 1, outfile);
	/*写入huffman表*/
	k = length * (sizeof(unsigned int) + 2) + 1;
	map_cache = malloc(k);
	memset(map_cache, 0, k);
	j = 0;
	for (i = 0; i < 256; ++i)
	{
		if(map[i].len > 0)
		{
			map_cache[j] = i;
			map_cache[j + 1] = map[i].len;
			temp = htonl(map[i].symbol);
			memcpy(map_cache + j + 2, &temp, sizeof(unsigned int));
			j += sizeof(unsigned int) + 2;
		}
	}
	fwrite(map_cache, sizeof(unsigned char), j, outfile);
	free(map_cache);
	/*写入编码*/
	j = 0;
	mask = 0;
	p = 0;
	while( (res = fgetc(infile)) != EOF )
	{
		source = (unsigned char)res;
		/*字节写入*/
		cache[j] = cache[j] & mask;
		k = map[source].len;
		symbol = map[source].symbol;
		while(k)
		{
			if ((8 - p) > k)
			{
				/* 不能补全字节 */
				p += k;
				cache[j] = cache[j] | (symbol << (8 - p));
				mask = 255 << (8 - p);
				break;
			}
			else if ((8 - p) < k)
			{
				/* 超出字节 */
				k -= 8 - p;
				p = 0;
				cache[j] = cache[j] | (symbol >> k);
				symbol = symbol & (255 >> (8 - k));
				++j;
				cache[j] = 0;
				if(j == 128)
				{
					fwrite(cache, sizeof(unsigned char), 128, outfile);
					j = 0;
				}
			}
			else
			{
				/* 整字节 */
				p = 0;
				cache[j] = cache[j] | symbol;
				mask = 0;
				++j;
				if(j == 128)
				{
					fwrite(cache, sizeof(unsigned char), 128, outfile);
					j = 0;
				}
				break;
			}
		}
	}
	if (ferror(infile))
	{
		printf("fail to read file!\n");
		fclose(outfile);
		return -1;
	}
	fwrite(cache, sizeof(unsigned char), j, outfile);
	fclose(outfile);
	fclose(infile);
	return 0;
}

int read_decode_file(file, cache, length)
FILE *file;
unsigned char *cache;
unsigned int length;
{
	unsigned int i;
	int res;

	i = 0;
	while( (res = fgetc(file)) != EOF )
	{
		cache[i] = (unsigned char)res;
		++i;
		if(i == length)break;
	}
	if (ferror(file))
	{
		printf("fail to read file!\n");
		return -1;
	}
	else if (feof(file))
	{
		if(i == 0)
		{
			return -2;
		}
	}
	return i;
}

int decode(infilename, outfilename)
unsigned char *infilename, *outfilename;
{
	FILE *infile, *outfile;
	unsigned int i, j, k, total, length, temp, mask;
	unsigned char line[128], cache[128], p;
	int res;
	HUFFMAN_MAP_D *d_map, map_cache, temp_map;

	if( (infile = fopen(infilename, "rb")) == NULL ){
		printf("fail to open file:%s!\n", infilename);
		return -1;
	}
	/*初始化表长度和信息长度*/
	if ( (res = read_decode_file(infile, line, 2 * sizeof(unsigned int))) <= 0 )
	{
		printf("read file fail:%d\n", res);
		return -1;
	}
	length = 0;
	p = sizeof(unsigned int);
	for(i = 0; i < p; ++i)
	{
		temp = line[i];
		length += temp << (8*(p - i - 1));
	}
	total = 0;
	p = 2 * sizeof(unsigned int);
	for(; i < p; ++i)
	{
		temp = line[i];
		total += temp << (8*(p - i - 1));
	}
	if (total == 0 || length == 0){
		printf("error file format!\n");
		fclose(infile);
		return -2;
	}
	/*初始化huffman编码表*/
	d_map = malloc(sizeof(HUFFMAN_MAP_D) * (length + 1));
	for (i = 0; i < length; ++i)
	{
		/*读取一个映射*/
		if ( (res = read_decode_file(infile, line, sizeof(unsigned int) + 2)) <= 0 )
		{
			printf("read file fail:%d\n", res);
			free(d_map);
			return -1;
		}
		map_cache.source = line[0];
		map_cache.map.len = line[1];
		map_cache.map.symbol = 0;
		for(j = 2; j < 2 + sizeof(unsigned int); ++j)
		{
			temp = line[j];
			map_cache.map.symbol += temp << (8*(sizeof(unsigned int) + 1 - j));
		}
		/*排序编码表(按编码有效长度从小到大)*/
		for (j = 0; j < i; ++j)
		{
			if (map_cache.map.len < d_map[j].map.len)break;
		}
		for (; j < i; ++j)
		{
			temp_map = d_map[j];
			d_map[j] = map_cache;
			map_cache = temp_map;
		}
		d_map[j] = map_cache;
	}
	/*解码*/
	if( (outfile = fopen(outfilename, "wb")) == NULL ){
		printf("fail to open file:%s!\n", outfilename);
		free(d_map);
		fclose(infile);
		return -1;
	}
	p = 0;
	k = 0; 
	mask = 0; 
	while( (res = read_decode_file(infile, line, 127)) > 0)
	{
		for (i = 0; i < res; ++i)
		{
			if (total == 0)break;
			if (mask > 0)
				mask = (mask << 8) + line[i];
			else 
				mask = line[i];
			p += 8;
			while(p)
			{
				temp = 0;
				for (j = 0; j < length; ++j)
				{
					if (d_map[j].map.len <= p)
					{
						if( d_map[j].map.symbol == (mask >> (p - d_map[j].map.len)) )
						{
							cache[k] = d_map[j].source;
							++k;
							--total;
							mask -= d_map[j].map.symbol << (p - d_map[j].map.len);
							p -= d_map[j].map.len;
							temp = 1;
							break;
						}
					}
					else break;
				}
				if (k == 127)
				{
					fwrite(cache, sizeof(unsigned char), 127, outfile);
					k = 0;
				}
				if (j == length)
				{
					printf("unknown code:%d\n", mask);
					free(d_map);
					fclose(outfile);
					fclose(infile);
					return -2;
				}
				if(temp == 0 || total == 0)break;
			}
		}
	}
	free(d_map);
	if (res == -1)
	{
		printf("fail to read file!\n");
		fclose(outfile);
		return -1;
	}
	if(k > 0)fwrite(cache, sizeof(unsigned char), k, outfile);
	fclose(outfile);
	fclose(infile);
	return 0;
}

int encode(infilename, outfilename)
unsigned char *infilename, *outfilename;
{
	unsigned int i, total, length;
	FREQ_POS freq[256];
	HUFFMAN_MAP map[256];

	if( (total = read(infilename, &length, freq)) == 0 ){
		printf("read file fail!\n");
		return -1;
	}
	/*Huffman索引*/
	encode_table(length, freq, map);
	/*生成Huffman编码后的文件*/
	if( encode_file(infilename, total, length, map, outfilename) != 0){
		printf("encode file fail!\n");
		return -2;
	}
	return 0;
}
