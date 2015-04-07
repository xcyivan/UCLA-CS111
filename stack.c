#include "stack.h"
#include "alloc.h"
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

stack
create_stack ()
{
    stack s;

    s = (stack)checked_malloc(sizeof(struct node));
    make_empty(s);
    return s;
}

int
is_empty (stack s)
{
    return s->next == NULL;
}

void
make_empty (stack s)
{
    if (s == NULL)
  	    error (1, 0, "Must use create_stack first");
    else
  	    while (!is_empty(s))
  	        pop(s);	
}

void
push (void* element, stack s)
{
	node_t tmp_node;

	tmp_node = (node_t)checked_malloc(sizeof(struct node));
	tmp_node->element = element;
	tmp_node->next = s->next;
	s->next = tmp_node;
}

void* 
pop (stack s)
{
	node_t first_cell;

	if (is_empty(s)) 
	{
		error (1, 0, "Empty stack");
		return 0;
	}
	else
	{
		first_cell = s->next;
		s->next = s->next->next;
		void* element = first_cell->element;
		free (first_cell);
		return element;
	}
}

void*
peer (stack s)
{
	if (!is_empty(s))
		return s->next->element;
	error (1, 0, "Empty stack");
	return 0;
}
