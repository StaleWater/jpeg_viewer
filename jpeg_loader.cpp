#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

using std::vector;
using std::unordered_map;
using byte = unsigned char;

struct HuffmanTableData {
    bool dcTable;
    int tableIndex;
    int bits[16];
    vector<byte> vals; 
};

class HuffmanDecoder {
    
    unordered_map<int, byte> tablesDC[4];
    unordered_map<int, byte> tablesAC[4];

    public: 
    void buildTable(HuffmanTableData data) {

        unordered_map<int, byte> table = data.dcTable ? tablesDC[data.tableIndex] : tablesAC[data.tableIndex];
        table.clear();
        
        int valIndex = 0;
        int len = 1;
        int code = 0;

        while(len <= 16) {
            int numCodes = 1;

            while(numCodes <= data.bits[len-1]) {
                table[code] = data.vals[valIndex];

                valIndex++;
                numCodes++;
                code++;
            }

            code = code << 1;
            len++;
        }

        std::cout << "built table " << data.tableIndex << std::endl;
    }

    bool lookup(byte tableIndex, bool dc, int code, byte& out) {
        auto table = dc ? tablesDC[tableIndex] : tablesAC[tableIndex];
        auto x = table.find(code);
        if(x == table.end()) return false;
        out = x->second;
        return true;
    }

    vector<byte> decode(byte tableIndex, bool dc, vector<byte> data) {
        vector<byte> output = vector<byte>();

        byte databuf = 0;
        int bitbuf = 0;
        int i = 0;
        while(i++ < data.size()) {
            int bit = 0;
            byte d = data[i];

            while(bit++ < 8) {
                bitbuf |= (d & 1) << bit;
                if(lookup(tableIndex, dc, bitbuf, databuf)) {
                    output.push_back(bitbuf);
                    bitbuf = 0;
                }
            }
        }

        return output;
    }

};


enum marker {
    SOF0,
    SOF1,
    SOF2,
    SOF3,
    SOF5,
    SOF6,
    SOF7,
    JPG,
    SOF9,
    SOF10,
    SOF11,
    SOF13,
    SOF14,
    SOF15,
    DHT,
    DAC,
    RST,
    SOI,
    EOI,
    SOS,
    DQT,
    DNL,
    DRI,
    DHP,
    EXP,
    APP,
    COM,
    TEM,
    RES,
    MARKER_ERROR
}; 

enum endian {
    EN_BIG,
    EN_LITTLE
};


class ByteStream {

