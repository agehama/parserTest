#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include <map>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

template<class T1, class T2>
inline bool SameType(const T1& t1, const T2& t2)
{
	return std::string(t1.name()) == std::string(t2.name());
}

struct Add;
struct Sub;
struct Mul;
struct Div;
struct Pow;
struct Assign;

struct DefFunc;

struct Identifer
{
	std::string name;

	Identifer() = default;

	Identifer(const std::string& name_) :
		name(name_)
	{}
};

struct FuncVal;

struct CallFunc;

template <class Op>
struct UnaryExpr;

template <class Op>
struct BinaryExpr;

struct Statement;
struct Lines;

using Expr = boost::variant<
	int,
	double,
	Identifer,
	boost::recursive_wrapper<Statement>,
	boost::recursive_wrapper<Lines>,
	boost::recursive_wrapper<DefFunc>,
	boost::recursive_wrapper<CallFunc>,
	boost::recursive_wrapper<UnaryExpr<Add>>,
	boost::recursive_wrapper<UnaryExpr<Sub>>,
	boost::recursive_wrapper<BinaryExpr<Add>>,
	boost::recursive_wrapper<BinaryExpr<Sub>>,
	boost::recursive_wrapper<BinaryExpr<Mul>>,
	boost::recursive_wrapper<BinaryExpr<Div>>,
	boost::recursive_wrapper<BinaryExpr<Pow>>,
	boost::recursive_wrapper<BinaryExpr<Assign>>
>;

struct ExprHolder
{
	Expr expr;

	ExprHolder() = default;

	ExprHolder(const Expr& expr_) :expr(expr_) {}

	~ExprHolder()
	{
		std::cout << "delete ExprHolder("  << ")" << std::endl;
	}
};

void printExpr(const Expr& expr);

using Evaluated = boost::variant<
	int,
	double,
	Identifer,
	boost::recursive_wrapper<FuncVal>
>;

extern std::map<std::string, Evaluated> globalVariables;
extern std::map<std::string, Evaluated> localVariables;

inline boost::optional<std::map<std::string, Evaluated>::iterator> findVariable(const std::string& variableName)
{
	std::map<std::string, Evaluated>::iterator itLocal = localVariables.find(variableName);
	if (itLocal != localVariables.end())
	{
		return itLocal;
	}

	std::map<std::string, Evaluated>::iterator itGlobal = globalVariables.find(variableName);
	if (itGlobal != globalVariables.end())
	{
		return itGlobal;
	}

	return boost::none;
}

template <class Op>
struct UnaryExpr
{
	Expr lhs;

	UnaryExpr(const Expr& lhs_) :
		lhs(lhs_)
	{}
};

template <class Op>
struct BinaryExpr
{
	Expr lhs;
	Expr rhs;

	BinaryExpr(const Expr& lhs_, const Expr& rhs_) :
		lhs(lhs_), rhs(rhs_)
	{}
};

struct Statement
{
	std::vector<Expr> exprs;

	Statement() = default;

	Statement(const Expr& expr) :
		exprs({ expr })
	{}

	Statement(const std::vector<Expr>& exprs_) :
		exprs(exprs_)
	{}

	void add(const Expr& expr)
	{
		exprs.push_back(expr);
	}
};

struct Lines
{
	std::vector<Expr> exprs;

	Lines() = default;

	Lines(const Expr& expr) :
		exprs({ expr })
	{}

	Lines(const std::vector<Expr>& exprs_) :
		exprs(exprs_)
	{}

	void add(const Expr& expr)
	{
		exprs.push_back(expr);
	}

	void concat(const Lines& lines)
	{
		exprs.insert(exprs.end(), lines.exprs.begin(), lines.exprs.end());
	}

	Lines& operator+=(const Lines& lines)
	{
		exprs.insert(exprs.end(), lines.exprs.begin(), lines.exprs.end());
		return *this;
	}
};

