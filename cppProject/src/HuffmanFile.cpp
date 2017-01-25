#include "HuffmanFile.h"
#include <fstream>
#include <stack>
#include <iostream>
#include <sstream>

/** \brief constructor for HuffmanFile
 */
HuffmanFile::HuffmanFile()
    {
    this->dodebug=false;
    }

/***********************************
 * \brief encodes a file
 * \param ename the name of the encoded file to which to write.
 * \param dname the name of the unencoded file from which to read.
 * \in the unencoded file should exist
 * \out the target encoded file will contain the result of the encoding
 ***********************************/
void HuffmanFile::encodeFile(string& ename,string& dname)
    {

    // load the file to decoded buffer; if loaded:
    if(loadfile(dname,ft_decoded))
        {
        // step through file and build tree
        buildEncodeTree();
        // encode to coded buffer using tree
        encodeWithTree();
        // save encoded file
        savefile(ename,ft_encoded);
        }

    // return
    return;
    }

/***********************************
 * \brief decodes a file
 * \param fname the filename
 * \in: the file should exist and be encoded coming in
 * \out: the file should be decoded and stored in raw buffer
 ***********************************/
void HuffmanFile::decodeFile(string& dname,string& ename)
    {
    // load the file to encoded buffer; if loaded:
    if(loadfile(ename,ft_encoded))
        {
        // init coded position
        cpos=0;
        // if proper header exists, then do this
        if(checkEncodedHeader())
            {
            // encode to coded buffer using tree
            decodeWithTree();
            // save encoded file
            savefile(dname,ft_decoded);
            }
        }
    // return
    return;
    }

/***********************************
 * \brief loads a file
 * \param fname the filename
 * \param how how this file should be loaded.  Valid values are
 *               HuffmanFile::ft_encoded
 *               HuffmanFile::ft_decoded
 * \in the file should exist and be encoded coming in.
 * \out the contents of the file should be stored in the
 *      either the raw or coded buffer depending on load type.
 * \return true on success
 ***********************************/
bool HuffmanFile::loadfile(string& fname,HuffmanFile::filetype how)
    {
    vector<unsigned char>*buf;
    if(how==HuffmanFile::ft_encoded)
        buf=&this->codebuf;
    else if(how==HuffmanFile::ft_decoded)
        buf=&this->rawbuf;

    //open the file for reading
    ifstream *fs=new ifstream(&fname[0],ios_base::binary);

    unsigned char c;

    buf->clear();

    do
        {
        c = fs->get();
        if(fs->good())
            buf->push_back(c);
        }   while (fs->good());

    delete fs;

    return true;
    }

/** \brief builds a huffman tree from the raw file data
 * \in the raw data buffer should have something in it from a load.
 * \out there will be a single tree in the nodes vector; it will contain the Huffman tree from which to encode.
 * \return void
 */
void HuffmanFile::buildEncodeTree()
    {
    // clear nodes and root indices
    nodes.clear();
    roots.clear();
    // count byte occurrences (weights) in file
    countOccurrences();
    // make a forest of leaves to start
    initEncodeForest();
    // while root list has more than one entry, consolidate the two smallest trees:
    while (roots.size()>1)
        {
        // find the Lightest trees (there should be at least 2 roots)
        findTwoLightestTrees();
        // join the lightest two trees
        joinLightestTrees();
        }
    // set root flags for any nodes in root list or any with no iparent value
    autoSetRootFlags();
    // build a code lookup map
    buildCodeMap();

    //return from building encode tree
    return;
    }

/** \brief Huffman-encode the file with a tree.
 * \in the tree should be built and there should only be one root.
 * \out the encoded data buffer will be filled and ready for file output
 */
