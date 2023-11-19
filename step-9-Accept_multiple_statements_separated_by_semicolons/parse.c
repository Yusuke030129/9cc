#include "9cc.h"


//
// parser
//

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *stmt();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();



Node *program() {
  Node head;
  head.next = NULL;
  Node *cur = &head;
  while(!(at_eof())) {
    cur->next = stmt();
    cur = cur->next;
}
  return head.next;
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

// expr = equality
Node *expr() {
 return equality();
}


// equality = relational( "==" relational| "!=" relational )*
Node *equality() {
  Node *node = relational();
  
  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}


// relational = add( "<" add | "<=" add | ">" add | ">=" add )*

Node *relational() {
  Node *node = add();
  
  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}


// add = mul ("+" mul | "-" mul)* ※ これまではexpr = mul ("+" mul | "-" mul)*

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}


// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary()); // 0 - primary で反転  何度繰り返しても結果は -が偶数 =>  lhs 0  -  rhs  (0 - primary) =  primary  -が奇数 lhs  x -  rhs primary = - primary
  return primary();
}


// primary = num | "(" expr ")"
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // そうでなければ数値のはず
  return new_num(expect_number());
}
