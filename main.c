/**
 * Пример использования контейнера map, где ключ - строка
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map.h>

int string_compare_func(const void *f, const void *s)
{
    const char *cf = *(const char **)f;
    const char *cs = *(const char **)s;
    return strcmp(cf, cs);
}

char *create_str(const char *str)
{
    char *cstr = (char *)malloc(strlen(str) + 1);
    strcpy(cstr, str);
    return cstr;
}

void key_deleter(void *k)
{
    char *str = *((char **)k);
    free(str);
}

int main(int argc, char *argv[])
{
    map *mp = map_create(sizeof(char *), sizeof(int), string_compare_func, key_deleter, NULL);

    int f = 1;
    int s = 2;
    int t = 3;

    char *a = create_str("hello");
    char *b = create_str("world");
    char *c = create_str("america");

    map_insert(mp, a, f);
    map_insert(mp, b, s);
    map_insert(mp, c, t);

    map_iterator it = map_iterator_first(mp);

    for (; !map_iterator_equal(it, map_iterator_end(mp)); map_iterator_next(mp, it)) {
        printf("%s - %d\n", map_iterator_get_key(it, char *), map_iterator_get_value(it, int));
    }

    map_free(mp);

    return EXIT_SUCCESS;
}