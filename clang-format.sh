#!/bin/bash

for folder in . plugin serialize tmux

do
for filename in $folder/*.h $folder/*.cpp 
do
clang-format $filename > tmp
mv tmp $filename
done
done

