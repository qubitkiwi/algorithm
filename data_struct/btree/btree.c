#include "btree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int btree_new(struct Btree *B ,size_t item_size, int (*compare)(const void *l, const void *r), int (*key_print)(const void *key))
{
    if (B == NULL) {
        perror("no struct Btree\n");
        return -1;
    }

    if (item_size == 0) {
        perror("no struct size\n");
        return -1;
    }

    if (compare == NULL) {
        perror("no compare fn\n");
        return -1;
    }

    if (key_print == NULL) {
        perror("no key_print fn\n");
        return -1;
    }


    B->compare = compare;
    B->key_print = key_print;
    B->item_size = item_size;

    B->root = malloc( sizeof(struct btree_node) );
    memset(B->root, 0, sizeof(struct btree_node));
    B->root->leaf = true;

    return 1;
}

int btree_drop(struct btree_node *node)
{
    size_t i;
    for (i=0; i<node->item_num; i++) {
        free(node->item[i]);
    }
    if (!node->leaf) {
        for (i=0; i<=node->item_num; i++) {
            btree_drop(node->children[i]);
        }    
    }

    free(node);
    return 0;
}


struct btree_node *btree_root_split(struct btree_node *node)
{
    struct btree_node *top_node, *left_node, *right_node;
    left_node = node;

    top_node = malloc( sizeof(struct btree_node) );
    memset(top_node, 0, sizeof(struct btree_node));
    right_node = malloc( sizeof(struct btree_node) );
    memset(right_node, 0, sizeof(struct btree_node));

    // copy top_node
    top_node->children[0] = left_node;
    top_node->children[1] = right_node;
    top_node->item[0] = left_node->item[MEDIAN];
    top_node->item_num++;

    // copy right node
    size_t i;
    for (i=0; i < ITEM_MAX/2; i++) {
        right_node->item[i] = left_node->item[i + MEDIAN + 1];
    }
    if (!left_node->leaf) {
        for (i=0; i <= ITEM_MAX/2; i++) {
            right_node->children[i] = left_node->children[i + MEDIAN + 1];
        }
    }
    right_node->item_num = ITEM_MAX/2;
    right_node->leaf = left_node->leaf;
        
    // left node
    left_node->item_num = MEDIAN;

    return top_node;
}


struct btree_node *btree_split(struct Btree *B ,struct btree_node *top_node, struct btree_node *children_node)
{
    struct btree_node *left_node, *right_node;
    left_node = children_node;

    right_node = malloc( sizeof(struct btree_node) );
    memset(right_node, 0, sizeof(struct btree_node));
    
    // top node
    size_t index = top_node->item_num;
    while (index > 0 && B->compare(top_node->item[index - 1], children_node->item[MEDIAN]) > 0) {
        top_node->item[index] = top_node->item[index - 1];
        index--;         
    }
    for (size_t i=top_node->item_num; i > index; i--) {
        top_node->children[i + 1] = top_node->children[i];
    }

    top_node->item[index] = children_node->item[MEDIAN];
    top_node->children[index] = left_node;
    top_node->children[index + 1] = right_node;
    top_node->item_num++;

    // copy right node
    size_t i;
    for (i=0; i < ITEM_MAX/2; i++) {
        right_node->item[i] = left_node->item[i + MEDIAN + 1];
    }
    if (!left_node->leaf) {
        for (i=0; i <= ITEM_MAX/2; i++) {
            right_node->children[i] = left_node->children[i + MEDIAN + 1];
        }
    }
    right_node->item_num = ITEM_MAX/2;
    right_node->leaf = left_node->leaf;
        
    // left node
    left_node->item_num = MEDIAN;

    return top_node;
}



