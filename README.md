# jpeg_viewer
This project will consist of a JPEG JFIF standard image compression decoder written from scratch,
along with an OpenGL UV mapping of the decoded bitmap into a 3D space where it can be interacted with
almost like a piece of paper.

# Recent
The baseline JPEG decoder is finished!
Some notes about it:
- Current baseline decoder assumes components are compressed in YCbCr format and interleaved. 
- ONLY the baseline version is supported. Progressive, lossless, and hierarchical modes will be supported in future updates.
- The algorithm is currently single-core, single-threaded. It was designed with multi-core processing in mind, so this will change soon.
- Only huffman entropy encoding is supported, arithmetic encoding is NOT yet supported.
- APP markers are currently not being processed, as they are not necessary for the baseline functionality.

To test its functionality, this JPEG decoder can be exported into a Qt project and used to convert a JPEG file into a multi-component bitmap.
This bitmap can then loaded into a QImage and displayed in a window.
