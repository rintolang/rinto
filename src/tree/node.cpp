#include "node.h"

Node::~Node()
{
        Node* nextChild = FirstChild;
        while (nextChild != nullptr) {
                Node* temp = nextChild->nextSibling();
                delete nextChild;
                nextChild = temp;
        }
}

bool Node::hasParent() {
        return (Parent != nullptr);
}

bool Node::hasSiblings()
{
        if (PrevSibling != nullptr || NextSibling != nullptr)
                return true;

        return false;
}

bool Node::hasChildren()
{
        if (FirstChild != nullptr && LastChild != nullptr)
                return true;

        return false;
}

void Node::setParent(Node* parent)
{
        if (parent == nullptr || Parent == parent)
                return;

        Node* prev = PrevSibling;
        Node* next = NextSibling;

        if (prev != nullptr)
                prev->setSiblings(prev->prevSibling(), next);

        if (next != nullptr)
                next->setSiblings(prev, next->nextSibling());

        /* Node's siblings will be configured by addChild().*/
        parent->addChild(this);
}

bool Node::addChild(Node* child)
{
        if (child == nullptr)
                return false;

        /* Adding same-value nodes is permitted, but addresses
         * shouldn't be the same.
         */
        if (child == this)
                return false;

        if (LastChild != nullptr)
                LastChild->setSiblings(LastChild->prevSibling(), child);

        child->setSiblings(LastChild, nullptr);
        LastChild = child;

        if (FirstChild == nullptr)
                FirstChild = child;

        child->Parent = this;
        return true;
}

void Node::getChildren(std::vector<Node*>& children)
{
        Node* nextChild = FirstChild;
        while (nextChild != nullptr) {
                children.push_back(nextChild);
                nextChild = nextChild->nextSibling();
        }
}

void Node::setWalkCallback(walkFn fn) {
        WalkCallback = fn;
}

void Node::walk()
{
        if (WalkCallback == nullptr)
                return;

        walkStatus status;
        if (hasChildren())
                status = WalkCallback(this, true);
        else
                status = WalkCallback(this, false);

        if (status == ABORT || status == SKIP_CHILDREN)
                return;

        Node* nextChild = FirstChild;
        while (nextChild != nullptr) {
                nextChild->setWalkCallback(WalkCallback);
                nextChild->walk();
                nextChild = nextChild->nextSibling();
        }
}

Node* Node::nextSibling() {
        return NextSibling;
}

Node* Node::prevSibling() {
        return PrevSibling;
}

Node* Node::firstChild() {
        return FirstChild;
}

Node* Node::lastChild() {
        return LastChild;
}

void Node::setSiblings(Node* prev, Node* next)
{
        NextSibling = next;
        PrevSibling = prev;
}

bool Node::operator==(const Node& rhs)
{
        if (this->Parent != rhs.Parent)
                return false;

        if (this->PrevSibling != rhs.PrevSibling)
                return false;

        if (this->NextSibling != rhs.NextSibling)
                return false;

        if (this->FirstChild != rhs.FirstChild)
                return false;

        if (this->LastChild != rhs.LastChild)
                return false;

        return true;
}

bool Node::operator!=(const Node& rhs) {
        return !(*this == rhs);
}
