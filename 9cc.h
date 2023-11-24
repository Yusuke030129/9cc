#ifndef CC_H
#define CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
//  tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;
};

extern char *user_input;
extern Token *token;

void error(char *fmt, ...) ;
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
char *strndup(char *p, int len);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();


Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize(char *p);


//
// parse.c
//

// Local variable
typedef struct Var Var;

struct Var {
    Var *next;
    char *name; // Variable name
    int offset; // Offset from RBP
};

// AST node 
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_WHILE,     // "while"
  ND_FOR,       // "for"
  ND_BLOCK,     // { ... }
  ND_FUNCALL,   // Function call
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       //  Variable 
  ND_NUM,       // 整数 integer
} NodeKind;


typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *next;

  Node *lhs;     // 左辺
  Node *rhs;     // 右辺

  // "if", "while" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // Block
  Node *body;

  // Fuction call
  char *funcname;

  //  char name;   // if Used if kind == ND_LVAR
  Var *var;     // if Used if kind == ND_LVAR
  int val;       // kindがND_NUMの場合のみ使う
};

typedef struct {
    Node *node; 
    Var *locals;
    int stack_size;
    } Program;


Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num(int val);

Program  *program();

//
// codegen.c
//

void gen(Node *node);
void codegen(Program *prog);
#endif
