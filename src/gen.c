#include "include/gen.h"
#include "include/string_utils.h"
#include "include/io.h"
#include "include/package.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* gen_args(list_T* list_value, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "(");

  for (unsigned int i = 0; i < list_value->size; i++)
  {
    char* child_str = gen((AST_T*)list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < list_value->size-1)
      str = str_append(&str, ",");
  }
  
  str = str_append(&str, ")");

  return str;
}

static char* gen_semi_args(list_T* list_value, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "(");

  for (unsigned int i = 0; i < list_value->size; i++)
  {
    char* child_str = gen((AST_T*)list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < list_value->size-1)
      str = str_append(&str, ";");
  }
  
  str = str_append(&str, ")");

  return str;
}

static char* gen_list(list_T* list_value, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "[");

  for (unsigned int i = 0; i < list_value->size; i++)
  {
    char* child_str = gen((AST_T*)list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < list_value->size-1)
      str = str_append(&str, ",");
  }
  
  str = str_append(&str, "]");

  return str;
}

static char* gen_tuple(list_T* list_value, GEN_FLAGS flags)
{
  char* str = 0;
  
  for (unsigned int i = 0; i < list_value->size; i++)
  {
    AST_T* child = (AST_T*)list_value->items[i];
    char* child_str = gen(child, flags);
    str = str_append(&str, child_str);

    if (i < list_value->size - 1 && child->type != AST_NOOP)
      str = str_append(&str, ", ");
  }
  
  return str;
}

static char* gen_semi_tuple(list_T* list_value, GEN_FLAGS flags)
{
  char* str = 0;

  for (unsigned int i = 0; i < list_value->size; i++)
  {
    char* child_str = gen((AST_T*)list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < list_value->size - 1)
      str = str_append(&str, ";");
  }
  
  return str ? str : strdup("/* empty semi tuple */");
}

char* gen(AST_T* ast, GEN_FLAGS flags){
  char* body = 0;
  char* str = 0;

  switch (ast->type) {
    case AST_ARRAY: body = gen_array(ast, flags); break;
    case AST_TUPLE: body = gen_tuple_ast(ast, flags); break;
    case AST_HEX: body = gen_int(ast, flags); break;
    case AST_INT: body = gen_int(ast, flags); break;
    case AST_FLOAT: body = gen_float(ast, flags); break;
    case AST_STRING: body = gen_string(ast, flags); break;
    case AST_ARROW_DEFINITION: body = gen_arrow_definition(ast, flags); break;
    case AST_ASSIGNMENT: body = gen_assignment(ast, flags); break;
    case AST_COLON_ASSIGNMENT: body = gen_colon_assignment(ast, flags); break;
    case AST_WHILE: body = gen_while(ast, flags); break;
    case AST_FOR: body = gen_for(ast, flags); break;
    case AST_COMPOUND: body = gen_compound(ast, flags); break;
    case AST_IMPORT: body = gen_import(ast, flags); break;
    case AST_UNDEFINED: body = gen_undefined(ast, flags); break;
    case AST_CALL: body = gen_call(ast, flags); break;
    case AST_FUNCTION: body = gen_function(ast, flags); break;
    case AST_SCOPE: body = gen_scope(ast, flags); break;
    case AST_SIGNATURE: body = gen_signature(ast, flags); break;
    case AST_NAME: body = gen_name(ast, flags); break;
    case AST_STATE: body = gen_state(ast, flags); break;
    case AST_BINOP: body = gen_binop(ast, flags); break;
    case AST_UNOP: body = gen_unop(ast, flags); break;
    case AST_INCREMENT: body = gen_increment(ast, flags); break;
    case AST_DECREMENT: body = gen_decrement(ast, flags); break;
    case AST_REGEX: body = gen_regex(ast, flags); break;
    case AST_OBJECT: body = gen_object(ast, flags); break;
    case AST_CONDITION: body = gen_condition(ast, flags); break;
    case AST_SWITCH: body = gen_switch(ast, flags); break;
    case AST_TRY: body = gen_try(ast, flags); break;
    case AST_LABEL: body = gen_label(ast, flags); break;
    case AST_TERNARY: body = gen_ternary(ast, flags); break;
    case AST_DO: body = gen_do(ast, flags); break;
    case AST_NOOP: body = gen_noop(ast, flags); break;
    default: { printf("[Gen]: missing generator for `%d`", ast->type); exit(1); } break;
  }

  if (ast->capsulated && body)
  {
    str = str_append(&str, "(");
    str = str_append(&str, body);
    str = str_append(&str, ")");
  }
  else if (body)
  {
    str = str_append(&str, body);
  }

  return str ? str : strdup("/* nothing */");
}
char* gen_array(AST_T* ast, GEN_FLAGS flags){
  return gen_list(ast->list_value, flags);
}

