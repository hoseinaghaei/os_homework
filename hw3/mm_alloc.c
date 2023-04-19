/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

s_block_ptr heap_start = NULL;

void *mm_malloc(size_t size) {
    if (size <= 0) {
        return NULL;
    }
    s_block_ptr current_block = heap_start;
    s_block_ptr last_block = heap_start;

    while (current_block != NULL) {
        if (current_block->is_free && (current_block->size >= size)) {
            split_block(current_block, size);
            current_block->is_free = 0;
            return current_block->ptr;
        }
        last_block = current_block;
        current_block = current_block->next;
    }

    s_block_ptr new_block = extend_heap(last_block, size);
    if (new_block != NULL) {
        new_block->is_free = 0;
        memset(new_block->ptr, 0, size);
        return new_block->ptr;
    }
    return NULL;
}

void *mm_realloc(void *ptr, size_t size) {
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    s_block_ptr block = get_block(ptr);
    if (block == NULL) {
        return NULL;
    }
    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    size_t size_to_copy = size <= block->size ? size : block->size;
    memcpy(new_ptr, block->ptr, size_to_copy);
    mm_free(block);
    return new_ptr;
}

void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    s_block_ptr block = get_block(ptr);
    if (block == NULL) {
        return;
    }

    block->is_free = 1;
    fusion(block);
}

void split_block(s_block_ptr b, size_t s) {
    if (b->size >= s + BLOCK_SIZE) {
        s_block_ptr new_block = (s_block_ptr) ((char *) b + s + BLOCK_SIZE);
        new_block->size = b->size - s - BLOCK_SIZE;
        new_block->is_free = 1;
        new_block->next = b->next;
        new_block->prev = b;
        new_block->ptr = (char *) (new_block + BLOCK_SIZE);

        if (b->next != NULL) {
            b->next->prev = new_block;
        }
        b->size = s;
        b->next = new_block;
    }
}

s_block_ptr extend_heap(s_block_ptr last, size_t s) {
    s_block_ptr new_block = (s_block_ptr) sbrk(s + BLOCK_SIZE);
    if (new_block == (void *) -1) {
        return NULL;
    }
    new_block->size = s;
    new_block->is_free = 1;
    if (last != NULL) {
        last->next = new_block;
        new_block->prev = last;
    } else {
        new_block->prev = NULL;
        heap_start = new_block;
    }
    new_block->ptr = new_block + 1;
    return new_block;
}

s_block_ptr get_block(void *p) {
    s_block_ptr current_block = heap_start;
    while (current_block != NULL) {
        if (current_block->ptr == p) {
            return current_block;
        }
        current_block = current_block->next;
    }
    return NULL;
}

void fusion(s_block_ptr b) {
    if (b->prev != NULL && b->prev->is_free) {
        b->prev->size += b->size + BLOCK_SIZE;
        b->prev->next = b->next;
        if (b->next != NULL) {
            b->next->prev = b->prev;
        }
        if (b->next != NULL && b->next->is_free) {
            b->prev->size += b->next->size + BLOCK_SIZE;
            b->prev->next = b->next->next;
            if (b->next->next != NULL) {
                b->next->next->prev = b->prev;
            }
        }
    } else if (b->next != NULL && b->next->is_free) {
        b->size += b->next->size + BLOCK_SIZE;
        b->next = b->next->next;
        if (b->next->next != NULL) {
            b->next->next->prev = b;
        }
    }
    memset(b->ptr, 0, b->size);
}

