%{
#include "ast/ast.h"
#include <iostream>
#include <vector>
#include <memory>

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

extern FILE* yyin;

ASTNode* ast_root = nullptr;

// 辅助函数：创建AST节点
ASTNode* createNode(NodeType type, const std::string& value = "") {
    auto node = new ASTNode(type, value);
    node->line_number = yylineno;
    return node;
}

// 辅助函数：创建二元操作节点
ASTNode* createBinaryOp(ASTNode* left, const std::string& op, ASTNode* right) {
    auto node = createNode(NODE_BINARY_OP, op);
    node->addChild(std::shared_ptr<ASTNode>(left));
    node->addChild(std::shared_ptr<ASTNode>(right));
    return node;
}

// 辅助函数：创建一元操作节点
ASTNode* createUnaryOp(const std::string& op, ASTNode* expr) {
    auto node = createNode(NODE_UNARY_OP, op);
    node->addChild(std::shared_ptr<ASTNode>(expr));
    return node;
}
%}

%union {
    int int_val;
    float float_val;
    char* string_val;
    ASTNode* node;
    std::vector<ASTNode*>* node_list;
}

%token<int_val> INT_CONST
%token<float_val> FLOAT_CONST
%token<string_val> IDENTIFIER
%token ERROR

%token INT FLOAT VOID CONST IF ELSE WHILE BREAK CONTINUE RETURN

%token AND OR NOT
%token LT LE GT GE EQ NE
%token ADD SUB MUL DIV MOD
%token ASSIGN
%token SEMICOLON COMMA LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE

%type<node> CompUnit
%type<node> Decl
%type<node> FuncDef
%type<node> Block
%type<node_list> BlockItemList
%type<node> BlockItem
%type<node> Stmt
%type<node> Exp
%type<node> PrimaryExp
%type<node> UnaryExp
%type<node> MulExp
%type<node> AddExp
%type<node> RelExp
%type<node> EqExp
%type<node> LAndExp
%type<node> LOrExp
%type<node> ConstExp
%type<node> LVal
%type<node_list> FuncRParams

%right ASSIGN
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left ADD SUB
%left MUL DIV MOD
%right NOT

%%

CompUnit
    : { ast_root = createNode(NODE_COMP_UNIT); }
    | CompUnit Decl
      { ast_root->addChild(std::shared_ptr<ASTNode>($2)); }
    | CompUnit FuncDef
      { ast_root->addChild(std::shared_ptr<ASTNode>($2)); }
    | Decl
      {
          ast_root = createNode(NODE_COMP_UNIT);
          ast_root->addChild(std::shared_ptr<ASTNode>($1));
      }
    | FuncDef
      {
          ast_root = createNode(NODE_COMP_UNIT);
          ast_root->addChild(std::shared_ptr<ASTNode>($1));
      }
    ;

Decl
    : ConstDecl
    | VarDecl
    ;

ConstDecl
    : CONST INT IDENTIFIER SEMICOLON
    | CONST FLOAT IDENTIFIER SEMICOLON
    | CONST INT IDENTIFIER ASSIGN ConstExp SEMICOLON
    | CONST FLOAT IDENTIFIER ASSIGN ConstExp SEMICOLON
    ;

VarDecl
    : INT IDENTIFIER SEMICOLON
    | FLOAT IDENTIFIER SEMICOLON
    | INT IDENTIFIER ASSIGN InitVal SEMICOLON
    | FLOAT IDENTIFIER ASSIGN InitVal SEMICOLON
    | INT IDENTIFIER LBRACK ConstExp RBRACK SEMICOLON
    | FLOAT IDENTIFIER LBRACK ConstExp RBRACK SEMICOLON
    | INT IDENTIFIER LBRACK ConstExp RBRACK ASSIGN InitVal SEMICOLON
    | FLOAT IDENTIFIER LBRACK ConstExp RBRACK ASSIGN InitVal SEMICOLON
    ;

InitVal
    : Exp
    | LBRACE RBRACE
    | LBRACE InitValList RBRACE
    ;