void HuffmanFile::encodeWithTree()
    {
    // clear encoded buffer
    codebuf.clear();
    // write the string "HUFFMAN_FILE" to encode buffer
    unsigned char header[]="HUFFMAN_FILE\0";
    int i=0;
    while (header[i]!=0)
        codebuf.push_back(header[i++]);
    // write zeros to encode buffer to hold place for 32-bit size value
    for (int i=0;i<4;i++)
        codebuf.push_back(0);
    // encode tree data
    encodeTreeData();
    // set bit position to 8*(byte position after tree write)
    long bytepos=codebuf.size();
    long bitpos=bytepos*8;
    // go through raw buffer and write encoded equivalent to encode buffer for each byte, using bit packing:
    for(int i=0;i<rawbuf.size();i++)
        {
        // lookup code for current byte on code lookup table
        unsigned char b=rawbuf.at(i);
        CodeMapEntry ce=codemap[b];  // current code entry
        // put bit code at bit position in encode buffer
        putEncodedPackedBit(bitpos,ce);
        bitpos+=ce.bitcount;
        }
    //write the amount of bits to 32-bit size value near the start of the file
    union _codesize { unsigned long full; unsigned char bytes[0]; } codesize;
    codesize.full=bitpos;
    for (int i=0;i<4;i++)
        codebuf.at(12+i)=codesize.bytes[i];
    // premature exit for testing
    //if(1)
    //    return;
    }

/** \brief count the number of occurrences for each byte value over the whole raw data buffer
 * \in the raw data should have a stream of bytes from a file
 * \out the bytecounts array will be filled with counts corresponding to each byte value
 */
void HuffmanFile::countOccurrences()
    {
    unsigned char v;  // the byte value
    // set all byte counts to zero
    for (int b=0;b<256;b++)
        bytecounts[b]=0;
    // go through each byte in raw data and do the following:
    for (int o=0;o<rawbuf.size();o++)
        {
        // add to the occurrence count of the currently encountered byte value
        v=rawbuf.at(o);
        ++bytecounts[v];
        }
    }

/** \brief create a starting forest for the Huffman tree algorithm, containing only leaf nodes.
 * \in list of nodes should be empty, as should the list of roots.
 * \out the list of nodes will be filled will leaf nodes corresponding to all of the characters that occur more than once.
 */
void HuffmanFile::initEncodeForest()
    {
    //clear both the root list and node list
    roots.clear();
    nodes.clear();
    // step through each byte value and do the following:
    for (unsigned short b=0;b<256;b++)
        {
        // if the occurrence count for the byte value is more than one then do this:
        int bc=bytecounts[b];
        if (bc>0)
            {
            // create a leaf node and root entry corresponding to this value
            HuffmanTreeNode node={bc,b,dir_root,nodes.size(),-1,-1,-1};
            roots.push_back(nodes.size());  // i.e. the next index
            nodes.push_back(node);
            }
        }
    }

/** \brief find the two lightest trees in the forest
 * \in there should be at least two trees (roots) in the forest of huffman trees
 * \out the light[] array will be filled with the root indices of the two lightest trees
 */
void HuffmanFile::findTwoLightestTrees()
    {
    // init the light indices to the first root index
    light[0]=0;
    if(nodes.size()<=1)
        {
        light[1]=0;
        return;
        }
    else
        light[1]=1;
    if(nodeweight(roots.at(light[0])) > nodeweight(roots.at(light[1])))
        {
        light[1]=0;
        light[0]=1;
        }
    // starting at the second root, step through root indices and do the following:
    for (int i=0;i<roots.size();i++)
        {
        int cri=roots[i];  // current root index
        // if current root index == one of the light root indices, then do this
        if( i==light[0] || i==light[1] )
            // continue on in the loop
            continue;
        // elif tree at root index is lighter than lightest tree so far then do this:
        else if (nodeweight(cri) < nodeweight(roots.at(light[0])))
            {
            // replace second lightest with lightest root index
            light[1]=light[0];
            // replace lightest root index with current root index
            light[0]=i;
            }
        // elif tree at root index is lighter than 2nd lightest tree so far then do this:
        else if (nodeweight(cri) < nodeweight(roots.at(light[1])))
            {
            // replace second lightest with current root index
            light[1]=i;
            }
        }
    }

/** \brief join the lightest trees
 * \in the node list should be filled, and the two lightest trees should be known
 * \out the two lightest trees will be replaced with one tree, also the light[] array entries index the new root
 */
