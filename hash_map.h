#ifndef HASH_MAP_H
#define HASH_MAP_H

#define INITIAL_BASE_SIZE 50
#define PRIME_1 151
#define PRIME_2 163

typedef struct {
  char *key;
  char *value;
} entry;

typedef struct hash_map hash_map;

struct hash_map {
  int base_size; /* desired minimum size (before rounding to prime) */
  int size;      /* actual allocated bucket count (always prime)    */
  int count;     /* number of live entries currently stored         */
  entry **entries;

  /* -- method pointers ------------------------------------------------ */
  void (*insert)(hash_map *self, const char *key, const char *value);
  char *(*search)(hash_map *self, const char *key);
  void (*delete)(hash_map *self, const char *key);
  void (*destroy)(hash_map *self);
};

/* Constructor */
hash_map *map_new(void);

#endif
