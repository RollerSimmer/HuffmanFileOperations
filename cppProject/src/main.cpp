
#include "HuffmanFile.h"
#include "HuffmanFilePrinter.h"
#include "FileComparer.h"
#include <iostream>

using namespace std;

int main(int argc,char*args[])
    {
    //load a file here

    string fname,ename,dname;

    #if 0
        fname="testtext.txt";
        ename="testtext.huf";
        dname="testtext_decoded.txt";
    #elif 0
        fname="testsmall.jpg";
        ename="testsmall.huf";
        dname="testsmall_decoded.jpg";
    #elif 0
        fname="lines.bmp";
        ename="lines.huf";
        dname="lines_decoded.bmp";
    #elif 0
        fname="testmov.webm";
        ename="testmov.huf";
        dname="testmov_decoded.webm";
    #else
        if(argc>=4)
            {
            fname=args[1];
            ename=args[2];
            dname=args[3];
            }
        else
            {
            cout<<"This program needs at least 3 arguments, specifying names of original, encoded, and decoded files."<<endl;
            return -1;
            }
    #endif

    FileComparer* fc= new FileComparer(fname,dname);

    HuffmanFile* hf=new HuffmanFile();
    HuffmanFilePrinter* hfp = new HuffmanFilePrinter(hf);

    hf->encodeFile(ename,fname);

    HuffmanFilePrinter::printtype pts[4]={HuffmanFilePrinter::pt_char,HuffmanFilePrinter::pt_binary,
                                          HuffmanFilePrinter::pt_decimal,HuffmanFilePrinter::pt_hex};
    int amtColumns=16;

    cout<<endl<<"************************ENCODE STEP********************************"<<endl<<endl;

    fc->setFileA(hf->getRawBuffer());

    hfp->printTree();
    hfp->printCodeMap();

    const bool doPrintData=false;

    if(doPrintData)
        {

        for (int i=0;i<4;i++)
            hfp->printDataStream(HuffmanFile::ft_decoded,pts[i],amtColumns);
        for (int i=0;i<4;i++)
            hfp->printDataStream(HuffmanFile::ft_encoded,pts[i],amtColumns);
        }

    cout<<endl<<"************************DECODE STEP****************************"<<endl<<endl;


    hf->decodeFile(dname,ename);

    fc->setFileB(hf->getRawBuffer());

    hfp->printTree();
    hfp->printCodeMap();
    if(doPrintData)
        {

        for (int i=0;i<4;i++)
            hfp->printDataStream(HuffmanFile::ft_decoded,pts[i],amtColumns);
        for (int i=0;i<4;i++)
            hfp->printDataStream(HuffmanFile::ft_encoded,pts[i],amtColumns);
        }

    cout<<endl<<"************************COMPARISON STEP****************************"<<endl<<endl;

    fc->printDifferences();

    delete hf;
    delete fc;
    }
