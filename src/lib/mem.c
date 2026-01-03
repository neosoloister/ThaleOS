#include "mem.h"
#include "kprintf.h"
#include <stdbool.h>

#define HEAP_SIZE 1024 * 1024 * 1024

extern char end[];

typedef struct Header {
    struct Header *next;
    uint32_t size;
    bool is_free;
} Header;

Header *head = NULL;

void init_mem() {
    head = (Header *)end;

    head->size = HEAP_SIZE - sizeof(Header);
    head->is_free = true;
    head->next = NULL;
}

void *malloc(uint32_t size) {
    Header *curr = head;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size > size + sizeof(Header)) {
                Header *next = (Header *)((char *)(curr + 1) + size);
                next->size = curr->size - size - sizeof(Header);
                next->is_free = true;
                next->next = curr->next;

                curr->size = size;
                curr->next = next;
            }
            curr->is_free = false;
            return (void *)(curr + 1);
        }
        curr = curr->next;
    }
    return NULL;
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    Header *header = (Header *)ptr - 1;
    header->is_free = true;

    Header *curr = head;
    while (curr != NULL && curr->next != NULL) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(Header) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void mem_dump() {
    Header *curr = head;
    kprintf("Heap Dump:\n");
    int index = 0;
    while (curr != NULL) {
        kprintf("(%d)Block at %p, size: %d, is_free: %d\n", index, curr, curr->size, curr->is_free);
        curr = curr->next;
        index++;
    }
}