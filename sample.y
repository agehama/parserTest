%require  "3.0.4"

%code requires {	
	#include "Node.hpp"
	#include "location.hh"

	namespace yy {
        class testLexer;
    };

	void parse(const std::vector<std::string>&, Statement* statement);
}

%code {
	#include "LexConfig.hpp"

	#undef yylex
    #define yylex scanner->lex
}

%skeleton "lalr1.cc"
%parse-param {testLexer* scanner} {Statement* statements}
%locations
%define parse.error verbose
%define parse.assert
%define api.value.type variant

%token <ExprHolder> VALUE
%token LF END
%type <ExprHolder> factor
%type <ExprHolder> expr term
%left '+' '-' '*' '/' '=' '>' '<'
%right '^'

%%

input : %empty
      | input line
	  ;

line  : END
      | LF
      | expr LF { std::cout << "Expr\n"; statements->add($1.expr); }
      | error LF { yyerrok; yyclearin; }
	  ;

expr  : term
      | expr '+' expr { std::cout << "Add\n"; $$ = ExprHolder(BinaryExpr<Add>($1.expr, $3.expr)); }
      | expr '-' expr { std::cout << "Sub\n"; $$ = ExprHolder(BinaryExpr<Sub>($1.expr, $3.expr)); }
	  | expr '=' expr { std::cout << "Assign\n"; $$ = ExprHolder(BinaryExpr<Assign>($1.expr, $3.expr)); }
	  ;

term  : factor
      | term '*' term { std::cout << "Mul("; printExpr($1.expr); std::cout <<", "; printExpr($3.expr); std::cout <<")\n"; $$ = ExprHolder(BinaryExpr<Mul>($1.expr, $3.expr)); }
      | term '/' term { std::cout << "Div\n"; $$ = ExprHolder(BinaryExpr<Div>($1.expr, $3.expr)); }
      | term '^' term { std::cout << "Pow\n"; $$ = ExprHolder(BinaryExpr<Pow>($1.expr, $3.expr)); }
	  ;

factor: VALUE         { $$ = $1;  printExpr($$.expr); }
      | '(' expr ')'  { std::cout << "(Expr)\n"; $$ = $2; std::cout << "()2\n"; }
	  | '+' factor    { std::cout << "Plus\n"; $$ = ExprHolder(UnaryExpr<Add>($2.expr)); }
      | '-' factor    { std::cout << "Minus\n"; $$ = ExprHolder(UnaryExpr<Sub>($2.expr)); }
	  ;

%%

#include <sstream>

/*
https://coldfix.eu/2015/05/16/bison-c++11/
*/

void yy::parser::error(const parser::location_type& l, const std::string& m)
{
    throw yy::parser::syntax_error(l, m);
}

void parse(const std::vector<std::string>& exprs, Statement* out)
{
	for (int row = 0; row < exprs.size(); ++row) {
		const std::string& line = exprs[row];
		std::istringstream in(line);
		yy::testLexer scanner(&in);
		yy::parser parser(&scanner, out);
		try {
			int result = parser.parse();
			if (result != 0) {
				throw std::runtime_error("Unknown parsing error");
			}
		}
		catch (yy::parser::syntax_error& e) {
			int col = e.location.begin.column;
			int len = 1 + e.location.end.column - col;

			std::cerr << e.what() << "\n"
				<< "in row " << row << " col " << col << ":\n\n"
				<< "    " << line << "\n"
				<< "    " << std::string(col - 1, ' ') << std::string(len, '^') << std::endl;
				//throw yy::parser::syntax_error(e.location, msg.str());
				throw e;
		}
	}
}

int main()
{
	std::string str;
	//std::cin >> str;
	str = "(1*2+3*(4+5/6))";

	str.push_back('\n');

	std::vector<std::string> lines;

	lines.push_back(str);
	Statement* pStatement = new Statement;

	std::cout << "=====================================" << std::endl;
	parse(lines, pStatement);
	std::cout << "-------------------------------------" << std::endl;

	printStatement(*pStatement);

	delete pStatement;
}