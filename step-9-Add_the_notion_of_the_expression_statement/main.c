#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = program();

  // 抽象構文木を下りながらコード生成 - Traverse the AST to emit assembly.
  codegen(node);
  return 0;
}

