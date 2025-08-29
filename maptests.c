#include <map.h>
#include <test.h>

/** 
 * Преобразует int ключ в строку
 * 
 * @param key Указатель на int ключ
 * @return Строковое представление ключа (требует free после использования)
 */
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

/**
 * Преобразует int значение в строку
 */
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

/**
 * 
 * 
 * 
 * 
 * 
 * 
 */

int int_compare_func(const void *f, const void *s) 
{
    int int_f = *(const int*)f;
    int int_s = *(const int*)s;
    if (int_f < int_s) { return -1; }
    if (int_f > int_s) { return 1; }
    return 0;
}

C_TEST(map_insert_test)
/**
 * Если возникает вопрос, откуда я узнал, как выглядит дерево на том
 * или ином этапе, то вот ответ: была написана функция для вывода дерева,
 * которую я удалил из кода перед релизом
 */
{
    map *mp = map_create(sizeof(int), sizeof(int), int_compare_func, NULL, NULL);

    int key, value;

    /* На данный момент дерево пустое */

    key = 1, value = 1;
    map_insert(mp, key, value);

    /**
     * На данный момент дерево выглядит следующим образом:
     * 
     * R <1,1,0>, где <1,1,0> - <key,value,balance>
     */

    key = 2, value = 2;
    map_insert(mp, key, value);

    /**
     * На данный момент дерево выглядит следующим образом:
     * 
     *             <2,2,0>
     *           /
     * R <1,1,-1> 
     */

    /**
     * Ожидается, что после вставки будет совершен малый поворот налево
     */
    key = 4, value = 4;
    map_insert(mp, key, value);

    /**
     * На данный момент дерево выглядит следующим образом:
     * 
     *             <4,4,0>
     *           /
     * R <2,2,0>
     *           \
     *             <1,1,0>
     */

    key = 7, value = 7;
    map_insert(mp, key, value);

    /**
     * Ожидается, что после вставки будет совершен большой поворот налево
     */
    key = 5, value = 5;
    map_insert(mp, key, value);

    /**
     * 
     * На данный момент дерево выглядит следующим образом:
     * 
     *                        <7,7,0>
     *                      /
     *              <5,5,0> 
     *            /         \
     *                        <4,4,0>
     * 
     * R <2,2,-1>
     *            \
     *              <1,1,0>
     */

    /**
     * Ожидается, что после вставки будет совершен большой поворот налево
     */
    key = 3, value = 3;
    map_insert(mp, key, value);

    /**
     * 
     * На данный момент дерево выглядит следующим образом:
     * 
     *                        <7,7,0>
     *                      /
     *              <5,5,-1> 
     *            /         
     * R <4,4,0>
     * 
     *                        <3,3,0>
     *            \         /
     *              <2,2,0>
     *                      \
     *                        <1,1,0>
     */

    /**
     * Ожидается, что после вставки будет совершен большой поворот налево
     */
    key = 6, value = 6;
    map_insert(mp, key, value);

    /**
     * 
     * На данный момент дерево выглядит следующим образом:
     * 
     *                        <7,7,0>
     *                      /
     *              <6,6,0> 
     *            /         \
     *                        <5,5,0>
     *    
     * R <4,4,0>
     * 
     *                        <3,3,0>
     *            \         /
     *              <2,2,0>
     *                      \
     *                        <1,1,0>
     */

    map_free(mp);



    /** 
     * Теперь потестим повороты направо 
     */



    mp = map_create(sizeof(int), sizeof(int), int_compare_func, NULL, NULL);

    key = 50, value = 50;
    map_insert(mp, key, value);
    key = 45, value = 45;
    map_insert(mp, key, value);

    /**
     * Ожидается, что после вставки будет совершен малый поворот направо
     */
    key = 40, value = 40;
    map_insert(mp, key, value);

    /**
     * На данный момент дерево выглядит следующим образом:
     * 
     *               <50,50,0>
     *             /
     * R <45,45,0>
     *             \ 
     *               <40,40,0>
     */

    key = 35, value = 35;
    map_insert(mp, key, value);
    key = 41, value = 41;
    map_insert(mp, key, value);

    /**
     * Ожидается, что после вставки будет совершен большой поворот направо
     */
    key = 42, value = 42;
    map_insert(mp, key, value);

    /**
     * На данный момент дерево выглядит следующим образом:
     * 
     *                           <50,50,0>
     *                         /
     *               <45,45,0>
     *             /           \
     *                           <42,42,0>
     * 
     * R <41,41,0>
     *             \ 
     *               <40,40,0>
     *                         \
     *                           <35,35,0>
     */

    map_free(mp);



    /**
     * Теперь попробуем заполнить дерево таким образом, чтобы не было
     * необходимости совершать повороты
     */



    mp = map_create(sizeof(int), sizeof(int), int_compare_func, NULL, NULL);

    key = 50, value = 50;
    map_insert(mp, key, value);
    key = 75, value = 75;
    map_insert(mp, key, value);
    key = 25, value = 25;
    map_insert(mp, key, value);
    key = 15, value = 15;
    map_insert(mp, key, value);
    key = 35, value = 35;
    map_insert(mp, key, value);
    key = 85, value = 85;
    map_insert(mp, key, value);
    key = 65, value = 65;
    map_insert(mp, key, value);

    /**
     * 
     * На данный момент дерево выглядит следующим образом:
     * 
     *                        <85,85,0>
     *                      /
     *              <75,75,0> 
     *            /         \
     *                        <65,65,0>
     *    
     * R <50,50,0>
     * 
     *                        <35,35,0>
     *            \         /
     *              <25,25,0>
     *                      \
     *                        <15,15,0>
     */

    map_free(mp);
}

C_TEST(map_erase_test)
{
    /**
     * Построим дерево, из которого в дальнейшем будем удалять элементы
     */

    

    map *mp = map_create(sizeof(int), sizeof(int), int_compare_func, NULL, NULL);
}

C_TEST_SUITE(all_tests)
{
    C_RUN_TEST(map_insert_test);
}

int main(int argc, char *argv[])
{
    C_RUN_SUITE(all_tests);

    C_TEST_REPORT();
    return EXIT_SUCCESS;
}