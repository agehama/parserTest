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
symbol [+\-*/\^=><(){}\[\]:\\]
other .

%%

%{
    yylloc->step();
%}
{whitespace}+ {}
{number} {
	yylval->build<ExprHolder>(Expr(std::stoi(yytext)));
	return P_Token::VALUE;
}
{number}\.[0-9]* {
	yylval->build<ExprHolder>(Expr(std::stod(yytext)));
	return P_Token::VALUE;
}
{symbol} {
	//std::cout << "Symbol(" << yytext << ")" << std::endl;
	return yytext[0];
}
"\n" {
	yylloc->lines(1);
	return P_Token::LF;
}
"\0" { return P_Token::END; }
{other} { std::cout << "Error(" << yytext << ")" << std::endl; }

%%

yy::testLexer::testLexer(std::istream* in, std::ostream* out)
    : yyFlexLexer(in, out)
{}

int yyFlexLexer::yylex()
{
    return 0;
}