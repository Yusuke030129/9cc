#include "9cc.h"


//
// parser
//
VarList *locals;

// Find a local variable by name
Var *find_var(Token *tok) {
  for(VarList *vl = locals; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  }
  return NULL;  
}

Node *new_node(NodeKind kind, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_unary(NodeKind kind, Node *expr, Token *tok) {
    Node *node = new_node(kind, tok);
    node->lhs = expr;
    return node;
}

Node *new_num(int val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

Node *new_var(Var *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

Var *push_var(char *name, Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->ty = ty;

  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

Function *function();
Node *declaration();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();



Function *program() {

  Function head;
  head.next = NULL;
  Function *cur = &head;
  
  while(!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }

  return head.next;
}

// basetype = "int" "*"*
Type *basetype() {
  expect("int");
  Type *ty = int_type();
  while (consume("*"))
    ty = pointer_to(ty);
  return ty;
}

VarList *read_func_param() {
  VarList *vl = calloc(1, sizeof(VarList));
  Type *ty = basetype();
  vl->var = push_var(expect_ident(), ty);
  return vl;
}

VarList *read_func_params() {
  if (consume(")"))
    return NULL;
  // read_func_paramへ分割
  //VarList *head = calloc(1, sizeof(VarList));
  //head->var = push_var(expect_ident());
  VarList *head = read_func_param();
  VarList *cur = head;

  while(!consume(")")) {
    expect(",");
    // read_func_paramへ分割
    //cur->next = calloc(1, sizeof(VarList));
    //cur->next->var = push_var(expect_ident());
    cur->next = read_func_param();
    cur = cur->next;
  }
  return head;
}


// function = basetype ident "(" params? ")" "{" stmt* "}"
// params   = param ("," param)*
// param    = basetype ident
Function *function() {
  locals = NULL;
  
  Function *fn = calloc(1, sizeof(Function));
  basetype();
  fn->name = expect_ident();
  expect("(");
  fn->params = read_func_params();
  expect("{");

  Node head;
  head.next = NULL;
  Node *cur = &head;
  while(!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
}

  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

// declaration = basetype ident ("=" expr) ";"
Node *declaration() {
  Token *tok = token;
  Type *ty = basetype();
  Var *var = push_var(expect_ident(), ty);

  if (consume(";")) // int 変数の後ろが  ";" ならば 初期化せず
    return new_node(ND_NULL, tok);

  // int <変数> の後ろが ";" でなければ 代入で初期化のはず
  expect("=");
  Node *lhs = new_var(var, tok);
  Node *rhs = expr();
  expect(";");
  Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
  return new_unary(ND_EXPR_STMT, node, tok);
}

Node *read_expr_stmt() {
    Token *tok = token;
    return new_unary(ND_EXPR_STMT, expr(), tok);
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" stmt* "}"
//      | declaration
//      | expr ";"
Node *stmt() {
  Token *tok;
  if (tok = consume("return")) {
    Node *node = new_unary(ND_RETURN, expr(), tok);
    expect(";");
    return node;
  }

  if (tok = consume("if")) {
    Node *node = new_node(ND_IF, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }
  
  if (tok = consume("while")) {
    Node *node = new_node(ND_WHILE, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (tok = consume("for")) {
    Node *node = new_node(ND_FOR, tok);
    expect("(");
    if (!consume(";")) {
      node->init = read_expr_stmt();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = read_expr_stmt();
      expect(")");
    }
    node->then = stmt();
    return node;
  }
  if (tok = consume("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while(!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    Node *node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    return node;
  }

  if (tok = peek("int"))
    return declaration();

  Node *node = read_expr_stmt();
  expect(";");
  return node;
}

// -expr = equality
// +expr = assign
Node *expr() {
 return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality(); // ※ 左辺値に不正な値もパースされてしまう 1+2=10など.
    Token *tok;
    if (tok = consume("="))
      node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;

}

// equality = relational( "==" relational| "!=" relational )*
Node *equality() {
  Node *node = relational();
  Token *tok; 
  for (;;) {
    if (tok = consume("=="))
      node = new_binary(ND_EQ, node, relational(), tok);
    else if (tok = consume("!="))
      node = new_binary(ND_NE, node, relational(), tok);
    else
      return node;
  }
}


// relational = add( "<" add | "<=" add | ">" add | ">=" add )*

Node *relational() {
  Node *node = add();
  Token *tok;
  
  for (;;) {
    if (tok = consume("<"))
      node = new_binary(ND_LT, node, add(), tok);
    else if (tok = consume("<="))
      node = new_binary(ND_LE, node, add(), tok);
    else if (tok = consume(">"))
      node = new_binary(ND_LT, add(), node, tok);
    else if (tok = consume(">="))
      node = new_binary(ND_LE, add(), node, tok);
    else
      return node;
  }
}


// add = mul ("+" mul | "-" mul)* ※ これまではexpr = mul ("+" mul | "-" mul)*

Node *add() {
  Node *node = mul();
  Token *tok;

  for (;;) {
    if (tok = consume("+"))
      node = new_binary(ND_ADD, node, mul(), tok);
    else if (tok = consume("-"))
      node = new_binary(ND_SUB, node, mul(), tok);
    else
      return node;
  }
}


// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  Token *tok;

  for (;;) {
    if (tok = consume("*"))
      node = new_binary(ND_MUL, node, unary(), tok);
    else if (tok = consume("/"))
      node = new_binary(ND_DIV, node, unary(), tok);
    else
      return node;
  }
}

// unary = ("+" | "-" | "*" | "&")? unary
//       | primary
Node *unary() {
  Token *tok;
  if (tok = consume("+"))
    return unary();
  if (tok = consume("-"))
    return new_binary(ND_SUB, new_num(0, tok), unary(), tok); // 0 - primary で反転  何度繰り返しても結果は -が偶数 =>  lhs 0  -  rhs  (0 - primary) =  primary  -が奇数 lhs  x -  rhs primary = - primary
  if (tok = consume("&"))
    return new_unary(ND_ADDR, unary(), tok); 
  if (tok = consume("*"))
    return new_unary(ND_DEREF, unary(), tok); 
  return primary();
}

// func-args = "(" (assign ("," assign)*)? ")"
Node *func_args() {
  if (consume(")"))
    return NULL;

  Node *head = assign();
  Node *cur = head;
  while (consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
  }

// -primary = num | "(" expr ")"
// -primary = "(" expr ")" | ident | num
// -primary = "(" expr ")" | ident args? | num
// +primary = "(" expr ")" | ident func-args? | num
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok;
  if (tok = consume_ident()) {
    if (consume("(")) {
      Node *node = new_node(ND_FUNCALL, tok);
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args();
      return node;
   }

    Var *var = find_var(tok);
    if (!var)
      //var = push_var(strndup(tok->str, tok->len));
      error_tok(tok, "undefined variable");
    return new_var(var, tok);
  }
  tok = token;
  if (tok->kind != TK_NUM)
    error_tok(tok, "expected expression");
  // そうでなければ数値のはず
  return new_num(expect_number(), tok);
}

