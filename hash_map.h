#ifndef HASH_MAP_H
#define HASH_MAP_H

/* ----- Constants --------------------------------------------------------- */

#define INITIAL_BASE_SIZE 50
#define PRIME_1           151
#define PRIME_2           163

/* ----- Data structures --------------------------------------------------- */

typedef struct {
    char* key;
    char* value;
} entry;

typedef struct {
    int base_size;   /* desired minimum size (before rounding to prime) */
    int size;        /* actual allocated bucket count (always prime)    */
    int count;       /* number of live entries currently stored         */
    entry** entries;
} hash_map;

/* ----- Public API -------------------------------------------------------- */

/* Create / destroy */
hash_map* map_new(void);
void      map_del(hash_map* map);

/* Core operations */
void  map_insert(hash_map* map, const char* key, const char* value);
char* map_search(hash_map* map, const char* key);
void  map_delete(hash_map* map, const char* key);

#endif /* HASH_MAP_H */
