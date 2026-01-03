%{
#include "ast/ast.h"
#include <iostream>
#include <vector>
#include <memory>
#include <map>

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

extern FILE* yyin;

std::shared_ptr<ASTNode> ast_root = nullptr;

// 用于保持节点存活的 vector 和映射
std::vector<std::shared_ptr<ASTNode>> node_keep_alive;
std::map<ASTNode*, std::shared_ptr<ASTNode>> node_registry;

// 辅助函数：创建AST节点
ASTNode* createNode(NodeType type, const std::string& value = "") {
    auto node = std::make_shared<ASTNode>(type, value);
    node->line_number = yylineno;
    node_keep_alive.push_back(node);
    node_registry[node.get()] = node;
    return node.get();
}

// 辅助函数：创建二元操作节点
ASTNode* createBinaryOp(ASTNode* left, const std::string& op, ASTNode* right) {
    auto node = std::make_shared<ASTNode>(NODE_BINARY_OP, op);
    node->line_number = yylineno;
    auto left_it = node_registry.find(left);
    auto right_it = node_registry.find(right);
    if (left_it != node_registry.end()) {
        node->addChild(left_it->second);
    }
    if (right_it != node_registry.end()) {
        node->addChild(right_it->second);
    }
    node_keep_alive.push_back(node);
    node_registry[node.get()] = node;
    return node.get();
}

// 辅助函数：创建一元操作节点
ASTNode* createUnaryOp(const std::string& op, ASTNode* expr) {
    auto node = std::make_shared<ASTNode>(NODE_UNARY_OP, op);
    node->line_number = yylineno;
    auto it = node_registry.find(expr);
    if (it != node_registry.end()) {
        node->addChild(it->second);
    }
    node_keep_alive.push_back(node);
    node_registry[node.get()] = node;
    return node.get();
}

// 辅助函数：添加已注册的子节点
void addChildToNode(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    auto it = node_registry.find(child);
    if (it != node_registry.end()) {
        auto parent_it = node_registry.find(parent);
        if (parent_it != node_registry.end()) {
            parent_it->second->addChild(it->second);
        }
    }
}

// 辅助函数：创建 shared_ptr（使用空 deleter，因为节点由 node_registry 管理）
std::shared_ptr<ASTNode> makeSharedPtr(ASTNode* ptr) {
    return std::shared_ptr<ASTNode>(ptr, [](ASTNode*){});
}

// 辅助函数：从注册表获取 shared_ptr
std::shared_ptr<ASTNode> getSharedPtr(ASTNode* ptr) {
    auto it = node_registry.find(ptr);
    if (it != node_registry.end()) {
        return it->second;
    }
    return std::shared_ptr<ASTNode>();
}
%}

%union {
    int int_val;
    float float_val;
    char* string_val;
    ASTNode* node;
    std::vector<std::shared_ptr<ASTNode>>* node_list;
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
%type<node> ConstDecl
%type<node> VarDecl
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
%type<node> InitVal
%type<node_list> InitValList
%type<node_list> FuncFParams
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
    : { ast_root = getSharedPtr(createNode(NODE_COMP_UNIT)); }
    | CompUnit Decl
      { ast_root->addChild($2); }
    | CompUnit FuncDef
      { ast_root->addChild($2); }
    | Decl
      {
          ast_root = getSharedPtr(createNode(NODE_COMP_UNIT));
          ast_root->addChild($1);
      }
    | FuncDef
      {
          ast_root = getSharedPtr(createNode(NODE_COMP_UNIT));
          ast_root->addChild($1);
      }
    ;

Decl
    : ConstDecl
      { $$ = $1; }
    | VarDecl
      { $$ = $1; }
    ;

ConstDecl
    : CONST INT IDENTIFIER SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");  // value 存储类型
          $$->addChild(createNode(NODE_IDENTIFIER, $3));  // children[0] 存储变量名
          free($3);
      }
    | CONST FLOAT IDENTIFIER SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $3));
          free($3);
      }
    | CONST INT IDENTIFIER ASSIGN ConstExp SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");
          $$->addChild(createNode(NODE_IDENTIFIER, $3));
          $$->addChild($5);  // children[1] 存储初始值
          free($3);
      }
    | CONST FLOAT IDENTIFIER ASSIGN ConstExp SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $3));
          $$->addChild($5);
          free($3);
      }
    ;

