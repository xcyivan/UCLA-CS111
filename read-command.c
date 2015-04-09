// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include "stack.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

const int MAXLINE=100;

typedef struct Separation{
  char* m_cmd;
  int m_line;
}Separation;

typedef Separation* Separation_t;

// void makeSeparation(FILE* fp, Separation_t*, int (*get_next_byte) (void *));

struct token {
  char *word; 
  int line_num;
};

typedef struct token *token_t;

char* str_append(char* orig, char c){
  size_t sz = strlen(orig);
  char* str = malloc(sz+2);
  strcpy(str,orig);
  str[sz]=c;
  str[sz+1]='\0';
  return str;
}

void 
makeSeparation(FILE* fp, Separation_t* m_separation, int (*get_next_byte) (void *))
{
  char* word[MAXLINE];
  int atLine[MAXLINE];
  char* str="";
  char ch = get_next_byte(fp);
  char preCh;
  int countWord=0;
  int countParath=0;
  int countLine=1; //this parameter records the line number that the program is reading
  int newLineFlag=1; //this flag is used to skip white space after '\n'
  int newWordFlag=0;
  int headFlag=1;
  //Separation_t m_separation[MAXLINE];

  //this while is to slit the text by 2 or more '\n' and the split results are stored in array word
  while(ch!=EOF){

    //this if is to skip white space after '\n'
    if((ch==' '||ch=='\t') && newLineFlag) {ch=get_next_byte(fp);continue;}

    //this if is to get rid of comment starting with '#', while the tailing '\n' is preserved 
    if(ch=='#'){
      while(get_next_byte(fp)!='\n'){
        continue;
      }
      countLine++;
      ch=get_next_byte(fp);
      continue;
    }
    //this if is to extract the subshell expressed as ()
    if(ch=='('){
      int badLine = countLine;
      printf("enter subshell: badLine=%d\n",badLine);
      do{
        str = str_append(str,ch);
        if(ch=='(') countParath++;
        if(ch==')') countParath--;
        if(ch=='\n') countLine++;
        //printf("2:countLine=%d\n",countLine);
        ch=getc(fp);
      }while(countParath && ch!=EOF);
      if(ch==EOF){
        word[countWord]=malloc(sizeof(str)+1);
        memset(word[countWord],'\0',sizeof(str)+1);
        word[countWord]=str;
        atLine[countWord]=badLine;
        countWord++;
        str="";
        break;
      }
    }

    if(ch!='\n'){
      str = str_append(str,ch);
      newLineFlag=0;
      if(headFlag){
        atLine[0]=countLine;
        headFlag=0;
      }
    }
    else{
      newLineFlag=1;
      countLine++;
      //printf("3:countLine=%d\n",countLine);
      if(preCh=='\n' && strlen(str)>0){
        word[countWord]=malloc(sizeof(str)+1);
        memset(word[countWord],'\0',sizeof(str)+1);
        word[countWord]=str;
        countWord++;
        str="";
        newWordFlag=1;
      }
      else if(strlen(str)>0){
        str = str_append(str,ch);
        if(newWordFlag){
          atLine[countWord]=countLine-1;
          newWordFlag=0;
        }
        if(headFlag){
          atLine[0]=countLine;
          headFlag=0;

        }
      }
        //printf(" (%c at line %d, the str=%s, strlen=%d) ",ch,countLine,str, (int)strlen(str));
    }
    preCh=ch;
    ch = get_next_byte(fp);
  }
  //if the last word doesn't end with "\n\n", also add it in
  if(str!=""){
    word[countWord]=malloc(sizeof(str)+1);
    memset(word[countWord],'\0',sizeof(str)+1);
    word[countWord]=str;
    countWord++;
    newWordFlag=1;
  }
  //if the last word doesn't end with '\n', revise the bug of counting line
  if(atLine[countWord-1]==0)
    atLine[countWord-1]=countLine;
  fclose(fp);
  //==============end of read file====================


  //============================merge word with tailing operator=======================================
  int merge[countWord+1];
  int i=0;
  // for(i=0; i<countWord; i++){
  //  printf("word %d at Line %d is:\n%s",i,atLine[i],word[i]);
  // }
  // for(i=0; i<countWord; i++)
  //  printf("%d\n",atLine[i]);

  //delete tailing '\n'
  for(i=0; i<countWord; i++){
    //get rid of tailing '\n'
    size_t sz=strlen(word[i]);
    if(word[i][sz-1]=='\n'){
      word[i][sz-1]='\0';
      sz--;
    }
    //printf("w%d:\n%s",i,word[i]);
    //if ending with operator: '|' ';' '||' '&&' '<' '>', mark this word in array merge[]
    if(word[i][sz-1]=='|' || word[i][sz-1]==';' || word[i][sz-1]=='<' || word[i][sz-1]=='>' || (word[i][sz-1]=='&' && word[i][sz-2]=='&'))
      merge[i]=strlen(word[i]);
    else
      merge[i]=-strlen(word[i]);
    //printf("merge[%d]=%d\n",i,merge[i]);
  }
  //printf("4:countLine=%d\n",countLine);
  
  //recompute the total word in merged array
  int newCountWord=0;
  for(i=0; i<countWord; i++){
    if(merge[i]<0) newCountWord++;
    else{
      newCountWord++;
      while(merge[i]>0 && i<countWord)  i++;
    }
  }
  //printf("newCountWord=%d\n",newCountWord);

  //merge and copy from old array to new array 
  char* commandStr[newCountWord];
  int newAtLine[newCountWord];
  int ptrOld=0;
  int ptrNew=0;
  while(ptrOld<countWord && ptrNew<newCountWord){
    if(merge[ptrOld]<0) {
      //printf("hi: %d\n",ptrOld);
      commandStr[ptrNew] = malloc(abs(merge[ptrOld])+1);
      strcpy(commandStr[ptrNew],word[ptrOld]);
      newAtLine[ptrNew]=atLine[ptrOld];
    }
    else {
      int startPtr=ptrOld;
      size_t cpyLen=0;
      while(merge[ptrOld]>0 && ptrOld<countWord){
        cpyLen+=merge[ptrOld];
        ptrOld++;
      }
      if(ptrOld==countWord){
        //printf("\n\nhuge bug!!!\n\n");
        //printf("\ncpyLen=%d\n\n",(int)cpyLen);
        commandStr[ptrNew] = malloc(cpyLen+1);
        memset(commandStr[ptrNew],'\0',cpyLen+1);
        newAtLine[ptrNew]=atLine[startPtr];
        while(startPtr<=--ptrOld){
          strcat(commandStr[ptrNew],word[startPtr]);
          startPtr++;
        }
        break;
      }
      cpyLen+=abs(merge[ptrOld]);
      //printf("\ncpyLen=%d\n\n",(int)cpyLen);
      commandStr[ptrNew] = malloc(cpyLen+1);
      memset(commandStr[ptrNew],'\0',cpyLen+1);
      newAtLine[ptrNew]=atLine[startPtr];
      while(startPtr<=ptrOld){
        strcat(commandStr[ptrNew],word[startPtr]);
        startPtr++;
      }
    }
    ptrNew++;
    ptrOld++;
  }
  for(i=0;i<newCountWord;i++){
    //printf("1111: CMD%d at line %d is:\n%s",i,newAtLine[i],commandStr[i]);
    m_separation[i]=malloc(sizeof(Separation));
    m_separation[i]->m_cmd = malloc(strlen(commandStr[i])+1);
    strcpy(m_separation[i]->m_cmd,commandStr[i]);
    m_separation[i]->m_line=newAtLine[i];
    //printf("CMD%d at line %d is:\n%s",i,m_separation[i]->m_line,m_separation[i]->m_cmd);
  } 

  //write an end indicating element
  m_separation[newCountWord]=malloc(sizeof(Separation));
  m_separation[newCountWord]->m_cmd=malloc(strlen("this is the end")+1);
  strcpy(m_separation[newCountWord]->m_cmd, "this is the end");
  m_separation[newCountWord]->m_line=-1;
  //=================================end of merge word======================================================

  //return m_separation;
}

