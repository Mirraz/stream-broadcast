#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

struct stack_node_struct_;
typedef struct stack_node_struct_ stack_node_struct;
struct stack_node_struct_ {
	int element;
	stack_node_struct *next;
};

typedef struct {
	stack_node_struct *head;
} stack_struct;

typedef struct {
	stack_node_struct **ref;
} stack_iterator_struct;

void stack_new(stack_struct *stack);
int stack_push(stack_struct *stack, int element);
void stack_clear(stack_struct *stack);

stack_iterator_struct stack_get_iterator(stack_struct *stack);

bool stack_iterator_is_null(stack_iterator_struct iterator);
bool stack_iterator_is_not_null(stack_iterator_struct iterator);
int stack_iterator_get_element(stack_iterator_struct iterator);
stack_iterator_struct stack_iterator_next(stack_iterator_struct iterator);
stack_iterator_struct stack_iterator_remove_and_next(stack_iterator_struct iterator);

#endif/*STACK_H*/

