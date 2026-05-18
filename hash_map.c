#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_map.h"
#include "prime.h"

static entry DELETED_ENTRY = {NULL, NULL};

/* ----- Internal helpers -------------------------------------------------- */

/* Allocate and initialise a single key-value entry. */
static entry *new_entry(const char *k, const char *v) {
  entry *e = malloc(sizeof(entry));
  if (!e) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  e->key = strdup(k);
  e->value = strdup(v);
  return e;
}

/* Free a single entry. */
static void del_entry(entry *e) {
  free(e->key);
  free(e->value);
  free(e);
}

/*
 * Core hash function.
 *
 * Maps an ASCII string to an integer in [0, m).
 * Uses a polynomial rolling hash with prime base `a`.
 * The modulo is applied at each step to keep intermediate values small.
 */
static int hash(const char *s, const int a, const int m) {
  long h = 0;
  const int len_s = (int)strlen(s);
  for (int i = 0; i < len_s; i++) {
    h += (long)pow(a, len_s - (i + 1)) * s[i];
    h = h % m;
  }
  return (int)h;
}

/*
 * Double-hashing probe function.
 *
 * Returns the bucket index to try after `attempt` collisions.
 * Two independent hash functions (different prime bases) are combined so that
 * the step size itself is well-distributed, avoiding clustering.
 */
static int get_hash(const char *s, const int num_buckets, const int attempt) {
  const int hash_a = hash(s, PRIME_1, num_buckets);
  const int hash_b = hash(s, PRIME_2, num_buckets);
  return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

static void map_impl_insert(hash_map *self, const char *key, const char *value);
static char *map_impl_search(hash_map *self, const char *key);
static void map_impl_delete(hash_map *self, const char *key);
static void map_impl_destroy(hash_map *self);

/* ----- Construction / destruction ---------------------------------------- */

/* Create a hash map with a specific base size. */
static hash_map *map_new_sized(const int base_size) {
  hash_map *map = malloc(sizeof(hash_map));
  if (!map) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  map->base_size = base_size;
  map->size = next_prime(base_size);
  map->count = 0;
  map->entries = calloc((size_t)map->size, sizeof(entry *));
  if (!map->entries) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  map->insert = map_impl_insert;
  map->search = map_impl_search;
  map->delete = map_impl_delete;
  map->destroy = map_impl_destroy;

  return map;
}

/* Public constructor – uses the default starting size. */
hash_map *map_new(void) { return map_new_sized(INITIAL_BASE_SIZE); }

/* Destroy the entire map and all entries it contains. */
static void map_impl_destroy(hash_map *self) {
  for (int i = 0; i < self->size; i++) {
    entry *e = self->entries[i];
    if (e != NULL && e != &DELETED_ENTRY) {
      del_entry(e);
    }
  }
  free(self->entries);
  free(self);
}

/* ----- Resizing ---------------------------------------------------------- */

/*
 * Resize the map to a new base size.
 *
 * A brand-new map is allocated, all live entries are re-inserted via the
 * method pointer, and then the guts of the two maps are swapped
 * so the caller's pointer remains valid.
 */
static void map_resize(hash_map *map, const int base_size) {
  /* Never shrink below the minimum size. */
  if (base_size < INITIAL_BASE_SIZE) {
    return;
  }

  hash_map *new_map = map_new_sized(base_size);
  for (int i = 0; i < map->size; i++) {
    entry *e = map->entries[i];
    if (e != NULL && e != &DELETED_ENTRY) {
      new_map->insert(new_map, e->key, e->value);
    }
  }

  map->base_size = new_map->base_size;
  map->count = new_map->count;

  int tmp_size = map->size;
  map->size = new_map->size;
  new_map->size = tmp_size;

  entry **tmp_entries = map->entries;
  map->entries = new_map->entries;
  new_map->entries = tmp_entries;

  new_map->destroy(new_map);
}

static void map_resize_up(hash_map *map) {
  map_resize(map, map->base_size * 2);
}

static void map_resize_down(hash_map *map) {
  map_resize(map, map->base_size / 2);
}

/* ----- Method implementations -------------------------------------------- */

/*
 * Insert (or update) a key-value pair.
 *
 * If the key already exists its value is replaced in-place.
 * Resizes the map up if the load factor exceeds 70 %.
 */
static void map_impl_insert(hash_map *self, const char *key,
                            const char *value) {
  /* Resize up if load > 70 % */
  const int load = self->count * 100 / self->size;
  if (load > 70) {
    map_resize_up(self);
  }

  entry *e = new_entry(key, value);
  int index = get_hash(e->key, self->size, 0);
  entry *cur = self->entries[index];

  for (int i = 1; cur != NULL; i++) {
    if (cur != &DELETED_ENTRY) {
      if (strcmp(cur->key, key) == 0) {
        del_entry(cur);
        self->entries[index] = e;
        return;
      }
    }
    index = get_hash(e->key, self->size, i);
    cur = self->entries[index];
  }

  self->entries[index] = e;
  self->count++;
}

/*
 * Search for a key and return its value, or NULL if not found.
 */
static char *map_impl_search(hash_map *self, const char *key) {
  int index = get_hash(key, self->size, 0);
  entry *e = self->entries[index];

  for (int i = 1; e != NULL; i++) {
    if (e != &DELETED_ENTRY) {
      if (strcmp(e->key, key) == 0) {
        return e->value;
      }
    }
    index = get_hash(key, self->size, i);
    e = self->entries[index];
  }

  return NULL;
}

/*
 * Delete a key-value pair.
 *
 * Entries are NOT removed outright because that would break collision chains.
 * Instead, the slot is replaced with the global DELETED_ENTRY sentinel.
 */
static void map_impl_delete(hash_map *self, const char *key) {
  /* Resize down if load < 10 %. */
  const int load = self->count * 100 / self->size;
  if (load < 10) {
    map_resize_down(self);
  }

  int index = get_hash(key, self->size, 0);
  entry *e = self->entries[index];

  for (int i = 1; e != NULL; i++) {
    if (e != &DELETED_ENTRY) {
      if (strcmp(e->key, key) == 0) {
        del_entry(e);
        self->entries[index] = &DELETED_ENTRY;
        self->count--;
        return;
      }
    }
    index = get_hash(key, self->size, i);
    e = self->entries[index];
  }
}
