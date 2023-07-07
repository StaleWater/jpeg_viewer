#include <stdio.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

using std::vector;
using std::unordered_map;
using byte = unsigned char;
using word = unsigned short;

class Image {
    int*** img;
    int rows, cols;

    public:

    Image(int*** im, int r, int c) {
        img = NULL;
        copyFrom(im, r, c);
    }

    ~Image() {
        deleteImg();
    }

    int getRows() {
        return rows;
    }

    int getCols() {
        return cols;
    }

    int sample(int c, int y, int x) {
        return img[c][y][x];
    }

    void copyFrom(int*** im, int r, int c) {
        rows = r; cols = c;
        deleteImg();
        allocImg();

        for(int i=0; i < 3; i++) {
            for(int j=0; j < r; j++) {
                for(int k=0; k < c; k++) {
                    img[i][j][k] = im[i][j][k];
                }
            } 
        }
    }

    void print() {
        for(int i=0; i < rows; i++) {
            for(int j=0; j < cols; j++) {
                printf("(%d, %d, %d) ", img[0][i][j], img[1][i][j], img[2][i][j]);
            }
            printf("\n");
        }
    }

    private:

    void allocImg() {
        img = new int**[3];
        for(int i=0; i < 3; i++) {
            img[i] = new int*[rows];
            for(int j=0; j < rows; j++) {
                img[i][j] = new int[cols];
            }
        }
        
    }

    void deleteImg() {
        if(img == NULL) return;
        for(int i=0; i < 3; i++) {
            for(int j=0; j < rows; j++) {
                delete[] img[i][j];
            }
            delete[] img[i];
        }
        delete[] img;
    }

};

struct HuffmanTableData {
    bool dcTable;
    int tableIndex;
    int bits[16];
    vector<byte> vals; 
};

struct HuffmanTable {
    unordered_map<int, byte> table;
    vector<int> codes;
    int lenMinIndex[16];
    int numCodes[16];
};

class HuffmanDecoder {
    
    HuffmanTable* tablesDC[4];
    HuffmanTable* tablesAC[4];
    

    public: 
    HuffmanDecoder() {
        for(int i=0; i < 4; i++) {
            tablesDC[i] = new HuffmanTable();
            tablesAC[i] = new HuffmanTable();
        }
    }

    ~HuffmanDecoder() {
        for(int i=0; i < 4; i++) {
            delete tablesDC[i];
            delete tablesAC[i];
        }
    }

    void buildTable(HuffmanTableData data) {
        printf("HUFFTABLE BUILD START\n");

        HuffmanTable* ht = data.dcTable ? tablesDC[data.tableIndex] : tablesAC[data.tableIndex];
        
        ht->table.clear();

        for(int i=0; i < 16; i++) ht->numCodes[i] = data.bits[i];
        
        int valIndex = 0;
        int len = 1;
        int code = 0;

        while(len <= 16) {
            int numCodes = 0;
            ht->lenMinIndex[len-1] = ht->codes.size();
            while(numCodes < data.bits[len-1]) {
                ht->table[code] = data.vals[valIndex];
                ht->codes.push_back(code); 


                valIndex++;
                numCodes++;
                code++;
            }

            code = code << 1;
            len++;
        }


    }

