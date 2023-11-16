// 9cc-7.0.0sjis.c - 比較演算子 等値 == != 関係 < <= > >= が使用可能に

// 構造体Token型 len要素追加
// consume関数 任意の文字列のポインタを受け取る仕様に
// expecet関数
// tokenize関数
// startwith関数を作って任意の長さの文字のトークン文字列をチェック 今回は2文字
// equality関数、relational関数
// 列挙体演算子のラベル追加
// 文字 '' → 文字列 "" に変更(先頭ポインタアドレスを渡す仕様) 例 '>' → ">"
// など


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
  int len;
};


// 入力プログラム
char *user_input;
// int flag = 0;
// char *flagp;
// int flagc = 0;

// 現在着目しているトークン
Token *token;



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


bool startwith(char *p, char *q)
{
   return memcmp(p, q, strlen(q)) == 0;
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

    if ( startwith(p, "==") || startwith(p, "!=")  || startwith(p, "<=") || startwith(p, ">=") ) {
      cur = new_token(TK_RESERVED, cur, p, 2); // length 2
      p += 2; // ポインタ演算 アドレスを2つ進める p++++はできないもよう
      continue;
     }

    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1); // length 1
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

    //
    // パーサー
    //

// 抽象構文木のノードの種類(ast node type) 比較演算子追加
// >と>=はコードジェネレータでサポートする必要はありません。パーサで両辺を入れ替えて<や<=として読み換えるようにしてください。

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数 integer
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
};


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

// Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 次のgen関数はこの手法をそのままCの関数で実装したものです。

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

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成 - Traverse the AST to emit assembly.
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}


