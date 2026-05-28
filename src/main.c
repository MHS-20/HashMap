#include "hash_map.h"
#include <stdio.h>

int main(void) {
  hash_map *map = map_new();
  printf("Hash map created.\n\n");

  /* ---- Insert ---- */
  map->insert(map, "name", "Alice");
  map->insert(map, "city", "Rome");
  map->insert(map, "lang", "C");
  map->insert(map, "version", "C11");
  printf("Inserted 4 key-value pairs.\n");

  /* ---- Search ---- */
  printf("\nSearching for keys:\n");
  const char *keys[] = {"name", "city", "lang", "version", "missing"};
  for (int i = 0; i < 5; i++) {
    char *val = map->search(map, keys[i]);
    if (val) {
      printf("  search(\"%s\") -> \"%s\"\n", keys[i], val);
    } else {
      printf("  search(\"%s\") -> (not found)\n", keys[i]);
    }
  }

  /* ---- Update ---- */
  printf("\nUpdating \"city\" to \"Milan\"...\n");
  map->insert(map, "city", "Milan");
  printf("  search(\"city\") -> \"%s\"\n", map->search(map, "city"));

  /* ---- Delete ---- */
  printf("\nDeleting \"lang\"...\n");
  map->delete(map, "lang");
  char *val = map->search(map, "lang");
  printf("  search(\"lang\") -> %s\n", val ? val : "(not found)");

  /* ---- Cleanup ---- */
  map->destroy(map); /* map pointer is invalid after this call */
  printf("\nHash map destroyed. Done.\n");

  return 0;
}
