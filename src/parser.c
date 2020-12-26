#include "include/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/gen.h"
#include "include/string_utils.h"
#include "include/array_utils.h"
#include "include/fjb.h"
#include "include/io.h"
#include "include/node.h"

parser_T* init_parser(lexer_T* lexer, const char* filepath)
{
  parser_T* parser = calloc(1, sizeof(struct FJB_PARSER_STRUCT));
  parser->lexer = lexer;
  parser->token = lexer_next(lexer);
  parser->filepath = filepath;

  return parser;
}

static list_T* parse_args(parser_T* parser) {
  parser_eat(parser, TOKEN_LPAREN);
  list_T* list_value = init_list(sizeof(AST_T*));

  if (parser->token->type != TOKEN_RPAREN) {
    AST_T* child = parser_parse_statement_or_expr(parser);
    list_push(list_value, child);

    while (parser->token->type == TOKEN_COMMA)
    {
      parser_eat(parser, TOKEN_COMMA);
      child = parser_parse_statement_or_expr(parser);
      list_push(list_value, child);
    }
  }
  
  parser_eat(parser, TOKEN_RPAREN);

  return list_value;
}

static list_T* parse_tuple(parser_T* parser) {
  list_T* list_value = init_list(sizeof(AST_T*));

  while (parser->token->type == TOKEN_COMMA)
  {
    parser_eat(parser, TOKEN_COMMA);
    AST_T* child = parser_parse_statement_or_expr(parser);

    if (child->type != AST_NOOP)
    {
      list_push(list_value, child);
    }
  }

  return list_value;
}

static list_T* parse_semi_args(parser_T* parser) {
  parser_eat(parser, TOKEN_LPAREN);
  list_T* list_value = init_list(sizeof(AST_T*));

  while (parser->token->type == TOKEN_SEMI)
  {
    parser_eat(parser, TOKEN_SEMI);
    list_push(list_value, init_ast(AST_NOOP));
  }

  if (parser->token->type != TOKEN_RPAREN) {
    AST_T* child = parser_parse_expr(parser);
    list_push(list_value, child);

    while (parser->token->type != TOKEN_RPAREN)
    {
      if (parser->token->type == TOKEN_SEMI)
        parser_eat(parser, TOKEN_SEMI);

      child = parser_parse_expr(parser);
      list_push(list_value, child);
    }
  }
  
  parser_eat(parser, TOKEN_RPAREN);

  return list_value;
}

static list_T* parse_array(parser_T* parser) {
  parser_eat(parser, TOKEN_LBRACKET);
  list_T* list_value = init_list(sizeof(AST_T*));

  while (parser->token->type == TOKEN_COMMA)
  {
    parser_eat(parser, TOKEN_COMMA);
    list_push(list_value, init_ast(AST_UNDEFINED));
  }

  if (parser->token->type != TOKEN_RBRACKET) {
    AST_T* child = parser_parse_statement_or_expr(parser);
    list_push(list_value, child);

    while (parser->token->type == TOKEN_COMMA)
    {
      parser_eat(parser, TOKEN_COMMA);
      child = parser_parse_statement_or_expr(parser);
      list_push(list_value, child);
    }
  }
  
  parser_eat(parser, TOKEN_RBRACKET);

  return list_value;
}

static AST_T* parse_block_linked_list(parser_T* parser, int types[], unsigned int length)
{
  AST_T* left = 0;
  
  for (unsigned int i = 0; i < length; i++)
  {
    if (parser->token->type == types[i])
    {
      left = init_ast(AST_TRY);
      left->token = parser->token;
      parser_eat(parser, parser->token->type);
      
      if (parser->token->type == TOKEN_LPAREN)
      {
        left->list_value = parse_args(parser);
      }

      if (parser->token->type == TOKEN_LBRACE)
      {
        parser_eat(parser, TOKEN_LBRACE);
        left->body = parser_parse(parser);
        parser_eat(parser, TOKEN_RBRACE);
      }

      if (left)
      {
        left->right = parse_block_linked_list(parser, types, length);

        if (left->right)
        {
          left->right->left = left;
        }
      }
    }
  }

  return left;
}