struct Arguments
{
	std::vector<Identifer> arguments;

	Arguments() = default;

	Arguments(const Identifer& identifer):
		arguments({ identifer })
	{}

	void concat(const Arguments& other)
	{
		arguments.insert(arguments.end(), other.arguments.begin(), other.arguments.end());
	}

	Arguments& operator+=(const Arguments& other)
	{
		arguments.insert(arguments.end(), other.arguments.begin(), other.arguments.end());
		return *this;
	}
};

struct FuncVal
{
	std::map<std::string, Evaluated> environment;
	std::vector<Identifer> arguments;
	Expr expr;

	FuncVal() = default;

	FuncVal(
		const std::map<std::string, Evaluated>& environment_,
		const std::vector<Identifer>& arguments_,
		const Expr& expr_) :
		environment(environment_),
		arguments(arguments_),
		expr(expr_)
	{}
};

struct DefFunc
{
	std::vector<Identifer> arguments;
	Expr expr;

	DefFunc() = default;

	DefFunc(const Expr& expr_) :
		expr(expr_)
	{}

	DefFunc(
		const std::vector<Identifer>& arguments_,
		const Expr& expr_) :
		arguments(arguments_),
		expr(expr_)
	{}

	DefFunc(
		const Arguments& arguments_,
		const Expr& expr_) :
		arguments(arguments_.arguments),
		expr(expr_)
	{}
};

inline FuncVal GetFuncVal(const Identifer& funcName)
{
	const auto funcItOpt = findVariable(funcName.name);

	if (!funcItOpt)
	{
		std::cerr << "Error(" << __LINE__ << "): function \"" << funcName.name << "\" was not found." << "\n";
	}

	const auto funcIt = funcItOpt.get();

	if (!SameType(funcIt->second.type(), typeid(FuncVal)))
	{
		std::cerr << "Error(" << __LINE__ << "): function \"" << funcName.name << "\" is not a function." << "\n";
	}

	return boost::get<FuncVal>(funcIt->second);
}

struct CallFunc
{
	boost::variant<FuncVal, Identifer> funcRef;
	std::vector<Expr> actualArguments;

	CallFunc(
		const FuncVal& funcVal_,
		const std::vector<Expr>& actualArguments_) :
		funcRef(funcVal_),
		actualArguments(actualArguments_)
	{}

	CallFunc(
		const Identifer& funcName,
		const std::vector<Expr>& actualArguments_) :
		funcRef(funcName),
		actualArguments(actualArguments_)
	{}
};

struct EvalOpt
{
	int m_0;
	double m_1;

	int m_witch;

	static EvalOpt Int(int v)
	{
		return { v,0,0 };
	}
	static EvalOpt Double(double v)
	{
		return { 0,v,1 };
	}
};


inline EvalOpt Ref(const Evaluated& lhs)
{
	if (SameType(lhs.type(), typeid(int)))
	{
		return EvalOpt::Int(boost::get<int>(lhs));
	}
	else if (SameType(lhs.type(), typeid(double)))
	{
		return EvalOpt::Double(boost::get<double>(lhs));
	}
	else if (SameType(lhs.type(), typeid(Identifer)))
	{
		const auto name = boost::get<Identifer>(lhs).name;
		const auto itOpt = findVariable(name);
		if (!itOpt)
		{
			std::cerr << "Error(" << __LINE__ << ")\n";
			return EvalOpt::Double(0);
		}
		return Ref(itOpt.get()->second);
		//return EvalOpt::Double();
	}

	std::cerr << "Error(" << __LINE__ << ")\n";
	return EvalOpt::Double(0);
}

//#define DEBUG_PRINT_EXPR

class Eval : public boost::static_visitor<Evaluated>
{
public:

	Evaluated operator()(int node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin-End int expression(" << ")" << std::endl;
#endif

		return node;
	}

	Evaluated operator()(double node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin-End double expression(" << ")" << std::endl;
#endif

		return node;
	}

