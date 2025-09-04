#include <map.h>
#include <memory.h>
#include <stdbool.h>

typedef struct _avl_node avl_node;

struct _avl_node
{
    avl_node *    parent;
    avl_node *    left_child;
    avl_node *    right_child;
    void *        key;
    void *        value;
    int8_t        balance; 
};

struct _map
{
    struct
    {
        avl_node *root;
        avl_node *most_left;
        avl_node *most_right;
    } header;

    uint16_t    key_size;
    uint16_t    value_size;
    int         (*compare_func)       (const void *f, const void *s);
    void        (*key_destroyer)      (void *key);
    void        (*value_destroyer)    (void *value);

    size_t      size;
};

typedef struct _map_iterator_impl
{
    map *     this_map;
    /**
     * Указатель типа void сделан по той причине, что this_node может указывать
     * как на avl_node, так и на header, а это два разных типа
     */
    void *    this_node;
} map_iterator_impl;


/**
 * Прототипы вспомогательных функций (начало)
 */


/**
 * Функция восстанавливает свойства дерева и при необходимости 
 * выполняет перебалансировку после вставки элемента
 * 
 * В качестве аргументов принимает указатель на map и только что вставленный узел
 */
static void 
map_restore_properties_after_insert
(
    map *         mp, 
    avl_node *    node
);

/**
 * Функция восстанавливает свойства дерева и при необходимости
 * выполняет перебалансировку после удаления элемента
 * 
 * В качестве аргументов принимает указатель на map и отца удаленного элемента
 */
static void 
map_restore_properties_after_erase
(
    map *         mp, 
    avl_node *    node
);

/**
 * Создаёт новый узел и возвращает на него указатель
 * 
 * В качестве аргументов принимает указатели на ключ и значение, а так же их размеры 
 * Возвращает указатель на созданный узел
 */
static avl_node *
map_create_new_node
(
    void *      key, 
    void *      value, 
    uint16_t    key_size, 
    uint16_t    value_size
);

/**
 * Малый правый поворот + смена балансов
 * 
 *                  X                  Y
 *                /   \              /   \ 
 *              Y       R   ===>   L       X
 *            /   \                      /   \ 
 *          L       W                  W       R
 * 
 * Принимает в качестве аргументов указатель на map и узел X
 * Возвращает узел Y
 */
static avl_node *
map_right_small_rotate
(
    map *mp, avl_node *node
);

/**
 * Малый левый поворот + смена балансов
 * 
 *              X                          Y
 *            /   \                      /   \ 
 *          L       Y       ===>       X       R
 *                /   \              /   \ 
 *              W       R          L       W
 * 
 * Принимает в качестве аргументов указатель на map и узел X
 * Возвращает узел Y
 */
static avl_node *
map_left_small_rotate
(
    map *         mp, 
    avl_node *    node
);

/**
 * Большой правый поворот + смена балансов
 * 
 *                     X                            Z   
 *                 /       \                    /       \
 *               Y           c1   ===>        Y           X
 *             /   \                        /   \       /   \
 *          c2       Z                   c2       c3 c4       c1
 *                 /   \
 *              c3       c4
 *
 * Принимает в качестве аргументов указатель на map и узел X
 * Возвращает узел Z
 */
static avl_node *
map_right_big_rotate
(
    map *         mp, 
    avl_node *    node
);

/**
 * Большой левый поворот + смена балансов
 * 
 *                 X                                Z
 *             /       \                        /       \
 *          c1           Y        ===>        X           Y
 *                     /   \                /   \       /   \
 *                   Z       c2          c1       c3 c4       c2
 *                 /   \
 *              c3       c4
 *
 * Принимает в качестве аргументов указатель на map и узел X 
 * Возвращает узел Z
 */
static avl_node *
map_left_big_rotate
(
    map *         mp, 
    avl_node *    node
);

/**
 * Удаление из дерева узла, у которого нет детей
 * 
 * Принимает в качестве аргументов указатель на дерево и на удаляемый узел
 * Возвращает предка удаляемого узла
 */
