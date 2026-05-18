#include <stdio.h>
#include "hash_map.h"

int main(void) {
    /* Create a new hash map */
    hash_map* map = map_new();
    printf("Hash map created.\n\n");

    /* ---- Insert ---- */
    map_insert(map, "name",    "Alice");
    map_insert(map, "city",    "Rome");
    map_insert(map, "lang",    "C");
    map_insert(map, "version", "C11");
    printf("Inserted 4 key-value pairs.\n");

    /* ---- Search ---- */
    printf("\nSearching for keys:\n");
    const char* keys[] = {"name", "city", "lang", "version", "missing"};
    for (int i = 0; i < 5; i++) {
        char* val = map_search(map, keys[i]);
        if (val) {
            printf("  search(\"%s\") -> \"%s\"\n", keys[i], val);
        } else {
            printf("  search(\"%s\") -> (not found)\n", keys[i]);
        }
    }

    /* ---- Update ---- */
    printf("\nUpdating \"city\" to \"Milan\"...\n");
    map_insert(map, "city", "Milan");
    printf("  search(\"city\") -> \"%s\"\n", map_search(map, "city"));

    /* ---- Delete ---- */
    printf("\nDeleting \"lang\"...\n");
    map_delete(map, "lang");
    char* val = map_search(map, "lang");
    printf("  search(\"lang\") -> %s\n", val ? val : "(not found)");

    /* ---- Cleanup ---- */
    map_del(map);
    printf("\nHash map destroyed. Done.\n");

    return 0;
}
