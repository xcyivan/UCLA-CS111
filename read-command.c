// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
/*  Stack data structure  */
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
peer (stack)
{
  if (!is_empty(s))
    return s->next->element;
  error (1, 0, "Empty stack");
  return 0;
}

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct commandNode {
    struct command *command;
    struct commandNode *next;
};

struct command_stream {
    struct commandNode *head;
    struct commandNode *tail;
    struct commandNode *cursor;
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
