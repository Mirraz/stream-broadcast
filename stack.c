#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "stack.h"

void stack_new(stack_struct *stack) {
	stack->head = NULL;
}

static bool stack_is_empty(const stack_struct *stack) {
	return stack->head == NULL;
}

int stack_push(stack_struct *stack, int element) {
	stack_node_struct *node = malloc(sizeof(stack_node_struct));
	if (node == NULL) {perror("malloc"); return -1;}
	node->element = element;
	node->next = stack->head;
	stack->head = node;
	return 0;
}

static int stack_pop(stack_struct *stack) {
	assert(!stack_is_empty(stack));
	stack_node_struct *node = stack->head;
	stack->head = node->next;
	int element = node->element;
	free(node);
	return element;
}

static int stack_front(const stack_struct *stack) {
	assert(!stack_is_empty(stack));
	return stack->head->element;
}

void stack_clear(stack_struct *stack) {
	while (!stack_is_empty(stack)) stack_pop(stack);
}

stack_iterator_struct stack_get_iterator(stack_struct *stack) {
	stack_iterator_struct iterator;
	iterator.ref = &stack->head;
	return iterator;
}

bool stack_iterator_is_null(stack_iterator_struct iterator) {
	stack_node_struct *node = *(iterator.ref);
	return node == NULL;
}

bool stack_iterator_is_not_null(stack_iterator_struct iterator) {
	stack_node_struct *node = *(iterator.ref);
	return node != NULL;
}

int stack_iterator_get_element(stack_iterator_struct iterator) {
	stack_node_struct *node = *(iterator.ref);
	assert(node != NULL);
	return node->element;
}

stack_iterator_struct stack_iterator_next(stack_iterator_struct iterator) {
	stack_node_struct *node = *(iterator.ref);
	assert(node != NULL);
	stack_iterator_struct new_iterator;
	new_iterator.ref = &(node->next);
	return new_iterator;
}

stack_iterator_struct stack_iterator_remove_and_next(stack_iterator_struct iterator) {
	stack_node_struct *node = *(iterator.ref);
	assert(node != NULL);
	*iterator.ref = node->next;
	free(node);
	return iterator;
}

