%{
#include "ast/ast.h"
#include "parser.tab.h"
#include <iostream>

extern int yylineno;
%}

%option noyywrap
%option yylineno

%x COMMENT

DIGIT [0-9]
FLOAT ([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([eE][+-]?[0-9]+)?
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*
WHITESPACE [ \t\r]+

%{
int comment_start_line = 0;
%}

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

"int"           { return INT; }
"float"         { return FLOAT; }
"void"          { return VOID; }
"const"         { return CONST; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }
"return"        { return RETURN; }

"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"=="            { return EQ; }
"!="            { return NE; }

"+"             { return ADD; }
"-"             { return SUB; }
"*"             { return MUL; }
"/"             { return DIV; }
"%"             { return MOD; }

"="             { return ASSIGN; }

";"             { return SEMICOLON; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACK; }
"]"             { return RBRACK; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }

{DIGIT}+        { 
                    yylval.int_val = atoi(yytext);
                    return INT_CONST;
                }

{FLOAT}         {
                    yylval.float_val = atof(yytext);
                    return FLOAT_CONST;
                }

{IDENTIFIER}    {
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
