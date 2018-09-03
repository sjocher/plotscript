#include "environment.hpp"

#include <cassert>
#include <cmath>

#include "environment.hpp"
#include "semantic_error.hpp"

/*********************************************************************** 
Helper Functions
**********************************************************************/

// predicate, the number of args is nargs
bool nargs_equal(const std::vector<Expression> & args, unsigned nargs){
  return args.size() == nargs;
}

/*********************************************************************** 
Each of the functions below have the signature that corresponds to the
typedef'd Procedure function pointer.
**********************************************************************/

// the default procedure always returns an expresison of type None
Expression default_proc(const std::vector<Expression> & args){
  args.size(); // make compiler happy we used this parameter
  return Expression();
};

Expression real(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args,1)) {
        if((args[0].isHeadComplex())) {
            result = real(args[0].head().asComplex());
        } else {
            throw SemanticError("Error in call for real: argument is not a complex number.");
        }
    } else {
        throw SemanticError("Error in call for real: Invalid number of arguments.");
    }
    return Expression(result);
}

Expression imag(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args,1)) {
        if((args[0].isHeadComplex())) {
            result = imag(args[0].head().asComplex());
        } else {
            throw SemanticError("Error in call for real: argument is not a complex number.");
        }
    } else {
        throw SemanticError("Error in call for real: Invalid number of arguments.");
    }
    return Expression(result);
}

Expression add(const std::vector<Expression> & args){
  // check all aruments are numbers, while adding
  std::complex<double> result = 0;
  bool complex = false;
  for(auto & a :args){
    if(a.isHeadNumber()){
      result += a.head().asNumber();      
    } else if(a.isHeadComplex()) {
        complex = true;
        result += a.head().asComplex();
    }
    else{
      throw SemanticError("Error in call to add, argument not a number");
    }
  }
  if(complex) {
      Atom a(real(result), imag(result));
      return Expression(a);
  }
  return Expression(real(result));
};

Expression mul(const std::vector<Expression> & args){
  // check all aruments are numbers, while multiplying
    std::complex<double> result(1,0);
    bool complex = false;
  for( auto & a :args){
    if(a.isHeadNumber()){
      result *= a.head().asNumber();      
    } else if(a.isHeadComplex()) {
        complex = true;
        result *= a.head().asComplex();
    }
    else{
      throw SemanticError("Error in call to mul, argument not a number");
    }
  }
    if(complex) {
        Atom a(real(result), imag(result));
        return Expression(a);
    }
  return Expression(real(result));
};

Expression subneg(const std::vector<Expression> & args){
    std::complex<double> result = 0;
    bool complex = false;
  // preconditions
  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
      result = -args[0].head().asNumber();
    } else if(args[0].isHeadComplex()) {
        complex = true;
        result = -args[0].head().asComplex();
    }
    else{
      throw SemanticError("Error in call to negate: invalid argument.");
    }
  }
  else if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber())) {
        result = args[0].head().asNumber() - args[1].head().asNumber();
    } else if(args[0].isHeadComplex() && args[1].isHeadComplex()) {
        complex = true;
        result = args[0].head().asComplex() - args[1].head().asComplex();
    } else if(args[0].isHeadComplex() && args[1].isHeadNumber()) {
        complex = true;
        result = args[0].head().asComplex() - args[1].head().asNumber();
    } else if(args[0].isHeadNumber() && args[1].isHeadComplex()) {
        complex = true;
        result = args[0].head().asNumber() - args[1].head().asComplex();
    }
    else{      
      throw SemanticError("Error in call to subtraction: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to subtraction or negation: invalid number of arguments.");
  }
    if(complex) {
        Atom a(real(result), imag(result));
        return Expression(a);
    }
  return Expression(real(result));
};

Expression div(const std::vector<Expression> & args){
  double result = 0;
  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() / args[1].head().asNumber();
    }
    else{      
      throw SemanticError("Error in call to division: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to division: invalid number of arguments.");
  }
  return Expression(result);
};

const double PI = std::atan2(0, -1);
const double EXP = std::exp(1);
const Atom I(0,1);

//Milestone 0 - Square Root
Expression sqrt(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args,1)) {
        if((args[0].isHeadNumber())) {
            if(args[0].head().asNumber() >= 0) {
                result = sqrt(args[0].head().asNumber());
            } else {
                throw SemanticError("Error in call for Square Root: Negative number.");
            }
        } else {
            throw SemanticError("Error in call for Square Root: Invalid argument.");
        }
    } else {
        throw SemanticError("Error in call for Square Root: Invalid number of arguments.");
    }
    return Expression(result);
};

//Milestone 0 - ^