AST_T* parser_parse(parser_T* parser)
{
  return parser_parse_compound(parser);
}

AST_T* parser_parse_id(parser_T* parser)
{
  AST_T* ast = init_ast(AST_NAME);
  ast->string_value = strdup(parser->token->value);
  ast->name = strdup(ast->string_value);
  
  if (token_is_statement_or_id(parser->token))
  {  
    parser_eat_any(parser);
  }
  else
  {
    printf("Error.\n");
  } 

  if (parser->token->type == TOKEN_DECREMENT || parser->token->type == TOKEN_INCREMENT)
  {
    AST_T* unop = init_ast(AST_UNOP);
    unop->left = ast;
    unop->token = parser->token;
    parser_eat(parser, parser->token->type);
    return unop;
  }

  while (parser->token->type == TOKEN_LPAREN)
  {
    AST_T* ast_call = parser_parse_call(parser);
    ast_call->left = ast; 

    ast = ast_call;

    return ast;
  } 

  return ast;
}

AST_T* parser_parse_int(parser_T* parser)
{
  AST_T* ast = init_ast(AST_INT);
  ast->int_value = atoi(parser->token->value);
  parser_eat(parser, TOKEN_INT); 

  return ast;
}

AST_T* parser_parse_float(parser_T* parser)
{
  AST_T* ast = init_ast(AST_FLOAT);
  ast->float_value = atof(parser->token->value);
  ast->string_value = strdup(parser->token->value);
  parser_eat(parser, TOKEN_FLOAT);

  return ast;
}

AST_T* parser_parse_string(parser_T* parser)
{
  AST_T* ast = init_ast(AST_STRING);

  ast->string_value = calloc(strlen(parser->token->value) + 1, sizeof(char));
  strcpy(ast->string_value, parser->token->value); 
  parser_eat(parser, TOKEN_STRING); 

  return ast;
}

AST_T* parser_parse_hex(parser_T* parser)
{
  AST_T* ast = init_ast(AST_HEX);

  ast->string_value = calloc(strlen(parser->token->value) + 1, sizeof(char));
  strcpy(ast->string_value, parser->token->value); 
  parser_eat(parser, TOKEN_HEX); 

  return ast;
}

AST_T* parser_parse_import(parser_T* parser)
{
  AST_T* ast = init_ast(AST_IMPORT);
  parser_eat(parser, TOKEN_IMPORT);

  ast->list_value = init_list(sizeof(AST_T*));

  if (parser->token->type == TOKEN_LBRACE) {
    parser_eat(parser, TOKEN_LBRACE);

    AST_T* ast_import_arg = parser_parse_id(parser);
    list_push(ast->list_value, ast_import_arg);

    while (parser->token->type == TOKEN_COMMA) {
      AST_T* ast_import_arg = parser_parse_id(parser);
      list_push(ast->list_value, ast_import_arg);
    }

    parser_eat(parser, TOKEN_RBRACE);
  } else if (parser->token->type != TOKEN_STRING) {
    AST_T* ast_import_arg = parser_parse_id(parser);
    list_push(ast->list_value, ast_import_arg);
  }

  if (ast->list_value->size > 0) {
    parser_eat(parser, TOKEN_FROM);
  }
  ast->string_value = calloc(strlen(parser->token->value) + 1, sizeof(char));
  strcpy(ast->string_value, parser->token->value);
  parser_eat(parser, TOKEN_STRING);

  char* final_file_to_read = resolve_import((char*) parser->filepath, ast->string_value);

  compiler_result_T* result = fjb((GEN_FLAGS){final_file_to_read, ast->string_value}, fjb_read_file(final_file_to_read));
  ast->compiled_value = result->stdout;

  return ast;
}