void 
make_token (token_t *token_array, int token_index, char *buffer, int line_num, int *in_simple_comman_t)
{
  char *tmp = malloc(sizeof (char) * 64);
  memset(tmp, 0, 64);
  strcpy(tmp, buffer);
  token_array[token_index] = malloc(sizeof (token_t));
  token_array[token_index]->word = tmp;
  token_array[token_index]->line_num = line_num;
  *in_simple_comman_t = 0;
}

void
tokenize (const char *command_tree, token_t *token_array, int line_num) 
{
  int i, token_index;
  char buffer[128];
  memset(buffer, 0, 128);
  strcpy(buffer, "");

  int line_offset = 0;

  int in_simple_comman = 0;
  int in_subshell = 0;
  int line_offset_in_subshell = 0;
  int after_operator = 0;
  int in_comment = 0;
  int in_first_line = 1;
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
      case '\t':
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
          memset(buffer, 0, 128);
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
          memset(buffer, 0, 128);
        }
        if (!after_operator && !in_first_line)
          make_token(token_array, token_index, ";", line_num + line_offset, &in_simple_comman);
        else 
        {
          if (c2 == '\n') {continue;}
          else 
          {
            after_operator = 0;
            line_offset++;
            continue;
          }
        }
        in_first_line = 0;
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
            memset(buffer, 0, 128);
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
            memset(buffer, 0, 128);
          }
          make_token(token_array, token_index, "|", line_num + line_offset, &in_simple_comman);
          token_index++;
        }
        in_first_line = 0;
        after_operator = 1;
        break;

      case '&': 
        if (c2 == '&') 
        {
          if (strlen (buffer) > 0)
          {
            make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
            token_index++;
            memset(buffer, 0, 128);
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
        in_first_line = 0;
        after_operator = 1;
        break;

      case '(': 
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 128);
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
          memset(buffer, 0, 128);
          line_offset += line_offset_in_subshell;
          line_offset_in_subshell = 0;
        }
        make_token(token_array, token_index, ")", line_num + line_offset, &in_simple_comman);
        token_index++;
        in_subshell = 0;
        break;

      case '>':
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 128);
        }
        make_token(token_array, token_index, ">", line_num + line_offset, &in_simple_comman);
        token_index++;
        in_first_line = 0;
        after_operator = 0;
        break;

      case '<':
        if (strlen (buffer) > 0)
        {
          make_token(token_array, token_index, buffer, line_num + line_offset, &in_simple_comman);
          token_index++;
          memset(buffer, 0, 128);
        }
        make_token(token_array, token_index, "<", line_num + line_offset, &in_simple_comman);
        token_index++;
        in_first_line = 0;
        after_operator = 0;
        break;

      default: 
        {
          char tmp[2];
          tmp[0] = c1;
          tmp[1] = '\0';
          // printf("in default tmp=%s len=%lu\n", tmp, strlen(tmp));
          strcat(buffer, tmp);
          in_simple_comman = 1;
          in_first_line = 0;
          after_operator = 0;
        }
        break;
        
    }


    
  }
  if (strlen (buffer) > 0) 
  {
    make_token(token_array, token_index, buffer, line_offset + line_num, &in_simple_comman);
    token_index++;
  }
  token_array[token_index] = malloc(sizeof (token_t));
  token_array[token_index]->line_num = -1;

}


