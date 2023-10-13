<p align="center"><h2>Hide Secret text in PNG Images.</h2></p>


<p>
Hide text in an ordinary looking png image, written in pure c without any external libraries. Supports PNG format images.
</p>


<h3>Usage</h2>

1. Encoding text in a png image.
```sh
main -f demo.png -e -t 'this text will get encoded'
>> FILENAME : demo.png
>> ENCODING MODE.
>> DATA TO BE ENCODED IN THE IMAGE : this text will get encoded (26 bytes).
>> ENCODING SECRET CHUNK.
>> ENCODING COMPLETE.
>> Total duration it took: 0.001000ms
```

This will result a image named `output.png` with the text you provided encoded in it.


2. Decoding text from an image.
```sh
main -f output.png -d
>> FILENAME : output.png
>> DECODING MODE.
>> DECODED TEXT : this text will get encoded
```
This will decode the text from the given image.



<h3>Building from source</h3>

```sh
gcc main.c -o main -O3
```
You can use any compiler of your choice.
also you can `#define DEBUG` to enable debug statements.