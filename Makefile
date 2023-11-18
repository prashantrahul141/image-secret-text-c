main:
	mkdir -p build
	cc main.c -o build/main -O3

test: debug
	./build/main -f ./images/demo.png -e -t "hello"

debug:
	mkdir -p build
	cc main.c -o build/main -s -g


clean:
	rm -r build
