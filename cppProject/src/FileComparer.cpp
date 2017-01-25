#include "FileComparer.h"
#include <iostream>
using namespace std;

FileComparer::FileComparer(string aname,string bname)
    {
    this->aname=aname;
    this->bname=bname;
    }

FileComparer::~FileComparer()
    {
    //dtor
    }

/** \brief file A setter */
void FileComparer::setFileA(std::vector<unsigned char>& buf)
    {
    a=buf;
    }

/** \brief file B setter */
void FileComparer::setFileB(std::vector<unsigned char>& buf)
    {
    b=buf;
    }

/** \brief prints the differences of two files. */
void FileComparer::printDifferences()
    {
    cout<<"Differences between file \'"<<aname<<"\' and file \'"<<bname<<"\'"<<endl;
    int asz=a.size();
    int bsz=b.size();
    int diffcount=0;

    int losize=std::min(asz,bsz);
    for (int i=0;i<losize;i++)
        {
        if(a[i]!=b[i])
            {
            // print the difference between bytes
            cout<<"Byte 0x"<<std::hex<<i<<std::dec<<": ";
            cout<<(short)a[i]<<" in file A, "<<(short)b[i]<<" in file B. ";
            cout<<endl;
            ++ diffcount;
            }
        }
    // print no differences in compared bytes if difference count = 0
    if(diffcount==0)
        cout<<"No differences in compared bytes."<< endl;
    else
        {
        cout<<"Counted "<<diffcount<<" difference"<<(diffcount>1? "s": "")<<" between A and B";
        cout<<" out of "<<losize<<" compared byte pairs."<<endl;
        }
    // print info on premature comparison ends
    if(asz<bsz) cout<<"A has "<<(bsz-asz)<<" less bytes than B."<<endl;
    else if(bsz<asz) cout<<"B has "<<(asz-bsz)<<" less bytes than A."<<endl;
    else cout<<"Files are the same size."<<endl;
    // one blank line to finish
    cout<<endl;
    }