	Evaluated operator()(const Identifer& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin-End Identifer expression(" << ")" << std::endl;
#endif

		return node;
	}
	
	Evaluated operator()(const UnaryExpr<Add>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin UnaryExpr<Add> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End UnaryExpr<Add> expression(" << ")" << std::endl;
#endif

		return lhs;
	}

	Evaluated  operator()(const UnaryExpr<Sub>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin UnaryExpr<Sub> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);

		const auto ref = Ref(lhs);
		if (ref.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End UnaryExpr<Sub> expression(" << ")" << std::endl;
#endif
			
			return -ref.m_0;
		}

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End UnaryExpr<Sub> expression(" << ")" << std::endl;
#endif
		
		return -ref.m_1;
	}

	Evaluated  operator()(const BinaryExpr<Add>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin BinaryExpr<Add> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);
		//return lhs + rhs;

		const auto vl = Ref(lhs);
		const auto vr = Ref(rhs);
		if (vl.m_witch == 0 && vr.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End BinaryExpr<Add> expression(" << ")" << std::endl;
#endif
			return vl.m_0 + vr.m_0;
		}

		const double dl = vl.m_witch == 0 ? vl.m_0 : vl.m_1;
		const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End BinaryExpr<Add> expression(" << ")" << std::endl;
#endif
		
		return dl + dr;
	}

	Evaluated operator()(const BinaryExpr<Sub>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin BinaryExpr<Sub> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);

		const auto vl = Ref(lhs);
		const auto vr = Ref(rhs);
		if (vl.m_witch == 0 && vr.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End BinaryExpr<Sub> expression(" << ")" << std::endl;
#endif
			return vl.m_0 - vr.m_0;
		}

		const double dl = vl.m_witch == 0 ? vl.m_0 : vl.m_1;
		const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End BinaryExpr<Sub> expression(" << ")" << std::endl;
#endif
		
		return dl - dr;
	}

	Evaluated operator()(const BinaryExpr<Mul>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin BinaryExpr<Mul> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);

		const auto vl = Ref(lhs);
		const auto vr = Ref(rhs);
		if (vl.m_witch == 0 && vr.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End BinaryExpr<Mul> expression(" << ")" << std::endl;
#endif
			return vl.m_0 * vr.m_0;
		}

		const double dl = vl.m_witch == 0 ? vl.m_0 : vl.m_1;
		const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End BinaryExpr<Mul> expression(" << ")" << std::endl;
#endif
		
		return dl * dr;
	}

	Evaluated operator()(const BinaryExpr<Div>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin BinaryExpr<Div> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);

		const auto vl = Ref(lhs);
		const auto vr = Ref(rhs);
		if (vl.m_witch == 0 && vr.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End BinaryExpr<Div> expression(" << ")" << std::endl;
#endif
			return vl.m_0 / vr.m_0;
		}

		const double dl = vl.m_witch == 0 ? vl.m_0 : vl.m_1;
		const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End BinaryExpr<Div> expression(" << ")" << std::endl;
#endif
		
		return dl / dr;
	}

	Evaluated operator()(const BinaryExpr<Pow>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin BinaryExpr<Pow> expression(" << ")" << std::endl;
#endif
		
		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);

		const auto vl = Ref(lhs);
		const auto vr = Ref(rhs);
		if (vl.m_witch == 0 && vr.m_witch == 0)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End BinaryExpr<Pow> expression(" << ")" << std::endl;
#endif	
			return vl.m_0 / vr.m_0;
		}

		const double dl = vl.m_witch == 0 ? vl.m_0 : vl.m_1;
		const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End BinaryExpr<Pow> expression(" << ")" << std::endl;
#endif

		return pow(dl, dr);
	}

	Evaluated operator()(const BinaryExpr<Assign>& node)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin Assign expression(" << ")" << std::endl;