char* gen_tuple_ast(AST_T* ast, GEN_FLAGS flags){
  return gen_tuple(ast->list_value, flags);
}
char* gen_int(AST_T* ast, GEN_FLAGS flags){ return strdup(int_to_str(ast->int_value)); }

char* gen_hex(AST_T* ast, GEN_FLAGS flags){ return strdup(ast->string_value); }

char* gen_float(AST_T* ast, GEN_FLAGS flags){ return strdup(ast->string_value ? ast->string_value : float_to_str(ast->float_value)); }

char* gen_string(AST_T* ast, GEN_FLAGS flags)
{
  char* str = calloc(strlen(ast->string_value) + 3, sizeof(char));
  sprintf(str, "\"%s\"", ast->string_value);

  return str;
}

char* gen_arrow_definition(AST_T* ast, GEN_FLAGS flags){
  char* str = 0;
  str = str_append(&str, "(");

  for (unsigned int i = 0; i < ast->list_value->size; i++)
  {
    char* child_str = gen((AST_T*)ast->list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < ast->list_value->size-1)
      str = str_append(&str, ",");
  }
  
  str = str_append(&str, ")");
  str = str_append(&str, "=>");
  
  if (ast->body && ast->body->type == AST_COMPOUND)
  {
    str = str_append(&str, "{");
    char* bodystr = gen(ast->body, flags);
    str = str_append(&str, bodystr);
    str = str_append(&str, "}");
  }
  else
  if (ast->body)
  {
    char* bodystr = gen(ast->body, flags);
    str = str_append(&str, bodystr);
  }

  return str; 
}

char* gen_assignment(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  
  if (ast->flags)
  {
    for (unsigned int i = 0; i < ast->flags->size; i++)
    {
      AST_T* ast_flag = (AST_T*) ast->flags->items[i];
      char* ast_flag_str = gen(ast_flag, flags);
      str = str_append(&str, ast_flag_str);
      str = str_append(&str, " ");
    }
  }

  if (ast->left)
  {
    char* refstr = gen(ast->left, flags);
    str = str_append(&str, refstr);
  }
  else if (ast->name)
  {
    str = str_append(&str, ast->name);
  }

  if (ast->list_value_left)
  {
    str = str_append(&str, ",");
    char* tuplestr = gen_tuple(ast->list_value_left, flags);
    str = str_append(&str, tuplestr);
  }
 
  if ((ast->left || ast->name) && ast->value)
  { 
    str = str_append(&str, "=");
  }

  if (ast->value)
  {
    char* valuestr = gen(ast->value, flags);
    str = str_append(&str, valuestr);
  }

  if (ast->list_value_right)
  {
    str = str_append(&str, ",");
    char* tuplestr = gen_tuple(ast->list_value_right, flags);
    str = str_append(&str, tuplestr);
  }

  return str;
}

char* gen_colon_assignment(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->left || ast->name)
  {
    str = str_append(&str, ast->name ? ast->name : gen(ast->left, flags));
    str = str_append(&str, " ");
    
    if (ast->expr)
    {
      str = str_append(&str, gen(ast->expr, flags));
    }

    str = str_append(&str, ":");
  }

  if (ast->right)
  {
    str = str_append(&str, gen(ast->right, flags));
  }

  return str;
}

char* gen_while(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "while(");
  if (ast->expr)
  {
    char* expr_str = gen(ast->expr, flags);
    str = str_append(&str, expr_str);
  }
  str = str_append(&str, ")");

  if (ast->body)
  {
    str = str_append(&str, "{");
    char* body_str = gen(ast->body, flags);
    str = str_append(&str, body_str);
    str = str_append(&str, "}");
  }

  return str;
}

char* gen_for(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "for ");
  char* args_str = gen_semi_args(ast->list_value, flags);
  str = str_append(&str, args_str);
  str = str_append(&str, "{");
  if (ast->body)
  {
    char* body_str = gen(ast->body, flags);
    str = str_append(&str, body_str);
  }
  str = str_append(&str, "}");

  return str;
}

char* gen_compound(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  for (unsigned int i = 0; i < ast->list_value->size; i++)
  {
    AST_T* child_ast = (AST_T*)ast->list_value->items[i];

    char* child_str = strdup(gen(child_ast, flags));

    if (child_ast->type != AST_IMPORT && child_ast->type != AST_UNDEFINED && child_ast->type != AST_NOOP && child_ast->type != AST_DO) {
      child_str = str_append(&child_str, ";");
    }

    str = str_append(&str, child_str);
  }

  return str;
}

