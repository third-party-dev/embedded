# sudo apt-get install libncurses-dev

all:
	gcc -static -o app app.c -lncurses -ltinfo
	aarch64-linux-gnu-gcc -static -o app-aarch64 app.c -lncurses -ltinfo
	aarch64-linux-gnu-gcc -static -g -o app-aarch64-debug app.c -lncurses -ltinfo

# gcc -static -o app app.c -lncurses -ltinfo && ./app

# sudo dpkg --add-architecture arm64 ; sudo apt-get update
# sudo apt-get install ncurses-dev:arm64
