#pragma once

struct TreeFileNode
    {
    union
        {
        unsigned char allflags;
        struct
            {
            unsigned char
                hasLeftChild:1,
                hasRightChild:1,
                isleaf:1,
                isroot:1,
                flag4:1,
                flag5:1,
                flag6:1,
                flag7:1;
            };
        }; // end of flag union
    unsigned char data;
    };



