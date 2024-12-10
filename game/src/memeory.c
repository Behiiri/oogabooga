#include <stdlib.h>

typedef unsigned int  uint32;
typedef unsigned char uint8;

typedef uint32 marker;

void stack_init(uint32 stack_size_bytes);
// void* stack_alloc(uint32 size_bytes);
marker get_marker();
void free_to_marker();
void clear();

typedef struct {
    uint8 *stack;     // Pointer to the memory block
    size_t size;      // Total size of the stack
    size_t top;       // Current top of the stack
} stack_allocator;

stack_allocator* create_stack_allocator(size_t size) {
    stack_allocator *allocator = malloc(sizeof(stack_allocator));
    if (!allocator) {
        return NULL;
    }

    allocator->stack = malloc(size);
    if (!allocator->stack) {
        free(allocator);
        return NULL;
    }

    allocator->size = size;
    allocator->top = 0;
    return allocator;
}

void* stack_alloc(stack_allocator *allocator, size_t size_bytes) {
    if (allocator->top + size_bytes > allocator->size) {
        return NULL;
    }

    void *ptr = allocator->stack + allocator->top;
    allocator->top += size_bytes;
    return ptr;
}

void stack_reset(stack_allocator *allocator) {
    allocator->top = 0;
}

void destroy_stack_allocator(stack_allocator *allocator) {
    if (allocator) {
        free(allocator->stack);
        free(allocator);
    }
}


