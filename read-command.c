// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include "stack.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct token {
  char *word; 
  int line_num;
};

typedef struct token *token_t;

void 
make_token (token_t *token_array, int token_index, char *buffer, int line_num, int *in_simple_comman_t)
{
  char *tmp = checked_malloc(sizeof (char) * 1024);
  memset(tmp, 0, 1024);
  strcpy(tmp, buffer);
  token_array[token_index] = checked_malloc(sizeof (token_t));
  token_array[token_index]->word = tmp;
  token_array[token_index]->line_num = line_num;
  *in_simple_comman_t = 0;
}

void
tokenize (const char *command_tree, token_t *token_array, int line_num) 
{
  int i, token_index;
  char buffer[1024];
  memset(buffer, 0, 1024);
  strcpy(buffer, "");

  int line_offset = 0;

  int in_simple_comman = 0;
  int in_subshell = 0;
  int line_offset_in_subshell = 0;
  int after_operator = 0;
  int in_comment = 0;
  for (i = 0, token_index = 0; i < strlen(command_tree); i++) 
  {
    char c1 = command_tree[i];
    // printf ("iteration %d buffer=%s c1=%c\n", i, buffer, c1);
    char c2;
    if (i == strlen(command_tree) - 1) c2 = ' ';
    else c2 = command_tree[i + 1];
    
    if (in_comment && c1 != '\n') continue;

    if (in_subshell && c1 != ')') {
      if (c1 == '\n') line_offset_in_subshell++;
      char tmp[2];
      tmp[0] = c1;
      tmp[1] = '\0';
      strcat (buffer, tmp);
      continue;
    }

    switch (c1)
    {
      case '#':
        in_comment = 1;
        continue;

      case ' ':
        if (!in_simple_comman) continue;
        else 
        {
          char tmp[2];
          tmp[0] = c1;
          tmp[1] = '\0';
          strcat (buffer, tmp);
        }
        break;

      case ';': 
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 1024);
        }
        make_token(token_array, token_index, ";", line_num + line_offset, &in_simple_comman);
        token_index++;
        break;

      case '\n':
        if (in_comment) {
          in_comment = 0;
          continue;
        }
        if (strlen (buffer) > 0) 
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 1024);
        }
        if (!after_operator)
          make_token(token_array, token_index, ";", line_num + line_offset, &in_simple_comman);
        else 
        {
          if (c2 == '\n') continue;
          else 
          {
            after_operator = 0;
            continue;
          }
        }
        token_index++;
        line_offset++;
        break;

      case '|': 
        if (c2 == '|') 
        {
          if (strlen (buffer) > 0)
          {
            make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
            token_index++;
            memset(buffer, 0, 1024);
          }
          make_token(token_array, token_index, "||", line_num + line_offset, &in_simple_comman);
          token_index++;
          i++;
        } 
        else 
        {
          if (strlen (buffer) > 0)
          {
            make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
            token_index++;
            memset(buffer, 0, 1024);
          }
          make_token(token_array, token_index, "|", line_num + line_offset, &in_simple_comman);
          token_index++;
        }
        after_operator = 1;
        break;

      case '&': 
        if (c2 == '&') 
        {
          if (strlen (buffer) > 0)
          {
            make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
            token_index++;
            memset(buffer, 0, 1024);
          }
          make_token(token_array, token_index, "&&", line_num + line_offset, &in_simple_comman);
          token_index++;
          i++;
        } 
        else 
        {
          char tmp[2];
          tmp[0] = c1;
          tmp[1] = '\0';
          strcat (buffer, tmp);
        }
        after_operator = 1;
        break;

      case '(': 
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 1024);
        }
        make_token(token_array, token_index, "(", line_num + line_offset, &in_simple_comman);
        token_index++;
        in_subshell = 1;
        break;

      case ')': 
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 1024);
          line_offset += line_offset_in_subshell;
          line_offset_in_subshell = 0;
        }
        make_token(token_array, token_index, ")", line_num + line_offset, &in_simple_comman);
        token_index++;
        in_subshell = 0;
        break;

      default: 
        {
          char tmp[2];
          tmp[0] = c1;
          tmp[1] = '\0';
          // printf("in default tmp=%s len=%lu\n", tmp, strlen(tmp));
          strcat(buffer, tmp);
          in_simple_comman = 1;
        }
        break;
        
    }


    
  }
  if (strlen (buffer) > 0) 
  {
    make_token(token_array, token_index, buffer, line_offset + line_num, &in_simple_comman);
    token_index++;
  }
  token_array[token_index] = checked_malloc(sizeof (token_t));
  token_array[token_index]->line_num = -1;

}

// /*  Stack data structure  */
// stack
// create_stack ()
// {
//     stack s;

//     s = (stack)checked_malloc(sizeof(struct node));
//     make_empty(s);
//     return s;
// }

// int
// is_empty (stack s)
// {
//     return s->next == NULL;
// }

// void
// make_empty (stack s)
// {
//     if (s == NULL)
//         error (1, 0, "Must use create_stack first");
//     else
//         while (!is_empty(s))
//             pop(s); 
// }

// void
// push (void* element, stack s)
// {
//   node_t tmp_node;

//   tmp_node = (node_t)checked_malloc(sizeof(struct node));
//   tmp_node->element = element;
//   tmp_node->next = s->next;
//   s->next = tmp_node;
// }

// void* 
// pop (stack s)
// {
//   node_t first_cell;

//   if (is_empty(s)) 
//   {
//     error (1, 0, "Empty stack");
//     return 0;
//   }
//   else
//   {
//     first_cell = s->next;
//     s->next = s->next->next;
//     void* element = first_cell->element;
//     free (first_cell);
//     return element;
//   }
// }

// void*
// peer (stack)
// {
//   if (!is_empty(s))
//     return s->next->element;
//   error (1, 0, "Empty stack");
//   return 0;
// }

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
  // get_next_byte(get_next_byte_argument);
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

