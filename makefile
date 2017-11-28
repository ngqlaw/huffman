huffman : main.o huffman.o
	cc -o huffman main.o huffman.o
main.o : main.c huffman.h
	cc -g -c main.c
huffman.o : huffman.c huffman.h
	cc -g -c huffman.c
clean : 
	rm huffman main.o huffman.o