AST_T* parser_parse_definition(parser_T* parser)
{
  list_T* flags = init_list(sizeof(AST_T*));

  while (
      parser->token->type == TOKEN_CONST ||
      parser->token->type == TOKEN_LET ||
      parser->token->type == TOKEN_VAR
  ) {
    AST_T* ast_flag = init_ast(AST_NAME);
    ast_flag->string_value = strdup(parser->token->value);
    list_push(flags, ast_flag);
    parser_eat(parser, parser->token->type); 
  }

  AST_T* astname = parser_parse_id(parser);
  astname->flags = flags;

  return astname;
}

AST_T* parser_parse_assignment(parser_T* parser) 
{
  AST_T* ast = init_ast(AST_ASSIGNMENT);
  ast->flags = init_list(sizeof(AST_T*)); 
  
  if (parser->token->type == TOKEN_EQUALS)
  {
    parser_eat(parser, TOKEN_EQUALS);
    ast->value = parser_parse_expr(parser); 
  } 

  return ast; 
}

AST_T* parser_parse_state(parser_T* parser)
{
  AST_T* ast = init_ast(AST_STATE);
  ast->string_value = strdup(parser->token->value);

  parser_eat(parser, parser->token->type);

  if (parser->token->type != TOKEN_SEMI)
    ast->value = parser_parse_expr(parser);

  return ast;
}

AST_T* parser_parse_try(parser_T* parser) {
  AST_T* ast = init_ast(AST_TRY);
  ast->token = parser->token;
  parser_eat(parser, TOKEN_TRY);

  parser_eat(parser, TOKEN_LBRACE);
  ast->body = parser_parse(parser);
  parser_eat(parser, TOKEN_RBRACE);

  ast->right = parse_block_linked_list(parser, (int[]){TOKEN_CATCH, TOKEN_FINALLY}, 2);

  return ast;
}

AST_T* parser_parse_condition(parser_T* parser) {
  AST_T* ast = init_ast(AST_CONDITION);

  if (parser->token->type == TOKEN_ELSE)
  {
    parser_eat(parser, TOKEN_ELSE);
  }

  if (parser->token->type == TOKEN_IF)
  {
    parser_eat(parser, TOKEN_IF);

    parser_eat(parser, TOKEN_LPAREN);
    ast->expr = parser_parse_statement_or_expr(parser);
    parser_eat(parser, TOKEN_RPAREN);
  }

  if (parser->token->type == TOKEN_LBRACE)
  {
    parser_eat(parser, TOKEN_LBRACE);


    if (parser->token->type != TOKEN_RBRACE)
    {
      ast->body = parser_parse(parser);
    }

    parser_eat(parser, TOKEN_RBRACE);
  } else {
    ast->body = parser_parse_statement_or_expr(parser);
  }

  if (parser->token->type == TOKEN_ELSE)
  {
    ast->right = parser_parse_condition(parser);
  }

  return ast;
}

AST_T* parser_parse_do(parser_T* parser) {
  AST_T* ast = init_ast(AST_DO);
  parser_eat(parser, TOKEN_DO);

  parser_eat(parser, TOKEN_LBRACE);

  if (parser->token->type != TOKEN_RBRACE)
  {
    ast->body = parser_parse(parser);
  }

  parser_eat(parser, TOKEN_RBRACE);

  return ast;
}

AST_T* parser_parse_while(parser_T* parser) {
  parser_eat(parser, TOKEN_WHILE);
  AST_T* ast = init_ast(AST_WHILE);

  parser_eat(parser, TOKEN_LPAREN);
  ast->expr = parser_parse_expr(parser);
  parser_eat(parser, TOKEN_RPAREN);

  if (parser->token->type == TOKEN_LBRACE)
  {
    parser_eat(parser, TOKEN_LBRACE);

    if (parser->token->type != TOKEN_RBRACE)
    {
      ast->body = parser_parse(parser);
    }

    parser_eat(parser, TOKEN_RBRACE);
  }
  else if (parser->token->type != TOKEN_SEMI)
  {
    ast->body = parser_parse_statement_or_expr(parser);
  }

  return ast;
}

