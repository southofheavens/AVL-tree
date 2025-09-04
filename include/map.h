#ifdef __cplusplus
extern "C" {
#endif

#ifndef __MAP_H__
#define __MAP_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Упорядоченный ассоциативный контейнер
 * 
 * Особенности:
 * - Автоматическая сортировка по ключам
 * - Быстрый поиск: O(log n)
 * - Уникальные ключи (дубликаты не допускаются)
 * - Память управляется автоматически
 * 
 * Для работы с контейнером используйте предоставленное API
 */
typedef struct _map map;

#define PTR_DELETER(type,func,p) do { type del = *((type *)p); func(del); } while(0)
#define DELETER(type,func,p) do { type del = *((type *)p); func(&del); } while(0)

/**
 * Итератор для обхода элементов контейнера map
 * 
 * ВАЖНО: Не пытайтесь напрямую работать с полями итератора или
 *        интерпретировать его содержимое. Для всех операций
 *        используйте предоставленное API
 * 
 * Итератор может находиться в одном из состояний:
 * - Содержать valid элемент
 * - Быть в end состоянии (содержать фиктивный элемент)
 * - Быть невалидным (после удаления элемента)
 */
typedef struct map_iterator
{
    int16_t dummy1;
    int16_t dummy2;
    int16_t dummy3;
    int16_t dummy4;
    int16_t dummy5;
    int16_t dummy6;
    int16_t dummy7;
    int16_t dummy8;
} map_iterator;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/** 
 * Выделяет ресурсы для контейнера map и возвращает указатель на него 
 * 
 * Принимает в качестве аргументов размер ключа, размер значения, функцию для
 * сравнения ключей, указатель на пользовательский удалитель ключа, указатель на
 * пользовательский удалитель значения
 * 
 * Если передать в качестве указателей на удалители NULL, то в таком случае
 * будет просто освобождена память, которая была выделена в контейнере 
 * 
 * Пример использования пользовательского удалителя можно посмотреть в examples
 */
map *
map_create
(
    uint16_t    key_size, 
    uint16_t    value_size, 
    int         (*compare_func)       (const void *f, const void *s),
    void        (*key_destroyer)      (void *key),
    void        (*value_destroyer)    (void *value)
);

/**
 * Освобождает ресурсы, занятые контейнером map
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
void 
map_free
(
    map *mp
);

/**
 * Возвращает количество элементов в контейнере
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
inline size_t 
map_size
(
    map *mp
);

/**
 * Возвращает true, если контейнер map пустой, и false в противном случае
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
inline bool
map_empty
(
    map *mp
);

/**
 * Добавляет пару ключ-значение в дерево
 * 
 * Принимает в качестве аргументов указатель на контейнер map, ключ key и 
 * значение value
 * key и value должны быть lvalue (иметь адрес)
 */
#define map_insert(mp,key,value) _map_insert(mp,&(key),&(value))

void 
_map_insert
(
    map *     mp, 
    void *    key, 
    void *    value
);

/**
 * Удаляет из контейнера map элемент, принадлежащий итератору
 * 
 * Принимает в качестве аргументов указатель на контейнер map и итератор
 */
void 
map_erase
(
    map *           mp, 
    map_iterator    iter
);

/**
 * Полностью очищает контейнер map от элементов
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
void
map_clear
(
    map *mp
);

/**
 * Перемещает итератор на следующий элемент контейнера map
 * 
 * Принимает в качестве аргументов указатель на контейнер map и итератор
 */
#define map_iterator_next(mp,iter) _map_iterator_next(mp,&iter)

void 
_map_iterator_next
(
    map *             mp,
    map_iterator *    iter
);

/**
 * Перемещает итератор на предыдущий элемент контейнера map
 * 
 * Принимает в качестве аргументов указатель на контейнер map и итератор
 * 
 * Если мы имеем итератор map_iterator it, удовлетворяющий условию
 * map_iterator_equal(iter, map_iterator_end(mp)) == true
 * и вызовем функцию map_iterator_prev(mp,it), то итератор будет содержать
 * последний элемент контейнера и условие
 * map_iterator_equal(iter, map_iterator_last(mp)) станет истинным
 * (при условии, что дерево не пустое)
 */
#define map_iterator_prev(mp,iter) _map_iterator_prev(mp,&iter)

void 
_map_iterator_prev
(
    map *             mp, 
    map_iterator *    iter
);

/**
 * Сравнивает два итератора и возвращает результат их сравнения
 * 
 * Принимает в качестве аргументов два итератора
 */
bool 
map_iterator_equal
(
    map_iterator first, 
    map_iterator second
);

/**
 * Выполняет поиск элемента с ключом key в контейнере map и возвращает итератор, 
 * содержащий элемент с ключом key 
 * Если элемент не найден, то map_iterator_equal(iter, map_iterator_end(mp)) == true
 * 
 * Принимает в качестве аргументов указатель на контейнер map и ключ key
 * key должен быть lvalue (иметь адрес)
 */
#define map_find(mp,key) _map_find(mp,&key)

map_iterator 
_map_find
(
    map *     mp, 
    void *    key
);

/**
 * Возвращает итератор, содержащий первый элемент контейнера
 * Если контейнер пуст, условие
 * map_iterator_equal(iter, map_iterator_end(mp)) будет истинным
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
map_iterator 
map_iterator_first
(
    map *mp
);

/**
 * Возвращает итератор, содержащий последний элемент контейнера
 * Если контейнер пуст, условие
 * map_iterator_equal(iter, map_iterator_end(mp)) будет истинным
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
map_iterator 
map_iterator_last
(
    map *mp
);

/**
 * Возвращает итератор, содержащий фиктивный элемент контейнера
 * Необходим для работы с, например, функцией map_find
 * 
 * Принимает в качестве аргумента указатель на контейнер map
 */
map_iterator 
map_iterator_end
(
    map *mp
);

/**
 * Возвращает ключ, содержащийся в итераторе iter
 * 
 * Принимает в качестве аргументов итератор и тип ключа
 * 
 * Обратите внимание: операция map_iterator_get_key(map_iterator_end(mp),T) запрещена
 */
#define map_iterator_get_key(iter,type) (*(type *)_map_iterator_get_key(iter))

void *
_map_iterator_get_key
(
    map_iterator iter
);

/**
 * Возвращает значение, содержащееся в итераторе iter
 * 
 * Принимает в качестве аргументов итератор и тип значения
 * 
 * Обратите внимание: операция map_iterator_get_value(map_iterator_end(mp),T) запрещена
 */
#define map_iterator_get_value(iter,type) (*(type *)_map_iterator_get_value(iter))

void *
_map_iterator_get_value
(
    map_iterator iter
);

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void map_print(map *mp, char *(*key_to_str)(const void *), char *(*value_to_str)(const void *));

#endif

#ifdef __cplusplus
}
#endif