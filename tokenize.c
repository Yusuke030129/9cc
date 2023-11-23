#include "9cc.h"

//
// Tokenizer
//

Token *token;
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する関数
// このとき，位置を算出するために，事前に入力コード全体の文字配列の先頭ポインタを持つuser_inputが存在し，
// それと，error_atにて渡される今現在見ている文字のポインタlocの差を取って，何文字目なのかを算出している
// pos = loc  ('1 ++ 100') -  user_input ('1 ++ 100')
//                 ^ kokonoadresswo8tosuru ^kokonoadresswo5tosuru  するとposは3で%sとなり一致
// 
// pos = loc ('1 + foo + 100') -  user_input ('1 + foo + 100')                 ^ kokonoadresswo8tosuru     ^kokonoadresswo4tosuru  するとposは4で%4sとなり一致
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

char *strndup(char *p, int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, p, len);
    buf[len] = '\0';
    return buf;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  Token *tmp = token;
  token = token->next;
  free(tmp);
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
      return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str; // "<="ならば 先頭アドレスを代入 ポインタは2つ進めているので問題なし
  tok->len = len;
  cur->next = tok;
  return tok;
}


bool startswith(char *p, char *q)
{
   return memcmp(p, q, strlen(q)) == 0;
}


bool is_alpha(char c) {
    return  ( 'a' <= c && c <= 'z') || ( 'A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) {
   return is_alpha(c) || ( '0' <= c  && c <= '9');
}


// 
char *starts_with_reserved(char *p) {
  // Keyword
  // 静的なローカル変数を初期化子で初期化 静的なローカル変数の初期化は初回のみ行われる
  static char *kw[] = {"return", "if", "else", "while", "for"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
     int len = strlen(kw[i]);
     if (startswith(p, kw[i]) && !is_alnum(p[len]))
       return kw[i];
  }
  // Multi-letter punctuator
  static char *ops[] = { "==", "!=", "<=", ">="};

  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
      if (startswith(p, ops[i]))
          return ops[i];
  }
  return NULL;
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
//    // keyword
//    if (startswith(p, "return") && !is_alnum(p[6])) {
//       cur  = new_token(TK_RESERVED, cur, p, 6);
//       p += 6;
//       continue;
//    }
//
//    if ( startswith(p, "==") || startswith(p, "!=")  || startswith(p, "<=") || startswith(p, ">=") ) {
//      cur = new_token(TK_RESERVED, cur, p, 2); // length 2
//      p += 2; // ポインタ演算 アドレスを2つ進める p++++はできないもよう

      // Keyword or multi-letter punctuator
      char *kw = starts_with_reserved(p);
      if (kw) {
        int len = strlen(kw);
        cur = new_token(TK_RESERVED, cur, p, len);
        p += len;
        continue;
      }

    if (strchr("+-*/()<>;=", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1); // length 1
      continue;
    }

//    if ('a' <= *p && *p <= 'z') {
//        cur = new_token(TK_IDENT, cur, p++, 1);
//        continue;
//    }

    if (is_alpha(*p)) {
        char*q = p++;
        while (is_alnum(*p))
          p++;
    cur = new_token(TK_IDENT, cur, q, p - q);
    continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0); // lenはまだ入れない
      char *q = p; // 先頭アドレスを代入
      cur->val = strtol(p, &p, 10); // 基数10 数値を代入 &pはendptr
      cur->len = p - q; // アドレス演算 末尾 - 先頭アドレスで長さを計算してここで代入
      continue;
    }
    // cur->str = p; // 無駄
    error_at(p, "トークナイズできません");
  }

  if ( '\0' == *p )
    new_token(TK_EOF, cur, p, 0);
  return head.next;
}