AST_T* parser_parse_for(parser_T* parser) {
  parser_eat(parser, TOKEN_FOR);
  AST_T* ast = init_ast(AST_FOR);
  ast->list_value = parse_semi_args(parser);


  if (parser->token->type == TOKEN_LBRACE)
  {
    parser_eat(parser, TOKEN_LBRACE);
    ast->body = parser_parse(parser);
    parser_eat(parser, TOKEN_RBRACE);
  } else {
    ast->body = parser_parse_statement_or_expr(parser);
  }
  
  return ast;
}

AST_T* parser_parse_switch(parser_T* parser) {
  parser_eat(parser, TOKEN_SWITCH);
  AST_T* ast = init_ast(AST_SWITCH);
  parser_eat(parser, TOKEN_LPAREN);
  ast->expr = parser_parse_expr(parser);
  parser_eat(parser, TOKEN_RPAREN);

  parser_eat(parser, TOKEN_LBRACE);
  ast->body = parser_parse(parser);
  parser_eat(parser, TOKEN_RBRACE);
  
  return ast;
}

AST_T* parser_parse_array(parser_T* parser)
{
  AST_T* ast = init_ast(AST_ARRAY);
  ast->list_value = parse_array(parser);

  return ast;
}

AST_T* parser_parse_regex(parser_T* parser)
{
  AST_T* ast = init_ast(AST_REGEX);
  ast->string_value = strdup(parser->token->value);
  parser_eat(parser, TOKEN_REGEX);
  return ast;
}

AST_T* parser_parse_object(parser_T* parser)
{
  AST_T* ast = init_ast(AST_OBJECT);
  ast->list_value = init_list(sizeof(token_T*));

  unsigned int is_object = 0;
  parser_eat(parser, TOKEN_LBRACE);
  
  if (parser->token->type != TOKEN_RBRACE)
  {
    list_push(ast->list_value, parser_parse_statement_or_expr(parser));

    while (parser->token->type != TOKEN_RBRACE)
    {
      if (parser->token->type == TOKEN_COMMA)
        parser_eat(parser, TOKEN_COMMA);

      if (parser->token->type != TOKEN_RBRACE)
      {
        list_push(ast->list_value, parser_parse_statement_or_expr(parser));
      }
    }
  }

  if (parser->token->type == TOKEN_COMMA)
  {
    parser_eat(parser, TOKEN_COMMA);
  }

  parser_eat(parser, TOKEN_RBRACE);

  for (unsigned int i = 0; i < ast->list_value->size; i++)
  {
    AST_T* child = (AST_T*) ast->list_value->items[i];

    if (child->type == AST_COLON_ASSIGNMENT || (child->type == AST_NAME && !child->flags))
      is_object = 1;
  }

  if (!is_object)
    ast->type = AST_SCOPE;

  return ast;
}

AST_T* parser_parse_function(parser_T* parser)
{
  parser_eat(parser, TOKEN_FUNCTION);
  AST_T* ast = init_ast(AST_FUNCTION);

  if (parser->token->type == TOKEN_ID)
  {
    ast->name = strdup(parser->token->value);
    parser_eat(parser, TOKEN_ID);
  }
  ast->list_value = parse_args(parser);
  parser_eat(parser, TOKEN_LBRACE);

  if (parser->token->type != TOKEN_RBRACE)
  {
    ast->body = parser_parse(parser);
  }

  parser_eat(parser, TOKEN_RBRACE);

  while (parser->token->type == TOKEN_LPAREN)
  {
    AST_T* ast_call = parser_parse_call(parser);
    ast_call->left = ast; 

    ast = ast_call;
  }

  return ast;
}

