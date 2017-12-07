#include "huffman.h"

unsigned char * reverse_bits(bits, len, p)
unsigned char *bits, len, p;
{
	unsigned char c, *new;
	int i, j, k, u;

	new = malloc(len*sizeof(unsigned char));
	j = 0;
	new[0] = 0;
	u = 0;
	k = p;
	for (i = len - 1; i > -1; --i)
	{
		c = bits[i];
		while(k > 0)
		{
			new[j] = (new[j] << 1) | (c & 1);
			c >>= 1;
			--k;
			++u;
			if(u == 8)
			{
				++j;
				new[j] = 0;
				u = 0;
			}
		}
		k = 8;
	}
	if(u > 0)new[j] <<= (8 - u);
	free(bits);
	return new;
}

/*释放映射内存*/
void release_map(map)
HUFFMAN_MAP *map;
{
	int i;
	for (i = 0; i < 256; ++i)
	{
		if (map[i].len > 0 && map[i].symbol != NULL)
			free(map[i].symbol);
	}
}

/*编码释放huffman树内存*/
void release_encode_tree(tree)
HUFFMAN_TREE_NODE *tree;
{
	int i, j, k;
	HUFFMAN_TREE_NODE *p[256], *next;

	/*收集动态内存*/
	k = 0;
	for (i = 0; i < 256; ++i)
	{
		if( (next = tree[i].parent) == NULL )continue;
		while (next != NULL)
		{
			for (j = 0; j < k; ++j)
			{
				if(next == p[j])break;
			}
			if(j == k)
			{
				p[k] = next;
				++k;
			}
			next = next->parent;
		}
	}
	/*释放指向的动态内存*/
	for (i = 0; i < k; ++i)
	{
		if(p[i] != NULL)free(p[i]);
	}
}

/*解码释放huffman树内存*/
void release_decode_tree(tree)
HUFFMAN_TREE_NODE *tree;
{
	while(tree->left_point != NULL)
	{
		release_decode_tree(tree->left_point);
		tree->left_point = NULL;
	}
	while(tree->right_point != NULL)
	{
		release_decode_tree(tree->right_point);
		tree->right_point = NULL;
	}
	free(tree);
}

/*读取文件内容,单个源码长度8bits*/
unsigned int read_for_encode(filename, tree)
char* filename;
HUFFMAN_TREE_NODE* tree;
{
	FILE* file;
	unsigned char cache;
  unsigned int total;
  int res;

  if( (file = fopen(filename, "rb")) == NULL ){
  	printf("fail to read file!\n");
		return 0;
	}
  /*读取内容*/
  total = 0;
	while( (res = fgetc(file)) != EOF ){
		cache = (unsigned char)res;
		++(tree[cache].freq);
		++total;
	}
	if (ferror(file))
	{
		printf("fail to read file!\n");
		return 0;
	}
	fclose(file);
	return total;
}

int encode_table(tree)
HUFFMAN_TREE_NODE* tree;
{
	unsigned int i;
	HUFFMAN_TREE_NODE *cache, *p_min1, *p_min2;

	/*构造Huffman树*/
	while(1)
	{
		p_min1 = NULL;
		p_min2 = NULL;
		for (i = 0; i < 256; ++i)
		{
			if(tree[i].freq > 0)
			{
				cache = tree + i;
				while(cache->parent != NULL)
				{
					cache = cache->parent;
				}
				if(p_min1 != NULL && p_min1 != cache)
				{
					if(p_min1->freq > cache->freq)
					{
						p_min2 = p_min1;
						p_min1 = cache;
					}
					else 
					{
						if(p_min2 != NULL)
						{
							if(p_min2->freq > cache->freq)
								p_min2 = cache;
						}
						else
							p_min2 = cache;
					}
				}
				else if(p_min1 == NULL)
					p_min1 = cache;
			}
		}
		if(p_min1 != NULL && p_min2 != NULL)
		{
			cache = malloc(sizeof(HUFFMAN_TREE_NODE));
			cache->is_leaf = 0;
			cache->freq = p_min1->freq + p_min2->freq;
			cache->parent = NULL;
			p_min1->is_right = 1;
			p_min1->parent = cache;
			p_min2->is_right = 0;
			p_min2->parent = cache;
		}
		else if(p_min1 != NULL)
		{
			cache = p_min1;
			return 0; 
		}
		else
			return -1;
	}
}

