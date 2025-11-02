#!/bin/zsh 

echo "Building editor..."
# gcc -Wall -Wextra -Wpedantic -g -Iinclude src/*.c -o build/editor
gcc -Iinclude src/*.c -o build/editor
echo "Build Complete"
./build/editor ./test/test.txt
