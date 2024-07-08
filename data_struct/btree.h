#include <stdbool.h>
#include <stddef.h>

#define CHILDREN_MAX    4
#define ITEM_MAX        (CHILDREN_MAX - 1)
#define ITEM_MIN        ((CHILDREN_MAX/2) - 1)
#define MEDIAN          (CHILDREN_MAX/2 - 1)


struct Btree {
  int (*compare)(const void *l, const void *r);
  int (*key_print)(const void *key);
  size_t item_size;
  struct btree_node *root;

};

struct btree_node {
    bool leaf;
    size_t item_num;
    void *item[ITEM_MAX];
    struct btree_node *children[ITEM_MAX + 1];
};


int btree_new(struct Btree *B ,size_t item_size, int (*compare)(const void *l, const void *r), int (*key_print)(const void *key));
int btree_drop(struct btree_node *node);
struct btree_node *btree_root_split(struct btree_node *node);
struct btree_node *btree_split(struct Btree *B ,struct btree_node *top_node, struct btree_node *children_node);
void btree_insert(struct Btree *B, void* data);
struct btree_node *btree_root_merge(struct btree_node *node);
struct btree_node *btree_merge(struct btree_node *top_node, size_t children_num);
void **find_predcessor(struct btree_node *node);
void **find_successor(struct btree_node *node);
int btree_delete(struct Btree *B, void* data);
void btree_print_node(struct btree_node *node, int (*key_print) (const void *key), size_t depth);
void btree_print(struct Btree *B);
void const *btree_search(struct Btree *B, void *data);