void
syntax_error(int n)
{
  error(1, 0, "Line %d: Syntax Error", n);
}


int
is_char_validated (char *command) 
{
  int i;
  for (i = 0; i < strlen(command); i++)
  {
    char c = command[i];
    if ((c != '!' && c != '%' && c != '+' && c != ',' && c != '-' && 
      c != '.' && c != '/' && c != ':' && c != '@' && c != '^' && 
      c != '_') && !isalnum(c) && (c != '&' && c != '|' && c != '<' 
      && c != '>' && c != '(' && c != ')' && c != ';' && c != ' ' 
      && c != '\n' && c != '\t' ))
      return 0;
    else
    {
      i++;
    }
  }
  return 1;
}

int 
is_valid_operator(char *operand) 
{
  if (strcmp(operand, "&&") && strcmp(operand, "||") &&
    strcmp(operand, "|") && strcmp(operand, ";") && strcmp(operand, ">")
    && strcmp(operand, "<") && strcmp(operand, "(") && 
      strcmp(operand, ")"))
    return 0;
  else
    return 1;
}

int
is_valid_word(char *operand)
{
  int i;
  for (i = 0; i < strlen(operand); i++)
  {
    char c = operand[i];
    if ((c != '!' && c != '%' && c != '+' && c != ',' && c != '-' && 
      c != '.' && c != '/' && c != ':' && c != '@' && c != '^' && 
      c != '_') && !isalnum(c) && ( c != ' ' && 
      c != '\n' && c != '\t' ))
      return 0;
    else
    {
      i++;
    }
  }
  return 1;
}

