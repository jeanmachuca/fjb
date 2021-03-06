#include "include/fjb.h"
#include "include/compound.h"
#include "include/gc.h"
#include "include/gen.h"
#include "include/io.h"
#include "include/js/jsx_headers.js.h"
#include "include/lexer.h"
#include "include/list.h"
#include "include/parser.h"
#include "include/signals.h"
#include "include/special_gen.h"
#include "include/string_utils.h"
#include "include/visitor.h"
#include <stdio.h>
#include <string.h>

AST_T* NOOP;
list_T* stack;
extern volatile fjb_signals_T* FJB_SIGNALS;

compiler_result_T* fjb(compiler_flags_T* flags)
{
  if (!flags->source)
    return 0;

  char* ext = (char*)get_filename_ext(flags->filepath);

  compiler_result_T* special = special_gen(flags, ext);
  if (special)
    return special;

  NOOP = init_ast(AST_NOOP);
  gc_mark(flags->GC, NOOP);

  /* ==== Lexing ==== */
  lexer_T* lexer = init_lexer(flags->source, flags->filepath);
  parser_T* parser = init_parser(lexer, flags);

  /* ==== Parsing ==== */
  parser_options_T options = EMPTY_PARSER_OPTIONS;
  AST_T* root = parser_parse(parser, options);

  if (!stack) {
    stack = NEW_STACK;
  }

  /* ==== Evaluate ==== */
  visitor_T* visitor = init_visitor(parser, flags);
  root = visitor_visit(visitor, root, stack);

  /* ==== Tree-shake ==== */
  AST_T* root_to_generate = new_compound(root, flags);

  FJB_SIGNALS->root = root_to_generate;

  /* ==== Generate ==== */
  char* str = 0;

  char* headers = fjb_get_headers(flags);
  if (headers)
    str = str_append(&str, headers);

  str = str_append(&str, "/* IMPORT `");

  str = str_append(&str, flags->filepath);
  str = str_append(&str, "` */ ");
  char* out = gen(root_to_generate, flags);
  str = str_append(&str, out);
  free(out);

  compiler_result_T* result = calloc(1, sizeof(compiler_result_T));
  result->stdout = str;
  result->node = root_to_generate;

  if (flags->filepath)
    result->filepath = strdup(flags->filepath);

  if (flags->should_dump) {
    char* dumped = visitor->flags->dumped_tree;
    char* newdump = _ast_to_str(result->node, 0);

    dumped = str_append(&dumped, newdump);

    result->dumped = dumped;
  }

  lexer_free(lexer);
  parser_free(parser);
  visitor_free(visitor);

  if (root_to_generate != root) {
    root_to_generate->list_value = 0;
    gc_mark(flags->GC, root_to_generate);
  }

  return result;
}

char* fjb_get_headers(compiler_flags_T* flags)
{
  char* str = 0;
  if (FJB_SIGNALS->is_using_jsx && !FJB_SIGNALS->has_included_jsx_headers) {
    str = str_append(&str, (const char*)_tmp_jsx_headers_js);
    FJB_SIGNALS->has_included_jsx_headers = 1;
  }

  return str;
}