void HuffmanFile::joinLightestTrees()
    {
    // return if lightest tree root indices are the same
    if (light[0]==light[1])
        return;
    // init light node indices
    int lni[2]={roots.at(light[0]),roots.at(light[1])};
    //create a new root
    int newWeight=nodeweight(lni[0])+nodeweight(lni[1]);
    HuffmanTreeNode newroot={newWeight,0,dir_root,nodes.size(),-1,lni[0],lni[1]};
    //delete 2 lightest tree roots from forest
    if(light[0]>light[1])  {
        roots.erase(roots.begin()+light[0]);
        roots.erase(roots.begin()+light[1]);  }
    else  {
        roots.erase(roots.begin()+light[1]);
        roots.erase(roots.begin()+light[0]);  }
    //add new tree root to forest
    roots.push_back(nodes.size());
    nodes.push_back(newroot);
    // set parent of previous two lightest roots, which are now branches of the new root
    int newRootIndex=roots.size()-1;
    HuffmanTreeNode& leftnode=nodes.at(lni[0]);
    HuffmanTreeNode& rightnode=nodes.at(lni[1]);
    leftnode.iparent=rightnode.iparent=newroot.index;
    leftnode.dirFromParent=dir_left;
    rightnode.dirFromParent=dir_right;
    //return
    return;
    }

/** \brief get the weight of a node
 * \param index of a node in the node list
 * \in the node list should be initialized, and the index should be a valid with respect to the node list
 * \out no change
 * \return the node's weight, or zero if error
 */
int HuffmanFile::nodeweight(int index)
    {
    if(index<nodes.size())
        return nodes.at(index).weight;
    else
        return 0;
    }

/** \brief build a code lookup map
 * \in the code tree should be already constructed and there should only be one root in root list
 * \out the code lookup map will be built and ready to use for quick reference
 */
void HuffmanFile::buildCodeMap()
    {
    // clear code map
    CodeMapEntry ece = {0,0,0};  // empty code entry
    for (int i=0;i<256;i++)
        {
        codemap[i]=ece;
        }
    // initialize code stack

    std::stack<CodeMapEntry> codestack;
    while(!codestack.empty()) codestack.pop();
    // initialize node index stack
    // push the only root; return if there are no roots
    CodeMapEntry nce;  // new code entry
    CodeMapEntry cce;  // current code entry
    if(roots.size() >= 1)
        {
        nce.bitcount=0;
        nce.bitcode=0;
        nce.nodeindex=roots.at(0);
        codestack.push(nce);
        }
    else
        return;

    // while the code stack has something on it, keep doing this:
    while (!codestack.empty())
        {
        // pop code entry
        cce=codestack.top();
        codestack.pop();
        // if node indexed by code entry is not null, then do this:
        HuffmanTreeNode node=nodes.at(cce.nodeindex);
        if (true)
            {
            bool isleaf=true;
            nce.bitcount=cce.bitcount+1;
            nce.bitcode=cce.bitcode;
            unsigned short bitmask=((unsigned short)1<<(nce.bitcount-1));
            // if node indexed by code entry has right branch then do this:
            if (node.ir!=-1)  {
                // push a code entry for right branch (tack a 1 bit to tail)
                if(nce.bitcount>=10)
                    int dummy=1;
                nce.bitcode=nce.bitcode | bitmask;
                nce.nodeindex=node.ir;
                codestack.push(nce);
                isleaf=false;
                }
            // if node indexed by code entry has left branch then do this:
            if (node.il!=-1)  {
                // push a code entry for left branch (tack a 0 bit to tail)
                nce.bitcode=nce.bitcode & ~bitmask;
                nce.nodeindex=node.il;
                codestack.push(nce);
                isleaf=false;
                }
            // if node indexed by code entry is a leaf then do this:
            if (isleaf)  {
                // add code entry to code map
                codemap[node.data]=cce;
                }
            }
        }
    }

/** \brief decode the tree data from the file
 * \in the encoded file should be loaded to the encoded data buffer and the coded position should be at the start of the tree data.
 * \out there will be one tree.
 */
