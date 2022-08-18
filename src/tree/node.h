#ifndef NODE_H
#define NODE_H

#include <vector>
class Node;

/* Simple interface for controlling walkFn. */
typedef enum {
        CONTINUE = 0,
        ABORT,
        SKIP_CHILDREN
} walkStatus;

/* TODO */
typedef enum {
        NONE = 0
} nodeType;

/*
 * walkStatus defines a callback fn tupe for preorder
 * tree traversal. Node is the current node visited and
 * entering signals true whenever an entered node has
 * children.
 */
typedef walkStatus (*walkFn) (Node* node, bool entering);

class Node
{
private:
        Node* Parent      = nullptr;
        Node* PrevSibling = nullptr;
        Node* NextSibling = nullptr;
        Node* FirstChild  = nullptr;
        Node* LastChild   = nullptr;

        nodeType Type         = NONE;
        walkFn   WalkCallback = nullptr;
public:
        Node(){};
        ~Node();

        nodeType getType();

        bool hasParent();
        bool hasSiblings();
        bool hasChildren();

        Node* nextSibling();
        Node* prevSibling();
        Node* firstChild();
        Node* lastChild();

        /*
         * Sets parent as the node's current parent and
         * updates the node's siblings.
         */
        void setParent(Node* parent);

        /*
         * Sets the node's prevSibling and nextSibling
         * pointers without updating its old siblings.
         */
        void setSiblings(Node* prev, Node* next);

        /*
         * Adds the child as the node's last child,
         * updating the previous lastChild's and
         * the child's siblings appropriately.
         */
        bool addChild(Node* child);

        /*
         * Appends the node's children (if any) to the
         * specified vector.
         */
        void getChildren(std::vector<Node*>& children);

        /*
         * fn is called each time the walk function enters
         * a new node. walk() returns if WalkCallback is
         * nullptr.
         */
        void setWalkCallback(walkFn fn);

        /*
         * Performs preorder tree traversal starting on
         * the current node and then its children. The
         * node's callback function is called each time.
         */
        void walk();
};

#endif /* NODE_H */