AST_T* parser_parse_signature(parser_T* parser)
{
  AST_T* ast = init_ast(AST_SIGNATURE);
  ast->list_value = parse_args(parser);

  return ast;
}

AST_T* parser_parse_factor(parser_T* parser)
{
  while (parser->token->type == TOKEN_SEMI)
  {
    parser_eat(parser, TOKEN_SEMI);
  }

  if (parser->token->type == TOKEN_LPAREN)
  {
    parser_eat(parser, TOKEN_LPAREN);
    AST_T* expr = parser_parse_expr(parser);
    expr->capsulated = 1;
    parser_eat(parser, TOKEN_RPAREN);
    return expr;
  }

  while (parser->token->type == TOKEN_DECREMENT || parser->token->type == TOKEN_INCREMENT || parser->token->type == TOKEN_NOT || parser->token->type == TOKEN_TILDE || parser->token->type == TOKEN_PLUS || parser->token->type == TOKEN_MINUS || parser->token->type == TOKEN_SPREAD)
  {
    AST_T* unop = init_ast(AST_UNOP);
    unop->token = parser->token;
    parser_eat(parser, parser->token->type);
    unop->right = parser_parse_expr(parser);
    return unop;
  } 

  if (token_is_statement_or_id(parser->token))
  {
    return parser_parse_id(parser);
  }

  switch (parser->token->type) {
    case TOKEN_ID: return parser_parse_id(parser); break; 
    case TOKEN_DIV: return parser_parse_regex(parser); break; 
    case TOKEN_INT: return parser_parse_int(parser); break; 
    case TOKEN_FLOAT: return parser_parse_float(parser); break; 
    case TOKEN_STRING: return parser_parse_string(parser); break;
    case TOKEN_HEX: return parser_parse_hex(parser); break;
    case TOKEN_REGEX: return parser_parse_regex(parser); break;
    case TOKEN_LBRACE: return parser_parse_object(parser); break;
    case TOKEN_SWITCH: return parser_parse_switch(parser); break;
    case TOKEN_DO: return parser_parse_do(parser); break;
    case TOKEN_LBRACKET: return parser_parse_array(parser); break;
    case TOKEN_FUNCTION: return parser_parse_function(parser); break;
    case TOKEN_RETURN: case TOKEN_DELETE: case TOKEN_EXPORT: case TOKEN_TYPEOF: case TOKEN_NEW: case TOKEN_THROW: case TOKEN_BREAK: case TOKEN_INSTANCEOF: case TOKEN_VOID: return parser_parse_state(parser); break;
    case TOKEN_CONST: case TOKEN_LET: case TOKEN_VAR: return parser_parse_definition(parser); break;
    case TOKEN_EOF: return init_ast(AST_NOOP); break;
    default: { return init_ast(AST_NOOP); }; break; 
  } 

  return 0;
}

AST_T* parser_parse_term(parser_T* parser)
{
  AST_T* left = parser_parse_factor(parser);
  AST_T* binop = 0;

  if (parser->token->type == TOKEN_EQUALS)
  {
    AST_T* assignment = parser_parse_assignment(parser);
    assignment->left = left;

    left = assignment;
  }

  while (
    parser->token->type == TOKEN_PLUS ||
    parser->token->type == TOKEN_MINUS ||
    parser->token->type == TOKEN_EQUALS_EQUALS ||
    parser->token->type == TOKEN_EQUALS_EQUALS_EQUALS ||
    parser->token->type == TOKEN_NOT_EQUALS_EQUALS ||
    parser->token->type == TOKEN_NOT_EQUALS
  )
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_expr(parser);

    left = binop;
  } 

  while (
    parser->token->type == TOKEN_DIV ||
    parser->token->type == TOKEN_STAR
  )
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;

    binop->token = parser->token;
    parser_eat(parser, parser->token->type); 
    binop->right = parser_parse_factor(parser);

    left = binop;
  }
  
  while (
    parser->token->type == TOKEN_AND_AND
  )
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_expr(parser);

    left = binop;
  }

  while (
    parser->token->type == TOKEN_LT ||
    parser->token->type == TOKEN_LT_EQUALS ||
    parser->token->type == TOKEN_GT ||
    parser->token->type == TOKEN_GT_EQUALS ||
    parser->token->type == TOKEN_PIPE_PIPE ||
    parser->token->type == TOKEN_IN
  )
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_expr(parser);

    left = binop;
  }

  while (parser->token->type == TOKEN_LPAREN)
  {
    AST_T* ast_call = parser_parse_call(parser);
    ast_call->left = left; 

    left = ast_call;
  }

  return left;
}

