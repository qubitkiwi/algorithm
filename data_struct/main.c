#include <stdio.h>
#include "btree.h"

struct key { 
    int data;
    char name[10];
};

int key_compare(const void *f, const void *s)
{
    const struct key *fd = f;
    const struct key *sd = s;
    
    if (fd->data > sd->data) {
        return 1;
    } else if (fd->data < sd->data) {
        return -1;
    } else {
        return 0;
    }
}

int key_print(const void *key)
{
    const struct key *item = key;
    return printf("%d ", item->data);
}

int main(void)
{

    struct key const *search_data;
    struct key tmp_key;

    struct Btree test;
    btree_new(&test, sizeof(struct key), key_compare, key_print);

    btree_insert(&test, &(struct key){ .data=33, .name="test" });
    btree_insert(&test, &(struct key){ .data=18, .name="aaaaa" });
    btree_insert(&test, &(struct key){ .data=20 });
    btree_insert(&test, &(struct key){ .data=17 });
    btree_insert(&test, &(struct key){ .data=16 });
    btree_insert(&test, &(struct key){ .data=44 });
    btree_insert(&test, &(struct key){ .data=55 });
    btree_insert(&test, &(struct key){ .data=66 });


    search_data = btree_search(&test, &(struct key){ .data=33 });
    if (search_data != NULL) {
        printf("search: %d %s\n", search_data->data, search_data->name);
    }

    search_data = btree_search(&test, &(struct key){ .data=18 });
    if (search_data != NULL) {
        printf("search: %d %s\n", search_data->data, search_data->name);
    }

    btree_print(&test);

    int command, data, run = 1;
    while (run) {
        printf("1: insert, 2: delete, 3: print, 4: search, 5: end\n");
        scanf("%d", &command);

        switch(command) {
            case 1:
                printf("input data : ");
                scanf("%d", &tmp_key.data);
                getchar();
                printf("input name[10] : ");
                fgets(tmp_key.name, 10, stdin);

                btree_insert(&test, &tmp_key);
                break;
            case 2:
                scanf("%d", &tmp_key.data);
                btree_delete(&test, &tmp_key);
                break;
            case 3:
                btree_print(&test);
                break;
            case 4:
                scanf("%d", &data);
                tmp_key.data = data;
                search_data = btree_search(&test, &tmp_key);
                if (search_data != NULL) {
                    printf("search: %d %s\n", search_data->data, search_data->name);
                }
                break;
            case 5:
                run = 0;
            default:
            
        };
        // btree_print(&test);

    }

    btree_drop(test.root);

    return 0;
}