# Hash Map in C

A hash map implementation written in C, structured in an object-oriented style using function pointers. Maps ASCII string keys to ASCII string values.

---

## Features

- **Open addressing** with **double hashing** for collision resolution — avoids clustering without the memory overhead of separate chaining
- **Dynamic resizing** — automatically grows when the load factor exceeds 70% and shrinks when it drops below 10%, keeping operations at O(1) average time
- **In-place updates** — inserting an existing key replaces the value rather than creating a duplicate
- **OOP-style API** — the `hash_map` struct carries its own method pointers (`insert`, `search`, `delete`, `destroy`), so all operations are called through the object itself
- No external dependencies beyond the C standard library and `libm`

---

### Methods

All methods take the map itself as their first argument (`self`), mirroring the implicit `this` pointer in object-oriented languages.

| Method | Signature | Description |
|---|---|---|
| `insert` | `void (*insert)(hash_map* self, const char* key, const char* value)` | Insert a new entry or update an existing key's value |
| `search` | `char* (*search)(hash_map* self, const char* key)` | Return the value for `key`, or `NULL` if not found |
| `delete` | `void (*delete)(hash_map* self, const char* key)` | Remove the entry for `key`; no-op if key does not exist |
| `destroy` | `void (*destroy)(hash_map* self)` | Free all memory, including the map struct itself |

---

## Design Notes

### OOP in C via function pointers

The `hash_map` struct embeds four function pointers that act as its method table (analogous to a C++ vtable). `map_new()` is the sole constructor and is responsible for wiring all pointers to their concrete implementations at allocation time. Callers never reference the implementation functions directly.

```c
/* How a method call looks at the call site */
map->insert(map, "key", "value");

/* What map_new() does internally */
map->insert  = map_impl_insert;
map->search  = map_impl_search;
map->delete  = map_impl_delete;
map->destroy = map_impl_destroy;
```

### Hashing

A polynomial rolling hash is used:

```
hash(s, a, m) = (a^(n-1)*s[0] + a^(n-2)*s[1] + ... + a^0*s[n-1]) % m
```

Double hashing computes two independent hashes with different prime bases (`151` and `163`) to determine the probe sequence after a collision:

```
index = (hash_a(key) + attempt * (hash_b(key) + 1)) % num_buckets
```

The `+1` on `hash_b` guarantees the step size is never zero, which would otherwise cause infinite looping on the same bucket.

### Deletion and the sentinel

Directly removing an entry from an open-addressed table would break the collision chain, making subsequent entries in that chain unreachable. Instead, deleted slots are marked with a global `DELETED_ENTRY` sentinel. `insert` can reuse sentinel slots; `search` skips over them and keeps probing.

### Resizing

The bucket count is always kept as a prime number to improve the distribution of the double-hash probe sequence. On resize, a new map is allocated at the target size, all live entries are re-inserted through the method pointer, and then the internal arrays are swapped before the temporary map is destroyed — leaving the original pointer valid throughout.

## Usage

### Creating and destroying a map

```c
#include "hash_map.h"

hash_map* map = map_new();   /* allocates and wires up all method pointers */

/* ... use the map ... */

map->destroy(map);           /* frees all entries and the map itself       */
                             /* map pointer is invalid after this call      */
```

### Insert / update

```c
map->insert(map, "language", "C");

/* Inserting the same key again updates the value in-place */
map->insert(map, "language", "C11");
```

### Search

```c
char* val = map->search(map, "language");
if (val) {
    printf("%s\n", val);   /* prints: C11 */
} else {
    printf("not found\n");
}
```

### Delete

```c
map->delete(map, "language");

/* Searching after deletion returns NULL */
char* val = map->search(map, "language");  /* val == NULL */
```

---

## API Reference

### Types

```c
/* A single key-value pair stored in the map */
typedef struct {
    char* key;
    char* value;
} entry;

/* The map object — all operations go through its method pointers */
typedef struct hash_map hash_map;
```

### Constructor

```c
hash_map* map_new(void);
```

Allocates a new hash map and wires up all method pointers. The initial bucket count is the first prime greater than `INITIAL_BASE_SIZE` (50).

---

## Limitations

- Keys and values must be null-terminated ASCII strings. Unicode is not supported.
- The map is not thread-safe. External locking is required for concurrent access.
- Keys and values are copied on insert (`strdup`), so the caller's originals can be freed safely, but this does add a heap allocation per entry.
