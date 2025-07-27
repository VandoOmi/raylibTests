#!/bin/bash
LIBRARY_PATH=/usr/lib
cc -Wall main.c -o raylib-test -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

