main:
	mkdir -p build
	cc main.c -o build/main -O3

debug:
	mkdir -p build
	cc main.c -o build/main -s -g


clean:
	rm -r build
