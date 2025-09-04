#include <map.h>
#include <test.h>

char *int_key_to_str(const void *key)
{
    if (key == NULL)
    {
        char *str = malloc(16 * sizeof(char));
        if (str != NULL)
            snprintf(str, 16, "NULL");
        return str;
    }
    
    char *str = malloc(16 * sizeof(char));
    if (str != NULL)
    {
        snprintf(str, 16, "%d", *(const int *)key);
    }
    return str;
}

char *int_value_to_str(const void *value)
{
    if (value == NULL)
    {
        char *str = malloc(16 * sizeof(char));
        if (str != NULL)
            snprintf(str, 16, "NULL");
        return str;
    }
    
    char *str = malloc(16 * sizeof(char));
    if (str != NULL)
    {
        snprintf(str, 16, "%d", *(const int *)value);
    }
    return str;
}

int int_compare_func(const void *f, const void *s) 
{
    int int_f = *(const int*)f;
    int int_s = *(const int*)s;
    if (int_f < int_s) { return -1; }
    if (int_f > int_s) { return 1; }
    return 0;
}



/*****************************************************************************/

/**
 * Данные структуры определены для того, чтобы мы могли протестировать внутреннее
 * устройство дерева
 */

typedef struct avl_node_test avl_node_test;

struct avl_node_test
{
    avl_node_test *    parent;
    avl_node_test *    left_child;
    avl_node_test *    right_child;
    void *             key;
    void *             value;
    int8_t             balance;
};

typedef struct map_test map_test;

struct map_test
{
    struct
    {
        avl_node_test *root;
        avl_node_test *most_left;
        avl_node_test *most_right;
    } header;

    uint16_t    key_size;
    uint16_t    value_size;
    int         (*compare_func)       (const void *f, const void *s);
    void        (*key_destroyer)      (void *key);
    void        (*value_destroyer)    (void *value);

    size_t      size;
};

typedef struct map_iterator_impl_test map_iterator_impl_test;

struct map_iterator_impl
{
    map *     this_map;
    /**
     * Указатель типа void сделан по той причине, что this_node может указывать
     * как на avl_node, так и на header, а это два разных типа
     */
    void *    this_node;
};

/*****************************************************************************/

C_TEST(insert_small_rotates_test)
{
    map *mp = map_create(sizeof(int), sizeof(int), int_compare_func, NULL, NULL);

    int key, value;

    key = 25, value = 25;
    map_insert(mp, key, value);
    key = 50, value = 50;
    map_insert(mp, key, value);
    key = 75, value = 75;
    map_insert(mp, key, value);

    /**
     * После последней вставки ожидался малый левый поворот. 
     * Убедимся, что это действительно так
     */

    avl_node_test *root = (((map_test *)mp)->header).root;
    avl_node_test *left_child = root->left_child;
    avl_node_test *right_child = root->right_child;

    ASSERT_EQ(*((int *)(root->key)), 50);
    ASSERT_EQ(*((int *)(left_child->key)), 25);
    ASSERT_EQ(*((int *)(right_child->key)), 75);

    ASSERT_EQ(root->parent, NULL);
    ASSERT_EQ(left_child->parent, root);
    ASSERT_EQ(right_child->parent, root);
    ASSERT_EQ(left_child->right_child, NULL);

    /**
     * Заполним дерево элементами и продолжим тестировать вставку, обращая
     * внимание на детей узлов, которые подвергаются повороту
     */

    key = 15, value = 15;
    map_insert(mp, key, value);
    key = 35, value = 35;
    map_insert(mp, key, value);
    key = 65, value = 65;
    map_insert(mp, key, value);
    key = 85, value = 85;
    map_insert(mp, key, value);
    key = 10, value = 10;
    map_insert(mp, key, value);
    key = 20, value = 20;
    map_insert(mp, key, value);
    key = 30, value = 30;
    map_insert(mp, key, value);
    key = 5, value = 5;
    map_insert(mp, key, value);

    /**
     * После последней вставки ожидался малый правый поворот относительно
     * корня и его левого потомка. Убедимся, что это действительно так
     */

    root = (((map_test *)mp)->header).root;
    ASSERT_EQ(*((int *)(root->key)), 25);
    ASSERT_EQ(*((int *)(root->right_child->key)), 50);
    ASSERT_EQ(*((int *)(root->right_child->right_child->key)), 75);
    ASSERT_EQ(*((int *)(root->right_child->right_child->right_child->key)), 85);
    ASSERT_EQ(*((int *)(root->right_child->right_child->left_child->key)), 65);
    ASSERT_EQ(*((int *)(root->right_child->left_child->key)), 35);
    ASSERT_EQ(*((int *)(root->right_child->left_child->left_child->key)), 30);
    ASSERT_EQ(*((int *)(root->left_child->key)), 15);
    ASSERT_EQ(*((int *)(root->left_child->right_child->key)), 20);
    ASSERT_EQ(*((int *)(root->left_child->left_child->key)), 10);
    ASSERT_EQ(*((int *)(root->left_child->left_child->left_child->key)), 5);

    ASSERT_EQ(root->parent, NULL);
    ASSERT_EQ(root->right_child->parent, root);
    ASSERT_EQ(root->right_child, root->right_child->left_child->parent);

    /**
     * Теперь посмотрим на то, как поведёт себя дерево, если поворот потребуется 
     * относительно вершин, ни одна из которых не является корнем 
     */

    key = 12, value = 12;
    map_insert(mp, key, value);
    key = 1, value = 1;
    map_insert(mp, key, value);

    /**
     * После последней вставки ожидался малый поворот направо относительно элементов
     * с ключами 10 и 15. Убедимся, что это действительно так
     */

    root = (((map_test *)mp)->header).root;
    ASSERT_EQ(*((int *)(root->left_child->key)), 10);
    ASSERT_EQ(*((int *)(root->left_child->right_child->key)), 15);
    ASSERT_EQ(*((int *)(root->left_child->right_child->right_child->key)), 20);
    ASSERT_EQ(*((int *)(root->left_child->right_child->left_child->key)), 12);
    ASSERT_EQ(*((int *)(root->left_child->left_child->key)), 5);
    ASSERT_EQ(*((int *)(root->left_child->left_child->left_child->key)), 1);

    ASSERT_EQ(root->left_child->parent, root);
    ASSERT_EQ(root->left_child->right_child->parent, root->left_child);
}

/*****************************************************************************/

C_TEST_SUITE(all_tests)
{
    C_RUN_TEST(insert_small_rotates_test);
}

int main(int argc, char *argv[])
{
    C_RUN_SUITE(all_tests);

    C_TEST_REPORT();
    return EXIT_SUCCESS;
}