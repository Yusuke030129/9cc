#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Function *prog = program();
  add_type(prog);

  for (Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
      Var *var = vl->var;
      offset += size_of(var->ty);
      var->offset = offset;
    }
  // ローカル変数の領域確保
    fn->stack_size = offset;
  }
  // 抽象構文木を下りながらコード生成 - Traverse the AST to emit assembly.
  codegen(prog);
  return 0;
}

