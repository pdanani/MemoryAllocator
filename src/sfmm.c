#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "sfmm.h"

#define MROW 8
#define ALIGN 16
#define BLOCK 32

static int initialized = 0;

/* Forward declarations */
sf_block* getBlockFooter(sf_block* block);
size_t getAllocatedSize(sf_block* block);
void updateBlockFooter(sf_block* block);
void insertBlockIntoFreeList(sf_block* block);
void coalesce(sf_block* block);

sf_block* getBlockFooter(sf_block* block) {
    return (sf_block*)((void*)block + (block->header & BLOCK_SIZE_MASK) - sizeof(sf_block));
}

size_t getAllocatedSize(sf_block* block) {
    return (block->header & BLOCK_SIZE_MASK) - sizeof(sf_block);
}

void updateBlockFooter(sf_block* block) {
    sf_block* footer = getBlockFooter(block);
    footer->header = block->header ^ sf_magic();
}

void insertBlockIntoFreeList(sf_block* block) {
    int index = getFreeListIndex(block->header & BLOCK_SIZE_MASK);
    sf_block* free_list_head = &sf_free_list_heads[index];

    block->body.links.next = free_list_head->body.links.next;
    block->body.links.prev = free_list_head;
    free_list_head->body.links.next->body.links.prev = block;
    free_list_head->body.links.next = block;
}