int encode_file(infilename, total, length, tree, outfilename)
char *infilename, *outfilename;
unsigned int total, length;
HUFFMAN_TREE_NODE* tree;
{
	FILE *infile, *outfile;
	unsigned char source, cache[128], p1, p2, symbol, *bits;
  unsigned int temp, mask;
  int i, j, k, p, res;
  HUFFMAN_TREE_NODE *pt; 
  HUFFMAN_MAP map[256];

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
	for (i = 0; i < 256; ++i)
	{
		map[i].len = 0;
		if(tree[i].parent == NULL)continue;
		/*生成映射*/
		pt = tree + i;
		bits = malloc(sizeof(unsigned char));
		p1 = 0;
		p2 = 0;
		bits[p2] = 0;
		k = 0;
		while(1)
		{
			bits[p2] = (bits[p2] << 1) | pt->is_right;
			++p1;
			++k;
			if( (pt = pt->parent) == NULL)break;
			if(p1 == 8)
			{
				++p2;
				bits = realloc(bits, (p2 + 1)*sizeof(unsigned char));
				bits[p2] = 0;
				p1 = 0;
			}
		}
		p2 = p1 > 0 ? ++p2 : p2;
		map[i].len = k;
		map[i].symbol = reverse_bits(bits, p2, p1);
		/*写入*/
		if(map[i].len > 0)
		{
			cache[0] = i;
			cache[1] = map[i].len;
			j = 0;
			k = map[i].len;
			while (k > 0)
			{
				cache[2 + j] = map[i].symbol[j];
				k -= 8;
				++j;
			}
			fwrite(cache, sizeof(unsigned char), j + 2, outfile);
		}
	}
	/*写入编码*/
	i = 0;
	p1 = 8;
	cache[0] = 0;
	while( (res = fgetc(infile)) != EOF )
	{
		source = (unsigned char)res;
		/*字节写入*/
		k = map[source].len;
		p = 0;
		p2 = (k >= 8) ? 8 : k;
		symbol = map[source].symbol[0];
		while(k > 0)
		{
			if (p1 > p2)
			{
				/* 不能补全字节 */
				cache[i] = cache[i] | (symbol >> (8 - p1));
				p1 -= p2;
				k -= p2;
				if (k > 0)
				{
					++p;
					p2 = (k >= 8) ? 8 : k;
					symbol = map[source].symbol[p];
				}
			}
			else if (p1 < p2)
			{
				/* 超出字节 */
				p2 -= p1;
				k -= p1;
				cache[i] = cache[i] | (symbol >> (8 - p1));
				symbol = (symbol << p1) & 255;
				++i;
				if(i == 127)
				{
					fwrite(cache, sizeof(unsigned char), 127, outfile);
					i = 0;
				}
				p1 = 8;
				cache[i] = 0;
			}
			else
			{
				/* 整字节 */
				k -= p2;
				cache[i] = cache[i] | (symbol >> (8 - p1));
				++i;
				if(i == 127)
				{
					fwrite(cache, sizeof(unsigned char), 127, outfile);
					i = 0;
				}
				p1 = 8;
				cache[i] = 0;
				if (k > 0)
				{
					++p;
					p2 = (k >= 8) ? 8 : k;
					symbol = map[source].symbol[p];
				}
			}
		}
	}
	if (ferror(infile))
	{
		printf("fail to read file!\n");
		fclose(outfile);
		release_map(map);
		return -1;
	}
	if(p1 > 0) ++i;
	fwrite(cache, sizeof(unsigned char), i, outfile);
	fclose(outfile);
	fclose(infile);
	/*释放映射内存*/
	release_map(map);
	return 0;
}

int encode(infilename, outfilename)
unsigned char *infilename, *outfilename;
{
	unsigned int i, total, length;
	int res;
	HUFFMAN_TREE_NODE tree[256];

	/*生成频次表*/
  for (i = 0; i < 256; ++i)
  {
  	tree[i].is_leaf = 1;
  	tree[i].freq = 0;
  	tree[i].parent = NULL;
  	tree[i].is_right = 0;
  }
	if( (total = read_for_encode(infilename, tree)) == 0 ){
		return -1;
	}
	length = 0;
	for (i = 0; i < 256; ++i)
	{
		if(tree[i].freq > 0)
			++length;
	}
	/*Huffman索引*/
	if(encode_table(tree) != 0)
	{
		printf("generate huffman tree fail!\n");
		release_encode_tree(tree);
		return -2;
	}
	/*生成Huffman编码后的文件*/
	if( (res = encode_file(infilename, total, length, tree, outfilename)) != 0){
		printf("encode file fail!\n");
	}
	/*释放动态内存*/
	release_encode_tree(tree);
	return res;
}