int
validate (token_t *token_array)
{
  int i = 0;
  int res = 1;
  int in_subshell = 0;
  int token_array_len = 0;
  while (!(token_array[i]->line_num == -1)) {
    token_array_len++;
    i++;
  }

  i = 0;
  while (i < token_array_len && !(token_array[i]->line_num == -1)) 
  {
    char *operand = token_array[i]->word;

    // if (!is_char_validated(operand)) syntax_error(token_array[i]->line_num);
    if (!is_valid_operator(operand) && !is_valid_word(operand)) syntax_error(token_array[i]->line_num);

    if (operand[0] == '(') 
    {
      in_subshell = 1;
      if (i + 1 >= token_array_len) {
        res = 0;
        syntax_error(token_array_len);
      } 
      else 
      {
        if (token_array[i + 1]->word[0] != ')') 
        {
          token_t *sub_token_array = malloc(sizeof (token_t) * 128);
          tokenize (token_array[i+1]->word, sub_token_array, token_array[i+1]->line_num);

          // int j = 0;
          // printf("subshell\n" );
          // while (sub_token_array[j]->line_num != -1) {
          //   printf ("token %d\n  word:'%s' line_number:%d\n", j, sub_token_array[j]->word, sub_token_array[j]->line_num);
          //   j++;
          // }

          if (validate (sub_token_array))
          {
            i++;
          }
        }
      }
      i++;
      continue;
    } 

    if (operand[0] == ')')
    {
      if (i - 1 < 0) {
        res = 0;
        syntax_error(0);
      } 
      else 
      {
        if (!in_subshell) 
        {
          res = 0;
          // error_number = (i - 1 >= 0) ? token_array[i - 1]->line_num: token_array[i]->line_num;
          syntax_error(token_array[i]->line_num);
        }
        else
        {
          in_subshell = 0;
        }
      }
      i++;
      continue;
    }

    if (!strcmp(operand, ";") || !strcmp(operand, "||") || !strcmp(operand, "|") || !strcmp(operand, "&&")) 
    {
      if (i - 1 < 0) 
      {
        res = 0;
        syntax_error(token_array[i]->line_num);
      }
      // else 
      // {
      //  if 
      // }
      if (!strcmp(operand, ";")) {
        if (i < token_array_len - 1 && (!strcmp(token_array[i + 1]->word, "&&") 
          || !strcmp(token_array[i + 1]->word, "||") || !strcmp(token_array[i + 1]->word, "|") 
          || token_array[i + 1]->word[0] == '>' || token_array[i + 1]->word[0] == '<' || 
          !strcmp(token_array[i+1]->word, ";")))
        {
          res = 0;
          syntax_error(token_array[i]->line_num);
        }

        if (i >= token_array_len - 1) 
        {
          res = 0;
          syntax_error(token_array[i]->line_num);
        }

        if (i - 1 >= 0)
        {
          char *tmp = token_array[i - 1]->word;
          int j = strlen(tmp) - 1;
          while (j >= 0)
          {
            if (tmp[j] == ' ' || tmp[j] == '\t') {j--; continue;}
            if (tmp[j] == '>' || tmp[j] == '<')
            {
              res = 0;
              syntax_error(token_array[i - 1]->line_num);
            }
            j--;
          }
        }
      }
      else 
      {
        if (i < token_array_len - 1 && (!strcmp(token_array[i + 1]->word, "&&") 
          || !strcmp(token_array[i + 1]->word, "||") || !strcmp(token_array[i + 1]->word, "|") 
          ))
        {
          res = 0;
          syntax_error(token_array[i]->line_num);
        } 
        if (i >= token_array_len - 1) 
        {
          res = 0;
          syntax_error(token_array[i]->line_num);
        }
      }
      
    }

    if (!strcmp(operand, "<") || !strcmp(operand, ">"))
    {
      if (i == token_array_len - 1 || i == 0) 
      {
        res = 0;
        syntax_error(token_array[i]->line_num);
      } 
      else 
      {
        if (!strcmp(token_array[i+1]->word, ">") || !strcmp(token_array[i+1]->word, "<"))
        {
          res = 0;
          syntax_error(token_array[i]->line_num);
        }
      }
    }
    

    i++;  
  }

  return res;
}

int
validate_all (Separation_t *commands_str)
{
  int i = 0;
  int res = 1;
  token_t *token_array = malloc(sizeof (token_t) * 128);
  while (commands_str[i]->m_line != -1)
  {
    tokenize (commands_str[i]->m_cmd, token_array, commands_str[i]->m_line);
    // printf("%d\n", (int)token_array);
    if (!validate (token_array)) res = 0;
    i++;
    memset(token_array, 0, sizeof(token_t) * 128);
  }
  free(token_array);
  return res;
}


