#!/bin/bash

echo "Compiling...\n"
gcc -c main.c -o main.o
gcc -c pager.c -o pager.o
gcc -c b_tree.c -o b_tree.o
gcc main.o pager.o b_tree.o -o main
echo "Done!"
