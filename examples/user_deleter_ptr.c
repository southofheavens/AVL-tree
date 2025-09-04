/**
 * Пример использования пользовательского удалителя (ptr)
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

typedef struct c_string
{
    char *str;
    size_t length;
} c_string;

c_string *c_string_create(const char *str)
{
    c_string *cstr = (c_string *)malloc(sizeof(c_string));
    cstr->length = strlen(str);
    cstr->str = (char *)malloc(cstr->length + 1);
    strcpy(cstr->str, str);
    return cstr;
}

void c_string_free(c_string *cstr)
{
    free(cstr->str);
    free(cstr);
}

void value_deleter(void *v)
{
    PTR_DELETER(c_string *, c_string_free, v);
}

int main(int argc, char *argv[])
/**
 * Данный код содержит определение контейнера map с тремя парами ключ-значение, где ключ - int, значение - 
 * c_string * и корректное освобождение ресурсов при вызове функции map_free
 * 
 * Если вместо value_deleter передать в функцию map_create NULL, то в таком случае будет очищена память,
 * которая использовалась для хранения указателя c_string *, но память, на которую указывает char *c_string::str,
 * очищена не будет, ровно как и та память, на которую указывает c_string *. Получается утечка
 */
{
    map *str_map = map_create(sizeof(int), sizeof(c_string *), int_compare_func, NULL, value_deleter);

    c_string *f = c_string_create("hello");
    c_string *s = c_string_create("world");
    c_string *t = c_string_create("america");

    int a = 1;
    int b = 2;
    int c = 3;

    map_insert(str_map, a, f);
    map_insert(str_map, b, s);
    map_insert(str_map, c, t);

    map_free(str_map);

    return EXIT_SUCCESS;
}