    bool lookup(byte tableIndex, bool dc, int code, int len, byte* out) {
        HuffmanTable* ht = dc ? tablesDC[tableIndex] : tablesAC[tableIndex];
        int n = ht->numCodes[len-1]; 
        if(n < 1) {
            return false;
        }
        
        int start = ht->lenMinIndex[len-1];
        int end = start + n;
        for(int i=start; i < end; i++) {
            if(ht->codes[i] == code) {
                *out = ht->table[code];
                //printf("code %x of len %d gives val %d\n", code, len, *out);
                return true;
            }
        }

        return false;
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
    byte bitsLeft;
    byte bitBuffer;

    std::ifstream file;
    endian hostEndian, fileEndian;

public:
    ByteStream(std::string filename) {
        ByteStream(filename, 0xFF, false);
    }

    ByteStream(std::string filename, byte markerPrefix) {
        ByteStream(filename, markerPrefix, false);
    } 

    ByteStream(std::string filename, byte markerPrefix, bool bigEndian) {
        file.open(filename, std::ios::binary | std::ios::in);
        prefix = markerPrefix;
        hostEndian = endianCheck();
        fileEndian = bigEndian ? EN_BIG : EN_LITTLE;
        bitsLeft = 0;
        bitBuffer = 0;
    }

    int getBits(int n) {
        if(n < 1) return 0;

        int b = 0;;
        for(int i=0; i < n; i++) {
            if(bitsLeft == 0) {
                if(file.eof()) {
                    printf("getBits hit end of file\n");
                    return -1;
                }

                if(lastRead == 0xFF) {
                    file.read((char*) &bitBuffer, 1);
                    //printf("{%x}", (int)bitBuffer);

                    if(bitBuffer == 0) {
                        //printf("found stuffing\n");
                    }
                    else {
                        printf("found marker in getBits()\n");
                        if(resolveHex(bitBuffer) == EOI) {
                            printf("IT WAS EOI!!!\n");
                        }
                        exit(0);
                    }
                }

                file.read((char*) &bitBuffer, 1);
                lastRead = bitBuffer;
                bitsLeft = 8;
                //printf("(%x)", (int)bitBuffer);
            }
             
            b = b << 1;
            b |= (bitBuffer & (1 << (bitsLeft - 1))) != 0; 
            bitsLeft--;
        }
        //printf("[%x]", (int)b);
        
        return b;
    }

    void clearBitBuffer() {
        bitBuffer = 0;
        bitsLeft = 0;
    }

    void setPrefix(byte pre) {
        prefix = pre;
    }

    byte lastReadByte() {
        return lastRead;
    }

    marker resolveHex(byte hex) {
        switch(hex) {
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
            default:
                std::cout << "some random FF?\n";
        }
        return MARKER_ERROR;
    }

    marker nextMarker() {
        byte buffer = 0x00;
        while (!file.eof()) {
            if(bitsLeft == 8) {
                bitsLeft = 0;
                buffer = bitBuffer;
            }
            else {
                while (buffer != 0xFF) {
                    file.read((char*)&buffer, 1);
                    printf("{%x}", buffer);
                }

                file.read((char*)&buffer, 1);
                lastRead = buffer;
            }

            switch(buffer) {
                case 0x00:
                    printf("this padding should be in the encoding\n");
                    // just padding
                    break;
                case 0xFF:
                    std::cout << "double FF padding\n";
                    buffer = 0x00;
                    break;
                default:
                    return resolveHex(buffer);
                
            }
            // no valid marker found, keep going
        }

        printf("end of file without finding EOI\n");

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

    void getBytesMatchEndian(byte* buffer, int numBytes) {
        if(file.eof()) {
            printf("getBytesME hit end of file\n");
            return;
        }
        file.read((char*)buffer, numBytes); 
        matchEndian(buffer, numBytes);
    }

    byte getByte() {
        if(file.eof()) {
            printf("getByte hit end of file\n");
            return 0;
        }
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
    int dcPred[4];
    int maxH, maxV;

    struct FrameComponentData {
        byte Ci;
        byte H;
        byte V;
        byte Tq;
    };

    struct FrameHeader {
        byte P;
        word Y, X;
        word oldY, oldX;
        byte Nf; 
        FrameComponentData components[4];
        int mcuX;
        int mcuY;
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

        printf("original Y: %d X: %d\n", header.Y, header.X);
        header.oldX = header.X;
        header.oldY = header.Y;

        if(header.Y % 8 != 0) header.Y += 8 - (header.Y % 8);
        if(header.X % 8 != 0) header.X += 8 - (header.X % 8);


        printf("P: %d Y: %d X: %d Nf: %d\n", header.P, header.Y, header.X, header.Nf);

        for(int i=0; i < header.Nf; i++) {
            int ci = bs.getByte() - 1;
            auto com = &(header.components[ci]);
            com->Ci = ci;
            
            byte sampling = bs.getByte();
            byte h = sampling >> 4;
            byte v = sampling & 0x0F;
            com->H = h; 
            com->V = v;
            if(h > maxH) maxH = h;
            if(v > maxV) maxV = v;

            com->Tq = bs.getByte();
            
            printf("component %d H: %d V: %d Tq: %d\n", com->Ci, com->H, com->V, com->Tq);
        }

        int unitsX = header.X / 8;
        if(unitsX % maxH != 0) {
            int unitsNeeded = maxH - (unitsX % maxH);
            header.X += unitsNeeded * 8;
            unitsX = header.X / 8;
        }
        int unitsY = header.Y / 8;
        if(unitsY % maxV != 0) {
            int unitsNeeded = maxV - (unitsY % maxV);
            header.Y += unitsNeeded * 8;
            unitsY = header.Y / 8;
        }

        
        printf("newest Y: %d X: %d\n", header.Y, header.X);

        header.mcuX = (unitsX / maxH);
        header.mcuY = (unitsY / maxV);

        printf("unitsX: %d unitsY: %d\n", unitsX, unitsY);
        printf("mcuX: %d mcuY %d\n", header.mcuX, header.mcuY);

        return header;
    }

    void buildQuantTables(ByteStream& bs) {
        word len = bs.getWord();
        len -= 2;
        if(len % 65 != 0) printf("len calc did not go as intended\n");
        int numTables = len / 65;

        for(int t=0; t < numTables; t++) {

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
            byte b = bs.getByte();
            header.vals.push_back(b);
        }
        
        
        printf("hft table %d ", header.tableIndex);
        if(header.dcTable) printf("DC\n");
        else printf("AC\n");

        huff.buildTable(header);

    }

    void reset() {
        for(int i=0; i < 4; i++) {
            dcPred[i] = 0;
        }
        maxH = 0;
        maxV = 0;
    }


    void startScan(ByteStream& bs, HuffmanDecoder& huff, FrameHeader fh, int*** img) {
        printf("start of scan\n");
        word len = bs.getWord();
        printf("scanheaderlen: %d\n", len);

        ScanHeader header; 
        header.Ns = bs.getByte();
        printf("scan components: %d\n", header.Ns);

        for(int i=0; i < header.Ns; i++) {
            byte cs = bs.getByte() - 1;
            auto com = &(header.components[cs]);
            com->Cs = cs;
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


        scan(bs, huff, fh, header, img);
    }

    void scan(ByteStream& bs, HuffmanDecoder& huff, FrameHeader fh, ScanHeader sh, int*** img) {
        int fi[4];

        for(int i=0; i < sh.Ns; i++) {
            auto scom = &(sh.components[i]);
            fi[i] = scom->Cs;
        }

        int unit[8][8];
        for(int y=0; y < fh.mcuY; y++) {
            for(int x=0; x < fh.mcuX; x++) {
            //printf("M %d %d \n", y, x);

                for(int c=0; c < sh.Ns; c++) { 
                    auto fhcom = &(fh.components[fi[c]]);
                    int** imgcom = img[c];
                    int dupRateX = maxH / fhcom->H;
                    int dupRateY = maxV / fhcom->V;
                    //printf("COMPONENT %d dupRate: %d %d\n", c, dupRateX, dupRateY);

                    for(int v=0; v < fhcom->V; v++) {
                        for(int h=0; h < fhcom->H; h++) {
                            decodeDataUnit(bs, huff, sh.components[c].Td, sh.components[c].Ta, c, unit);
                            quantizeDataUnit(fhcom->Tq, unit);
                            IDCT(unit);
                            levelShift(unit);

                            int imgX = x * maxH * 8 + h * dupRateX * 8;
                            int imgY = y * maxV * 8 + v * dupRateY * 8;
                            if(imgX < 0 || imgY < 0 || imgX >= fh.X || imgY >= fh.Y) {
                                printf("OUT OF BOUNDS PLACEMENT\n");
                                exit(0);
                            }

                            placeUnit(unit, imgcom, imgX, imgY, dupRateX, dupRateY);
                        }
                    }
                }
            }
        }

    }

    void placeUnit(int (*mat)[8], int** img, int x, int y, int dupRateX, int dupRateY) {
        for(int i=0; i < 8; i++) {
            for(int j=0; j < 8; j++) {
                for(int dy=0; dy < dupRateY; dy++) {
                    for(int dx=0; dx < dupRateX; dx++) {
                        int yi = y + i * dupRateY + dy;
                        int xi = x + j * dupRateX + dx;

                        img[yi][xi] = mat[i][j];
                    }
                }
            }
        }
    }

    void printUnit(int (*mat)[8]) {
        for(int i=0; i < 8; i++) {
            for(int j=0; j < 8; j++) {
                printf("%d ", mat[i][j]);
            }
        }
        printf("\n");
    }

    void levelShift(int (*mat)[8]) {
        for(int i=0; i < 8; i++) {
            for(int j=0; j < 8; j++) {
                mat[i][j] += 128;
            }
        }
    }

    void IDCT(int (*mat)[8]) {
        int temp[8][8];
        float C0 = 1.0f / sqrt(2.0f);
        float C1 = 1.0f;
        for(int y=0; y < 8; y++) {
            for(int x=0; x < 8; x++) {
                float sum = 0;

                for(int u=0; u < 8; u++) {
                    for(int v=0; v < 8; v++) {
                        float S = (float)mat[v][u];
                        float Cu = u == 0 ? C0 : C1;
                        float Cv = v == 0 ? C0 : C1;
                        float added = Cu * Cv * S * cos((2*x+1) * u * M_PI * 0.0625f) * cos((2*y+1) * v * M_PI * 0.0625f);
                        sum += added;
                    }
                }
                sum *= 0.25f;

                temp[y][x] = (int)sum;
            }
        }

        for(int i=0; i < 8; i++) {
            for(int j=0; j < 8; j++) {
                mat[i][j] = temp[i][j];
            }
        }
    }

    void quantizeDataUnit(int qtTable, int (*mat)[8]) {
        int (*qt)[8] = quantTables[qtTable];

        for(int i=0; i < 8; i++) {
            for(int j=0; j < 8; j++) {
                mat[i][j] *= qt[i][j];
            }
        }

    }


    byte decodeNext(ByteStream& bs, HuffmanDecoder& huff, int table, bool dc) {
        int code = 0;
        byte value = 0;

        for(int len=1; len <= 16; len++)
        {
            code = code << 1;
            code |= bs.getBits(1);
            if(huff.lookup(table, dc, code, len, &value)) {
                return value;
            }

        }
        
        printf("Failed to decode val: %x\n", code);
        return 0;
    }

    int getAppendedValue(ByteStream& bs, int bitsNeeded) {
        int val = bs.getBits(bitsNeeded);
        if((val & (1 << (bitsNeeded - 1))) == 0) {
            // negative num
            int offset = pow(2, bitsNeeded) - 1;
            val -= offset;
        }
        return val;
    }

    int decodeDC(ByteStream& bs, HuffmanDecoder& huff, int table, int comp) {
        byte bitsNeeded = decodeNext(bs, huff, table, true);
        int diff = 0;
        if(bitsNeeded > 0) diff = getAppendedValue(bs, bitsNeeded);
        int output = dcPred[comp] + diff;

        dcPred[comp] = output;
        return output;
    }

    vector<int> decodeAC(ByteStream& bs, HuffmanDecoder& huff, int table) {
        int c=1;

        vector<int> output;
        bool endOfBlock = false;

        for(int i=0; i < 63; i++) output.push_back(0);

        for(byte i=0; i < 63; i++) {
            //printf("index: %d\n", i);
            byte run = decodeNext(bs, huff, table, false);

            byte numZeros = run >> 4;
            byte bitsNeeded = run & 0x0F;
            //printf("run %d %d\n", numZeros, bitsNeeded);

            if(bitsNeeded == 0) {
                if(numZeros == 0) {
                    //printf("ENDOFBLOCK\n");
                    break;
                }
                else if(numZeros != 15) {
                    printf("this should not appear in the run. ever.\n");
                    exit(0);
                }
                else if(i + 16 >= 63) {
                    printf("ALSO NOW TOO MANY ZEROS BAD BAD\n");
                    exit(0);
                }

                i += 15;
                continue;
            }
            
            if(i + numZeros > 62) {
                printf("TOO MANY ZEROS THIS IS BAD\n");
                exit(0);
            }
            i += numZeros;

            int val = getAppendedValue(bs, bitsNeeded);
            output[i] = val;
        }

        return output;
    }

    void decodeDataUnit(ByteStream& bs, HuffmanDecoder& huff, int dcTable, int acTable, int index, int (*mat)[8]) {
        int dc = decodeDC(bs, huff, dcTable, index);
        vector<int> ac = decodeAC(bs, huff, acTable);

        int aci = 0;
        int i = 0;
        int j = 0;
        bool secondHalf = false;
        bool rowPos = false;

        // zigzag placement order
        while(i < 8 && j < 8) {
            if(!secondHalf && (i == 7 || j == 7)) secondHalf = true;

            while(i >= 0 && i < 8 && j >= 0 && j < 8) {
                if(i == 0 && j == 0) mat[i][j] = dc;
                else mat[i][j] = ac[aci++];

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

    void YCbCrToRGB(int y, int cb, int cr, int& r, int& g, int& b) {
        double Y = (double) y;
        double Cb = (double) cb;
        double Cr = (double) cr;

        r = (int) (Y + 1.40200 * (Cr - 0x80));
        g = (int) (Y - 0.34414 * (Cb - 0x80) - 0.71414 * (Cr - 0x80));
        b = (int) (Y + 1.77200 * (Cb - 0x80));

        r = std::max(0, std::min(255, r));
        g = std::max(0, std::min(255, g));
        b = std::max(0, std::min(255, b));
    }

    void ImgYCbCrToRGB(int*** img, int rows, int cols) {
        int r, g, b;
        for(int i=0; i < rows; i++) {
            for(int j=0; j < cols; j++) {
                YCbCrToRGB(img[0][i][j], img[1][i][j], img[2][i][j], r, g, b);
                img[0][i][j] = r;
                img[1][i][j] = g;
                img[2][i][j] = b;
            }
        }
    }

    void printImg(int*** img, int rows, int cols) {
        for(int i=0; i < rows; i++) {
            for(int j=0; j < cols; j++) {
                printf("(%d, %d, %d) ", img[0][i][j], img[1][i][j], img[2][i][j]);
            }
            printf("\n");
        }
    }

    public:
    Image processFile(std::string filename) {
        reset();

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
                    exit(0);

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
            exit(0);
        }

        FrameHeader fh = baselineFrame(bs);

        int*** img = new int**[fh.Nf];
        for(int i=0; i < fh.Nf; i++) {
            img[i] = new int*[fh.Y];
            for(int j=0; j < fh.Y; j++) {
                img[i][j] = new int[fh.X];
            }
        }


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
                    startScan(bs, huff, fh, img);
                    break;

                case MARKER_ERROR:
                    printf("VERY BAD MARKER ERROR\n");
                    exit(0);

                default:
                    printf("unsupported marker in frame\n");
            }

            markerBuf = bs.nextMarker();
        }

        printf("reached end of image!\n");

        ImgYCbCrToRGB(img, fh.Y, fh.X);

        Image output = Image(img, fh.oldY, fh.oldX);

        for(int i=0; i < fh.Nf; i++) {
            for(int j=0; j < fh.Y; j++) {
                delete[] img[i][j];
            }
            delete[] img[i];
        }
        delete[] img;

        return output;
    }

};