InitValList
    : InitVal
    | InitValList COMMA InitVal
    ;

FuncDef
    : VOID IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($5));
      }
    | INT IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($5));
      }
    | FLOAT IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($5));
      }
    | VOID IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($6));
      }
    | INT IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($6));
      }
    | FLOAT IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          free($2);
          $$->addChild(std::shared_ptr<ASTNode>($6));
      }
    ;

FuncFParams
    : INT IDENTIFIER
    | FLOAT IDENTIFIER
    | INT IDENTIFIER LBRACK RBRACK
    | FLOAT IDENTIFIER LBRACK RBRACK
    | FuncFParams COMMA INT IDENTIFIER
    | FuncFParams COMMA FLOAT IDENTIFIER
    | FuncFParams COMMA INT IDENTIFIER LBRACK RBRACK
    | FuncFParams COMMA FLOAT IDENTIFIER LBRACK RBRACK
    ;

Block
    : LBRACE RBRACE
      { $$ = createNode(NODE_BLOCK); }
    | LBRACE BlockItemList RBRACE
      { 
          $$ = createNode(NODE_BLOCK);
          for (auto node : *$2) {
              $$->addChild(std::shared_ptr<ASTNode>(node));
          }
          delete $2;
      }
    ;

BlockItemList
    : BlockItem
      {
          $$ = new std::vector<ASTNode*>();
          $$->push_back($1);
      }
    | BlockItemList BlockItem
      {
          $$ = $1;
          $$->push_back($2);
      }
    ;

BlockItem
    : Decl
    | Stmt
    ;

Stmt
    : SEMICOLON
      { $$ = createNode(NODE_STMT); }
    | Exp SEMICOLON
      { $$ = $1; }
    | Block
    | IF LPAREN Exp RPAREN Stmt
      {
          $$ = createNode(NODE_IF);
          $$->addChild(std::shared_ptr<ASTNode>($3));
          $$->addChild(std::shared_ptr<ASTNode>($5));
      }
    | IF LPAREN Exp RPAREN Stmt ELSE Stmt
      {
          $$ = createNode(NODE_IF);
          $$->addChild(std::shared_ptr<ASTNode>($3));
          $$->addChild(std::shared_ptr<ASTNode>($5));
          $$->addChild(std::shared_ptr<ASTNode>($7));
      }
    | WHILE LPAREN Exp RPAREN Stmt
      {
          $$ = createNode(NODE_WHILE);
          $$->addChild(std::shared_ptr<ASTNode>($3));
          $$->addChild(std::shared_ptr<ASTNode>($5));
      }
    | BREAK SEMICOLON
      { $$ = createNode(NODE_BREAK); }
    | CONTINUE SEMICOLON
      { $$ = createNode(NODE_CONTINUE); }
    | RETURN SEMICOLON
      { $$ = createNode(NODE_RETURN); }
    | RETURN Exp SEMICOLON
      {
          $$ = createNode(NODE_RETURN);
          $$->addChild(std::shared_ptr<ASTNode>($2));
      }
    ;

Exp
    : LVal ASSIGN Exp
      {
          $$ = createNode(NODE_ASSIGN);
          $$->addChild(std::shared_ptr<ASTNode>($1));
          $$->addChild(std::shared_ptr<ASTNode>($3));
      }
    | LOrExp
      { $$ = $1; }
    ;

ConstExp
    : AddExp
      { $$ = $1; }
    ;

PrimaryExp
    : LPAREN Exp RPAREN
      { $$ = $2; }
    | LPAREN INT RPAREN UnaryExp
      {
          $$ = createNode(NODE_UNARY_OP, "(int)");
          $$->addChild(std::shared_ptr<ASTNode>($4));
      }
    | LPAREN FLOAT RPAREN UnaryExp
      {
          $$ = createNode(NODE_UNARY_OP, "(float)");
          $$->addChild(std::shared_ptr<ASTNode>($4));
      }
    | LVal
      { $$ = $1; }
    | INT_CONST
      {
          $$ = createNode(NODE_INT_CONST, std::to_string($1));
      }
    | FLOAT_CONST
      {
          $$ = createNode(NODE_FLOAT_CONST, std::to_string($1));
      }
    ;

