/* Lexer.l - Sys语言的词法定义 */
/* 使用Flex生成词法分析器 */

%{
#include "ast/ast.h"
#include "parser.tab.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

extern int yylineno;

/* Token结构体 */
struct Token {
    std::string type;
    std::string value;
    int line;
};

extern std::vector<Token> token_list;
extern bool collect_tokens;
int comment_start_line = 0;

std::string tokenTypeToString(int token) {
    switch (token) {
        case INT: case FLOAT: case VOID: case CONST:
        case IF: case ELSE: case WHILE: case BREAK: case CONTINUE: case RETURN:
            return "KEYWORD";
        case AND: case OR: case NOT: case LT: case LE: case GT: case GE:
        case EQ: case NE: case ADD: case SUB: case MUL: case DIV: case MOD:
        case ASSIGN:
            return "OPERATOR";
        case SEMICOLON: case COMMA: case LPAREN: case RPAREN:
        case LBRACK: case RBRACK: case LBRACE: case RBRACE:
            return "SYMBOL";
        case INT_CONST: return "NUMBER";
        case FLOAT_CONST: return "FLOAT";
        case IDENTIFIER: return "IDENTIFIER";
        default: return "UNKNOWN";
    }
}

void addToken(int token, const char* text) {
    if (collect_tokens) {
        Token t;
        t.type = tokenTypeToString(token);
        t.value = text;
        t.line = yylineno;
        token_list.push_back(t);
    }
}
%}

%option noyywrap
%option yylineno

%x COMMENT

DIGIT       [0-9]
FLOAT       ([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([eE][+-]?[0-9]+)?
IDENTIFIER  [a-zA-Z_][a-zA-Z0-9_]*
WHITESPACE  [ \t\r]+

%%

"//".*          { }
"/*"            { comment_start_line = yylineno; BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>\n     { }
<COMMENT>.      { }
<COMMENT><<EOF>> {
    std::cerr << "错误: 未闭合的注释，始于第 " << comment_start_line << " 行" << std::endl;
    BEGIN(INITIAL);
    return ERROR;
}

"int"           { addToken(INT, yytext); return INT; }
"float"         { addToken(FLOAT, yytext); return FLOAT; }
"void"          { addToken(VOID, yytext); return VOID; }
"const"         { addToken(CONST, yytext); return CONST; }
"if"            { addToken(IF, yytext); return IF; }
"else"          { addToken(ELSE, yytext); return ELSE; }
"while"         { addToken(WHILE, yytext); return WHILE; }
"break"         { addToken(BREAK, yytext); return BREAK; }
"continue"      { addToken(CONTINUE, yytext); return CONTINUE; }
"return"        { addToken(RETURN, yytext); return RETURN; }

"&&"            { addToken(AND, yytext); return AND; }
"||"            { addToken(OR, yytext); return OR; }
"!"             { addToken(NOT, yytext); return NOT; }
"<"             { addToken(LT, yytext); return LT; }
"<="            { addToken(LE, yytext); return LE; }
">"             { addToken(GT, yytext); return GT; }
">="            { addToken(GE, yytext); return GE; }
"=="            { addToken(EQ, yytext); return EQ; }
"!="            { addToken(NE, yytext); return NE; }

"+"             { addToken(ADD, yytext); return ADD; }
"-"             { addToken(SUB, yytext); return SUB; }
"*"             { addToken(MUL, yytext); return MUL; }
"/"             { addToken(DIV, yytext); return DIV; }
"%"             { addToken(MOD, yytext); return MOD; }

"="             { addToken(ASSIGN, yytext); return ASSIGN; }

";"             { addToken(SEMICOLON, yytext); return SEMICOLON; }
","             { addToken(COMMA, yytext); return COMMA; }
"("             { addToken(LPAREN, yytext); return LPAREN; }
")"             { addToken(RPAREN, yytext); return RPAREN; }
"["             { addToken(LBRACK, yytext); return LBRACK; }
"]"             { addToken(RBRACK, yytext); return RBRACK; }
"{"             { addToken(LBRACE, yytext); return LBRACE; }
"}"             { addToken(RBRACE, yytext); return RBRACE; }

{DIGIT}+        {
    addToken(INT_CONST, yytext);
    yylval.int_val = atoi(yytext);
    return INT_CONST;
}

{FLOAT}         {
    addToken(FLOAT_CONST, yytext);
    yylval.float_val = atof(yytext);
    return FLOAT_CONST;
}

{IDENTIFIER}    {
    addToken(IDENTIFIER, yytext);
    yylval.string_val = strdup(yytext);
    return IDENTIFIER;
}

{WHITESPACE}    { }
\n              { }

.               {
    std::cerr << "错误: 未知字符 '" << yytext << "' 在第 " << yylineno << " 行" << std::endl;
    return ERROR;
}

%%
