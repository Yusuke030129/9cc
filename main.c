#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Program *prog = program();

  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
}
  // ローカル変数の領域確保
  prog->stack_size = offset;

  // 抽象構文木を下りながらコード生成 - Traverse the AST to emit assembly.
  codegen(prog);
  return 0;
}

