#pragma once

#include "Direction.h"

struct  HuffmanTreeNode
    {
    long weight; // the weight for this tree node
    unsigned char data;    // the byte data if this is a leaf
    Direction dirFromParent;  //direction this node is from its parent
    int index;           // the index of this node
    int iparent;         // the index of this node's parent, -1 = none
    int il;              // left branch index, -1 = none
    int ir;              // right branch index, -1 = none
    bool visited;        // has this been visited yet?  only used for certain algorithms like post-order construction.
    bool isroot;         // flag that can be used to check root status without checking iparent
    };

