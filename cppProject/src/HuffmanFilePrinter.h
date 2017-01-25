#ifndef HUFFMANFILEPRINTER_H
#define HUFFMANFILEPRINTER_H

#include "HuffmanFile.h"

class HuffmanFilePrinter
{
    public:

        enum printtype { pt_char,pt_binary,pt_decimal,pt_hex };

        HuffmanFilePrinter(HuffmanFile*hf);
        virtual ~HuffmanFilePrinter();

        void printTree();
            void printTreeNode(int level,HuffmanTreeNode& node,string& codestr);
            void printListPostOrder();
        void printCodeMap();
        void printDataStream(HuffmanFile::filetype ft,printtype pt,short amtColumns);
            static string& makeBytePrintable(unsigned char b,printtype pt);

    protected:
        HuffmanFile *hf;        /**< the huff man file which this class prints */
    private:
};

#endif // HUFFMANFILEPRINTER_H