void HuffmanFile::decodeTreeData()
    {
    // *********************** NEW WAY **************************************

    // clear node and root index list
    nodes.clear();
    roots.clear();

    // init stack
    std::stack<HuffmanTreeNode> nodestack;
    //std::stack<TreeFileNode> fnodestack;
    // init done flag to false
    bool done=false;
    // while not done reading nodes
    while (!done)
        {
        // read the next node
        TreeFileNode tfn;              // tree file nodes
        tfn=readTreeFileNode();
        HuffmanTreeNode node,left,right;  // the current node and its children
        node = {1, tfn.isleaf? tfn.data: 0, dir_root, 0, -1, -1, -1, false,tfn.isroot};
        // if node is a leaf, then do this
        if(tfn.isleaf)
            {
            // push the node onto the stack
            nodestack.push(node);
            }
        // else, do this
        else
            {
            // if node has right child, do this
            if(tfn.hasRightChild)
                {
                // pop the right child node from the stack
                right=nodestack.top();
                nodestack.pop();
                // link nodes to new right child node properly before adding to list
                int iright=nodes.size();
                right.index=node.ir=iright;
                if(right.ir!=-1)
                    nodes.at(right.ir).iparent=iright;
                if(right.il!=-1)
                    nodes.at(right.il).iparent=iright;
                // add the right child node to the tree
                nodes.push_back(right);
                }
            // if node has left child, do this
            if(tfn.hasLeftChild)
                {
                // pop the left child node from the stack
                left=nodestack.top();
                nodestack.pop();
                // link nodes to new left child node properly before adding to list
                int ileft=nodes.size();
                left.index=node.il=ileft;
                if(left.ir!=-1)
                    nodes.at(left.ir).iparent=ileft;
                if(left.il!=-1)
                    nodes.at(left.il).iparent=ileft;
                // add the right child node to the tree
                nodes.push_back(left);
                }
            // push the recently read node onto the stack
            nodestack.push(node);
            }
        // if stack size = 1, do this
        if(nodestack.size()==1)
            {
            // check the top of the stack
            HuffmanTreeNode* top = &nodestack.top();
            // set done flag to is-root flag of node at stack-top
            done = top->isroot || cpos>=codebuf.size();
            // if top is root, then do this
            if(top->isroot)
                {
                // pop the root from the stack
                HuffmanTreeNode root=nodestack.top();
                nodestack.pop();
                // link nodes to new root node properly before adding to list
                int iroot=nodes.size();
                root.index=iroot;
                if(root.ir!=-1)
                    nodes.at(root.ir).iparent=iroot;
                if(root.il!=-1)
                    nodes.at(root.il).iparent=iroot;
                // add root to the tree
                nodes.push_back(root);
                roots.push_back(iroot);
                }
            }
        } // end of while(!done) loop block

    // *********************** OLD WAY **************************************
    if (0)
        {
        // clear nodes and root indices
        nodes.clear();
        roots.clear();
        // init node stacks, direction stack, and tree file nodes
        std::stack<TreeFileNode> tfnstack;     // tree file node stack
        TreeFileNode tfn,nextTfn;              // tree file nodes
        std::stack<HuffmanTreeNode> nodestack; // tree node stack
        HuffmanTreeNode node,nextnode;         // tree nodes
        // read first entry in encode tree data stream
        nextTfn=readTreeFileNode();
        nextnode=(HuffmanTreeNode){ 1, nextTfn.data, dir_root, 0, -1, -1, -1 };
        // set first root index to zero
        roots.push_back(0);
        // push that node entry onto stack
        tfnstack.push(nextTfn);
        nodestack.push(nextnode);
        // add an entry to the tree for node entry
        smartAddTreeEntry(nextnode);
        // while stack of entries is not empty, do this
        while(!tfnstack.empty())
            {
            // pop node entry
            tfn = tfnstack.top();
            node = nodestack.top();
            // if node is leaf, then do this
            if(tfn.isleaf)
                {
                // pop top leaf off of stack since we are done with it
                tfnstack.pop();
                nodestack.pop();
                // continue while-loop without checking for children
                continue;
                }
            // elif node has right child, then do this
            else if(tfn.hasRightChild)
                {
                // read next node entry in tree data stream
                nextTfn=readTreeFileNode();
                nextnode=(HuffmanTreeNode){ 1, nextTfn.data, dir_right, 0, node.index, -1, -1 };
                // clear has right child flag from top node since it was just processed
                tfnstack.top().hasRightChild=0;
                // push that node entry onto stack
                tfnstack.push(nextTfn);
                nodestack.push(nextnode);
                // add an entry to the tree for node entry
                smartAddTreeEntry(nextnode);
                }
            // elif node has left child, then do this
            else if(tfn.hasLeftChild)
                {
                // read next node entry in tree data stream
                nextTfn=readTreeFileNode();
                nextnode=(HuffmanTreeNode){ 1, nextTfn.data, dir_left, 0, node.index, -1, -1 };
                // clear has left child flag from top node since it was just processed
                tfnstack.top().hasLeftChild=0;
                // push that node entry onto stack
                tfnstack.push(nextTfn);
                nodestack.push(nextnode);
                // add an entry to the tree for node entry
                smartAddTreeEntry(nextnode);
                }
            tfnstack.top().isleaf=!tfnstack.top().hasRightChild&&!tfnstack.top().hasLeftChild;
            }
        // build a code lookup map
        buildCodeMap();
        } // end of if(0) block

    //return from building encode tree
    return;
    }

