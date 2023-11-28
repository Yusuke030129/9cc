#ifndef CC_H
#define CC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Type Type;

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
void error_tok(Token *tok, char *fmt, ...);
Token *peek(char *s);
// bool consume(char *op);
Token *consume(char *op);
char *strndup(char *p, int len);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
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
    char *name; // Variable name
    Type *ty;   // Type
    int offset; // Offset from RBP
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
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
  ND_ADDR,      // unary &
  ND_DEREF,     // unary *
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_WHILE,     // "while"
  ND_FOR,       // "for"
  ND_BLOCK,     // { ... }
  ND_FUNCALL,   // Function call
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       //  Variable 
  ND_NUM,       // 整数 integer
  ND_NULL,      // Empty statement 
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *next;
  Type *ty;      // Type, e.g. int or pointer to int
  Token *tok; // Representative token

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
  Node *args;

  //  char name;   // if Used if kind == ND_LVAR
  Var *var;     // if Used if kind == ND_LVAR
  int val;       // kindがND_NUMの場合のみ使う
};

typedef struct Function Function;
struct Function {
    Function *next;
    char *name;
    VarList *params;
    Node *node; 
    VarList *locals;
    int stack_size;
    };


Node *new_node(NodeKind kind, Token *tok);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok);
Node *new_num(int val, Token *tok);

Function  *program();

//
// typing.c
//

typedef enum { TY_INT, TY_PTR} Typekind;

struct Type {
  Typekind kind;
  Type *base;
};

Type *int_type();
Type *pointer_to(Type *base);

void add_type(Function *prog);
//
// codegen.c
//

void gen(Node *node);
void codegen(Function *prog);
#endif
