/**
 * Пример использования пользовательского удалителя 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map.h>

int int_compare_func(const void *f, const void *s) 
{
    int int_f = *(const int*)f;
    int int_s = *(const int*)s;
    if (int_f < int_s) { return -1; }
    if (int_f > int_s) { return 1; }
    return 0;
}

typedef struct darray
{
    void *data;
    size_t size;
} darray;

void darray_construct(darray *darr, size_t elem_size, size_t size)
{
    darr->data = malloc(elem_size * size);
    darr->size = size;
}

void darray_destroy(darray *darr)
{
    free(darr->data);
}                                       

void value_deleter(void *v)
{
    darray darr = *((darray *)v);
    darray_destroy(&darr);
}

int main(int argc, char *argv[])
/**
 * Данный код содержит определение контейнера map с тремя парами ключ-значение, где ключ - int, значение - 
 * darray и корректное освобождение ресурсов при вызове функции map_free 
 * 
 * Если вместо value_deleter передать в функцию map_create NULL, то в таком случае будет очищена память,
 * которая использовалась для хранения darray, но память, на которую указывает void *darray::data,
 * очищена не будет. Получается утечка
 */
{
    map *darr_map = map_create(sizeof(int), sizeof(darray), int_compare_func, NULL, value_deleter);

    darray f;
    darray_construct(&f, sizeof(int), 5);
    darray s;
    darray_construct(&s, sizeof(int), 10);
    darray t;
    darray_construct(&t, sizeof(int), 15);

    int a = 1;
    int b = 2;
    int c = 3;

    map_insert(darr_map, a, f);
    map_insert(darr_map, b, s);
    map_insert(darr_map, c, t);

    map_free(darr_map);

    return EXIT_SUCCESS;
}
