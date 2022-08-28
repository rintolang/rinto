#include "radix-tree.h"

bool parseDef::operator==(const parseDef& rhs)
{
        if (this->wantToken != rhs.wantToken)
                return false;

        if (this->tok != rhs.tok)
                return false;

        if (this->type != rhs.type)
                return false;

        return true;
}

unsigned int _countVectorEq(std::vector<parseDef>& x, std::vector<parseDef>& y)
{
        unsigned int count = 0;
        auto xit = x.begin();
        auto yit = y.begin();

        while (xit != x.end() && yit != y.end()) {
                if (*xit == *yit)
                        count++;

                ++xit;
                ++yit;
        }

        return count;
}

void RadixNode::setParent(RadixNode* parent)
{
        if (parent == nullptr || this->Parent == parent)
                return;

        parent->addChild(this);
}

bool RadixNode::addChild(RadixNode* child)
{
        if (child == nullptr || *child == *this)
                return false;

        if (child->data.size() == 0 || child->data.size() > 3)
                return false;

        unsigned int same = _countVectorEq(data, child->data);
        if (!same)
                return false;

        for (int i = 0; i < same; i++)
                child->data.erase(child->data.begin());

        RadixNode* sChild = (RadixNode*)firstChild();
        while (sChild != nullptr) {
                // child belongs lower down the tree
                if (_countVectorEq(sChild->data, child->data))
                        return sChild->addChild(child);

                sChild = (RadixNode*)sChild->nextSibling();
        }

        // No common children found, split parent into new parent
        RadixNode* newParent = new RadixNode;
        for (int i = 0; i < same; i++) {
                newParent->data.push_back(*data.begin());
                data.erase(data.begin());
        }

        // Node base class doesn't doesn't care about radix.
        newParent->Node::setParent(this->Parent);
        newParent->Node::addChild(this);
        newParent->Node::addChild(child);

        return true;
}

bool RadixNode::operator==(RadixNode& rhs)
{
        if (this->data.size() != rhs.data.size())
                return false;

        if (_countVectorEq(this->data, rhs.data) != this->data.size())
                return false;

        return true;
}

bool RadixNode::operator!=(RadixNode& rhs) {
        return !(*this == rhs);
}
