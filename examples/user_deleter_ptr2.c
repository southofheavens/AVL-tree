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

char *strdup(const char * const str)
{
    char *s = (char *)malloc(strlen(str)+1);
    strcpy(s, str);
    return s;
}

void value_deleter(void *v)
{
    PTR_DELETER(char *, free, v);
}

int main(int argc, char *argv[])
{
    map *str_map = map_create(sizeof(int), sizeof(char *), int_compare_func, NULL, value_deleter);

    char *f = strdup("hello");
    char *s = strdup("world");
    char *t = strdup("america");

    int a = 1;
    int b = 2;
    int c = 3;

    map_insert(str_map, a, f);
    map_insert(str_map, b, s);
    map_insert(str_map, c, t);

    map_free(str_map);

    return EXIT_SUCCESS;
}