void
make_compound_command(command_t cmd1, command_t cmd2, char* oper, command_t output_cmd)
{
  enum command_type type;
  if (!strcmp(oper, ";")) 
    type = SEQUENCE_COMMAND;
  else if (!strcmp(oper, "||"))
    type = OR_COMMAND;
  else if (!strcmp(oper, "&&"))
    type = AND_COMMAND;
  else
    type = PIPE_COMMAND;
  output_cmd->type = type;
  output_cmd->status = -1;
  output_cmd->input = NULL;
  output_cmd->output = NULL;
  output_cmd->u.command[0] = cmd1;
  output_cmd->u.command[1] = cmd2;
}

void
make_subshell_command(command_t cmd, command_t output_cmd)
{
  output_cmd->type = SUBSHELL_COMMAND;
  output_cmd->status = -1;
  output_cmd->input = NULL;
  output_cmd->output = NULL;
  output_cmd->u.subshell_command = cmd;
}

char*
eat_tail_white_space(char* word)
{
  int len = strlen(word);
  int i;
  char *res;
  for (i = len - 1; word[i] == ' ' || word[i] == '\t'; i--);
  res = malloc(sizeof(char) * (i+2));
  memset(res, 0, i+2);
  int j;
  for (j = 0; j <= i; j++)
  {
    res[j] = word[j];
  }
  return res;
}

int
oper_cmp(char* token, char* oper)
{
  if (!strcmp(token, ";"))
  {
    if (!strcmp(oper, ";"))
      return 0;
    else 
      return -1;
  }
  else if (!strcmp(token, "&&") || !strcmp(token, "||"))
  {
    if (!strcmp(oper, ";"))
      return 1;
    else if (!strcmp(oper, "&&") || !!strcmp(oper, "||"))
      return 0;
    else 
      return -1;
  }
  else 
  {
    if (!strcmp(oper, ";") || !strcmp(oper, "&&") || !strcmp(oper, "||"))
      return 1;
    else 
      return 0;
  }
}

int  
make_simple_command(char* token, command_t cmd){
  cmd->input=NULL;
  cmd->output=NULL;
  cmd->status=-1;
  cmd->type=SIMPLE_COMMAND;

  char str[128][128];//temperarilly stores the separated word
  int count=0;
  int i=0;
  int j=0;

  while(i<strlen(token)){
    while(i<strlen(token) && token[i]!=' ' && token[i]!='\t'){
      str[count][j++]=token[i++];
      str[count][j]='\0';
    }
    count++;
    j=0;
    if(i==strlen(token)){
      break;
    }
    else{
      while(i<strlen(token) && (token[i]==' '||token[i]=='\t')) {
        i++;
      }
    }
  }
  cmd->u.word = malloc(count * 128);
  memset(cmd->u.word, 0, count * 128);
  for(i=0;i<count;i++){
    cmd->u.word[i]=malloc(strlen(str[i])+1);
    strcpy(cmd->u.word[i],str[i]);
  } 

  return count;
}


