#pragma once

#include <vector>
#include <string>

using namespace std;

class FileComparer
    {
    public:
        FileComparer(string aname,string bname);
        virtual ~FileComparer();
        void setFileA(std::vector<unsigned char>& buf);
        void setFileB(std::vector<unsigned char>& buf);
        void printDifferences();

    protected:
        string aname;                // name of file A
        string bname;                // name of file B
        std::vector<unsigned char> a; // file A
        std::vector<unsigned char> b; // file B
    private:
    };
