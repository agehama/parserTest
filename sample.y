%require  "3.0.4"

%code requires {	
	#include "Node.hpp"

	namespace yy {
        class testLexer;
    };

	void parse(const std::vector<std::string>&, Lines* program);

	//#define PRINT_EXPR(expr) 
	#define PRINT_EXPR(expr) printExpr(expr)
}

%code {
	#include "LexConfig.hpp"

	#undef yylex
    #define yylex scanner->lex
}

%skeleton "lalr1.cc"
%parse-param {testLexer* scanner} {Lines* program}
%locations
%define parse.error verbose
%define parse.assert
%define api.value.type variant

%token <Identifer> NAME
%token <Expr> VALUE
%token LF arrow
%type <Expr> factor
%type <Expr> expr term def_func
%type <Lines> lines expr_seq
%type <Arguments> arguments
%left '+' '-' '*' '/' '=' '>' '<'
%right '^'

%%

prog  : %empty
	  | lines    { *program = $1; }
      | error LF { yyerrok; yyclearin; }
	  ;

def_func : '(' ')' arrow '(' ')'                 { $$ = DefFunc(); }
         | '(' ')' arrow '(' lines ')'           { $$ = DefFunc($5); }
         | '(' arguments ')' arrow '(' lines ')' { $$ = DefFunc($2, $6); }
		 ;

arguments : NAME               { printExpr($1); $$ = $1; }
          | NAME ',' arguments { printExpr($1); $$ = $1; $$.concat($3); }
		  ;

lines : LF             {}
      | expr_seq       { $$.concat($1); }
	  | expr_seq LF    { $$.concat($1); }
	  ;

expr_seq : expr              { $$ = $1; }
	     | expr ',' expr_seq { $$ = $1; $$.concat($3); }
		 | expr LF expr_seq  { $$ = $1; $$.concat($3); }
	     ;

expr  : term          { $$ = $1;  /*PRINT_EXPR($$);*/ }
      | expr '+' expr { /*std::cout << "Add\n";*/ $$ = BinaryExpr<Add>($1, $3); }
      | expr '-' expr { /*std::cout << "Sub\n";*/ $$ = BinaryExpr<Sub>($1, $3); }
	  | expr '=' expr { /*std::cout << "Assign\n";*/ $$ = BinaryExpr<Assign>($1, $3); }
	  ;

term  : factor        { $$ = $1;  /*PRINT_EXPR($$);*/ }
      | term '*' term { /*std::cout << "Mul("; printExpr($1); std::cout <<", "; printExpr($3); std::cout <<")\n";*/ $$ = BinaryExpr<Mul>($1, $3); }
      | term '/' term { /*std::cout << "Div\n";*/ $$ = BinaryExpr<Div>($1, $3); }
      | term '^' term { /*std::cout << "Pow\n";*/ $$ = BinaryExpr<Pow>($1, $3); }
	  ;

factor: VALUE         { $$ = $1;  /*PRINT_EXPR($$);*/ }
      | '(' expr ')'  { /*std::cout << "(Expr)\n";*/ $$ = $2; }
	  | '(' lines ')' {  $$ = $2; }
	  | '+' factor    { /*std::cout << "Plus\n";*/ $$ = UnaryExpr<Add>($2); }
      | '-' factor    { /*std::cout << "Minus\n";*/ $$ = UnaryExpr<Sub>($2); }
	  | def_func      { $$ = $1; }
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

bool parse(const std::string& program, Lines* out)
{
	std::istringstream in(program);
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
			<< "in " << program << "\n"
			<< "    " << std::string(col - 1, ' ') << std::string(len, '^') << std::endl;
			
			return false;
	}

	return true;
}

int main()
{
	std::vector<std::string> test_ok({
		"(1*2 + 3*(4 + 5/6))",
		"1 + 2, 3 + 4",
		"\n 4*5",
		"1 + 1 \n 2 + 3",
		"1 + 2 \n 3 + 4 \n",
		"1 + 1, \n 2 + 3",
		"1 + 1 \n \n \n 2 + 3",
		"1 + 2 \n , 4*5",
		"()->()",
		"()->(1 + 2)",
		"()->(1 + 2 \n 3)",
		"(x, y)->(x + y)"
	});

	std::vector<std::string> test_ng({
		", 3*4",
		"1 + 1 , , 2 + 3",
		"1 + 2, 3 + 4,",
		"1 + 2, \n , 3 + 4",
		"1 + 3 * , 4 + 5",
		"1 + 3 * \n 4 + 5"
	});

	int ok_wrongs = 0;
	int ng_wrongs = 0;

	std::cout << "==================== Test Case OK ====================" << std::endl;
	for(size_t i = 0; i < test_ok.size(); ++i)
	{
		std::cout << "Case[" << i << "]\n\n";

		std::cout << "input:\n";
		std::cout << test_ok[i] << "\n\n";

		std::cout << "preprocess:\n";
		std::cout << preprocess(test_ok[i]) << "\n\n";

		std::cout << "parse:\n";
		
		Lines expr;
		const bool succeed = parse(preprocess(test_ok[i]), &expr);
		
		printExpr(expr);

		std::cout << "\n";

		if(succeed)
		{
		    /*
			std::cout << "eval:\n";
			printEvaluated(evalExpr(expr));
			std::cout << "\n";
			*/
		}
		else
		{
			std::cout << "[Wrong]\n";
			++ok_wrongs;
		}

		std::cout << "-------------------------------------" << std::endl;
	}

	std::cout << "==================== Test Case NG ====================" << std::endl;
	for(size_t i = 0; i < test_ng.size(); ++i)
	{
		std::cout << "Case[" << i << "]\n\n";

		std::cout << "input:\n";
		std::cout << test_ng[i] << "\n\n";

		std::cout << "preprocess:\n";
		std::cout << preprocess(test_ng[i]) << "\n\n";

		std::cout << "parse:\n";

		Lines expr;
		const bool failed = !parse(preprocess(test_ng[i]), &expr);
		
		printExpr(expr);

		std::cout << "\n";

		if(failed)
		{
			std::cout << "no result\n";
		}
		else
		{
			std::cout << "eval:\n";
			printEvaluated(evalExpr(expr));
			std::cout << "\n";
			std::cout << "[Wrong]\n";
			++ng_wrongs;
		}

		std::cout << "-------------------------------------" << std::endl;
	}

	std::cout << "Result:\n";
	std::cout << "Correct programs: (Wrong / All) = (" << ok_wrongs << " / " << test_ok.size() << ")\n";
	std::cout << "Wrong   programs: (Wrong / All) = (" << ng_wrongs << " / " << test_ng.size() << ")\n";
}