/** \brief
 * \in
 * \out
 */
void HuffmanFile::decodeWithTree()
    {
    // init encoded stream position
    cpos=0;
    // clear raw/decoded buffer
    rawbuf.clear();
    // check first 12 bytes for "HUFFMAN_FILE" signature without null terminator
    bool sigmatch=true;
    string targetsig="HUFFMAN_FILE";
    for (int i=0;i<targetsig.length();i++)
        {
        if(targetsig.at(i)!=codebuf.at(cpos++))
            {
            sigmatch=false;
            break;
            }
        }
    // if signature doesn't match, then do this
    if(!sigmatch)
        // return since this is not a Huffman File
        return;
    // read bit count
    union { unsigned long value; unsigned char bytes[4];  } bitcount;
    for (int i=0;i<4;i++)
        bitcount.bytes[i]=codebuf.at(cpos++);

    // decode the tree data
    this->decodeTreeData();

    // while current bit pos < bit count over whole encoded file buffer, do this
    cbitpos=cpos*8;
    // print debugging info for bits encountered
    if(this->dodebug) cout<<endl<<"Bits encountered (in order):"<<endl;

    while(cbitpos<bitcount.value)
        {
        // get current bit-packed encoded data
        unsigned char data=this->getBitPackedEncodedData();
        // add data to decoded data stream
        rawbuf.push_back(data);
        }
    // print debugging info for bits encountered
    if(this->dodebug) cout<<endl<<endl;
    }

/** \brief save the data as either encoded or decoded.
 * \param fname the file to which to save data.
 * \param how the file should be saved.  Valid values are
 *               HuffmanFile::ft_encoded
 *               HuffmanFile::ft_decoded
 * \in the corresponding data stream to the save type should be filled and ready to save.
 * \out the file will contain the contents of the corresponding stream.
 */
void HuffmanFile::savefile(string& fname, HuffmanFile::filetype how)
    {
    vector<unsigned char>*buf;
    if(how==HuffmanFile::ft_encoded)
        buf=&this->codebuf;
    else if(how==HuffmanFile::ft_decoded)
        buf=&this->rawbuf;

    //open the file for reading
    ofstream *fs=new ofstream(&fname[0],ios_base::binary);

    unsigned char c;

    for (int i=0;i<buf->size();i++)
        {
        c = buf->at(i);
        fs->put(c);
        }

    fs->flush();
    delete fs;

    return;
    }

/** \brief adds a code to the encoded data stream using bit packing.
 * \param bitpos the absolute bit position in the encoded data stream.
 * \param ce the code map entry from which to derive the output code.
 * \in the encoded data stream should have all the headers and bits up to this point written.
 * \out the encoded data stream will have the new code added to it.
 */