Expression power(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args, 2)) {
        if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
            result = pow(args[0].head().asNumber(), args[1].head().asNumber());
        }
        else{
            throw SemanticError("Error in call to division: invalid argument.");
        }
    } else {
        throw SemanticError("Error in call to exponent: Invalid number of arguments.");
    }
    return Expression(result);
}

//Milestone 0 - ln
Expression ln(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args,1)) {
        if((args[0].isHeadNumber())) {
            if(args[0].head().asNumber() >= 0) {
                result = log(args[0].head().asNumber());
            } else {
                throw SemanticError("Error in call for Natural Log: Negative number.");
            }
        } else {
            throw SemanticError("Error in call for Natural Log: Invalid argument.");
        }
    } else {
        throw SemanticError("Error in call for Natural Log: Invalid number of arguments.");
    }
    return Expression(result);
}

Expression sine(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args, 1)) {
        if( (args[0].isHeadNumber())){
            result = sin(args[0].head().asNumber());
        }
        else{
            throw SemanticError("Error in call to Sine: invalid argument.");
        }
    } else {
        throw SemanticError("Error in call to Sine: Invalid number of arguments.");
    }
    return Expression(result);
}

Expression cosine(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args, 1)) {
        if( (args[0].isHeadNumber())){
            result = cos(args[0].head().asNumber());
        }
        else{
            throw SemanticError("Error in call to Cosine: invalid argument.");
        }
    } else {
        throw SemanticError("Error in call to Cosine: Invalid number of arguments.");
    }
    return Expression(result);
}

Expression tangent(const std::vector<Expression> & args) {
    double result = 0;
    if(nargs_equal(args, 1)) {
        if( (args[0].isHeadNumber())){
            result = tan(args[0].head().asNumber());
        }
        else{
            throw SemanticError("Error in call to Tanget: invalid argument.");
        }
    } else {
        throw SemanticError("Error in call to Tangent: Invalid number of arguments.");
    }
    return Expression(result);
}

Environment::Environment(){

  reset();
}

bool Environment::is_known(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  return envmap.find(sym.asSymbol()) != envmap.end();
}

bool Environment::is_exp(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ExpressionType);
}

Expression Environment::get_exp(const Atom & sym) const{

  Expression exp;
  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ExpressionType)){
      exp = result->second.exp;
    }
  }

  return exp;
}

void Environment::add_exp(const Atom & sym, const Expression & exp){

  if(!sym.isSymbol()){
    throw SemanticError("Attempt to add non-symbol to environment");
  }
    
  // error if overwriting symbol map
  if(envmap.find(sym.asSymbol()) != envmap.end()){
    throw SemanticError("Attempt to overwrite symbol in environemnt");
  }

  envmap.emplace(sym.asSymbol(), EnvResult(ExpressionType, exp)); 
}

bool Environment::is_proc(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ProcedureType);
}

Procedure Environment::get_proc(const Atom & sym) const{

  //Procedure proc = default_proc;

  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ProcedureType)){
      return result->second.proc;
    }
  }

  return default_proc;
}

/*
Reset the environment to the default state. First remove all entries and
then re-add the default ones.
 */
void Environment::reset(){
    envmap.clear();
  
    // Built-In value of pi
    envmap.emplace("pi", EnvResult(ExpressionType, Expression(PI)));

    // Procedure: add;
    envmap.emplace("+", EnvResult(ProcedureType, add));

    // Procedure: subneg;
    envmap.emplace("-", EnvResult(ProcedureType, subneg));

    // Procedure: mul;
    envmap.emplace("*", EnvResult(ProcedureType, mul));

    // Procedure: div;
    envmap.emplace("/", EnvResult(ProcedureType, div));
  
    // Milestone 0
    // Built-In value of e
    envmap.emplace("e", EnvResult(ExpressionType, Expression(EXP)));
    // Procedure: sqrt
    envmap.emplace("sqrt", EnvResult(ProcedureType, sqrt));
    //Procedure: pow
    envmap.emplace("^", EnvResult(ProcedureType, power));
    //Procedure: ln
    envmap.emplace("ln", EnvResult(ProcedureType, ln));
    //Procedure: Sine
    envmap.emplace("sin", EnvResult(ProcedureType, sine));
    //Procedure: Cosine
    envmap.emplace("cos", EnvResult(ProcedureType, cosine));
    //Procedure: Tangent
    envmap.emplace("tan", EnvResult(ProcedureType, tangent));
    //Built-In Value of I
    envmap.emplace("I", EnvResult(ExpressionType, Expression(I)));
    //Procedure: real
    envmap.emplace("real", EnvResult(ProcedureType, real));
    //Procedure: imag
    envmap.emplace("imag", EnvResult(ProcedureType, imag));
}