void
make_command(token_t *token_array, command_t* out_cmd)
{
  int i = 0;
  stack operator_stack= create_stack();
  // printf("%lu\n", operator_stack);
  
  // printf("%lu\n", operator_stack);
  stack command_stack = create_stack();
  // printf("%lu\n", command_stack);
  while (token_array[i]->line_num != -1)
  {
    char *token_origin = token_array[i]->word;
    char *token = eat_tail_white_space(token_origin);
    
    if (!strcmp(token, "&&") || !strcmp(token, "||") || !strcmp(token, "|") 
      || !strcmp(token, ";") || !strcmp(token, "<") || !strcmp(token, ">")
      || !strcmp(token, "(") || !strcmp(token, ")"))
    {
      if (!strcmp(token, "<")) 
      {
        char *word = eat_tail_white_space(token_array[i+1]->word);
        command_t tmp_cmd = (command_t)peer(command_stack);
        tmp_cmd->input = word;
        // free(word);
        i+=2;
        continue;
      } 
      else if (!strcmp(token, ">"))
      {
        char *word = eat_tail_white_space(token_array[i+1]->word);
        command_t tmp_cmd = (command_t)peer(command_stack);
        tmp_cmd->output = word;
        // free(word);
        i+=2;
        continue;
      }

      if (!strcmp(token, ")"))
      {
        char *oper = (char*)pop(operator_stack);
        while (strcmp(oper, "("))
        {
          pop(operator_stack);
        }
        command_t sub_cmd = malloc(sizeof(struct command));
        command_t subshell = (command_t)pop(command_stack);
        make_subshell_command(subshell, sub_cmd);
        push(sub_cmd, command_stack);
      }
      else if (is_empty(operator_stack)) 
        push(token, operator_stack);
      else
      {
        char *oper = (char*)peer(operator_stack);
        while (strcmp(token, "(") && oper_cmp(token, oper) <= 0 && !is_empty(operator_stack))
        {
          oper = (char*)pop(operator_stack);
          command_t cmd2 = (command_t)pop(command_stack);
          command_t cmd1 = (command_t)pop(command_stack);
          command_t new_cmd = malloc(sizeof(struct command));
          make_compound_command(cmd1, cmd2, oper, new_cmd);
          push(new_cmd, command_stack);
          if (!is_empty(operator_stack))
            oper = (char*)peer(operator_stack);
        }
        
        push(token, operator_stack);
        
      }
      // if (!strcmp(token, "&&"))

    }
    else 
    {
      // command_t cmd = malloc(sizeof(struct command));
      if (i > 0 && !strcmp(token_array[i - 1]->word, "("))
      {
        token_t *sub_token_array = malloc(sizeof(struct token) * 128);
        tokenize(token, sub_token_array, token_array[i]->line_num);
        command_t cmd[1];
        make_command(sub_token_array, cmd);
        push(cmd[0], command_stack);
      }
      else
      {
        command_t cmd = malloc(sizeof(struct command));
        make_simple_command(token, cmd);
        push(cmd, command_stack);
      }
      // push(cmd, command_stack);
    }
    i++;
  }

  if (is_empty(operator_stack))
  {
    out_cmd[0] = (command_t)pop(command_stack);
  } 
  else
  {
    while (!is_empty(operator_stack)) 
    {
      char *oper = (char*)pop(operator_stack);
      command_t cmd2 = (command_t)pop(command_stack);
      command_t cmd1 = (command_t)pop(command_stack);
      out_cmd[0] = malloc(sizeof(struct command));
      make_compound_command(cmd1, cmd2, oper, out_cmd[0]);
      push(out_cmd[0], command_stack);
    }
  }



  // free(operator_stack);
  // free(command_stack);
}

void
make_commands (FILE* filename, int (*get_next_byte) (void *), command_t* res)
{
  Separation_t m_sp[MAXLINE];
  makeSeparation(filename, m_sp, get_next_byte);
  if (!validate_all(m_sp)) return;
  else
  {
    int i = 0;
    while (m_sp[i]->m_line != -1)
    {
      // command_t cmd = malloc(sizeof(struct command));
      command_t cmd[1];
      token_t *token_array = malloc(sizeof(struct token)*128);
      tokenize(m_sp[i]->m_cmd, token_array, m_sp[i]->m_line);
      make_command(token_array, cmd);
      res[i] = cmd[0];
      i++;
    }
    res[i] = NULL;
  }
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

typedef struct commandNode* commandNode_t;

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  // get_next_byte(get_next_byte_argument);

  command_t *res = malloc(sizeof(command_t) * 128);
  memset(res, 0, 128);
  
  make_commands(get_next_byte_argument, get_next_byte, res);

  commandNode_t node = malloc(sizeof(struct commandNode));
  node->next = NULL;
  commandNode_t cursor = node;

  int i = 0;
  while (res[i] != NULL)
  {
    commandNode_t tmp_node = malloc(sizeof(struct commandNode));
    cursor->next = tmp_node;
    cursor = cursor->next;
    cursor->command = res[i];
    
    i++;
  }
  cursor->next = NULL;

  command_stream_t stream = malloc(sizeof(struct command_stream));
  stream->head = node->next;
  stream->cursor = node->next;
  stream->tail = cursor;

  // error (1, 0, "command reading not yet implemented");
  return stream;
}


command_t
read_command_stream (command_stream_t s)
{
  command_t temp;
  //if 0 Node in linkedlist
  if (s->cursor==NULL) 
    return NULL;
  //else
  else{
    temp = s->cursor->command;
    s->cursor = s->cursor->next;
  }
  return temp;
}