void coalesce(sf_block* block) {
    sf_block* current_block = block;
    int deciphered = current_block->prev_footer ^ sf_magic();
    void* temp = (void*)current_block - (deciphered & BLOCK_SIZE_MASK);
    sf_block* prev_block = (sf_block*)temp;
    temp = (void*)current_block + (current_block->header & BLOCK_SIZE_MASK);
    sf_block* next_block = (sf_block*)temp;
    sf_block* new_ftr = next_block;

    if (((current_block->header) & 1) == 1 && ((next_block->header & 2) == 2)) {
        if ((current_block->header & 2) == 2) {
            current_block->header -= 2;
            next_block->prev_footer = (current_block->header) ^ sf_magic();
        }
        if ((next_block->header & 1) == 1) {
            next_block->header -= 1;
            void* tran = (void*)new_ftr + (next_block->header & BLOCK_SIZE_MASK);
            new_ftr = tran;
            new_ftr->prev_footer = next_block->header ^ sf_magic();
        }

        int pick = 0;
        int remainder = current_block->header & BLOCK_SIZE_MASK;
        if (remainder <= 32)
            pick = 0;
        if (remainder > 32 && remainder <= 64)
            pick = 1;
        if (remainder > 64 && remainder <= 128)
            pick = 3;
        if (remainder > 256 && remainder <= 512)
            pick = 4;
        if (remainder > 512 && remainder <= 1024)
            pick = 5;
        if (remainder > 1024 && remainder <= 2048)
            pick = 6;
        if (remainder > 2048 && remainder <= 4096)
            pick = 7;
        if (remainder > 4096)
            pick = 8;

        current_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        current_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = current_block;
        sf_free_list_heads[pick].body.links.next = current_block;
    }

    if (((prev_block->header & 2) == 2) && ((next_block->header & 2) == 0)) {
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        void* tran = (char*)new_ftr + (next_block->header & BLOCK_SIZE_MASK);
        new_ftr = tran;
        prev_block->header = (prev_block->header & BLOCK_SIZE_MASK) +
            (current_block->header & BLOCK_SIZE_MASK) + 1;

        current_block->header = 0;
        if ((next_block->header & 1) == 1) {
            next_block->header -= 1;
            new_ftr->prev_footer = next_block->header ^ sf_magic();
        }

        current_block->prev_footer = 0;

        next_block->prev_footer = prev_block->header ^ sf_magic();

        int pick = 0;
        int remainder = prev_block->header & BLOCK_SIZE_MASK;
        if (remainder <= 32)
            pick = 0;
        if (remainder > 32 && remainder <= 64)
            pick = 1;
        if (remainder > 64 && remainder <= 128)
            pick = 3;
        if (remainder > 256 && remainder <= 512)
            pick = 4;
        if (remainder > 512 && remainder <= 1024)
            pick = 5;
        if (remainder > 1024 && remainder <= 2048)
            pick = 6;
        if (remainder > 2048 && remainder <= 4096)
            pick = 7;
        if (remainder > 4096)
            pick = 8;

        prev_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        prev_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = prev_block;
        sf_free_list_heads[pick].body.links.next = prev_block;
    }

    if (((prev_block->header & 2) == 2) && (next_block->header & 2) == 0) {
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;

        void* tran = (void*)new_ftr + (next_block->header & BLOCK_SIZE_MASK);
        new_ftr = tran;
        current_block->header = (current_block->header & BLOCK_SIZE_MASK) +
            (next_block->header & BLOCK_SIZE_MASK) + 1;

        next_block->header = 0;
        next_block->prev_footer = 0;

        new_ftr->prev_footer = current_block->header ^ sf_magic();

        int pick = 0;
        int remainder = current_block->header & BLOCK_SIZE_MASK;
        if (remainder <= 32)
            pick = 0;
        if (remainder > 32 && remainder <= 64)
            pick = 1;
        if (remainder > 64 && remainder <= 128)
            pick = 3;
        if (remainder > 256 && remainder <= 512)
            pick = 4;
        if (remainder > 512 && remainder <= 1024)
            pick = 5;
        if (remainder > 1024 && remainder <= 2048)
            pick = 6;
        if (remainder > 2048 && remainder <= 4096)
            pick = 7;
        if (remainder > 4096)
            pick = 8;

        current_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        current_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = current_block;
        sf_free_list_heads[pick].body.links.next = current_block;
    }

    if (((prev_block->header & 2) == 0) && ((next_block->header & 2) == 2)) {
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        void* tran = (void*)new_ftr + (next_block->header & BLOCK_SIZE_MASK);
        new_ftr = tran;
        prev_block->header = (prev_block->header & BLOCK_SIZE_MASK) +
            (current_block->header & BLOCK_SIZE_MASK) + 1;

        current_block->header = 0;

        new_ftr->prev_footer = prev_block->header ^ sf_magic();

        int pick = 0;
        int remainder = prev_block->header & BLOCK_SIZE_MASK;
        if (remainder <= 32)
            pick = 0;
        if (remainder > 32 && remainder <= 64)
            pick = 1;
        if (remainder > 64 && remainder <= 128)
            pick = 3;
        if (remainder > 256 && remainder <= 512)
            pick = 4;
        if (remainder > 512 && remainder <= 1024)
            pick = 5;
        if (remainder > 1024 && remainder <= 2048)
            pick = 6;
        if (remainder > 2048 && remainder <= 4096)
            pick = 7;
        if (remainder > 4096)
            pick = 8;

        prev_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        prev_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = prev_block;
        sf_free_list_heads[pick].body.links.next = prev_block;
    }

    if (((prev_block->header & 2) == 0) && ((next_block->header & 2) == 0)) {
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;

        void* tran = (void*)new_ftr + (next_block->header & BLOCK_SIZE_MASK);
        new_ftr = tran;
        prev_block->header = (prev_block->header & BLOCK_SIZE_MASK) +
            (current_block->header & BLOCK_SIZE_MASK) +
            (next_block->header & BLOCK_SIZE_MASK) + 1;

        current_block->header = 0;
        next_block->header = 0;

        new_ftr->prev_footer = prev_block->header ^ sf_magic();

        int pick = 0;
        int remainder = prev_block->header & BLOCK_SIZE_MASK;
        if (remainder <= 32)
            pick = 0;
        if (remainder > 32 && remainder <= 64)
            pick = 1;
        if (remainder > 64 && remainder <= 128)
            pick = 3;
        if (remainder > 256 && remainder <= 512)
            pick = 4;
        if (remainder > 512 && remainder <= 1024)
            pick = 5;
        if (remainder > 1024 && remainder <= 2048)
            pick = 6;
        if (remainder > 2048 && remainder <= 4096)
            pick = 7;
        if (remainder > 4096)
            pick = 8;

        prev_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        prev_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = prev_block;
        sf_free_list_heads[pick].body.links.next = prev_block;
    }
}