char* gen_import(AST_T* ast, GEN_FLAGS flags)
{
  if (!ast->compiled_value) return 0;
  char* head_str = 0; 

  if (ast->type == AST_IMPORT && ast->list_value)
  {
    const char* head_template = "let %s = (function(){\n let exports = {}; let module = { 'exports': exports };\n";
    char* head_elements_str = gen_tuple(ast->list_value, flags);
    head_str = calloc(strlen(head_template) + strlen(head_elements_str) + 1, sizeof(char));
    sprintf(head_str, head_template, head_elements_str);
  }
  else if (ast->type == AST_CALL)
  {
    head_str = str_append(&head_str, "(function(){\n let exports = {}; let module = { 'exports': exports };\n");
  }
  
  char* str = 0;

  if (head_str)
  {
    str = str_append(&str, head_str);
    str = str_append(&str, ast->compiled_value);
    str = str_append(&str, "\n");
    str = str_append(&str, "return module && module.exports ? module.exports : exports;\n");
    str = str_append(&str, "})()\n");
  }
  else
  {
    str = str_append(&str, ast->compiled_value);
  }

  return str;
}

char* gen_undefined(AST_T* ast, GEN_FLAGS flags)
{
  return strdup("/* undefined */");
}

char* gen_call(AST_T* ast, GEN_FLAGS flags)
{ 
  char* str = 0;
  char* refstr = ast->left ? gen(ast->left, flags) : strdup("/* no ref */");

  if (strcmp(refstr, "require") == 0 && ast->compiled_value)
  {
    return gen_import(ast, flags);
  }

  str = str_append(&str, refstr);
  str = str_append(&str, "(");

  for (unsigned int i = 0; i < ast->list_value->size; i++)
  {
    char* child_str = gen((AST_T*)ast->list_value->items[i], flags);
    str = str_append(&str, child_str);

    if (i < ast->list_value->size-1)
      str = str_append(&str, ",");
  }
  
  str = str_append(&str, ")");

  return str;
}

char* gen_function(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "function ");

  if (ast->name)
  {
    str = str_append(&str, ast->name);
  }

  if (ast->list_value)
  {
    char* args_str = gen_args(ast->list_value, flags);
    str = str_append(&str, args_str);
  }
  
  str = str_append(&str, "{");
  if (ast->body)
  {
    char* body_str = gen(ast->body, flags);
    str = str_append(&str, body_str);
  }
  str = str_append(&str, "}");

  return str;
}

char* gen_scope(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  
  str = str_append(&str, "/*scope*/{");

  if (ast->list_value)
  {
    char* args_str = gen_semi_tuple(ast->list_value, flags);
    str = str_append(&str, args_str);
  }
  
  str = str_append(&str, "}");

  return str;
}

char* gen_signature(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->list_value)
  {
    char* list_str = gen_args(ast->list_value, flags);
    str = str_append(&str, list_str);
  }

  if (ast->body)
  {
    str = str_append(&str, "=>");
    if (ast->body->type == AST_COMPOUND)
    {
      str = str_append(&str, "{");
      char* body_str = gen(ast->body, flags);
      str = str_append(&str, body_str);
      str = str_append(&str, "}");
    }
    else
    {
      char* body_str = gen(ast->body, flags);
      str = str_append(&str, body_str);
    }
  }

  return str ? str : strdup("/* empty function */");
}

char* gen_name(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->flags)
  {
    for (unsigned int i = 0; i < ast->flags->size; i++)
    {
      AST_T* ast_flag = (AST_T*) ast->flags->items[i];
      char* ast_flag_str = gen(ast_flag, flags);
      str = str_append(&str, ast_flag_str);
      str = str_append(&str, " ");
    }
  }

  if (ast->phony_value)
  {
    char* phony = gen(ast->phony_value, flags);
    str = str_append(&str, phony);
    str = str_append(&str, ".");
  }
    
  char* namestr = strdup(ast->name ? ast->name : ast->string_value);
  str = str_append(&str, namestr);

  if (ast->value)
  {
    str = str_append(&str, "=");
    char* valuestr = gen(ast->value, flags);
    str = str_append(&str, valuestr);
  }

  return str;
}

char* gen_state(AST_T* ast, GEN_FLAGS flags)
{
  char* str = strcmp(ast->string_value, "export") == 0 ? 0 : strdup(ast->string_value);

  if (ast->value)
  {
    str = str_append(&str, " ");
    char* valuestr = gen(ast->value, flags);
    str = str_append(&str, valuestr);
  }

  return str;
}

