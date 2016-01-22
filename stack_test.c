#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

#include "stack.h"

void test00_range(int from, int to) {
	stack_struct stack_;
	stack_struct *stack = &stack_;
	stack_new(stack);
	
	int i;
	for (i=from; i<to; ++i) {
		assert(!stack_push(stack, i));
	}
	
	stack_iterator_struct it;
	
	it = stack_get_iterator(stack);
	while (stack_iterator_is_not_null(it)) {
		int element = stack_iterator_get_element(it);
		printf("%d ", element);
		it = stack_iterator_next(it);
	}
	printf("\n");
	
	it = stack_get_iterator(stack);
	while (stack_iterator_is_not_null(it)) {
		int element = stack_iterator_get_element(it);
		if ((element % 3) == 0) {
			it = stack_iterator_remove_and_next(it);
		} else {
			it = stack_iterator_next(it);
		}
	}
	
	it = stack_get_iterator(stack);
	while (stack_iterator_is_not_null(it)) {
		int element = stack_iterator_get_element(it);
		printf("%d ", element);
		it = stack_iterator_next(it);
	}
	printf("\n");
}

void test00() {
	test00_range(0, 10);
	test00_range(0, 11);
	test00_range(0, 12);
	test00_range(0, 13);
	
	test00_range(1, 10);
	test00_range(1, 11);
	test00_range(1, 12);
	test00_range(1, 13);
	
	test00_range(2, 10);
	test00_range(2, 11);
	test00_range(2, 12);
}

/*********************/

#define vector_data_size (1024*1024)
int vector_data[vector_data_size];
unsigned int vector_size;
#define MARK_TO_REMOVE_VALUE INT_MIN

typedef int vector_iterator_struct;

void vector_init() {
	vector_size = 0;
}

bool vector_is_empty() {
	return vector_size == 0;
}

bool vector_is_full() {
	return vector_size < vector_data_size;
}

void vector_push(int element) {
	assert(vector_size < vector_data_size);
	assert(element != MARK_TO_REMOVE_VALUE);
	vector_data[vector_size++] = element;
}

vector_iterator_struct vector_get_iterator() {
	return (int)vector_size - 1;
}

bool vector_iterator_is_null(vector_iterator_struct iterator) {
	return iterator < 0;
}

bool vector_iterator_is_not_null(vector_iterator_struct iterator) {
	return iterator >= 0;
}

int vector_iterator_get_element(vector_iterator_struct iterator) {
	return vector_data[iterator];
}

vector_iterator_struct vector_iterator_next(vector_iterator_struct iterator) {
	return iterator - 1;
}

void vector_iterator_mark_to_remove(vector_iterator_struct iterator) {
	vector_data[iterator] = MARK_TO_REMOVE_VALUE;
}

void vector_reduce() {
	unsigned int i, j;
	for (i=0, j=0; i<vector_size; ++i) {
		if (vector_data[i] != MARK_TO_REMOVE_VALUE) {
			if (j != i) vector_data[j] = vector_data[i];
			++j;
		}
	}
	vector_size = j;
}

/*********************/

void compare_stack_and_vector(stack_struct *stack) {
	stack_iterator_struct  s_it = stack_get_iterator(stack);
	vector_iterator_struct v_it = vector_get_iterator();
	while (stack_iterator_is_not_null(s_it) && vector_iterator_is_not_null(v_it)) {
		int s_el =  stack_iterator_get_element(s_it);
		int v_el = vector_iterator_get_element(v_it);
		//printf("s_el = %d, v_el = %d\n", s_el, v_el);
		assert(s_el == v_el);
		s_it =  stack_iterator_next(s_it);
		v_it = vector_iterator_next(v_it);
	}
	assert( stack_iterator_is_null(s_it));
	assert(vector_iterator_is_null(v_it));
}

void remove_random_elements_from_stack_and_vector(stack_struct *stack) {
	stack_iterator_struct  s_it = stack_get_iterator(stack);
	vector_iterator_struct v_it = vector_get_iterator();
	while (stack_iterator_is_not_null(s_it) && vector_iterator_is_not_null(v_it)) {
		int s_el =  stack_iterator_get_element(s_it);
		int v_el = vector_iterator_get_element(v_it);
		assert(s_el == v_el);
		if (rand() & 1) {
			s_it = stack_iterator_remove_and_next(s_it);
			vector_iterator_mark_to_remove(v_it);
		} else {
			s_it =  stack_iterator_next(s_it);
		}
		v_it = vector_iterator_next(v_it);

	}
	assert( stack_iterator_is_null(s_it));
	assert(vector_iterator_is_null(v_it));
	vector_reduce();
}

void test01(unsigned int count) {
	assert(count <= vector_data_size);
	
	stack_struct stack_;
	stack_struct *stack = &stack_;
	stack_new(stack);
	
	vector_init();
	
	unsigned int i;
	int el;
	for (i=0; i<count; ++i) {
		el = (rand() << 1) | rand();
		//printf("el = %d\n", el);
		assert(!stack_push(stack, el));
		vector_push(el);
	}
	
	compare_stack_and_vector(stack);
	
	stack_clear(stack);
}

void test02(unsigned int elements_count, unsigned int remove_loops_count) {
	assert(elements_count <= vector_data_size);
	
	stack_struct stack_;
	stack_struct *stack = &stack_;
	stack_new(stack);
	
	vector_init();
	
	unsigned int i;
	int el;
	for (i=0; i<elements_count; ++i) {
		el = (rand() << 1) | rand();
		//printf("el = %d\n", el);
		assert(!stack_push(stack, el));
		vector_push(el);
	}
	
	for (i=0; i<remove_loops_count; ++i) {
		remove_random_elements_from_stack_and_vector(stack);
	}
	if (vector_is_empty()) printf("empty\n");
	compare_stack_and_vector(stack);
	
	stack_clear(stack);
}

int main() {
	unsigned int seed = (unsigned int)time(NULL);
	printf("seed = %u\n", seed);
	srand(seed);
	
	unsigned int i;
	for (i=0; i<256; ++i) {
		printf("%d\n", i);
		test02(1024*64, 16);
	}
	
	printf("Success\n");
	return 0;
}