void HuffmanFile::putEncodedPackedBit(long bitpos,CodeMapEntry ce)
    {
    // init code chunk and mask
    union DwordByByte
        {
        unsigned long all;
        unsigned char bytes[4];
        };
    DwordByByte codechunk,codemask;
    // set up bit position modulo and byte position
    long bytepos=bitpos/8;
    short bitmod8=bitpos%8;
    // set up shift value
    short shiftval=bitmod8;
    // shift code chunk and masks left by shift value
    codechunk.all=ce.bitcode;
    codechunk.all=codechunk.all<<shiftval;
    codemask.all=ce.bitcount==32? (unsigned long)0xffffffff: ((unsigned long)1<<ce.bitcount)-1;
    codemask.all=codemask.all<<shiftval;
    // AND code chunk with its mask
    codechunk.all&=codemask.all;
    // test for very large codes
    if(shiftval+ce.bitcount>=32)
        cout<<"How did you get so large of a code?  This will most certainly produce a loss of data."<<endl<<endl;
    // for every byte in the mask, do this
    for (int i=0;i<4;i++)
        {
        // while byte pos is out of bounds, do this
        while(bytepos>=codebuf.size())
            // add a zero to the encoded data stream
            codebuf.push_back(0);
        // if byte of mask is not zero, do this
        if(codemask.bytes[i]!=0)
            {
            // get current coded byte
            unsigned char b=codebuf.at(bytepos);
            // AND the bits at the current data stream position with the inverted mask byte
            b&=~codemask.bytes[i];
            // OR the bits at the current data stream position with the chunk byte
            b|=codechunk.bytes[i];
            // put current coded byte
            codebuf.at(bytepos)=b;
            }
        // increment byte pos by 1
        bytepos++;
        }
    }

/** \brief
 * \in the Huffman tree should already be built and the header and file size should be already added to the encoded data stream.
 * \out the encoded data stream will contain the full tree data in depth-first post order
 */
void HuffmanFile::encodeTreeData()
    {
    // if a root exists, write the tree in post order, depth first, with a stack:
    if(roots.size()>0)
        {
        // ************************* NEW WAY ******************************
        // pre-emptive returns for zero-length lists
        if(roots.size()==0) return;
        if(nodes.size()==0) return;
        // clear visited flag for each node in tree
        clearVisitedFlags();
        // init stack
        std::stack<int> indexstack;
        // push root
        indexstack.push(roots.at(0));
        // while stack is not empty, do this
        while(!indexstack.empty())
            {
            // look at top node
            int itop=indexstack.top();
            // init node pointers
            HuffmanTreeNode*top = &nodes.at(itop);
            HuffmanTreeNode*left = top->il==-1? 0: &nodes.at(top->il);
            HuffmanTreeNode*right = top->ir==-1? 0: &nodes.at(top->ir);
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
                // convert node to a tree file entry, with flags properly set
                TreeFileNode tfn;
                tfn.allflags=0;
                tfn.hasLeftChild=top->il!=-1;
                tfn.hasRightChild=top->ir!=-1;
                tfn.isleaf=!(tfn.hasRightChild||tfn.hasLeftChild);
                tfn.isroot=top->isroot;
                tfn.data=top->data;
                // add flags of that entry to encoded data stream
                codebuf.push_back(tfn.allflags);
                // if node is leaf, then do this
                if(tfn.isleaf)
                    // add leaf's data to the encoded data stream
                    codebuf.push_back(tfn.data);
                // pop top off of stack
                indexstack.pop();
                }
            } // end of while(!indexstack.empty()) loop block

        }
    }


/** \brief check to see if proper header exists in encoded data stream.
 * \in the encoded data stream should be loaded from file and the current coded position should be 0.
 * \out the current coded position will be the size of the signature string + 4 for the 32-bit file size.
 * \return true if the proper values were read, false otherwise.
 */
bool HuffmanFile::checkEncodedHeader()
    {
    //set up target header and its length
    string header="HUFFMAN_FILE";
    int hlen=header.length();
    // in order to check for header string, for each letter in target header, do this
    for (int i=0;i<hlen;i++)
        {
        // if the current coded position > size of coded buffer, then do this
        if(cpos>=codebuf.size())
            // return false
            return false;
        // if the current letter doesn't match what is in the encoded data stream at that position, then do this
        if( header[i]!= this->codebuf.at(cpos))
            // return false
            return false;
        // increment coded position
        ++cpos;
        }
    // the 32-bit size should equal to the size of the coded stream, but skip for now
    cpos+=4;
    // return true if execution successfully reaches this point
    return true;
    }

/** \brief reads a single tree file node entry from coded buffer and returns it.
 * \in coded buffer should have the tree data, and coded buffer position should be at the next tree to be read.
 * \out coded position advances.
 * \return the tree file node read.
 */
TreeFileNode& HuffmanFile::readTreeFileNode()
    {
    static TreeFileNode tfn;
    tfn.allflags=codebuf.at(cpos++);
    if(tfn.isleaf)
        tfn.data=codebuf.at(cpos++);
    return tfn;
    }