void btree_insert(struct Btree *B, void* data)
{
    struct btree_node *node = B->root;
    int (*compare_fn)(const void*, const void*) = B->compare;

    // root split
    if (node->item_num == ITEM_MAX) {

        B->root = btree_root_split(node);
        node = B->root;

        if (compare_fn(node->item[0] , data) < 0) {
            node = node->children[1];
        } else {
            node = node->children[0];
        }
    }

    
    while (1) {
        if (node->leaf) {
            // leaf insert
            size_t index = node->item_num;
            while (index > 0 && compare_fn(node->item[index - 1], data) > 0) {
                node->item[index] = node->item[index - 1];
                index--;         
            }

            void *tmp = malloc(B->item_size);
            memcpy(tmp, data, B->item_size);
            node->item[index] = tmp;
            node->item_num++;
            return ;
        } else {
            // search leaf
            struct btree_node *children_node;
            // search children
            size_t index = node->item_num;
            while (index > 0 && compare_fn(node->item[index - 1], data) > 0) {
                index--;
            }
            children_node = node->children[index];

            if (children_node->item_num == ITEM_MAX) {
                node = btree_split(B, node, children_node);
            } else {
                node = children_node;
            }               
        }
    }
}


struct btree_node *btree_root_merge(struct btree_node *node)
{
    struct btree_node *left_node, *right_node, *top_node;

    top_node = node;

    left_node = top_node->children[0];
    right_node = top_node->children[1];

    // top node copy
    left_node->item[left_node->item_num] = top_node->item[0];
    left_node->item_num++;
    free(top_node);

    // right node copy
    for (size_t idx=0; idx < right_node->item_num; idx++) {
        left_node->item[left_node->item_num + idx] = right_node->item[idx];
    }
    if (!right_node->leaf) {
        for (size_t idx=0; idx <= right_node->item_num; idx++) {
            left_node->children[left_node->item_num + idx] = right_node->children[idx];
        }
    }
    left_node->item_num += right_node->item_num;
    free(right_node);


    return left_node;
}


struct btree_node *btree_merge(struct btree_node *top_node, size_t children_num)
{
    struct btree_node *left_node, *target_node, *right_node;

    target_node = top_node->children[children_num];

    //merge left
    if (children_num > 0) {
        left_node = top_node->children[children_num - 1];
        if (left_node->item_num > ITEM_MIN) {
            // shift item
            size_t idx;
            for (idx=target_node->item_num; idx>0; idx--) {
                target_node->item[idx] = target_node->item[idx - 1];
            }
            target_node->item_num++;
            target_node->item[0] = top_node->item[children_num - 1];

            top_node->item[children_num - 1] = left_node->item[left_node->item_num - 1];
            left_node->item_num--;

            return target_node;
        } else {
            size_t idx;
            // copy top_node
            left_node->item[left_node->item_num] = top_node->item[children_num - 1];
            left_node->item_num++;
            // copy target node
            for (idx = 0; idx < target_node->item_num; idx++) {
                left_node->item[left_node->item_num + idx] = target_node->item[idx];
            }
            // copy target node children
            if (!target_node->leaf) {
                for (idx=0; idx < target_node->item_num; idx++) {
                    left_node->children[left_node->item_num + idx] = target_node->children[idx];
                }
            }
            left_node->item_num += target_node->item_num;
            free(target_node);

            // shift top node
            for (idx = children_num - 1; idx < top_node->item_num; idx++) {
                top_node->item[idx] = top_node->item[idx + 1];
                top_node->children[idx + 1] = top_node->children[idx + 2];
            }
            top_node->item_num--;

            return left_node;
        }

    }

    //merge right
    if (children_num < top_node->item_num) {
        right_node = top_node->children[children_num + 1];
        if (right_node->item_num > ITEM_MIN) {
            
            target_node->item[target_node->item_num] = top_node->item[children_num];
            target_node->item_num++;

            top_node->item[children_num] = right_node->item[0];
            right_node->item_num--;

            for (size_t idx=0; idx<right_node->item_num; idx++) {
                right_node->item[idx] = right_node->item[idx + 1];
            }

            return target_node;
        } else {
            size_t idx;
            // copy top_node
            target_node->item[target_node->item_num] = top_node->item[children_num];
            target_node->item_num++;
            // copy right node
            for (idx = 0; idx < right_node->item_num; idx++) {
                target_node->item[target_node->item_num + idx] = right_node->item[idx];
            }
            // copy right node children
            if (!right_node->leaf) {
                for (idx=0; idx < right_node->item_num; idx++) {
                    target_node->children[target_node->item_num + idx] = right_node->children[idx];
                }
            }
            target_node->item_num += right_node->item_num;
            free(right_node);

            // shift top node
            for (idx = children_num; idx < top_node->item_num; idx++) {
                top_node->item[idx] = top_node->item[idx + 1];
                top_node->children[idx + 1] = top_node->children[idx + 2];
            }
            top_node->item_num--;

            return target_node;
        }
    }



