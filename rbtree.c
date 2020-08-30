/*
#include <stdio.h>
//#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
*/

#include <linux/string.h>
#include <linux/time.h>
#include <linux/kernel.h>


#define BLACK 'b'
#define RED 'r'

struct node {
    u_char key[48];
    struct timespec spec;
    u_char color;
    struct node *left, *right, *parent;
};

uint8_t g = 0;

void insert_repair_tree(struct node *n);

void delete_case1(struct node *n);

void delete_case2(struct node *n);

void delete_case3(struct node *n);

void delete_case4(struct node *n);

void delete_case5(struct node *n);

struct node *LEAF;

struct node *parent(struct node *n) {
    return n->parent; // NULL for root node
}

struct node *grandparent(struct node *n) {
    struct node *p = parent(n);
    if (p == NULL)
        return NULL; // No parent means no grandparent
    return parent(p); // NULL if parent is root
}

struct node *sibling(struct node *n) {
    struct node *p = parent(n);
    if (p == NULL)
        return NULL; // No parent means no sibling
    if (n == p->left)
        return p->right;
    else
        return p->left;
}

struct node *uncle(struct node *n) {
    struct node *p = parent(n);
    struct node *g = grandparent(n);
    if (g == NULL)
        return NULL; // No grandparent means no uncle
    return sibling(p);
}

void rotate_left(struct node *n) {
    struct node *nnew = n->right;
    struct node *p = parent(n);
    if (nnew != NULL) { // since the leaves of a red-black tree are empty, they cannot become internal nodes
        n->right = nnew->left;
        nnew->left = n;
        n->parent = nnew;
        // handle other child/parent pointers
        if (n->right != NULL)
            n->right->parent = n;
        if (p != NULL) // initially n could be the root
        {
            if (n == p->left)
                p->left = nnew;
            else if (n == p->right) // if (...) is excessive
                p->right = nnew;
        }
        nnew->parent = p;
    }
    else {
        printk("nnew = NULL");
    }
}

void rotate_right(struct node *n) {
    struct node *nnew = n->left;
    struct node *p = parent(n);
    if (nnew != NULL) { // since the leaves of a red-black tree are empty, they cannot become internal nodes
        n->left = nnew->right;
        nnew->right = n;
        n->parent = nnew;
        // handle other child/parent pointers
        if (n->left != NULL)
            n->left->parent = n;
        if (p != NULL) // initially n could be the root
        {
            if (n == p->left)
                p->left = nnew;
            else if (n == p->right) // if (...) is excessive
                p->right = nnew;
        }
        nnew->parent = p;
    }
    else {
        printk("nnew == NULL");
    }
}

void insert_recurse(struct node *root, struct node *n) {
    // recursively descend the tree until a leaf is found

    if (root != NULL && (strcmp(n->key, root->key) < 0 )) {
        if (root->left != LEAF) {
            insert_recurse(root->left, n);
            return;
        } else
            root->left = n;
    } else if (root != NULL) {
        if (root->right != LEAF) {
            insert_recurse(root->right, n);
            return;
        } else
            root->right = n;
    }

    // insert new node n
    n->parent = root;
    n->left = LEAF;
    n->right = LEAF;
    n->color = RED;
}


void insert_case1(struct node *n) {
    if (parent(n) == NULL)
        n->color = BLACK;
}

void insert_case2(struct node *n) {
    return; /* Do nothing since tree is still valid */
}

void insert_case3(struct node *n) {
    parent(n)->color = BLACK;
    uncle(n)->color = BLACK;
    grandparent(n)->color = RED;
    insert_repair_tree(grandparent(n));
}

void insert_case4step2(struct node *n) {
    struct node *p = parent(n);
    struct node *g = grandparent(n);

    if (n == p->left)
        rotate_right(g);
    else
        rotate_left(g);
    p->color = BLACK;
    g->color = RED;
}


void insert_case4(struct node *n) {
    struct node *p = parent(n);
    struct node *g = grandparent(n);

    if (n == g->left->right) {
        rotate_left(p);
        n = n->left;
    } else if (n == g->right->left) {
        rotate_right(p);
        n = n->right;
    }

    insert_case4step2(n);
}


void insert_repair_tree(struct node *n) {
    if (parent(n) == NULL) {
        insert_case1(n);
    } else if (parent(n)->color == BLACK) {
        insert_case2(n);
    } else if (uncle(n)->color == RED) {
        insert_case3(n);
    } else {
        insert_case4(n);
    }
}

struct node *insert(struct node *root, struct node *n) {
    // insert new node into the current tree    
    insert_recurse(root, n);
    
    // repair the tree in case any of the red-black properties have been violated
    insert_repair_tree(n);

    // find the new root to return
    root = n;
    while (parent(root) != NULL)
        root = parent(root);
    return root;
}


