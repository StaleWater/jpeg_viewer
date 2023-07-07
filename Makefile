c:
	g++ jpeg_loader.cpp -o jpeg.exe

r: c
	./jpeg.exe hinanotwo.jpeg > debugdata.txt
	nvim debugdata.txt
