#ifndef _STACK_H
#define _STACK_H

struct node;
typedef struct node *node_t;
typedef node_t stack;

int is_empty(stack s);
stack create_stack();
void make_empty(stack s);
void push(void* element, stack s);
void* pop(stack s);
void* peer(stack s);

struct node{
	void* element;
	node_t next;
};



#endif