#endif

		const Evaluated lhs = boost::apply_visitor(*this, node.lhs);
		const Evaluated rhs = boost::apply_visitor(*this, node.rhs);

		//const auto vr = Ref(rhs);
		//const double dr = vr.m_witch == 0 ? vr.m_0 : vr.m_1;

		if (!SameType(lhs.type(), typeid(Identifer)))
		{
			std::cerr << "Error(" << __LINE__ << ")\n";
#ifdef DEBUG_PRINT_EXPR
			std::cout << "End Assign expression(" << ")" << std::endl;
#endif
			
			return 0.0;
		}

		const auto name = boost::get<Identifer>(lhs).name;
		const auto it = globalVariables.find(name);
		if (it == globalVariables.end())
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "New Variable(" << name << ")\n";
#endif	
		}
		//std::cout << "Variable(" << name << ") -> " << dr << "\n";
		//variables[name] = dr;
		globalVariables[name] = rhs;

		//return dr;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End Assign expression(" << ")" << std::endl;
#endif
		return rhs;
	}

	Evaluated operator()(const DefFunc& defFunc)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin DefFunc expression(" << ")" << std::endl;
#endif

		auto val = FuncVal(globalVariables, defFunc.arguments, defFunc.expr);

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End DefFunc expression(" << ")" << std::endl;
#endif

		return val;
	}

	Evaluated operator()(const CallFunc& callFunc)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin CallFunc expression(" << ")" << std::endl;
#endif

		const auto buckUp = localVariables;

		FuncVal funcVal;

		if (SameType(callFunc.funcRef.type(), typeid(FuncVal)))
		{
			funcVal = boost::get<FuncVal>(callFunc.funcRef);
		}
		else if (auto itOpt = findVariable(boost::get<Identifer>(callFunc.funcRef).name))
		{
			const Evaluated& funcRef = (*itOpt.get()).second;
			if (SameType(funcRef.type(), typeid(FuncVal)))
			{
				funcVal = boost::get<FuncVal>(funcRef);
			}
			else
			{
				std::cerr << "Error(" << __LINE__ << "): variable \"" << (*itOpt.get()).first << "\" is not a function.\n";
				return 0;
			}
		}

		//const auto& funcVal = callFunc.funcVal;
		assert(funcVal.arguments.size() == callFunc.actualArguments.size());

		/*
		引数に与えられた式の評価
		この時点ではまだ関数の外なので、ローカル変数は変わらない。
		*/
		std::vector<Evaluated> argumentValues(funcVal.arguments.size());

		for (size_t i = 0; i < funcVal.arguments.size(); ++i)
		{
			argumentValues[i] = boost::apply_visitor(*this, callFunc.actualArguments[i]);
		}

		/*
		関数の評価
		ここでのローカル変数は関数を呼び出した側ではなく、関数が定義された側のものを使うのでローカル変数を置き換える。
		*/
		localVariables = funcVal.environment;

		for (size_t i = 0; i < funcVal.arguments.size(); ++i)
		{
			localVariables[funcVal.arguments[i].name] = argumentValues[i];
		}

		Evaluated result = boost::apply_visitor(*this, funcVal.expr);

		/*
		最後にローカル変数の環境を関数の実行前のものに戻す。
		*/
		localVariables = buckUp;

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End CallFunc expression(" << ")" << std::endl;
#endif

		return result;
	}

	Evaluated operator()(const Statement& statement)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin Statement expression(" << ")" << std::endl;
#endif
		
		Evaluated result;
		int i = 0;
		for (const auto& expr : statement.exprs)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "Evaluate expression(" << i << ")" << std::endl;