LVal
    : IDENTIFIER
      {
          $$ = createNode(NODE_IDENTIFIER, $1);
          free($1);
      }
    | IDENTIFIER LBRACK Exp RBRACK
      {
          auto id_node = createNode(NODE_IDENTIFIER, $1);
          free($1);
          $$ = createNode(NODE_INDEX);
          $$->addChild(std::shared_ptr<ASTNode>(id_node));
          $$->addChild(std::shared_ptr<ASTNode>($3));
      }
    | IDENTIFIER LBRACK Exp RBRACK LBRACK Exp RBRACK
      {
          auto id_node = createNode(NODE_IDENTIFIER, $1);
          free($1);
          auto index1 = createNode(NODE_INDEX);
          index1->addChild(std::shared_ptr<ASTNode>(id_node));
          index1->addChild(std::shared_ptr<ASTNode>($3));
          auto index2 = createNode(NODE_INDEX);
          index2->addChild(std::shared_ptr<ASTNode>(index1));
          index2->addChild(std::shared_ptr<ASTNode>($6));
          $$ = index2;
      }
    ;

UnaryExp
    : PrimaryExp
      { $$ = $1; }
    | ADD UnaryExp
      { $$ = createUnaryOp("+", $2); }
    | SUB UnaryExp
      { $$ = createUnaryOp("-", $2); }
    | NOT UnaryExp
      { $$ = createUnaryOp("!", $2); }
    | IDENTIFIER LPAREN RPAREN
      {
          $$ = createNode(NODE_CALL, $1);
          free($1);
      }
    | IDENTIFIER LPAREN FuncRParams RPAREN
      {
          $$ = createNode(NODE_CALL, $1);
          for (auto node : *$3) {
              $$->addChild(std::shared_ptr<ASTNode>(node));
          }
          delete $3;
          free($1);
      }
    ;

FuncRParams
    : Exp
      {
          $$ = new std::vector<ASTNode*>();
          $$->push_back($1);
      }
    | FuncRParams COMMA Exp
      {
          $$ = $1;
          $$->push_back($3);
      }
    ;

MulExp
    : UnaryExp
      { $$ = $1; }
    | MulExp MUL UnaryExp
      { $$ = createBinaryOp($1, "*", $3); }
    | MulExp DIV UnaryExp
      { $$ = createBinaryOp($1, "/", $3); }
    | MulExp MOD UnaryExp
      { $$ = createBinaryOp($1, "%", $3); }
    ;

AddExp
    : MulExp
      { $$ = $1; }
    | AddExp ADD MulExp
      { $$ = createBinaryOp($1, "+", $3); }
    | AddExp SUB MulExp
      { $$ = createBinaryOp($1, "-", $3); }
    ;

RelExp
    : AddExp
      { $$ = $1; }
    | RelExp LT AddExp
      { $$ = createBinaryOp($1, "<", $3); }
    | RelExp LE AddExp
      { $$ = createBinaryOp($1, "<=", $3); }
    | RelExp GT AddExp
      { $$ = createBinaryOp($1, ">", $3); }
    | RelExp GE AddExp
      { $$ = createBinaryOp($1, ">=", $3); }
    ;

EqExp
    : RelExp
      { $$ = $1; }
    | EqExp EQ RelExp
      { $$ = createBinaryOp($1, "==", $3); }
    | EqExp NE RelExp
      { $$ = createBinaryOp($1, "!=", $3); }
    ;

LAndExp
    : EqExp
      { $$ = $1; }
    | LAndExp AND EqExp
      { $$ = createBinaryOp($1, "&&", $3); }
    ;

LOrExp
    : LAndExp
      { $$ = $1; }
    | LOrExp OR LAndExp
      { $$ = createBinaryOp($1, "||", $3); }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "语法错误: " << s << " 在第 " << yylineno << " 行" << std::endl;
}
