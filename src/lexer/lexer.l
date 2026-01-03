%{
#include "ast/ast.h"
#include "parser.tab.h"
#include <iostream>
#include <vector>
#include <string>

extern int yylineno;

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
        case INT: return "KEYWORD";
        case FLOAT: return "KEYWORD";
        case VOID: return "KEYWORD";
        case CONST: return "KEYWORD";
        case IF: return "KEYWORD";
        case ELSE: return "KEYWORD";
        case WHILE: return "KEYWORD";
        case BREAK: return "KEYWORD";
        case CONTINUE: return "KEYWORD";
        case RETURN: return "KEYWORD";
        case AND: return "OPERATOR";
        case OR: return "OPERATOR";
        case NOT: return "OPERATOR";
        case LT: return "OPERATOR";
        case LE: return "OPERATOR";
        case GT: return "OPERATOR";
        case GE: return "OPERATOR";
        case EQ: return "OPERATOR";
        case NE: return "OPERATOR";
        case ADD: return "OPERATOR";
        case SUB: return "OPERATOR";
        case MUL: return "OPERATOR";
        case DIV: return "OPERATOR";
        case MOD: return "OPERATOR";
        case ASSIGN: return "OPERATOR";
        case SEMICOLON: return "SYMBOL";
        case COMMA: return "SYMBOL";
        case LPAREN: return "SYMBOL";
        case RPAREN: return "SYMBOL";
        case LBRACK: return "SYMBOL";
        case RBRACK: return "SYMBOL";
        case LBRACE: return "SYMBOL";
        case RBRACE: return "SYMBOL";
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

"//".*          { /* 单行注释 - 忽略 */ }

"/*"            { comment_start_line = yylineno; BEGIN(COMMENT); }

<COMMENT>"*/"   { BEGIN(INITIAL); }

<COMMENT>\n     { /* 注释中的换行符 */ }

<COMMENT>.      { /* 注释内容 */ }

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

{WHITESPACE}    { /* 忽略空白符 */ }

\n              { /* 忽略换行符 */ }

.               {
    std::cerr << "错误: 未知字符 '" << yytext << "' 在第 " << yylineno << " 行" << std::endl;
    return ERROR;
}

%%
