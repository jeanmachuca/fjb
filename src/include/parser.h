#ifndef FJB_PARSER_H
#define FJB_PARSER_H
#include "AST.h"
#include "flags.h"
#include "lexer.h"
#include "list.h"

typedef struct FJB_PARSER_STRUCT
{
  lexer_T* lexer;
  token_T* token;
  compiler_flags_T* flags;
} parser_T;

typedef struct FJB_PARSER_OPTIONS_STRUCT
{
  int stop_token;
  AST_T* parent;
} parser_options_T;

#define EMPTY_PARSER_OPTIONS                                                                       \
  {                                                                                                \
    -1, 0                                                                                          \
  }

parser_T* init_parser(lexer_T* lexer, compiler_flags_T* flags);

AST_T* parser_parse(parser_T* parser, parser_options_T options);

AST_T* parser_parse_any(parser_T* parser, parser_options_T options);

AST_T* parser_parse_call(parser_T* parser, parser_options_T options);

AST_T* parser_parse_id(parser_T* parser, parser_options_T options);

AST_T* parser_parse_expr(parser_T* parser, parser_options_T options);

AST_T* parser_parse_definition(parser_T* parser, parser_options_T options);

AST_T* parser_parse_assignment(parser_T* parser, parser_options_T options, AST_T* id);

AST_T* parser_parse_compound(parser_T* parser, parser_options_T options);

AST_T* parser_parse_statement(parser_T* parser, parser_options_T options);

AST_T* parser_parse_statement_or_expr(parser_T* parser, parser_options_T options);

AST_T* parser_parse_array(parser_T* parser, parser_options_T options);

void parser_eat(parser_T* parser, int token_type);

void parser_eat_any(parser_T* parser);

AST_T* parser_parse_term(parser_T* parser, parser_options_T options);

AST_T* parser_parse_factor(parser_T* parser, parser_options_T options);

void parser_free(parser_T* parser);
#endif