VarDecl
    : INT IDENTIFIER SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");  // value 存储类型
          $$->addChild(createNode(NODE_IDENTIFIER, $2));  // children[0] 存储变量名
          free($2);
      }
    | FLOAT IDENTIFIER SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          free($2);
      }
    | INT IDENTIFIER ASSIGN InitVal SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);  // children[1] 存储初始值
          free($2);
      }
    | FLOAT IDENTIFIER ASSIGN InitVal SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);
          free($2);
      }
    | INT IDENTIFIER LBRACK ConstExp RBRACK SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);  // 数组大小
          free($2);
      }
    | FLOAT IDENTIFIER LBRACK ConstExp RBRACK SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);
          free($2);
      }
    | INT IDENTIFIER LBRACK ConstExp RBRACK ASSIGN InitVal SEMICOLON
      {
          $$ = createNode(NODE_DECL, "int");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);  // 数组大小
          free($2);
      }
    | FLOAT IDENTIFIER LBRACK ConstExp RBRACK ASSIGN InitVal SEMICOLON
      {
          $$ = createNode(NODE_DECL, "float");
          $$->addChild(createNode(NODE_IDENTIFIER, $2));
          $$->addChild($4);
          free($2);
      }
    ;

InitVal
    : Exp
      { $$ = $1; }
    | LBRACE RBRACE
      { $$ = createNode(NODE_STMT, ""); }
    | LBRACE InitValList RBRACE
      {
          // 数组初始化列表，创建一个临时节点
          $$ = createNode(NODE_STMT, "");
          for (auto node : *$2) {
              $$->addChild(node);
          }
          delete $2;
      }
    ;

InitValList
    : InitVal
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          $$->push_back(makeSharedPtr($1));
      }
    | InitValList COMMA InitVal
      {
          $$ = $1;
          $$->push_back(makeSharedPtr($3));
      }
    ;

FuncDef
    : VOID IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);  // value 存储函数名
          $$->addChild(createNode(NODE_TYPE, "void"));  // children[0] 存储返回类型
          $$->addChild($5);  // children[1] 是 Block
          free($2);
      }
    | INT IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          $$->addChild(createNode(NODE_TYPE, "int"));
          $$->addChild($5);
          free($2);
      }
    | FLOAT IDENTIFIER LPAREN RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          $$->addChild(createNode(NODE_TYPE, "float"));
          $$->addChild($5);
          free($2);
      }
    | VOID IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);  // value 存储函数名
          $$->addChild(createNode(NODE_TYPE, "void"));  // children[0] 存储返回类型
          // 添加所有参数节点
          for (auto param : *$4) {
              $$->addChild(param);
          }
          delete $4;
          $$->addChild($6);  // Block
          free($2);
      }
    | INT IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          $$->addChild(createNode(NODE_TYPE, "int"));
          for (auto param : *$4) {
              $$->addChild(param);
          }
          delete $4;
          $$->addChild($6);
          free($2);
      }
    | FLOAT IDENTIFIER LPAREN FuncFParams RPAREN Block
      {
          $$ = createNode(NODE_FUNC_DEF, $2);
          $$->addChild(createNode(NODE_TYPE, "float"));
          for (auto param : *$4) {
              $$->addChild(param);
          }
          delete $4;
          $$->addChild($6);
          free($2);
      }
    ;