#endif	
			result = boost::apply_visitor(*this, expr);
			++i;
		}

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End Statement expression(" << ")" << std::endl;
#endif

		return result;
	}

	Evaluated operator()(const Lines& statement)const
	{
#ifdef DEBUG_PRINT_EXPR
		std::cout << "Begin Statement expression(" << ")" << std::endl;
#endif
		

		Evaluated result;
		int i = 0;
		for (const auto& expr : statement.exprs)
		{
#ifdef DEBUG_PRINT_EXPR
			std::cout << "Evaluate expression(" << i << ")" << std::endl;
#endif
			
			result = boost::apply_visitor(*this, expr);
			++i;
		}

#ifdef DEBUG_PRINT_EXPR
		std::cout << "End Statement expression(" << ")" << std::endl;
#endif

		return result;
	}
};

class Printer : public boost::static_visitor<void>
{
public:

	auto operator()(int node)const -> void
	{
		std::cout << "Int(" << node << ")";
	}

	auto operator()(double node)const -> void
	{
		std::cout << "Double(" << node << ")";
	}

	auto operator()(const Identifer& node)const -> void
	{
		std::cout << "Identifer(" << node.name << ")";
	}

	auto operator()(const UnaryExpr<Add>& node)const -> void
	{
		std::cout << "Plus(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ")";
	}

	auto operator()(const UnaryExpr<Sub>& node)const -> void
	{
		std::cout << "Minus(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Add>& node)const -> void
	{
		std::cout << "Add(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Sub>& node)const -> void
	{
		std::cout << "Sub(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Mul>& node)const -> void
	{
		std::cout << "Mul(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Div>& node)const -> void
	{
		std::cout << "Div(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Pow>& node)const -> void
	{
		std::cout << "Pow(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const BinaryExpr<Assign>& node)const -> void
	{
		std::cout << "Assign(";

		boost::apply_visitor(*this, node.lhs);

		std::cout << ", ";

		boost::apply_visitor(*this, node.rhs);

		std::cout << ")";
	}

	auto operator()(const DefFunc& defFunc)const -> void
	{
		std::cout << "DefFunc(";
		
		std::cout << "Arguments(";

		for (size_t i = 0; i < defFunc.arguments.size(); ++i)
		{
			std::cout << defFunc.arguments[i].name;
			if (i + 1 != defFunc.arguments.size())
			{
				std::cout << ", ";
			}
		}

		std::cout << "), ";

		std::cout << "Definition(";
		boost::apply_visitor(*this, defFunc.expr);
		std::cout << ")";

		std::cout << ")";
	}

	auto operator()(const CallFunc& callFunc)const -> void
	{
		std::cout << "CallFunc()\n";
	}

	void operator()(const Statement& statement)const
	{
		std::cout << "Statement begin" << std::endl;
		
		int i = 0;
		for (const auto& expr : statement.exprs)
		{
			std::cout << "Expr(" << i << "): " << std::endl;
			boost::apply_visitor(*this, expr);
			++i;
		}

		std::cout << "Statement end" << std::endl;
	}

	void operator()(const Lines& statement)const
	{
		std::cout << "Sequence(" << std::endl;

		const auto& exprs = statement.exprs;

		for (size_t i = 0; i < exprs.size(); ++i)
		{
			boost::apply_visitor(*this, exprs[i]);

			if (i + 1 != exprs.size())
			{
				std::cout << ", ";
			}

			std::cout << "\n";
		}

		std::cout << ")" << std::endl;
	}
};

inline void printExpr(const Expr& expr)
{
	boost::apply_visitor(Printer(), expr);
}

inline Evaluated evalExpr(const Expr& expr)
{
	return boost::apply_visitor(Eval(), expr);
}

inline void printEvaluated(const Evaluated& evaluated)
{
	if (SameType(evaluated.type(), typeid(int)))
	{
		std::cout << boost::get<int>(evaluated);
	}
	else if (SameType(evaluated.type(), typeid(double)))
	{
		std::cout << boost::get<double>(evaluated);
	}
	else if (SameType(evaluated.type(), typeid(Identifer)))
	{
		std::cout << boost::get<Identifer>(evaluated).name;
	}
	else
	{
		std::cout << "Unknown value";
	}
}