HUFFMAN_TREE_NODE *new_decode_node(parent)
HUFFMAN_TREE_NODE *parent;
{
	HUFFMAN_TREE_NODE *node;

	node = malloc(sizeof(HUFFMAN_TREE_NODE));
	/*初始化*/
	node->symbol = 0;
	node->is_leaf = 0;
	node->parent = parent;
	node->right_point = NULL;
	node->left_point = NULL;
	return node;
}

unsigned char generate_huffman_tree(tree, source, res, p)
HUFFMAN_TREE_NODE **tree;
unsigned char source, res, p;
{
	unsigned char i, is_right;
	HUFFMAN_TREE_NODE *next;

	next = *tree;
	for (i = 0; i < 8; ++i)
	{
		is_right = ((128 & res) == 128 ) ? 1 : 0;
		if(is_right == 1)
		{
			if(next->right_point == NULL)
				next->right_point = new_decode_node(next);
			next = next->right_point;
		}
		else
		{
			if(next->left_point == NULL)
				next->left_point = new_decode_node(next);
			next = next->left_point;
		}
		--p;
		if(p == 0)
		{
			next->symbol = source;
			next->is_leaf = 1;
			break;
		}
		res <<= 1;
	}
	*tree = next;
	return p;
}

int decode(infilename, outfilename)
unsigned char *infilename, *outfilename;
{
	FILE *infile, *outfile;
	unsigned int i, j, k, total, length, is_right;
	unsigned char line[128], byte_cache, p;
	int res;
	HUFFMAN_TREE_NODE *tree, **root, *next, cache;

	if( (infile = fopen(infilename, "rb")) == NULL ){
		printf("fail to open file:%s!\n", infilename);
		return -1;
	}
	/*初始化表长度和信息长度*/
	length = 0;
	total = 0;
	p = 2 * sizeof(unsigned int);
	i = 0;
	j = sizeof(unsigned int);
	while( i < p && ((res = fgetc(infile)) != EOF) )
	{
		if ( i < j ) 
			length += res << (8 * (j - i - 1));
		else
			total += res << (8 * (p - i - 1));
		++i;
	}
	if (ferror(infile))
	{
		printf("read file fail:%d\n", res);
		return -1;
	}
	else if (feof(infile))
	{
		return 0;
	}
	if (total == 0 || length == 0){
		printf("error file format!\n");
		fclose(infile);
		return -2;
	}
	/*初始化huffman编码表*/
	tree = new_decode_node(NULL);
	root = &tree;
	next = *root;
	for (i = 0; i < length; ++i)
	{
		/*读取一个映射*/
		j = 0;
		p = 1;
		while( (res = fgetc(infile)) != EOF )
		{
			if(j == 0)
			{
				byte_cache = res;
			}
			else if(j == 1)
			{
				p = res;
			}
			else
			{
				p = generate_huffman_tree(&next, byte_cache, res, p);
			}
			if(p <= 0)
			{
				next = *root;
				break;
			}
			++j;
		}
		if (ferror(infile))
		{
			printf("read file fail:%d\n", res);
			/*释放内存*/
			release_decode_tree(tree);
			return -1;
		}
	}
	/*解码*/
	if( (outfile = fopen(outfilename, "wb")) == NULL ){
		printf("fail to open file:%s!\n", outfilename);
		/*释放内存*/
		release_decode_tree(tree);
		fclose(infile);
		return -1;
	}
	k = 0;
	next = *root; 
	while((res = fgetc(infile)) != EOF)
	{
		if (total == 0)break;
		p = 8;
		byte_cache = res;
		while(p > 0)
		{
			is_right = ((128 & byte_cache) == 128 ) ? 1 : 0;
			next = (is_right == 1) ? next->right_point : next->left_point;
			if(next->is_leaf == 1)
			{
				line[k] = next->symbol;
				++k;
				if (k == 127)
				{
					fwrite(line, sizeof(unsigned char), 127, outfile);
					k = 0;
				}
				--total;
				if(total == 0)break;
				next = *root;
			}
			--p;
			byte_cache <<= 1;
		}	
	}
	if (ferror(infile))
	{
		printf("fail to read file!\n");
		fclose(outfile);
		return -1;
	}
	if(k > 0)fwrite(line, sizeof(unsigned char), k, outfile);
	/*释放内存*/
	release_decode_tree(tree);
	fclose(outfile);
	fclose(infile);
	return 0;
}