AST_T* parser_parse_expr(parser_T* parser)
{
  AST_T* left = parser_parse_term(parser);
  AST_T* binop = 0; 

  while (parser->token->type == TOKEN_ARROW_RIGHT)
  {
    parser_eat(parser, TOKEN_ARROW_RIGHT);
    AST_T* ast_arrow_def = init_ast(AST_ARROW_DEFINITION);
    ast_arrow_def->list_value = init_list(sizeof(AST_T*));
    list_push(ast_arrow_def->list_value, left);

    if (parser->token->type == TOKEN_LBRACE)
    {
      parser_eat(parser, TOKEN_LBRACE);
      if (parser->token->type != TOKEN_RBRACE)
      {
        ast_arrow_def->body = parser_parse(parser);
      }
      parser_eat(parser, TOKEN_RBRACE);
    }
    else
    {
      ast_arrow_def->body = parser_parse_expr(parser);
    }

    left = ast_arrow_def;
  } 


  while (
    parser->token->type == TOKEN_PLUS ||
    parser->token->type == TOKEN_PLUS_EQUALS ||
    parser->token->type == TOKEN_PIPE_EQUALS ||
    parser->token->type == TOKEN_AND_EQUALS ||
    parser->token->type == TOKEN_STAR_EQUALS ||
    parser->token->type == TOKEN_MINUS_EQUALS ||
    parser->token->type == TOKEN_MOD ||
    parser->token->type == TOKEN_SHIFT_RIGHT_UNSIGNED ||
    parser->token->type == TOKEN_SHIFT_RIGHT_UNSIGNED_EQUALS ||
    parser->token->type == TOKEN_AND ||
    parser->token->type == TOKEN_PIPE ||
    parser->token->type == TOKEN_PIPE_PIPE ||
    parser->token->type == TOKEN_MINUS
  )
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_term(parser);

    left = binop;
  }

  if (parser->token->type == TOKEN_INSTANCEOF)
  {
    binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_term(parser);

    left = binop;
  } 

 while (
    parser->token->type == TOKEN_LBRACKET
  )
  {
    AST_T* binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->right = parser_parse_term(parser);
    left = binop;
  }

 while (
    parser->token->type == TOKEN_DOT
  )
  {
    AST_T* binop = init_ast(AST_BINOP);
    binop->left = left;
    binop->token = parser->token;
    parser_eat(parser, parser->token->type);
    binop->right = parser_parse_expr(parser);
    left = binop;
  }

 while (parser->token->type == TOKEN_QUESTION)
  {
    parser_eat(parser, TOKEN_QUESTION);
    AST_T* ast_tern = init_ast(AST_TERNARY);
    ast_tern->expr = left;
     
    ast_tern->value = parser_parse_expr(parser);
    
    parser_eat(parser, TOKEN_COLON);

    ast_tern->right = parser_parse_expr(parser);

    left = ast_tern;
  }

  while (parser->token->type == TOKEN_COMMA)
  {
     AST_T* tuple = init_ast(AST_TUPLE);
     tuple->list_value = parse_tuple(parser);
     list_prefix(tuple->list_value, left);

     left = tuple;
  }  

  return left;
}

