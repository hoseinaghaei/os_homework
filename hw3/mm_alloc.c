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
    if (size == 0) {
        return NULL;
    }
    s_block_ptr current_block = heap_start;
    s_block_ptr last_block = NULL;

    while (current_block != NULL) {
        if (current_block->is_free && current_block->size >= size) {
            split_block(current_block, size);
            current_block->is_free = 0;
            return current_block->ptr;
        }
        last_block = current_block;
        current_block = current_block->next;
    }

    return extend_heap(last_block, size);
}

void *mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    s_block_ptr block = get_block(ptr);
    if (block == NULL) {
        return NULL;
    }
    if (size <= block->size) {
        split_block(block, size);
        return block->ptr;
    }

    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    memcpy(new_ptr, block->ptr, block->size);
    mm_free(block->ptr);
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
    memset(block->ptr, 0, block->size);
    fusion(block);
}

void split_block(s_block_ptr b, size_t s) {
    if (b->size > s + BLOCK_SIZE) {
        s_block_ptr new_block = (s_block_ptr) (b->ptr + s);
        new_block->size = b->size - s - BLOCK_SIZE;
        new_block->is_free = 1;
        new_block->next = b->next;
        new_block->prev = b;
        new_block->ptr = b->ptr + s + BLOCK_SIZE;

        if (b->next != NULL) {
            b->next->prev = new_block;
        }
        b->size = s;
        b->next = new_block;
    }
}

void *extend_heap(s_block_ptr last, size_t s) {
    s_block_ptr new_block = (s_block_ptr) sbrk(s + BLOCK_SIZE);
    if (new_block == (void *) -1) {
        return NULL;
    }
    if (last != NULL) {
        last->next = new_block;
    } else {
        heap_start = new_block;
    }
    new_block->prev = last;
    new_block->next = NULL;
    new_block->is_free = 0;
    new_block->size = s;
    new_block->ptr = new_block + 1;
    memset(new_block->ptr, 0, s);
    return new_block->ptr;
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
        b = b->prev;
    }
    if (b->next != NULL && b->next->is_free) {
        b->size += b->next->size + BLOCK_SIZE;
        b->next = b->next->next;
        if (b->next != NULL) {
            b->next->prev = b;
        }
    }
}

