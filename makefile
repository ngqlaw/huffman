huffman : huffman.o
	cc -o huffman huffman.o
huffman.o : huffman.c huffman.h
	cc -g -c huffman.c
clean : 
	rm huffman huffman.o