    byte prefix;
    byte lastRead;
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
        fileEndian = bigEndian ? EN_BIG : EN_LITTLE;
    }

    void setPrefix(byte pre) {
        prefix = pre;
    }

    byte lastReadByte() {
        return lastRead;
    }

    marker nextMarker() {
        byte buffer = 0x00;
        while (!file.eof()) {
            while (buffer != 0xFF) file.read((char*)&buffer, 1);

            file.read((char*)&buffer, 1);
            lastRead = buffer;
            switch(buffer) {
                case 0x00:
                    // just padding
                    break;
                case 0xC0: return SOF0;
                case 0xC1: return SOF1;
                case 0xC2: return SOF2;
                case 0xC3: return SOF3;
                case 0xC4: return DHT;
                case 0xC5: return SOF5;
                case 0xC6: return SOF6;
                case 0xC7: return SOF7;
                case 0xC8: return JPG;
                case 0xC9: return SOF9;
                case 0xCA: return SOF10;
                case 0xCB: return SOF11;
                case 0xCC: return DAC;
                case 0xCD: return SOF13;
                case 0xCE: return SOF14;
                case 0xCF: return SOF15;
                case 0xD0: return RST;
                case 0xD2: return RST;
                case 0xD3: return RST;
                case 0xD4: return RST;
                case 0xD5: return RST;
                case 0xD6: return RST;
                case 0xD7: return RST;
                case 0xD8: return SOI;
                case 0xD9: return EOI;
                case 0xDA: return SOS;
                case 0xDB: return DQT;
                case 0xDC: return DNL;
                case 0xDD: return DRI;
                case 0xDE: return DHP;
                case 0xDF: return EXP;
                case 0xE0: return APP;
                case 0xE1: return APP;
                case 0xE2: return APP;
                case 0xE3: return APP;
                case 0xE4: return APP;
                case 0xE5: return APP;
                case 0xE6: return APP;
                case 0xE7: return APP;
                case 0xE8: return APP;
                case 0xE9: return APP;
                case 0xEA: return APP;
                case 0xEB: return APP;
                case 0xEC: return APP;
                case 0xED: return APP;
                case 0xEE: return APP;
                case 0xEF: return APP;
                case 0xF0: return JPG;
                case 0xF1: return JPG;
                case 0xF2: return JPG;
                case 0xF3: return JPG;
                case 0xF4: return JPG;
                case 0xF5: return JPG;
                case 0xF6: return JPG;
                case 0xF7: return JPG;
                case 0xF8: return JPG;
                case 0xF9: return JPG;
                case 0xFA: return JPG;
                case 0xFB: return JPG;
                case 0xFC: return JPG;
                case 0xFD: return JPG;
                case 0xFE: return COM;

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
        return bytes[0] ? EN_LITTLE : EN_BIG;
    }

    void matchEndian(byte* data, int len) {
        if (hostEndian == fileEndian) return;

        byte temp;
        for(int i=0; i < len/2; i++) {
            temp = data[i];
            data[i] = data[len-i-1];
            data[len-i-1] = temp; 
        }
    }

    void getBytes(byte* buffer, int numBytes) {
        file.read((char*)buffer, numBytes);
    }

    void getBytesMatchEndian(byte* buffer, int numBytes) {
        file.read((char*)buffer, numBytes); 
        matchEndian(buffer, numBytes);
    }

    byte getByte() {
        byte buf;
        file.read((char*)(&buf), 1);
        return buf;
    }

    short getUShort() {
        byte* buf[2];
        getBytesMatchEndian((byte*) buf, 2);
        return *((unsigned short*)buf);
    }

    void skipBytes(int numBytes) {
        file.seekg(numBytes, std::ios::cur);
    }

};

class JPEGDecoder {


    void mkrAPP0(ByteStream& bs) {

        unsigned short len = bs.getUShort();
        len -= 2; //exclude the length itself
        std::cout << "APP0 length: " << len << std::endl; 

        // if we need APP0 stuff, I'll get it here. for now, skip
        bs.skipBytes(len);
    }

    void buildQuantTables() {
        std::cout << "quant tables time\n";
        return;
    }


    void buildHuffmanTables(ByteStream& bs, HuffmanDecoder& huff) {
        std::cout << "huff tables time\n";

        HuffmanTableData header;
        unsigned short len = bs.getUShort();

        byte b = bs.getByte();
        header.dcTable = (b >> 4) == 0;
        header.tableIndex = (int)(b & 0x0F);
        
        int totalCodes = 0;
        for(int i=0; i < 16; i++) {
            int numCodes = (int)bs.getByte();
            header.bits[i] = numCodes;
            totalCodes += numCodes;
        }

        while(totalCodes-- > 0) {
            header.vals.push_back(bs.getByte());
        }
        

        std::cout << "huffman table length: " << std::dec << len << std::endl;
        std::cout << "hft table " << header.tableIndex << std::endl;
        std::cout << "hft is DC Table: " << header.dcTable << std::endl;
        std::cout << "hft has " << header.vals.size() << " codes" << std::endl;

        huff.buildTable(header);

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

    public:
    void processFile(const char* filename) {

        ByteStream bs = ByteStream(filename, 0xFF, true);
        HuffmanDecoder huff = HuffmanDecoder();

        marker markerBuf;
        markerBuf = bs.nextMarker();

        if(markerBuf != SOI) return;
        std::cout << "SOI found!\n";


        while((markerBuf = bs.nextMarker()) != EOI) {
            std::cout << std::hex << (int)(bs.lastReadByte()) << std::endl;
            switch(markerBuf) {
                case APP:
                    std::cout << "APP found\n";
                    break;
                case DQT:
                    buildQuantTables();
                    break;

                case DHT:
                    buildHuffmanTables(bs, huff);
                    break;

                case DRI:
                    setRestartInterval();
                    break; 

                case SOF0:
                    startOfFrame();
                    break;
                
                case SOS:
                   startScan();
                   break;

                case MARKER_ERROR:
                    std::cout << "Something went very bad. Oh god.\n";
                    return;

                default:
                    std::cout << "Some other random marker?\n";
                    break;

            }
        }
        std::cout << "reached end of image!\n";
        
    }

};



int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename\n";
        return 0;
    }
    auto decoder = JPEGDecoder();
    decoder.processFile(argv[1]);
    return 0;
}
