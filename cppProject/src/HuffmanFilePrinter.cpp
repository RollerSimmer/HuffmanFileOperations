#include "HuffmanFilePrinter.h"
#include <fstream>
#include <stack>
#include <iostream>
#include <sstream>

using namespace std;

HuffmanFilePrinter::HuffmanFilePrinter(HuffmanFile*hf)
    {
    this->hf=hf;
    }

HuffmanFilePrinter::~HuffmanFilePrinter()
    {
    //dtor
    }


/** \brief print the huffman file.
 * \in the tree and map should be built and coded and raw buffers should be full.
 * \out the console will show the contents of the file.
 */
void HuffmanFilePrinter::printTree()
    {
    // init level, node, short list and code stack
    stringstream shortlist;  // short string
    shortlist.str("");
    int x=1;
    x+=1;
    std::stack<int> levelstack;
    std::stack<HuffmanTreeNode> nodestack;
    std::stack<string> codestack;
    // if there are no roots, then do this
    if(hf->roots.size()<=0)
        {
        // print an error message and return
        std::cout<<"printCodeTree() error: no root nodes."<<endl;
        return;
        }

    // print node list
    std::cout<<"-- Node list --"<<endl;
    for (int i=0;i < hf->nodes.size();i++)
        {
        string codestr="";
        this->printTreeNode(0,hf->nodes.at(i),codestr);
        }
    std::cout<<endl;

    // print tree heading
    std::cout<<"-- Tree Data --"<<endl;
    // for every root in the file, do this
    for (int i=0;i<hf->roots.size();i++)
        {
        // push a root node
        nodestack.push(hf->nodes.at(hf->roots.at(i)));
        // push level 0
        levelstack.push(0);
        // push level null codestr
        codestack.push("");
        }
    // while the stack is not empty, do this
    while (!nodestack.empty()&&!levelstack.empty())
        {
        // pop the next tree node
        HuffmanTreeNode node=nodestack.top();
        nodestack.pop();
        // pop the current level
        int level=levelstack.top();
        levelstack.pop();
        // pop the current code
        string codestr=codestack.top();
        codestack.pop();
        // print node with level as indentation level argument
        this->printTreeNode(level,node,codestr);
        // add node index to short tree list string
        if (node.iparent!=-1)
            shortlist<<", ";
        shortlist<<node.index;
        // if node has right child do this.
        if(node.ir!=-1)
            {
            // push right child node
            nodestack.push(hf->nodes.at(node.ir));
            // push next level
            levelstack.push(level+1);
            // push codestr concatentated with "1"
            codestack.push("1"+codestr);
            }
        // if node has left child, do this.
        if(node.il!=-1)
            {
            // push left child node
            nodestack.push(hf->nodes.at(node.il));
            // push next level
            levelstack.push(level+1);
            // push codestr concatentated with "0"
            codestack.push("0"+codestr);
            }
        }
    // print blank line
    cout<<endl;
    // print short tree list
    cout<<"Short list for tree"<<endl;
    cout<<shortlist.str()<<endl;
    // print blank line
    cout<<endl;
    // print the list in post order
    this->printListPostOrder();
    // return
    return;
    }

/** \brief print a single node of the huffman tree.
 * \param level the tree level used for indentation
 * \param node the node to print
 * \in the tree should be built.
 * \out the console will show the contents of the node at the proper indentation.
 */
void HuffmanFilePrinter::printTreeNode(int level,HuffmanTreeNode& node,string& codestr)
    {
    const string tab=".  ";
    // print indention
    for (int i=0;i<level+1;i++)
        cout<<tab;
    // make data hex string
    std::stringstream datahex;
    datahex<<(node.data<0x10?"0":"")<<std::hex<<(unsigned short)node.data<<std::dec;
    // print tree data in format "Node: index=<index>, parent=<parent>, data=<data>, code=<code, direction=<direction>, weight=<weight>"
    bool isleaf=node.il==-1&&node.ir==-1;
    if(isleaf)
        std::cout<<"Leaf-";
    std::cout<<"Node: index="<<node.index;
    std::cout<<", parent="<<node.iparent;
    if(isleaf)
        {
        std::cout<<", code1="<<codestr;
        string code2str="";
        CodeMapEntry code2=hf->codemap[node.data];
        for (int b=0;b<code2.bitcount;b++)
            {
            unsigned short bitmask=(unsigned short)1<<b;
            code2str = (code2.bitcode&bitmask? "1": "0") + code2str;
            }
        std::cout<<", code2="<<code2str;
        }
    std::cout<<", direction="<<node.dirFromParent;
    std::cout<<", data="<<(short)node.data<<" ("<<datahex.str()<<")";
    std::cout<<", weight="<<node.weight;
    std::cout<<", left="<<node.il;
    std::cout<<", right="<<node.ir;
    std::cout<<endl;
    }

/** \brief prints the codes mapped to each data value.
 * \in the codes should already be mapped.
 * \out the console should show the list of code-to-data mappings.
 */
void HuffmanFilePrinter::printCodeMap()
    {
    // print heading
    cout<<"-- Code Map --"<<endl;
    cout<<"   data -> code"<<endl;
    cout<<"   ------------"<<endl;
    // init code count
    int amtcodes=0;


    // for each byte value, do this
    for (int i=0;i<256;i++)
        {
        // init code map entry reference
        CodeMapEntry& cme=hf->codemap[i];
        // if code has zero length, do this
        if(cme.bitcount==0)
            // continue without printing
            continue;
        // else if bit count >=8
        else if (cme.bitcount>=8)
            // execute dummy code for a break.
            int dummy=1;
        //increment amount of codes
        ++amtcodes;
        // init code string
        string codestr="";
        for (int j=0;j<cme.bitcount;j++)
            codestr = ( cme.bitcode&((unsigned short)1<<j)? "1": "0" ) + codestr;
        // print the map table entry with format "   data -> deccode (bincode)"
        cout<<"   "<<i<<" -> "<<(short)cme.bitcode<<" ("<<codestr<<")"<<endl;
        }

    // print the amount of codes
    cout<<"code count: "<<amtcodes<<endl;
    // print blank line
    cout<<endl;
    }

