main:
	mkdir -p build
	cc main.c -o build/main -O3

clean:
	rm -r build
