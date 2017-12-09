#include <string.h>

#include "stack.h"

struct sStack {
    void* data;
    size_t size;
    size_t datasize;
    int bot;
    int top;
};

stack_t* initStack(size_t size, size_t datasize) {
    stack_t* s = (stack_t*)calloc(1, sizeof(stack_t));
    s->data = calloc(size, datasize);
    s->size = size;
    s->datasize = datasize;
    s->top = s->bot = -1;
    return s;
}

size_t getStackSize(stack_t* stack) {
    return stack->size;
}

int getStackTop(stack_t* stack) {
    return stack->top;
}

int getStackBottom(stack_t* stack) {
    return stack->bot;
}

int push(stack_t* stack, void* data) {
    if (!stack->size || !data)
        return 0;
    stack->top = (stack->top + 1) % stack->size;
    if (stack->bot < 0 || stack->top == stack->bot)
        stack->bot = (stack->bot + 1) % stack->size;
    memcpy((char*)stack->data + stack->top*stack->datasize, data, stack->datasize);
    return 1;
}

int pop(stack_t* stack, void* out) {
    if (stack->top < 0 || !stack->size)
        return 0;
    if (out) {
        memcpy(out, (char*)stack->data + stack->top*stack->datasize, stack->datasize);
    }
    if (stack->top == stack->bot) {
        stack->top = stack->bot = -1;
    } else {
        stack->top = (stack->top - 1 + stack->size) % stack->size;
    }
    return 1;
}

void clearStack(stack_t* stack) {
    stack->top = stack->bot = -1;
}

void destroyStack(stack_t* stack) {
    free(stack->data);
    free(stack);
}
