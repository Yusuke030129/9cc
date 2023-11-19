#include "9cc.h"

// 次のgen関数はこの手法をそのままCの関数で実装したものです。

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs); // AST NODE の先頭の数値までGO(先頭ノードの数値をPRINTF PUSH→gen(node->rhs)→以下pripopriposwitch 最後に  pripuを 末尾ノードまでN回繰り返し)
  gen(node->rhs); 

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}


void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

 for (Node *n = node; n; n = n->next) {
#ifdef DEBUG
   n->next == 0 ? printf("True\n"): printf("False\n") ;
   printf("アドレス= %p\n", n);
   (Node *) n;
#endif
 gen(n);
 printf("  pop rax\n"); // 複文ならばラスト文以外の結果は捨てるということか？
  }
 printf("  ret\n");
}