static avl_node *
map_erase_case_no_children
(
    map *         mp, 
    avl_node *    node
);

/**
 * Удаление из дерева узла, у которого один ребёнок
 * 
 * Принимает в качестве аргументов указатель на дерево и на удаляемый узел
 * Возвращает предка удаляемого узла
 */
static avl_node *
map_erase_case_one_children
(
    map *         mp, 
    avl_node *    node
);

/**
 * Удаление из дерева узла, у которого два ребенка
 * 
 * Принимает в качестве аргументов указатель на дерево и на удаляемый узел
 * Возвращает предка удаляемого узла
 * 
 * Стоит отметить, что фактически мы не удаляем узел с двумя потомками, а
 * находим узел с минимальным ключом из правого поддерева, свапаем ключ и значение
 * с ключом и значением узла, который мы хотели удалить, и удаляем уже тот самый
 * узел с мин. ключом из правого поддерева, у которого, в свою очередь, либо один,
 * либо вообще нет потомков. Соответственно, функция возвращает предка этого самого узла
 */
static avl_node *
map_erase_case_two_children
(
    map *         mp, 
    avl_node *    node
);

/**
 * После удаления проверяет, не изменились ли свойства header`а, который 
 * содержит указатели на самый левый узел (с минимальным значением) и на самый правый 
 * узел (с максимальным значением) 
 * 
 * Принимает в качестве аргументов указатель на map и указатель на удаляемый узел
 */
static void 
map_restore_header_properties_after_erase
(
    map *         mp, 
    avl_node *    node
);

/**
 * После вставки проверяет, не изменились ли свойства header`а, который 
 * содержит указатели на самый левый узел (с минимальным значением ключа) и на самый правый 
 * узел (с максимальным значением ключа) 
 * 
 * Принимает в качестве аргументов указатель на map и указатель на вставляемый узел
 */
static void 
map_restore_header_properties_after_insert
(
    map *         mp, 
    avl_node *    node
);

/**
 * Вспомогательная функция для рекурсивного обхода дерева с целью освобождения
 * ресурсов
 * 
 * Принимает в качестве аргументов указатель на map и указатель на корень
 */
static void 
map_free_helper
(
    map *         mp, 
    avl_node *    node
);


/**
 * Прототипы вспомогательных функций (конец)
 */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**
 * Определения основных функций (API) (начало)
 */


map *
map_create
(
    uint16_t    key_size, 
    uint16_t    value_size, 
    int         (*compare_func)       (const void *f, const void *s),
    void        (*key_destroyer)      (void *key),
    void        (*value_destroyer)    (void *value)
)
{
    if (compare_func == NULL)
    {
        fprintf(stderr, "map_create: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    map *mp = (map *)malloc(sizeof(map));
    if (mp == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    *mp = (map) 
    {
        .header.root = NULL,
        .header.most_left = NULL,
        .header.most_right = NULL,
        .key_size = key_size,
        .value_size = value_size,
        .compare_func = compare_func,
        .key_destroyer = key_destroyer,
        .value_destroyer = value_destroyer,
        .size = 0
    };

    return mp;
}

void
map_free
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_free: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    if (mp->header.root != NULL) {
        map_free_helper(mp, mp->header.root);
    }

    free(mp);
}

inline size_t 
map_size
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_size: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    return mp->size;
}

inline bool
map_empty
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_empty: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    return (mp->size == 0 ? true : false);
}

