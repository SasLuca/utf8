#!/bin/sh

if [ "$1" = "release" ]
then
  params=-Ofast
elif [ "$1" = "debug" ]
then
  params=-g
else
  params=0
fi

if [ $params = 0 ]
then
  echo Please provide release or debug as command line argument 1
else
  clang $params -c extern/utf8proc/utf8proc.c -obuild/utf8proc_$1.o -DUTF8PROC_STATIC
  clang++ $params -c -std=c++17 utf8.cc -obuild/utf8_$1.o
  ar rvs build/utf8_$1.a build/utf8proc_$1.o build/utf8_$1.o
fi