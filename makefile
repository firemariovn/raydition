all: build run
build:
	gcc -ggdb -o game main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
run:
	./game