FuncFParams
    : INT IDENTIFIER
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          // 创建参数节点，包含类型和名称
          auto param = createNode(NODE_DECL, "int");
          param->addChild(createNode(NODE_IDENTIFIER, $2));
          free($2);
          $$->push_back(makeSharedPtr(param));
      }
    | FLOAT IDENTIFIER
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          auto param = createNode(NODE_DECL, "float");
          param->addChild(createNode(NODE_IDENTIFIER, $2));
          free($2);
          $$->push_back(makeSharedPtr(param));
      }
    | INT IDENTIFIER LBRACK RBRACK
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          auto param = createNode(NODE_DECL, "int");
          param->addChild(createNode(NODE_IDENTIFIER, $2));
          free($2);
          $$->push_back(makeSharedPtr(param));
      }
    | FLOAT IDENTIFIER LBRACK RBRACK
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          auto param = createNode(NODE_DECL, "float");
          param->addChild(createNode(NODE_IDENTIFIER, $2));
          free($2);
          $$->push_back(makeSharedPtr(param));
      }
    | FuncFParams COMMA INT IDENTIFIER
      {
          $$ = $1;
          auto param = createNode(NODE_DECL, "int");
          param->addChild(createNode(NODE_IDENTIFIER, $4));
          free($4);
          $$->push_back(makeSharedPtr(param));
      }
    | FuncFParams COMMA FLOAT IDENTIFIER
      {
          $$ = $1;
          auto param = createNode(NODE_DECL, "float");
          param->addChild(createNode(NODE_IDENTIFIER, $4));
          free($4);
          $$->push_back(makeSharedPtr(param));
      }
    | FuncFParams COMMA INT IDENTIFIER LBRACK RBRACK
      {
          $$ = $1;
          auto param = createNode(NODE_DECL, "int");
          param->addChild(createNode(NODE_IDENTIFIER, $4));
          free($4);
          $$->push_back(makeSharedPtr(param));
      }
    | FuncFParams COMMA FLOAT IDENTIFIER LBRACK RBRACK
      {
          $$ = $1;
          auto param = createNode(NODE_DECL, "float");
          param->addChild(createNode(NODE_IDENTIFIER, $4));
          free($4);
          $$->push_back(makeSharedPtr(param));
      }
    ;

Block
    : LBRACE RBRACE
      { $$ = createNode(NODE_BLOCK); }
    | LBRACE BlockItemList RBRACE
      { 
          $$ = createNode(NODE_BLOCK);
          for (auto node : *$2) {
              $$->addChild(node);
          }
          delete $2;
      }
    ;

BlockItemList
    : BlockItem
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          $$->push_back(makeSharedPtr($1));
      }
    | BlockItemList BlockItem
      {
          $$ = $1;
          $$->push_back(makeSharedPtr($2));
      }
    ;

BlockItem
    : Decl
      { $$ = $1; }
    | Stmt
      { $$ = $1; }
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
          $$->addChild($3);
          $$->addChild($5);
      }
    | IF LPAREN Exp RPAREN Stmt ELSE Stmt
      {
          $$ = createNode(NODE_IF);
          $$->addChild($3);
          $$->addChild($5);
          $$->addChild($7);
      }
    | WHILE LPAREN Exp RPAREN Stmt
      {
          $$ = createNode(NODE_WHILE);
          $$->addChild($3);
          $$->addChild($5);
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
          $$->addChild($2);
      }
    ;

Exp
    : LVal ASSIGN Exp
      {
          $$ = createNode(NODE_ASSIGN);
          $$->addChild($1);
          $$->addChild($3);
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
          $$->addChild($4);
      }
    | LPAREN FLOAT RPAREN UnaryExp
      {
          $$ = createNode(NODE_UNARY_OP, "(float)");
          $$->addChild($4);
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
          $$->addChild(id_node);
          $$->addChild($3);
      }
    | IDENTIFIER LBRACK Exp RBRACK LBRACK Exp RBRACK
      {
          auto id_node = createNode(NODE_IDENTIFIER, $1);
          free($1);
          auto index1 = createNode(NODE_INDEX);
          index1->addChild(id_node);
          index1->addChild($3);
          auto index2 = createNode(NODE_INDEX);
          index2->addChild(index1);
          index2->addChild($6);
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
              $$->addChild(node);
          }
          delete $3;
          free($1);
      }
    ;

FuncRParams
    : Exp
      {
          $$ = new std::vector<std::shared_ptr<ASTNode>>();
          $$->push_back(makeSharedPtr($1));
      }
    | FuncRParams COMMA Exp
      {
          $$ = $1;
          $$->push_back(makeSharedPtr($3));
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
