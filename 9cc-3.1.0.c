#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// エラーメッセージを改良

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
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
};

// 現在着目しているトークン
Token *token;
// 入力プログラム
char *user_input;

// エラー箇所を報告する関数
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,"数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error_at(token->str,"引数の個数が正しくありません");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  token = tokenize(argv[1]);

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 式の最初は数でなければならないので、それをチェックして
  // 最初のmov命令を出力
  printf("  mov rax, %d\n", expect_number());

  // `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
  // アセンブリを出力
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}


// 150行程度のあまり短いとはいえないコードですが、あまりトリッキーことは行なっていないので、上から読んでいけば読めるはずです。
// 
// 上のコードで使われているプログラミングテクニックをいくつか説明しておきましょう。
// 
//     パーサが読み込むトークン列は、グローバル変数tokenで表現することにしました。パーサは、連結リストになっているtokenをたどっていくことで入力を読み進めていきます。このようなグローバル変数を使うプログラミングスタイルは、きれいなスタイルには見えないかもしれません。しかし実際には、ここで行なっているように、入力トークン列を標準入力のようなストリームとして扱うほうがパーサのコードが読みやすくなることが多いようです。従ってここではそのようなスタイルを採用しました。
//     tokenを直接触るコードはconsumeやexpectといった関数にわけて、それ以外の関数ではtokenを直接触らないようにしました。
//     tokenize関数では連結リストを構築しています。連結リストを構築するときは、ダミーのhead要素を作ってそこに新しい要素を繋げていって、最後にhead->nextを返すようにするとコードが簡単になります。このような方法ではhead要素に割り当てられたメモリはほとんど無駄になりますが、ローカル変数をアロケートするコストはほぼゼロなので、特に気にする必要はありません。
//     callocはmallocと同じようにメモリを割り当てる関数です。mallocとは異なり、callocは割り当てられたメモリをゼロクリアします。ここでは要素をゼロクリアする手間を省くためにcallocを使うことにしました。
// 
// この改良版では空白文字がスキップできるようになったはずなので、次のようなテストを1行test.shに追加しておきましょう。
// > https://www.sigbus.info/compilerbook#%E3%82%B9%E3%83%86%E3%83%83%E3%83%974%E3%82%A8%E3%83%A9%E3%83%BC%E3%83%A1%E3%83%83%E3%82%BB%E3%83%BC%E3%82%B8%E3%82%92%E6%94%B9%E8%89%AF
