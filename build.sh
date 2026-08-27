#!/bin/bash

echo "Compiling...\n"
gcc -c main.c -o main.o
gcc -c pager.c -o pager.o
gcc main.o pager.o -o main
echo "Done!"
