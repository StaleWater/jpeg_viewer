#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

using std::vector;
using std::unordered_map;
using byte = unsigned char;
using word = unsigned short;

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
    SOF,
    JPG,
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
                case 0xC0: return SOF;
                case 0xC1: return SOF;
                case 0xC2: return SOF;
                case 0xC3: return SOF;
                case 0xC4: return DHT;
                case 0xC5: return SOF;
                case 0xC6: return SOF;
                case 0xC7: return SOF;
                case 0xC8: return JPG;
                case 0xC9: return SOF;
                case 0xCA: return SOF;
                case 0xCB: return SOF;
                case 0xCC: return DAC;
                case 0xCD: return SOF;
                case 0xCE: return SOF;
                case 0xCF: return SOF;
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

    word getWord() {
        byte* buf[2];
        getBytesMatchEndian((byte*) buf, 2);
        return *((word*)buf);
    }

    void skipBytes(int numBytes) {
        file.seekg(numBytes, std::ios::cur);
    }

};

class JPEGDecoder {

    int quantTables[4][8][8];

    struct FrameComponentData {
        byte Ci;
        byte H;
        byte V;
        byte Tq;
    };

    struct FrameHeader {
        byte P;
        word Y;
        word X;
        byte Nf; 
        FrameComponentData components[4];
    };

    struct ScanComponentData {
        byte Cs;
        byte Td;
        byte Ta;
    };

    struct ScanHeader {
        byte Ns;
        ScanComponentData components[4];
        byte Ss;
        byte Se;
        byte Ah;
        byte Al;
    };


    FrameHeader baselineFrame(ByteStream& bs) {
        std::cout << "baseline Frame\n";
        word len = bs.getWord();
        
        FrameHeader header;
        header.P = bs.getByte();
        header.Y = bs.getWord();
        header.X = bs.getWord();
        header.Nf = bs.getByte();

        printf("P: %d Y: %d X: %d Nf: %d\n", header.P, header.Y, header.X, header.Nf);

        for(int i=0; i < header.Nf; i++) {
            auto com = &(header.components[i]);
            com->Ci = bs.getByte();
            
            byte sampling = bs.getByte();
            com->H = sampling >> 4; 
            com->V = sampling & 0x0F;

            com->Tq = bs.getByte();
            
            printf("component %d H: %d V: %d Tq: %d\n", com->Ci, com->H, com->V, com->Tq);
        }

        return header;
    }

    void buildQuantTables(ByteStream& bs) {
        word len = bs.getWord();

        byte b = bs.getByte();
        byte Pq = b >> 4;
        byte Tq = b & 0x0F;

        if(Pq != 0) {
            printf("Precision of 16 bits is not supported! QT\n");
            return;
        }

        printf("buliding quant table %d\n", Tq);

        int (*table)[8] = quantTables[Tq];

        int i = 0;
        int j = 0;
        bool secondHalf = false;
        bool rowPos = false;

        while(i < 8 && j < 8) {
            if(!secondHalf && (i == 7 || j == 7)) secondHalf = true;

            while(i >= 0 && i < 8 && j >= 0 && j < 8) {
                table[i][j] = (int) bs.getByte(); 
                
                if(rowPos) {
                    i++; j--;
                }
                else {
                    i--; j++;
                }
            }

            if(rowPos && secondHalf) {
                i--; j += 2;
            }
            else if(rowPos) {
                j++;
            }
            else if(secondHalf) {
                i += 2; j--;
            }
            else {
                i++;
            }

            rowPos = !rowPos;
        }

    }

    void defineRestartInterval(ByteStream& bs) {
        printf("restart interval setting!!\n");

        word len = bs.getWord();
        word restartInterval = bs.getWord();
        printf("interval is %d", restartInterval);
    }


    void buildHuffmanTables(ByteStream& bs, HuffmanDecoder& huff) {

        HuffmanTableData header;
        word len = bs.getWord();

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
        
        printf("hft table %d ", header.tableIndex);
        if(header.dcTable) printf("DC\n");
        else printf("AC\n");

        huff.buildTable(header);

    }


    void startScan(ByteStream& bs) {
        printf("start of scan\n");
        word len = bs.getWord();

        ScanHeader header; 
        header.Ns = bs.getByte();
        printf("scan components: %d\n", header.Ns);

        for(int i=0; i < header.Ns; i++) {
            auto com = &(header.components[i]);
            com->Cs = bs.getByte();
            byte b = bs.getByte();
            com->Td = b >> 4;
            com->Ta = b & 0x0F;

            printf("component %d TDC %d TAC %d\n", com->Cs, com->Td, com->Ta);
        }

        header.Ss = bs.getByte();
        header.Se = bs.getByte();
        byte b = bs.getByte();
        header.Ah = b >> 4;
        header.Al = b & 0x0F;
    }

    public:
    void processFile(const char* filename) {

        ByteStream bs = ByteStream(filename, 0xFF, true);
        HuffmanDecoder huff = HuffmanDecoder();

        marker markerBuf;
        markerBuf = bs.nextMarker();

        if(markerBuf != SOI) {
            printf("SOI not found\n");
        }

        // go until start of frame is found
        markerBuf = bs.nextMarker();
        while(markerBuf != SOF && markerBuf != EOI) {

            // need to handle optional xFF fill bytes

            switch(markerBuf) {
                case APP:
                    printf("APP found!\n");
                    break;

                case COM:
                    printf("comment found, not supported yet\n");
                    break;

                case DRI:
                    defineRestartInterval(bs);
                    break;

                case DQT:
                    buildQuantTables(bs);
                    break;

                case DHT:
                    buildHuffmanTables(bs, huff);
                    break;

                case MARKER_ERROR:
                    printf("Something went very bad. Oh god.\n");
                    return;

                default:
                    printf("unsupported marker ");
                    std::cout << std::hex << (int)(bs.lastReadByte()) << std::dec << std::endl;
                    break;

            }

            markerBuf = bs.nextMarker();
        }

        byte sofType = bs.lastReadByte() - 0xC0;
        if(sofType != 0) {
            printf("SOF%d is not supported yet!\n", sofType);
            return;
        }

        FrameHeader fh = baselineFrame(bs);

        // inside frame
        markerBuf = bs.nextMarker();
        while(markerBuf != EOI) {
            switch(markerBuf) {
                case DQT:
                    buildQuantTables(bs);
                    break;

                case DHT:
                    buildHuffmanTables(bs, huff);
                    break;

                case SOS:
                    startScan(bs);
                    break;

                default:
                    printf("unsupported marker in frame\n");
            }

            markerBuf = bs.nextMarker();
        }

        printf("reached end of image!\n");
        
    }

};



int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 0;
    }
    auto decoder = JPEGDecoder();
    decoder.processFile(argv[1]);
    return 0;
}
