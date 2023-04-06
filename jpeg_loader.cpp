#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using std::vector;
using byte = unsigned char;

enum marker {
    SOI,
    EOI,
    APP0,
    RST,
    DRI,
    DHT,
    DQT,
    SOF,
    SOS,
    CMT,
    MARKER_ERROR
}; 

enum endian {
    BIG_ENDIAN,
    LITTLE_ENDIAN 
};


class ByteStream {

    byte prefix;
    std::ifstream file;
    endian hostEndian, fileEndian;

public:
    ByteStream(const char* filename) {
        ByteStream(filename, 0xFF, false);
    }

    ByteStream(const char* filename, byte markerPrefix) {
        ByteStream(filename, markerPrefix, false);
    } 

    ByteStream(const char* filename, byte markerPrefix, bool bigEndian) {
        file.open(filename, std::ios::binary | std::ios::in);
        prefix = markerPrefix;
        hostEndian = endianCheck();
        fileEndian = bigEndian ? BIG_ENDIAN : LITTLE_ENDIAN;
    }

    void setPrefix(byte pre) {
        prefix = pre;
    }

    marker nextMarker() {
        byte buffer = 0x00;
        while (!file.eof()) {
            while (buffer != 0xFF) file.read((char*)&buffer, 1);

            file.read((char*)&buffer, 1);
            switch(buffer) {
                case 0xD8:
                    return SOI;
                case 0xE0:
                    return APP0;
                case 0xC0:
                    return SOF;
                case 0xC4:
                    return DHT;
                case 0xDB:
                    return DQT;
                case 0xDD:
                    return DRI;
                case 0xDA:
                    return SOS;
                case 0xD0:
                    return RST;
                case 0xFE:
                    return CMT;
                case 0xD9:
                    return EOI;
                case 0x00:
                    // just padding
                    break;
                case 0xFF:
                    std::cout << "double FF padding\n";
                    buffer = 0x00;
                    break;
                default:
                    std::cout << "some random FF?\n";
                
            }
            // no valid marker found, keep going
        }
        // end of all loops
        return MARKER_ERROR;
    }

    endian endianCheck() {
        short num = 0x0001;
        char* bytes = (char*)&num;
        return bytes[0] ? LITTLE_ENDIAN : BIG_ENDIAN;
    }

    void matchEndian(byte* data, int len) {
        if (hostEndian == fileEndian) return;

        byte temp;
        for(int i=0; i < len/2; i++) {
            temp = data[i];
            data[i] = data[len-i-1];
            data[len-i-1] = temp; 
        }
        std::cout << "flipped to match " << fileEndian << '\n';
    }

    void getBytes(byte* buffer, int numBytes) {
        file.read((char*)buffer, numBytes);
    }

    void getBytesMatchEndian(byte* buffer, int numBytes) {
        file.read((char*)buffer, numBytes); 
        matchEndian(buffer, numBytes);
    }

    void skipBytes(int numBytes) {
        file.seekg(numBytes, std::ios::cur);
    }

};

struct JPEGStandard {
    vector<vector<byte>> huffmanTables;
    vector<vector<byte>> quantizationTables;
    bool isProgressive;
   
};

vector<byte> jpegToBitmap(std::string filename) {
    // run length decoding
    // huffman decoding
    // zigzag scan each block
    // dequantization
    // inverse DCT
    // YCrCb to sRGB
    // return as byte vector
    return vector<byte>();
}

void mkrAPP0(ByteStream& bsfile) {

    byte lenBuffer[2];
    bsfile.getBytesMatchEndian(lenBuffer, 2);

    unsigned short len = *((unsigned short*)&lenBuffer);
    len -= 2; //exclude the marker
    std::cout << "APP0 length: " << len << std::endl; 

    // if we need APP0 stuff, I'll get it here. for now, skip
    bsfile.skipBytes(len);
}

void buildQuantTables() {
    std::cout << "quant tables time\n";
    return;
}

void buildHuffmanTables() {
    std::cout << "huff tables time\n";
}

void setRestartInterval() {
    std::cout << "setting restart interval\n";

}

void startOfFrame() {
    std::cout << "start of frame\n";
}

void startScan() {
    std::cout << "start of scan\n";
}

void processFile(const char* filename) {

    ByteStream bsfile = ByteStream(filename, 0xFF, true);

    marker markerBuf;
    markerBuf = bsfile.nextMarker();

    if(markerBuf != SOI) return;
    std::cout << "SOI found!\n";

    markerBuf = bsfile.nextMarker();
    if(markerBuf != APP0) return;
    std::cout << "APP0 found!\n";
    mkrAPP0(bsfile);

    while((markerBuf = bsfile.nextMarker()) != EOI) {
        switch(markerBuf) {
            case DQT:
                buildQuantTables();
                break;

            case DHT:
                buildHuffmanTables();
                break;

            case DRI:
                setRestartInterval();
                break; 

            case SOF:
                startOfFrame();
                break;
            
            case SOS:
               startScan();
               break;

            case MARKER_ERROR:
                std::cout << "Something went very bad. Oh god.\n";
                return;

        }
    }
    std::cout << "reached end of image!\n";

}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename\n";
        return 0;
    }
    processFile(argv[1]);
    return 0;
}