
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

class Var
{

public:

	class Expression
	{
		private:

		using ExpFunc = std::function< double (Var *, Var * ) >;
		ExpFunc func;
		Var * input_x;
		Var * input_y;

		public:

		Expression( ExpFunc f, Var * x, Var * y ) : func( f ), input_x( x ), input_y( y ) {}
		double eval() const
		{
			return func( input_x, input_y );
		}

	};

	Var( double val = 0.0 ) : m_val( val ), m_grad( 1.0 )
	{

		auto init_val_exp = [ ]( Var * x, Var * y ) {
				return x->getVal();
			};
		m_val_expressions.emplace_back( Expression( init_val_exp, this, nullptr ) );

		auto init_grad_exp = [ ]( Var * x, Var * y )
			{
				return 1;
			};
		m_grad_expressions[ this ] = std::vector< Expression >{ Expression( init_grad_exp, this, nullptr ) };
	}

	void addValExpression( const Expression exp )
	{
		m_val_expressions.emplace_back( exp );
	}

	void addGradExpression( const Var * ptr, const Expression & exp )
	{
		auto it = m_grad_expressions.find( ptr );
		if( it != m_grad_expressions.end() )
		{
			it->second.emplace_back(exp);
		}
		else
		{
			m_grad_expressions[ptr] = std::vector< Expression > { exp };
		}
	}


	void addGradExpression( const Var * ptr, const std::vector<Expression> exps )
	{
		auto it = m_grad_expressions.find( ptr );
		if( it != m_grad_expressions.end() )
		{
			it->second.insert( it->second.end(), exps.begin(), exps.end() );
		}
		else
		{
			m_grad_expressions[ptr] = exps;
		}
	}

	void setVal( double val )
	{
		m_val = val;
	}

	double getVal() const
	{
		double res = 0.0;
		for(const auto & exp : m_val_expressions )
		{
			res += exp.eval();
		}
		return res;
	}
	
	double getGrad( const Var & on_input ) const
	{
		double res = 0.0;
		
		auto it = m_grad_expressions.find( &on_input );
		if( it != m_grad_expressions.end() )
		{
			auto exps = it->second;
			for( const auto & exp : exps )
			{
				res += exp.eval();
			}
		}

		return res;
	}

	Var operator+( const Var & other )
	{
		Var z;

		for( auto & [ptr, exps] : m_grad_expressions )
		{ 
			z.addGradExpression( ptr, exps );
		}
		for( auto &[ptr, exps] : other.m_grad_expressions )
		{
			z.addGradExpression( ptr, exps );
		}

		return std::move( z );	//in case RVO is not enabled
	}

private:

	double m_val;
	double m_grad;

	std::vector< Expression > m_val_expressions;
	std::unordered_map< const Var *, std::vector< Expression > > m_grad_expressions;

};