void replace_node(struct node *n, struct node *child) {
    if (n->parent) {
        child->parent = n->parent;
        if (n == n->parent->left)
            n->parent->left = child;
        else
            n->parent->right = child;
    }
        /*
        if (child == n->left) {
            root = child;
            n->right->parent = child;
            child->right = n->right;
        }
        else {
            root = child;
            n->left->parent = child;
            child->left = n->left;
        }
        /*
        if (child == n->left) {
            n->right->parent = child;
            child->right = n->right;
            child->parent = NULL;
        }
        else {
            n->left->parent = child;
            child->left = n->left;
            child->parent = NULL;
        }
        */
}

int is_leaf(struct node *n) {
    if (n->right == NULL && n->left == NULL)
        return 1;
    else return 0;
}

void delete_one_child(struct node *n) {
    /*
     * Precondition: n has at most one non-leaf child.
     */
    if (g < 3) 
    {
        struct node *child = is_leaf(n->right) ? n->left : n->right;

        replace_node(n, child);
        if (n->color == BLACK) {
            if (child->color == RED)
                child->color = BLACK;
            else
                delete_case1(child);
        }
        printk("deleted!");
        kfree(n);
        g++;
    }
}

void delete_case1(struct node *n) {
    if (n->parent != NULL)
        delete_case2(n);
}

void delete_case2(struct node *n) {
    struct node *s = sibling(n);

    if (s->color == RED) {
        n->parent->color = RED;
        s->color = BLACK;
        if (n == n->parent->left)
            rotate_left(n->parent);
        else
            rotate_right(n->parent);
    }
    delete_case3(n);
}

void delete_case3(struct node *n) {
    struct node *s = sibling(n);

    if ((n->parent->color == BLACK) &&
        (s->color == BLACK) &&
        (s->left->color == BLACK) &&
        (s->right->color == BLACK)) {
        s->color = RED;
        delete_case1(n->parent);
    } else
        delete_case4(n);
}

void delete_case4(struct node *n) {
    struct node *s = sibling(n);

    if ((n->parent->color == RED) &&
        (s->color == BLACK) &&
        (s->left->color == BLACK) &&
        (s->right->color == BLACK)) {
        s->color = RED;
        n->parent->color = BLACK;
    } else
        delete_case5(n);
}

void delete_case6(struct node *n) {
    struct node *s = sibling(n);

    s->color = n->parent->color;
    n->parent->color = BLACK;

    if (n == n->parent->left) {
        s->right->color = BLACK;
        rotate_left(n->parent);
    } else {
        s->left->color = BLACK;
        rotate_right(n->parent);
    }
}

void delete_case5(struct node *n) {
    struct node *s = sibling(n);

    if (s->color == BLACK) { /* this if statement is trivial,
due to case 2 (even though case 2 changed the sibling to a sibling's child,
the sibling's child can't be red, since no red parent can have a red child). */
/* the following statements just force the red to be on the left of the left of the parent,
   or right of the right, so case six will rotate correctly. */
        if ((n == n->parent->left) &&
            (s->right->color == BLACK) &&
            (s->left->color == RED)) { /* this last test is trivial too due to cases 2-4. */
            s->color = RED;
            s->left->color = BLACK;
            rotate_right(s);
        } else if ((n == n->parent->right) &&
                   (s->left->color == BLACK) &&
                   (s->right->color == RED)) {/* this last test is trivial too due to cases 2-4. */
            s->color = RED;
            s->right->color = BLACK;
            rotate_left(s);
        }
    }
    delete_case6(n);
}

struct node *search(struct node *temp, char key[48]) {
    int diff;
    if (temp == NULL) return 0;
    while (!is_leaf(temp)) {
        diff = strcmp(key, temp->key);
        if (diff > 0) {
            temp = temp->right;
        } else if (diff < 0) {
            temp = temp->left;
        } else {
            //printk("Search Element Found - %s!!\n", temp->key);
            return temp;
        }
    }
    //printk("Given Data Not Found in the tree!!\n");
    return 0;
}

// Returns the number of black nodes in a subtree of the given node
// or -1 if it is not a red black tree.
int computeBlackHeight(struct node* currNode) {
    // For an empty subtree the answer is obvious
    if (currNode == NULL)
        return 0;
    // Computes the height for the left and right child recursively
    int leftHeight = computeBlackHeight(currNode->left);
    int rightHeight = computeBlackHeight(currNode->right);
    int add = currNode->color == BLACK ? 1 : 0;
    // The current subtree is not a red black tree if and only if
    // one or more of current node's children is a root of an invalid tree
    // or they contain different number of black nodes on a path to a null node.
    if (leftHeight == -1 || rightHeight == -1 || leftHeight != rightHeight)
        return -1;
    else
        return leftHeight + add;
}