/** \brief adds a node to the tree in a smart manner.
 * \param node the node to add.
 * \in the tree contain the node's parent node and the node itself should be ready to use.
 * \out the tree will have the node added to its parent in proper direction relative to its parent.
 */
void HuffmanFile::smartAddTreeEntry(HuffmanTreeNode& node)
    {
    // set node's index to size of node list before adding this node
    node.index=nodes.size();
    // if node has a parent, do this
    if(node.iparent!=-1)
        {
        // init parent reference
        HuffmanTreeNode &pnode=nodes.at(node.iparent);         // parent tree node
        //if node is left of parent, do this
        if(node.dirFromParent==dir_left)
            // set parent's left node index to index of this node
            pnode.il=node.index;
        //elif node is right of parent, do this
        else if(node.dirFromParent==dir_right)
            // set parent's right node index to index of this node
            pnode.ir=node.index;
        }
    // now add node to node list
    nodes.push_back(node);
    }

/** \brief read an encoded tree from the coded data stream.
 * \in the coded data stream should have enough bits to construct a coherent Huffman branch code at current bit position.
 * \out the current bit position of the whole file will be incremented by the number of bits read.
 * \return the extracted data byte.
 */
unsigned char HuffmanFile::getBitPackedEncodedData()
    {
    // initialize done value
    bool done = false;
    // init tree node to root node
    HuffmanTreeNode node=nodes.at(roots.at(0));
    // print debugging info for bits encountered
    if(this->dodebug) cout<<" ";
    //while not done
    while(!done)
        {
        // read bit
        unsigned char bit=this->getCodeBit();
        // print debugging info for bits encountered
        if(this->dodebug) cout<<(short)bit;
        // if bit is set, do this on the right side
        if(bit==1)
            {
            // if current tree node has a right child, do this
            if(node.ir!=-1)
                {
                // set node to left child node of current
                node=nodes.at(node.ir);
                // set done to true if node is a leaf
                done = node.il==-1 && node.ir==-1;
                }
            // else, do this
            else
                {
                // break since we didn't find our node
                done=true;
                if(this->dodebug)
                    cout<<endl<<"Premature end of code branching (left.)"<<endl;
                }
            }
        // else, do this on the left side
        else
            {
            // if current tree node has a left child, do this
            if(node.il!=-1)
                {
                // set node to left child node of current
                node=nodes.at(node.il);
                // set done to true if node is a leaf
                done = node.il==-1 && node.ir==-1;
                }
            // else, do this
            else
                {
                // break since we didn't find our node
                done=true;
                if(this->dodebug)
                    cout<<endl<<"Premature end of code branching (left.)"<<endl;
                }
            }
        }
    // return the current node's data
    return node.data;
    }

/** \brief clear the visited flags for all nodes.
 * \in there should be nodes present.
 * \out the visited flag for each node will be set to false.
 */
void HuffmanFile::clearVisitedFlags()
    {
    for (int i=0;i<nodes.size();i++)
        nodes.at(i).visited=false;
    }

/** \brief set root flags for any nodes in root list or any with no iparent value
 * \in there should be nodes present.
 * \out any node that meets the root requirements will have their is-root flag set
 */
void HuffmanFile::autoSetRootFlags()
    {
    // set according to parent index
    for (int i=0;i<nodes.size();i++)
        {
        HuffmanTreeNode*node=&nodes.at(i);
        node->isroot=node->iparent==-1;
        }
    // or set according to existence in root list
    for (int i=0;i<roots.size();i++)
        {
        int iroot=roots.at(i);
        HuffmanTreeNode*node=&nodes.at(iroot);
        node->isroot=true;
        }
    }


/** \brief get an encoded bit
 * \in the coded data stream should have enough bits to address current bit position
 * \out current bit and byte positions change by one bit
 * \return the v
 */
unsigned char HuffmanFile::getCodeBit()
    {
    unsigned char bit;
    // read bit at current bit position
    cpos=cbitpos/8;
    int shiftval=cbitpos%8;
    unsigned char mask = (unsigned char)1<<shiftval;
    bit=codebuf.at(cpos)&mask;
    bit =  bit >> shiftval;
    // increment bit position
    ++cbitpos;
    // update byte position
    cpos=cbitpos/8;

    return bit;
    }
