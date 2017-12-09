#ifndef STACK_H
#define STACK_H

#include <stdlib.h>

typedef struct sStack stack_t;

stack_t* initStack(size_t size, size_t datasize);

size_t getStackSize(stack_t* stack);
int getStackTop(stack_t* stack);
int getStackBottom(stack_t* stack);

int push(stack_t* stack, void* data);
int pop(stack_t* stack, void* out);
void clearStack(stack_t* stack);
void destroyStack(stack_t* stack);

#endif // STACK_H