AST_T* parser_parse_case(parser_T* parser)
{
  AST_T* ast = init_ast(AST_COLON_ASSIGNMENT);
  ast->name = strdup(parser->token->value);
  parser_eat(parser, TOKEN_CASE);
  ast->expr = parser_parse_expr(parser);
  parser_eat(parser, TOKEN_COLON);
  ast->right = parser_parse_statement_or_expr(parser);

  return ast;
}

AST_T* parser_parse_statement(parser_T* parser)
{
  AST_T* left = 0;

  switch (parser->token->type)
  {
    case TOKEN_IMPORT: left = parser_parse_import(parser); break;
    case TOKEN_FOR: left = parser_parse_for(parser); break;
    case TOKEN_CASE: left = parser_parse_case(parser); break;
    case TOKEN_IF: left = parser_parse_condition(parser); break;
    case TOKEN_TRY: left = parser_parse_try(parser); break;
    case TOKEN_WHILE: left = parser_parse_while(parser); break;
    default: return 0; break;
  }

  return left;
}

AST_T* parser_parse_statement_or_expr(parser_T* parser)
{
  AST_T* left = parser_parse_statement(parser);
  left = left ? left : parser_parse_expr(parser);  

  if (!left)
  {
    printf("[Parser] (%s):%d: Unexpected token `%s` (%d)\n", parser->filepath, parser->lexer->line, parser->token->value, parser->token->type);
    exit(1);
  }

  if (left && parser->token->type == TOKEN_COLON)
  {
    AST_T* colon_ass = init_ast(AST_COLON_ASSIGNMENT);
    colon_ass->left = left;
    parser_eat(parser, TOKEN_COLON);
    colon_ass->right = parser_parse_statement_or_expr(parser);
    left = colon_ass;
  } 

  if (parser->token->type == TOKEN_SEMI)
    parser_eat(parser, TOKEN_SEMI);

  return left;
}

AST_T* parser_parse_compound(parser_T* parser)
{
  AST_T* ast = init_ast(AST_COMPOUND);
  ast->list_value = init_list(sizeof(AST_T*));

  AST_T* child = parser_parse_statement_or_expr(parser);
  list_push(ast->list_value, child);

  while (child->type != AST_NOOP)
  {
    if (parser->token->type == TOKEN_SEMI)
      parser_eat(parser, TOKEN_SEMI);

    child = parser_parse_statement_or_expr(parser);
    list_push(ast->list_value, child);
  }

  return ast;
}

AST_T* parser_parse_call(parser_T* parser)
{
  AST_T* ast_call = init_ast(AST_CALL);
  ast_call->list_value = parse_args(parser);
  ast_call->from_module = strdup(parser->filepath);

  if (parser->token->type == TOKEN_ARROW_RIGHT)
  {
    parser_eat(parser, TOKEN_ARROW_RIGHT);
    ast_call->type = AST_SIGNATURE;

    if (parser->token->type == TOKEN_LBRACE)
    {
      parser_eat(parser, TOKEN_LBRACE);
      if (parser->token->type != TOKEN_RBRACE)
      {
        ast_call->body = parser_parse(parser);
      }
      parser_eat(parser, TOKEN_RBRACE);
    }
    else
    {
      ast_call->body = parser_parse_statement_or_expr(parser);
    }
  }

  return ast_call; 
}

void parser_eat(parser_T* parser, int token_type)
{
  if (parser->token->type == token_type) {
    parser->token = lexer_next(parser->lexer);
  } else {
    printf("[Parser] (%s):%d: Unexpected token `%s` (%d), was expecting `%s`\n", parser->filepath, parser->lexer->line, parser->token->value, parser->token->type, token_type_to_str(token_type));
    exit(1);
  }
}

void parser_eat_any(parser_T* parser)
{
  parser->token = lexer_next(parser->lexer);
}