    return top_node;
}


// smaller
void **find_predcessor(struct btree_node *node)
{
    while (1) {
        if (node->leaf) {
            return &node->item[node->item_num - 1];              
        } else {
            node = node->children[node->item_num];
        }
    }
}
 
// bigger
void **find_successor(struct btree_node *node)
{
    while (1) {
        if (node->leaf) {
            return &node->item[0];
        } else {
            node = node->children[0];
        }
    }
}

int btree_delete(struct Btree *B, void* data)
{
    void **predcessor = NULL;
    struct btree_node *node = B->root;
    if (node->item_num <= 1 && !node->leaf && node->children[0]->item_num <= ITEM_MIN && node->children[1]->item_num <= ITEM_MIN) {
        B->root = btree_root_merge(node);
        node = B->root;
    }

    while (1) {
        if (node->leaf) {
            // just deletea
            size_t idx;
            bool find = false;
            for (idx = 0; idx < node->item_num; idx++) {
                if (B->compare(node->item[idx], data) == 0) {
                    free(node->item[idx]);
                    find = true;
                    idx++;
                    break;
                }
            }
            if (!find) {
                return -1; // no data
            }

            for (; idx < node->item_num; idx++) {
                node->item[idx - 1] = node->item[idx];
            }
            node->item_num--;

            return 0; // sucess
        } else {
            // this node is not poor

            // search item
            size_t idx;
            for (idx = 0; idx < node->item_num; idx++) {
                if (B->compare(node->item[idx], data) >= 0) {
                    break;
                }
            }

            if ((predcessor == NULL) && (idx < node->item_num) && (B->compare(node->item[idx], data) == 0)) {
                //change leaf item
                predcessor = find_predcessor(node->children[idx]);
                B->key_print(*predcessor);
                void *tmp = node->item[idx];
                node->item[idx] = *predcessor;
                *predcessor = tmp;
            }
            
            if (node->children[idx]->item_num <= ITEM_MIN) {
                node = btree_merge(node, idx);
            } else {
                node = node->children[idx];
            }
        }
    }
}

void btree_print_node(struct btree_node *node, int (*key_print) (const void *key), size_t depth)
{
    size_t i;
    for (i=0; i<depth; i++) {
        printf("  ");
    }
    printf("|-");

    for (i=0; i < node->item_num; i++) {
        key_print(node->item[i]);
    }
    printf("\n");

    if (!node->leaf) {
        for (i=0; i <= node->item_num; i++) {
            btree_print_node(node->children[i], key_print, depth + 1);
        }
    }
}

void btree_print(struct Btree *B)
{
    btree_print_node(B->root, B->key_print, 0);
}



void const *btree_search(struct Btree *B, void *data)
{
    struct btree_node *node = B->root;

    while (1) {
        if (node->leaf) {
            for (size_t idx=0; idx < node->item_num; idx++) {
                if (B->compare(node->item[idx], data) == 0) {
                    return node->item[idx];
                }
            }
            return NULL;
        } else {
            size_t idx;
            for (idx = 0; idx < node->item_num; idx++) {
                if (B->compare(node->item[idx], data) >= 0) {
                    break;
                }
            }

            if ((idx < node->item_num) && (B->compare(node->item[idx], data) == 0)) {
                return node->item[idx];
            }
            node = node->children[idx];
        }
    }
}