char* gen_binop(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  char* leftstr = ast->left ? gen(ast->left, flags): "";

  str = str_append(&str, leftstr); 

  if (ast->token)
  { 
    str = str_append(&str, " ");
    str = str_append(&str, ast->token->value);
    str = str_append(&str, " "); 
  }

  if (ast->right)
  {
    if (ast->right->lazy)
    {
      str = str_append(&str, "() => (");
    }
    char* rightstr = gen(ast->right, flags);
    str = str_append(&str, rightstr);
    if (ast->right->lazy)
    {
      str = str_append(&str, ")");
    }
  }

  return str;
}

char* gen_unop(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->right)
  {
    str = str_append(&str, ast->token->value);
    char* rightstr = gen(ast->right, flags);
    str = str_append(&str, rightstr);
  }
  else if (ast->left)
  {
    char* leftstr = gen(ast->left, flags);
    str = str_append(&str, leftstr);
    str = str_append(&str, ast->token->value);
  }

  return str;
}

char* gen_block_linked_list(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0; 

  if (ast->token && ast->token->value)
  {
    str = str_append(&str, ast->token->value);
  }

  if (ast->list_value)
  {
    str = str_append(&str, gen_args(ast->list_value, flags));
  }

  if (ast->body)
  {
    str = str_append(&str, "{");
    str = str_append(&str, gen(ast->body, flags));
    str = str_append(&str, "}");
  }

  if (ast->right)
  {
    char* right = gen_block_linked_list(ast->right, flags);
    str = str_append(&str, right);
  }

  return str;
}

char* gen_increment(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  if (ast->left)
  {
    char* leftstr = gen(ast->left, flags);
    str = str_append(&str, leftstr);
  }
  str = str_append(&str, ast->token->value);

  return str;
}

char* gen_decrement(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->left)
  {
    char* leftstr = gen(ast->left, flags);
    str = str_append(&str, leftstr);
  }
  str = str_append(&str, ast->token->value);

  return str;
}

char* gen_regex(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  if (ast->string_value)
    str = str_append(&str, ast->string_value);

  return str;
}

char* gen_object(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "{");

  if (ast->list_value)
  {
    char* body_str = gen_tuple(ast->list_value, flags);

    if (body_str)
    {
      str = str_append(&str, body_str);
    }
  }

  str = str_append(&str, "}");

  return str;
}

char* gen_try(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, gen_block_linked_list(ast, flags));

  return str;
}

char* gen_condition(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str,  ast->expr ? "if" : "");

  if (ast->expr)
  {
    str = str_append(&str,  "(");
    char* exprstr = gen(ast->expr, flags);
    str = str_append(&str, exprstr);
    str = str_append(&str,  ")");
  }
  str = str_append(&str,  "{");

  if (ast->body)
  {
    char* body_str = gen(ast->body, flags);
    str = str_append(&str, body_str);
  }
  str = str_append(&str, "}");

  if (ast->right)
  {
    str = str_append(&str, " else ");
    char* right_str = gen(ast->right, flags);
    str = str_append(&str, right_str);
  }

  return str;
}

char* gen_switch(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, "switch");
  str = str_append(&str, "(");
  if (ast->expr)
  {
    char* expr_str = gen(ast->expr, flags);
    str = str_append(&str, expr_str);
  }
  str = str_append(&str, ")");
  str = str_append(&str, "{");

  if (ast->body)
  {
    char* body_str = gen(ast->body, flags);
    str = str_append(&str, body_str);
  }
  str = str_append(&str, "}");

  return str;
}

char* gen_noop(AST_T* ast, GEN_FLAGS flags)
{
  return strdup("");
}

char* gen_label(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;
  str = str_append(&str, ast->left ? gen(ast->left, flags) : "(nil label left)");
  //str = str_append(&str, ":");

  if (ast->label_value)
  {
    char* label_str = gen(ast->label_value, flags);
    str = str_append(&str, label_str);
  }

  return str;
}

char* gen_ternary(AST_T* ast, GEN_FLAGS flags)
{
  char* str = 0;

  str = str_append(&str, gen(ast->expr, flags));
  str = str_append(&str, "?");
  str = str_append(&str, gen(ast->value, flags));

  str = str_append(&str, ":");
  str = str_append(&str, gen(ast->right, flags));

  return str;
}

char* gen_do(AST_T* ast, GEN_FLAGS flags)
{
  char* str = strdup("do {");
  if (ast->body)
  {
    str = str_append(&str, gen(ast->body, flags));
  }
  str = str_append(&str, "}");

  return str;
}