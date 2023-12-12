#include "9cc.h"

int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}
int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Program *prog = program(); // funcs? non non non => funcs+global-vars yes yes yes
  add_type(prog);
  
  // Assign offsets to local variables.
  for (Function *fn = prog->fns; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next) {
      Var *var = vl->var;
      offset += size_of(var->ty);
      var->offset = offset;
    }
  // ローカル変数の領域確保
//    fn->stack_size = offset;
    fn->stack_size = align_to(offset, 8);
  }
  // 抽象構文木を下りながらコード生成 - Traverse the AST to emit assembly.
  codegen(prog);
  return 0;
}