void *sf_malloc(size_t size) {
    if (size == 0 || size > (PAGE_SZ * 4)) {
        sf_errno = EINVAL;
        return NULL;
    }

    if (!initialized) {
        sf_mem_init();
        initialized = 1;
    }

    size_t adjusted_size = size + sizeof(sf_header) + sizeof(sf_footer);
    if (adjusted_size < BLOCK) {
        adjusted_size = BLOCK;
    }
    if (adjusted_size % ALIGN != 0) {
        adjusted_size = adjusted_size + (ALIGN - (adjusted_size % ALIGN));
    }

    size_t padding = adjusted_size - size;

    int list_index = 0;
    size_t remainder = adjusted_size;
    if (remainder <= 32)
        list_index = 0;
    if (remainder > 32 && remainder <= 64)
        list_index = 1;
    if (remainder > 64 && remainder <= 128)
        list_index = 2;
    if (remainder > 128 && remainder <= 256)
        list_index = 3;
    if (remainder > 256 && remainder <= 512)
        list_index = 4;
    if (remainder > 512 && remainder <= 1024)
        list_index = 5;
    if (remainder > 1024 && remainder <= 2048)
        list_index = 6;
    if (remainder > 2048 && remainder <= 4096)
        list_index = 7;
    if (remainder > 4096)
        list_index = 8;

    sf_block* curr_free_block = &sf_free_list_heads[list_index];
    sf_block* alloc_block = NULL;

    while (curr_free_block != &sf_free_list_heads[list_index] || curr_free_block->body.links.next != &sf_free_list_heads[list_index]) {
        curr_free_block = curr_free_block->body.links.next;
        size_t block_size = curr_free_block->header & BLOCK_SIZE_MASK;

        if (block_size >= adjusted_size) {
            alloc_block = curr_free_block;
            alloc_block->body.links.prev->body.links.next = alloc_block->body.links.next;
            alloc_block->body.links.next->body.links.prev = alloc_block->body.links.prev;

            if (block_size >= (adjusted_size + BLOCK)) {
                size_t new_block_size = block_size - adjusted_size;
                sf_block* new_free_block = (sf_block*)((void*)alloc_block + adjusted_size);

                new_free_block->header = new_block_size | 2;
                new_free_block->prev_footer = (new_block_size | 2) ^ sf_magic();

                sf_block* footer = getBlockFooter(new_free_block);
                footer->header = new_free_block->header ^ sf_magic();

                insertBlockIntoFreeList(new_free_block);
            }

            alloc_block->header = adjusted_size | 1;
            alloc_block->prev_footer = (adjusted_size | 1) ^ sf_magic();

            sf_block* footer = getBlockFooter(alloc_block);
            footer->header = alloc_block->header ^ sf_magic();

            break;
        }
    }

    if (alloc_block == NULL) {
        sf_block* new_page = sf_mem_grow();
        if (new_page == NULL) {
            sf_errno = ENOMEM;
            return NULL;
        }

        new_page->header = PAGE_SZ | 3;
        new_page->prev_footer = (PAGE_SZ | 3) ^ sf_magic();

        sf_block* new_footer = getBlockFooter(new_page);
        new_footer->header = new_page->header ^ sf_magic();

        coalesce(new_page);

        alloc_block = sf_malloc(size);
    }

    void* payload_ptr = (void*)alloc_block + sizeof(sf_header);

    return payload_ptr;
}

void sf_free(void *ptr) {
    if (ptr == NULL) {
        abort();
    }

    sf_block* block = (sf_block*)((void*)ptr - sizeof(sf_header));
    if (((block->header) & 1) == 0 || ((block->prev_footer ^ sf_magic()) & 2) == 0) {
        abort();
    }

    block->header -= 1;
    sf_block* footer = getBlockFooter(block);
    footer->header = block->header ^ sf_magic();

    insertBlockIntoFreeList(block);

    coalesce(block);
}

void *sf_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return sf_malloc(size);
    }

    if (size == 0) {
        sf_free(ptr);
        return NULL;
    }

    sf_block* block = (sf_block*)((void*)ptr - sizeof(sf_header));
    if (((block->header) & 1) == 0 || ((block->prev_footer ^ sf_magic()) & 2) == 0) {
        abort();
    }

    size_t allocated_size = getAllocatedSize(block);

    if (size <= allocated_size) {
        size_t padding = allocated_size - size;
        if (padding >= BLOCK) {
            size_t new_block_size = size + sizeof(sf_header) + sizeof(sf_footer);
            sf_block* new_free_block = (sf_block*)((void*)block + new_block_size);

            new_free_block->header = padding | 2;
            new_free_block->prev_footer = (padding | 2) ^ sf_magic();

            sf_block* footer = getBlockFooter(new_free_block);
            footer->header = new_free_block->header ^ sf_magic();

            insertBlockIntoFreeList(new_free_block);

            block->header = size | 1;
            block->prev_footer = (size | 1) ^ sf_magic();

            footer = getBlockFooter(block);
            footer->header = block->header ^ sf_magic();
        }

        return ptr;
    } else {
        void* new_ptr = sf_malloc(size);
        if (new_ptr == NULL) {
            return NULL;
        }

        memcpy(new_ptr, ptr, allocated_size - sizeof(sf_header));

        sf_free(ptr);

        return new_ptr;
    }
}
