%option c++
%option noyywrap

%{
#include <iostream>
#include <string>
#include "sample.tab.h"
#include "LexConfig.hpp"

typedef yy::parser::token P_Token;
%}

whitespace [ \t\r]
number	0|[1-9][0-9]*
symbol [+\-*/\^=><(){}\[\]:\\,]
identifer [a-zA-Z_][a-zA-Z0-9_]*
other .

%%

%{
    yylloc->step();
%}
{whitespace}+ {}
{number} {
	yylval->build<Expr>(Expr(std::stoi(yytext)));
	return P_Token::VALUE;
}
{number}\.[0-9]* {
	yylval->build<Expr>(Expr(std::stod(yytext)));
	return P_Token::VALUE;
}
{identifer} {
	const auto ii = Identifer(yytext);
	printExpr(ii);
	yylval->build<Identifer>(ii);
	return P_Token::NAME;
}
"->" {
	return P_Token::arrow;
}
{symbol} {
	return yytext[0];
}
"\n" {
	yylloc->lines(1);
	return P_Token::LF;
}
{other} { std::cout << "Error(" << yytext << ")" << std::endl; }

%%

yy::testLexer::testLexer(std::istream* in, std::ostream* out)
    : yyFlexLexer(in, out)
{}

int yyFlexLexer::yylex()
{
    return 0;
}
