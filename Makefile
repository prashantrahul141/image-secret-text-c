main:
	mkdir -p build
	cc main.c -o build/main -O3 -Wall

test: debug
	./build/main -f ./images/demo.png -e -t "hello"

debug:
	mkdir -p build
	cc main.c -o build/main -s -g -Wall


clean:
	rm -r build