void 
_map_insert
(
    map *     mp, 
    void *    key, 
    void *    value
)
{
    if (mp == NULL || key == NULL || value == NULL)
    {
        fprintf(stderr, "map_insert: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    /**
     * Если true - вставляем элемент, в противном случае элемент с ключом key
     * в дереве уже есть и мы просто заменяем старое значение (value) на новое
     */
    bool insert = true;
    /**
     * Результат сравнения ключа узла из дерева со вставляемым ключом
     */
    int cmp;
    avl_node *parent = NULL;
    avl_node *current = mp->header.root;
    while (current) 
    {
        parent = current;

        cmp = mp->compare_func(current->key, key);

        if (cmp < 0) {
            current = current->right_child;
        }
        else if (cmp > 0) {
            current = current->left_child;
        }
        else
        {
            memcpy(current->value, value, mp->value_size);
            insert = false;
            break;
        }
    }

    if (insert)
    /**
     * Вставляем элемент (либо дерево пустое, либо элемент с ключом key
     * в дереве не обнаружен)
     */
    {
        avl_node *insert_node = map_create_new_node(key, value, mp->key_size, mp->value_size);
        map_restore_header_properties_after_insert(mp, insert_node);

        if (parent != NULL)
        {
            if (cmp < 0) 
            {
                parent->right_child = insert_node;
                if (insert_node->key > mp->header.most_right->key) {
                    mp->header.most_right = insert_node;
                }
            }
            else 
            {
                parent->left_child = insert_node;
                if (insert_node->key < mp->header.most_left->key) {
                    mp->header.most_left = insert_node;
                }
            }
            insert_node->parent = parent;
        }
    
        (mp->size)++;

        map_restore_properties_after_insert(mp, insert_node);
    }
}

void 
map_erase
(
    map *           mp, 
    map_iterator    iter
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_erase: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    /* Удостоверимся, что итератор принадлежит данному дереву */
    map_iterator_impl input_iter_impl = *(map_iterator_impl *)&iter;

    map_iterator find_elem = _map_find(mp, ((avl_node *)(input_iter_impl.this_node))->key);
    if (map_iterator_equal(find_elem, map_iterator_end(mp))) 
    {
        fprintf(stderr, "map_erase: итератор iter не принадлежит контейнеру\n");
        exit(EXIT_FAILURE);
    }
    else if (!map_iterator_equal(iter, find_elem))
    {
        fprintf(stderr, "map_erase: итератор iter не принадлежит контейнеру\n");
        exit(EXIT_FAILURE);   
    }

    /* Дошли до сюда - всё ок */

    map_iterator_impl find_iter_impl = *(map_iterator_impl *)&find_elem;
    avl_node *erase_node = find_iter_impl.this_node;
    map_restore_header_properties_after_erase(mp, erase_node);

    avl_node *parent;

    if (erase_node->left_child == NULL && erase_node->right_child == NULL) {
        parent = map_erase_case_no_children(mp, erase_node);
    }  
    else if (erase_node->left_child != NULL && erase_node->right_child != NULL) {
        parent = map_erase_case_two_children(mp, erase_node);
    }                                         
    else {
        parent = map_erase_case_one_children(mp, erase_node);
    }       
    
    (mp->size)--;
    map_restore_properties_after_erase(mp, parent);
}

void
map_clear
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_clear: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    if (mp->header.root != NULL) {
        map_free_helper(mp, mp->header.root);
    }    

    *mp = (map) 
    {
        .header.root = NULL,
        .header.most_left = NULL,
        .header.most_right = NULL,
        .size = 0
    };
}

void 
_map_iterator_next
(
    map *             mp, 
    map_iterator *    iter
)
{
    if (iter == NULL)
    {
        fprintf(stderr, "map_iterator_next_elem: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }
 
    map_iterator_impl *implementation_of_iter = (map_iterator_impl *)iter;
    avl_node *curr_node = (avl_node *)(implementation_of_iter->this_node);

    if (map_iterator_equal(*iter, map_iterator_last(mp))) {
        curr_node = (avl_node *)((void *)(&(mp->header)));
    }
    else if (map_iterator_equal(*iter, map_iterator_end(mp))) 
    {
        fprintf(stderr, "map_iterator_next_elem: произведена попытка выйти за границы контейнера\n");
        exit(EXIT_FAILURE);
    }
    else if (curr_node->right_child)
    {
        curr_node = curr_node->right_child;
        while (curr_node->left_child) {
            curr_node = curr_node->left_child;
        }
    }
    else
    {
        while (curr_node->parent && curr_node->parent->left_child != curr_node) {
            curr_node = curr_node->parent;
        }
        curr_node = curr_node->parent;
    }

    implementation_of_iter->this_node = curr_node;
}   

void 
_map_iterator_prev
(
    map *             mp, 
    map_iterator *    iter
)
{
    if (iter == NULL)
    {
        fprintf(stderr, "map_iterator_next_elem: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }
 
    map_iterator_impl *implementation_of_iter = (map_iterator_impl *)iter;
    avl_node *curr_node = implementation_of_iter->this_node;

    if (map_iterator_equal(*iter, map_iterator_end(mp))) {
        curr_node = mp->header.most_right;
    }
    else if (map_iterator_equal(*iter, map_iterator_first(mp))) 
    {
        fprintf(stderr, "map_iterator_prev: произведена попытка выйти за границы контейнера\n");
        exit(EXIT_FAILURE);
    }
    else if (curr_node->left_child)
    {
        curr_node = curr_node->left_child;
        while (curr_node->right_child) {
            curr_node = curr_node->right_child;
        }
    }
    else
    {
        while (curr_node->parent && curr_node->parent->right_child != curr_node) {
            curr_node = curr_node->parent;
        }
        curr_node = curr_node->parent;
    }

    implementation_of_iter->this_node = curr_node;
}

bool 
map_iterator_equal
(
    map_iterator f, 
    map_iterator s
)
{
    map_iterator_impl iter_impl_f = *(map_iterator_impl *)&f;
    map_iterator_impl iter_impl_s = *(map_iterator_impl *)&s;

    return (iter_impl_f.this_map == iter_impl_s.this_map
        && iter_impl_f.this_node == iter_impl_s.this_node);
}

map_iterator 
_map_find
(
    map *     mp, 
    void *    key
)
{
    if (mp == NULL || key == NULL)
    {
        fprintf(stderr, "map_find: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    map_iterator_impl iter_impl = {.this_map = mp, .this_node = NULL};

    avl_node *curr_elem = mp->header.root;
    while (curr_elem != NULL)
    {
        int result_of_compare_func = mp->compare_func(curr_elem->key, key);
        if (result_of_compare_func < 0) {
            curr_elem = curr_elem->right_child;
        }
        else if (result_of_compare_func > 0) {
            curr_elem = curr_elem->left_child;
        }
        else
        {
            iter_impl.this_node = curr_elem;
            break;
        }
    }   
    if (curr_elem == NULL) {
        iter_impl.this_node = &(mp->header);
    }
    return *(map_iterator *)&iter_impl;
}

map_iterator 
map_iterator_first
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_iterator_first: в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    if (mp->size == 0) {
        return map_iterator_end(mp);
    }

    map_iterator_impl iter_impl = {.this_map = mp, .this_node = mp->header.most_left};
    return *((map_iterator *)&iter_impl);
}

map_iterator 
map_iterator_last
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_iterator_last в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    if (mp->size == 0) {
        return map_iterator_end(mp);
    }

    map_iterator_impl iter_impl = {.this_map = mp, .this_node = mp->header.most_right};
    return *((map_iterator *)&iter_impl);
}

map_iterator 
map_iterator_end
(
    map *mp
)
{
    if (mp == NULL)
    {
        fprintf(stderr, "map_iterator_end в качестве аргумента передан нулевой указатель\n");
        exit(EXIT_FAILURE);
    }

    map_iterator_impl iter_impl = {.this_map = mp, .this_node = &(mp->header)};
    return *((map_iterator *)(&iter_impl));
}


void *
_map_iterator_get_key
(
    map_iterator iter
)
{
    map_iterator_impl iter_impl = *(map_iterator_impl *)&iter;

    return ((avl_node *)(iter_impl.this_node))->key;
}

void *
_map_iterator_get_value
(
    map_iterator iter
)
{
    map_iterator_impl iter_impl = *(map_iterator_impl *)&iter;

    return ((avl_node *)(iter_impl.this_node))->value;
}


/**
 * Определения основных функций (API) (конец)
 */

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/**
 * Вспомогательные функции (начало)
 */


static void 
map_restore_properties_after_insert
(
    map *         mp, 
    avl_node *    node
)
{
    while (node->parent != NULL)
    {
        avl_node *parent = node->parent;

        parent->balance += (node->parent->left_child == node) ?  1 : -1;
        
        if (parent->balance == 0) 
        /* Восстановление свойств окончено */
        {
            break;
        }
        else if (abs(parent->balance) == 2)
        {
            if (parent->balance == 2 && node->balance == 1)
            /* Малый правый поворот */
            {
                node = map_right_small_rotate(mp, parent);
                break;
            }
            else if (parent->balance == -2 && node->balance == -1)
            /* Малый левый поворот */
            {
                node = map_left_small_rotate(mp, parent);
                break;
            }
            else if (parent->balance == 2 && node->balance == -1)
            /* Большой правый поворот */
            {
                node = map_right_big_rotate(mp, parent);
                if (node->balance == 0) {
                    break;
                }
            }
            else
            /* Большой левый поворот */
            {
                node = map_left_big_rotate(mp, parent);
                if (node->balance == 0) {
                    break;
                }
            }
        }

        node = node->parent;
    }
}

static void 
map_restore_properties_after_erase
(
    map *         mp, 
    avl_node *    node
)
{
    while (node != NULL)
    {
        if (abs(node->balance) == 2)
        {
            if ((node->balance == 2 && node->left_child->balance == 1)
                || (node->balance == 2 && node->left_child->balance == 0))
            {
                node = map_right_small_rotate(mp, node);
            }
            else if ((node->balance == -2 && node->right_child->balance == -1)
                || (node->balance == -2 && node->right_child->balance == 0))
            {
                node = map_left_small_rotate(mp, node);
            }
            else if (node->balance == 2 && node->left_child->balance == -1) {
                node = map_right_big_rotate(mp, node);
            }
            else {
                node = map_left_big_rotate(mp, node);
            }
        }

        if (abs(node->balance) == 1) {
            break;
        }

        if (node->parent != NULL)
        {
            if (node->parent->right_child == node) {
                node->parent->balance++;
            }
            else {
                node->parent->balance--;
            }   
        }

        node = node->parent;
    }
}

static avl_node *
map_create_new_node
(
    void *      key, 
    void *      value, 
    uint16_t    key_size, 
    uint16_t    value_size
)
{
    avl_node *insert_node = (avl_node *)malloc(sizeof(avl_node));
    if (insert_node == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    *insert_node = (avl_node)
    {
        .parent = NULL,
        .left_child = NULL,
        .right_child = NULL,
        .key = NULL,
        .value = NULL,
        .balance = 0
    };

    insert_node->key = malloc(key_size);
    if (insert_node->key == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    memcpy(insert_node->key, key, key_size);

    insert_node->value = malloc(value_size);
    if (insert_node->value == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    memcpy(insert_node->value, value, value_size);

    return insert_node;
}

static avl_node *
map_right_small_rotate
(
    map *         mp, 
    avl_node *    node
)
{
    avl_node *new_parent = node->left_child;
    avl_node *weak_node = new_parent->right_child;

    int8_t old_node_balance = node->balance;
    int8_t old_new_parent_balance = new_parent->balance;

    new_parent->parent = node->parent;
    if (node->parent) 
    {
        if (node->parent->left_child == node) {
            node->parent->left_child = new_parent;
        }
        else {
            node->parent->right_child = new_parent;
        }
    } 
    else {
        mp->header.root = new_parent;
    }

    new_parent->right_child = node;
    node->parent = new_parent;

    node->left_child = weak_node;
    if (weak_node) {
        weak_node->parent = node;
    }

    node->balance = old_node_balance - 1 - (old_new_parent_balance > 0 ? old_new_parent_balance : 0);
    new_parent->balance = old_new_parent_balance - 1 + (node->balance < 0 ? node->balance : 0);

    return new_parent;
}

static avl_node *
map_left_small_rotate
(
    map *         mp, 
    avl_node *    node
)
{
    avl_node *new_parent = node->right_child;
    avl_node *weak_node = new_parent->left_child;

    int8_t old_node_balance = node->balance;
    int8_t old_new_parent_balance = new_parent->balance;

    new_parent->parent = node->parent;
    if (node->parent) 
    {
        if (node->parent->left_child == node) {
            node->parent->left_child = new_parent;
        }
        else {
            node->parent->right_child = new_parent;
        }
    } 
    else {
        mp->header.root = new_parent;
    }

    new_parent->left_child = node;
    node->parent = new_parent;

    node->right_child = weak_node;
    if (weak_node) {
        weak_node->parent = node;
    }

    node->balance = old_node_balance + 1 - (old_new_parent_balance < 0 ? old_new_parent_balance : 0);
    new_parent->balance = old_new_parent_balance + 1 + (node->balance > 0 ? node->balance : 0);

    return new_parent;
}

static avl_node *
map_right_big_rotate
(
    map *         mp, 
    avl_node *    node
)
{
    map_left_small_rotate(mp, node->left_child);
    return map_right_small_rotate(mp, node);
}

static avl_node *
map_left_big_rotate
(
    map *         mp, 
    avl_node *    node
)
{
    map_right_small_rotate(mp, node->right_child);
    return map_left_small_rotate(mp, node);
}

static avl_node *
map_erase_case_no_children
(
    map *         mp, 
    avl_node *    node
)
{       
    avl_node *parent = node->parent;

    if (parent != NULL)
    {
        if (parent->right_child == node) 
        {
            parent->balance++;
            parent->right_child = NULL;
        }
        else 
        {
            parent->balance--;
            parent->left_child = NULL;
        }   
    }
    else 
    {
        mp->header.root = NULL;
        mp->header.most_left = NULL;
        mp->header.most_right = NULL;
    }

    free(node->key);
    free(node->value);
    free(node);

    return parent;
}   

static avl_node *
map_erase_case_one_children
(
    map *         mp, 
    avl_node *    node
)
{
    avl_node *parent = node->parent;    
    
    if (parent != NULL)
    {
        if (node->right_child != NULL) 
        {
            node->right_child->parent = parent;

            if (parent->right_child == node) 
            {
                parent->right_child = node->right_child;
                parent->balance++;
            }
            else 
            {
                parent->left_child = node->right_child;
                parent->balance--;
            }
        }
        else 
        {
            node->left_child->parent = parent;

            if (parent->right_child == node) 
            {
                parent->right_child = node->left_child;
                parent->balance++;
            }
            else 
            {
                parent->left_child = node->left_child;
                parent->balance--;
            }
        }
    }
    else 
    {
        if (node->right_child != NULL) 
        {
            mp->header.root = node->right_child;
            mp->header.root->parent = NULL;
        }
        else 
        {
            mp->header.root = node->left_child;
            mp->header.root->parent = NULL;
        }
    }

    free(node->key);
    free(node->value);
    free(node);

    return parent;
}

static avl_node *
map_erase_case_two_children
(
    map *         mp, 
    avl_node *    node
)
{       
    avl_node *parent;
    avl_node *replacement = node->right_child;

    while (replacement->left_child) {
        replacement = replacement->left_child;
    }

    void *temp_node_key = replacement->key;
    void *temp_node_value = replacement->value;

    replacement->key = node->key;
    replacement->value = node->value;

    node->key = temp_node_key;
    node->value = temp_node_value;

    if (replacement->right_child == NULL && replacement->left_child == NULL) {
        parent = map_erase_case_no_children(mp, replacement);
    }
    else {
        parent = map_erase_case_one_children(mp, replacement);
    }

    return parent;
}                                  

static void 
map_restore_header_properties_after_erase
(
    map *         mp,
    avl_node *    node
)
{
    if (node->key == mp->header.most_left->key)
    {
        if (node->right_child != NULL) {
            mp->header.most_left = node->right_child;
        }
        else {
            mp->header.most_left = node->parent;
        }
    }
    else if (node->key == mp->header.most_right->key)
    {
        if (node->left_child != NULL) {
            mp->header.most_right = node->left_child;
        }
        else {
            mp->header.most_right = node->parent;
        }
    }
}

static void 
map_restore_header_properties_after_insert
(
    map *         mp, 
    avl_node *    node
)
{
    if (mp->header.root == NULL)
    {
        mp->header.root = node;
        mp->header.most_left = node;
        mp->header.most_right = node;
    }
    else 
    {
        if (mp->compare_func(node->key, mp->header.most_left->key) < 0) {
            mp->header.most_left = node;
        }
        else if (mp->compare_func(node->key, mp->header.most_right->key) > 0) {
            mp->header.most_right = node;
        }
    }
}

static void 
map_free_helper
(
    map *         mp, 
    avl_node *    node
)
{
    if (node->left_child != NULL) {
        map_free_helper(mp, node->left_child);
    }
    if (node->right_child != NULL) {
        map_free_helper(mp, node->right_child);
    }

    if (mp->key_destroyer != NULL && mp->key_destroyer != free) {
        mp->key_destroyer(node->key);                
    }
    free(node->key);

    if (mp->value_destroyer != NULL && mp->value_destroyer != free) {
        mp->value_destroyer(node->value);
    }
    free(node->value);

    free(node);
}


/**
 * Вспомогательные функции (конец)
 */











/**
 * Функции для печати дерева (добавить в соответствующие разделы)
 */

// Прототипы вспомогательных функций для печати
static void map_print_subtree(avl_node *node, int depth, char prefix, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *));
static void map_print_node(avl_node *node, char prefix, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *));

/**
 * Печатает всё дерево в удобочитаемом формате
 * 
 * @param mp Указатель на map
 * @param key_to_str Функция для преобразования ключа в строку (если NULL, будет печатать указатели)
 * @param value_to_str Функция для преобразования значения в строку (если NULL, будет печатать указатели)
 */
void map_print(map *mp, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *))
{
    if (mp == NULL)
    {
        printf("Map is NULL\n");
        return;
    }
    
    printf("Map (size: %zu):\n", mp->size);
    
    if (mp->header.root == NULL)
    {
        printf("  [Empty]\n");
        return;
    }
    
    map_print_subtree(mp->header.root, 0, 'R', key_to_str, value_to_str);
}

/**
 * Рекурсивно печатает поддерево
 */
static void map_print_subtree(avl_node *node, int depth, char prefix, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *))
{
    if (node == NULL)
        return;
    
    // Сначала правый ребенок (будет напечатан выше)
    map_print_subtree(node->right_child, depth + 1, '/', key_to_str, value_to_str);
    
    // Затем текущий узел
    for (int i = 0; i < depth; i++)
        printf("    ");
    
    printf("%c-- ", prefix);
    map_print_node(node, prefix, key_to_str, value_to_str);
    printf("\n");
    
    // Затем левый ребенок (будет напечатан ниже)
    map_print_subtree(node->left_child, depth + 1, '\\', key_to_str, value_to_str);
}

/**
 * Печатает информацию об узле
 */
static void map_print_node(avl_node *node, char prefix, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *))
{
    char *key_str = NULL;
    char *value_str = NULL;
    
    if (key_to_str != NULL)
        key_str = key_to_str(node->key);
    
    if (value_to_str != NULL)
        value_str = value_to_str(node->value);
    
    printf("[K:");
    if (key_str != NULL)
        printf("%s", key_str);
    else
        printf("%p", node->key);
    
    printf(" V:");
    if (value_str != NULL)
        printf("%s", value_str);
    else
        printf("%p", node->value);
    
    printf(" B:%d]", node->balance);
    
    // Очищаем временные строки, если они были созданы
    // (предполагается, что вызывающая функция отвечает за освобождение памяти)
}
