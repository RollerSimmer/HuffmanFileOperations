#pragma once

#include "HuffmanTreeNode.h"
#include "TreeFileNode.h"
#include "CodeMapEntry.h"
#include <string>
#include <vector>
#include <hash_map>

using namespace std;

class HuffmanFilePrinter;
class FileComparer;

// HuffmanFile is used to work encode and decode files via Huffman coding
class HuffmanFile
    {
    friend HuffmanFilePrinter;
    friend FileComparer;
    protected:
    vector<unsigned char> codebuf;  // buffer for coded bytes
    vector<unsigned char> rawbuf;   // buffer for raw bytes
    int bytecounts[256];            // number of occurences for each byte value in file
    CodeMapEntry codemap[256];      // map specifying the bit code for each byte that occurs in the file.
    std::vector<HuffmanTreeNode> nodes;  // forest of huffman trees for encoding and decoding
    std::vector<int> roots;         // indices of tree roots
    int light[2];                   // indices into roots[] of lightest two trees
    long cpos=0;                    // current byte position in the coded data stream
    long cbitpos=0;                 // current bit position in the coded data stream
    long rpos=0;                    // current byte position in the raw data stream
    bool dodebug=true;              // debug flag

    public:
    enum filetype { ft_encoded,ft_decoded };

    HuffmanFile();
    void encodeFile(string& ename,string& dname);
        bool loadfile(string& fname,HuffmanFile::filetype how);
        void buildEncodeTree();
            void countOccurrences();
            void initEncodeForest();
            void findTwoLightestTrees();
                int nodeweight(int index);
            void joinLightestTrees();
            void autoSetRootFlags();
            void buildCodeMap();
        void encodeWithTree();
            void encodeTreeData();
            void putEncodedPackedBit(long bitpos,CodeMapEntry ce);
        void savefile(string& fname,HuffmanFile::filetype how);
    void decodeFile(string& dname,string& ename);
        bool checkEncodedHeader();
        void decodeWithTree();
            void decodeTreeData();
                TreeFileNode& readTreeFileNode();
                void smartAddTreeEntry(HuffmanTreeNode& node);
            unsigned char getBitPackedEncodedData();
                unsigned char getCodeBit();
    void clearVisitedFlags();

    vector<unsigned char>& getRawBuffer() { return rawbuf; }
    };