/** \brief print the raw data stream.
 * \in the raw data should exist.
 * \out the contents of the raw data stream are printed to the console.
 */
void HuffmanFilePrinter::printDataStream(HuffmanFile::filetype ft,HuffmanFilePrinter::printtype pt,short amtColumns)
    {
    //initialize buffer pointer
    std::vector<unsigned char>* buf;
    if(ft==HuffmanFile::ft_decoded)
        buf=&hf->rawbuf;
    else if(ft==HuffmanFile::ft_encoded)
        buf=&hf->codebuf;
    else
        {
        cout<<"HuffmanFile::printDataStream(): Bad file type."<<endl;
        return;
        }

    // print heading
    cout<<"-- ";
    switch(ft)
        {
        case HuffmanFile::ft_decoded: cout<<"Decoded/Raw"; break;
        case HuffmanFile::ft_encoded: cout<<"Encoded"; break;
        }
    cout<<" file";
    switch(pt)
        {
        case pt_char: cout<<" (character)"; break;
        case pt_binary: cout<<" (binary)"; break;
        case pt_decimal: cout<<" (decimal)"; break;
        case pt_hex: cout<<" (hexadecimal)"; break;
        }
    cout<<" --"<<endl;

    // for each byte value in data stream, do this
    for (int i=0;i<buf->size();i++)
        {
        // if at start of row, then do this
        if (i%amtColumns==0)
            {
            // if i > 0, then do this
            if(i>0)
                // print new line
                cout<<endl;
            // print the current index and a colon
            cout<<std::hex<<i<<std::dec;
            cout<<": ";
            }
        // if index >= 1, then do this
        else if (i > 0)
            // print a comma and (optionally) a space
            cout<<", ";
        // print a byte
        cout<<makeBytePrintable(buf->at(i),pt);
        }
    // print 2 blank lines
    cout<<endl<<endl;
    }

/** \brief make a byte printable in specific format.
 * \param b the byte to print.
 * \param pt its format.
 * \in static function
 * \out static function
 */
string& HuffmanFilePrinter::makeBytePrintable(unsigned char b,HuffmanFilePrinter::printtype pt)
    {
    // initialize the static printable byte string for return value
    static string mbp;
    static stringstream hexs;
    static stringstream decs;
    mbp="";
    hexs.str("");
    decs.str("");

    // build hex string
    hexs << ( b<16? "0": "" ) << std::hex << (short)b;

    // select a print type
    switch(pt)
        {
        // on binary, do this
        case pt_binary:
            // for 8 bits, do this
            for (int i=0;i<8;i++)
                // add "1" to return string if current bit is set, otherwise add "0"
                mbp = ( b&(1<<i)? "1": "0" ) + mbp;
            // break out of case
            break;
        // on decimal, do this
        case pt_decimal:
            // build decimal string
            decs<<(short)b;
            // add decimal to string
            mbp+=decs.str();
            // break out of case
            break;
        // on hex, do this
        case pt_hex:
            mbp+=hexs.str();
            // break out of case
            break;
        // on character, do this
        case pt_char:
        default:
            // add starting single quote
            mbp+='\'';
            // if character is in valid printable range, do this
            bool printable=( b>=' ' && b<=0x7E );
            if(printable)
                // add character to return string
                mbp+=b;
            // else, do this
            else
                {
                // add to the return string "\xHH" where HH is the hex code for byte
                mbp+="\\x";
                mbp+=hexs.str();
                }
            // add trailing single quote
            mbp+='\'';
            // break out of case
            break;
        }

    //return the printable byte
    return mbp;
    }

/** \brief print the tree list in post order.
 * \in the tree should already be built.
 * \out the console will show the post-order list.
 */
void HuffmanFilePrinter::printListPostOrder()
    {
    // print heading
    cout<<"Post order short list:"<<endl;
    // pre-emptive returns for zero-length lists
    if(hf->roots.size()==0) return;
    if(hf->nodes.size()==0) return;
    // clear all visited flags
    hf->clearVisitedFlags();
    // init stack
    std::stack<int> indexstack;
    // push root
    indexstack.push(hf->roots.at(0));
    // init node count
    int nodeCountSoFar=0;
    // while stack is not empty, do this
    while(!indexstack.empty())
        {
        // look at top node, and mark it visited
        int itop=indexstack.top();
        // init node pointers
        HuffmanTreeNode*top = &hf->nodes.at(itop);
        HuffmanTreeNode*left = top->il==-1? 0: &hf->nodes.at(top->il);
        HuffmanTreeNode*right = top->ir==-1? 0: &hf->nodes.at(top->ir);
        // set top to visited and can-go flags
        top->visited=true;
        bool canGoLeft, canGoRight;
        canGoLeft = left!=0? !left->visited: false;
        canGoRight = right!=0? !right->visited: false;
        // if top node has unvisited left child, do this
        if(canGoLeft)
            // push the left child of the top node
            indexstack.push(top->il);
        // elif top node has unvisited right child, do this
        else if(canGoRight)
            // push the right child of the top node
            indexstack.push(top->ir);
        // else, do this
        else
            {
            // print top node
            if(nodeCountSoFar>0)
                cout<<", ";
            cout<<itop;
            // pop top
            indexstack.pop();
            ++nodeCountSoFar;
            }
        }
    // print blank line
    cout<<endl<<endl;
    }
