#include "huffman.h"

int main(int argc, char *argv[])
{
	/*huffman编码*/
	if(encode(argv[1], argv[2]) == 0)
	{
		decode(argv[2], argv[3]);
	}
	return 0;
}
