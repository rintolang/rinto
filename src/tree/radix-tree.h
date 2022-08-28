#ifndef RADIX_TREE_H
#define RADIX_TREE_H

#include <cstdarg>
#include <vector>

#include "node.h"
#include "../lang.h"

struct parseDef {
        bool wantToken;

        token     tok  = ILLEGAL;
        tokenType type = t_ILLEGAL;

        bool operator==(const parseDef& rhs);

};
typedef struct parseDef parseDef;

class RadixNode : public Node
{
public:
        RadixNode(){};

        std::vector<parseDef> data;

        void setParent(RadixNode* parent);
        bool addChild(RadixNode* child);

        bool operator==(RadixNode& rhs);
        bool operator!=(RadixNode& rhs);
};

#endif /* RADIX_TREE_H */
