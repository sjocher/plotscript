#include "expression.hpp"

#include <sstream>
#include <list>
#include <iostream>
#include <unordered_map>

#include "environment.hpp"
#include "semantic_error.hpp"

Expression::Expression(){}

Expression::Expression(const Atom & a){
  m_head = a;
}

Expression::Expression(const std::list<Expression> & list) {
    Atom a("list");
    m_head = a;
    m_head.tagAtom();
    m_list.clear();
    for(auto e = list.begin(); e != list.end(); ++e)
        m_list.push_back(*e);
}

// recursive copy
Expression::Expression(const Expression & a){
  m_head = a.m_head;
  for(auto e : a.m_tail){
    m_tail.push_back(e);
  }
  if(a.m_head.isTagged()) m_head.tagAtom();
  if(a.m_head.isLambda()) m_head.markLambda();
  m_list.clear();
  for(auto e = a.m_list.begin(); e != a.m_list.end(); ++e)
      m_list.push_back(*e);
  properties.clear();
    for(auto e = a.properties.begin(); e != a.properties.end(); e++) {
        properties.emplace(e->first, e->second);
    }
}

Expression & Expression::operator=(const Expression & a){
  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
    }
    if(a.m_head.isTagged()) m_head.tagAtom();
    if(a.m_head.isLambda()) m_head.markLambda();
    m_list.clear();
    for(auto e = a.m_list.begin(); e != a.m_list.end(); ++e)
        m_list.push_back(*e);
    properties.clear();
      for(auto e = a.properties.begin(); e != a.properties.end(); e++) {
          properties.emplace(e->first, e->second);
      }
  }
  return *this;
}


Atom & Expression::head(){
  return m_head;
}

const Atom & Expression::head() const{
  return m_head;
}

bool Expression::isHeadNumber() const noexcept{
  return m_head.isNumber();
}

bool Expression::isHeadSymbol() const noexcept{
  return m_head.isSymbol();
}

bool Expression::isHeadComplex() const noexcept {
    return m_head.isComplex();
}

bool Expression::isHeadList() const noexcept {
    return m_head.isTagged();
}

void Expression::append(const Atom & a){
  m_tail.emplace_back(a);
}

Expression * Expression::tail(){
  Expression * ptr = nullptr;
  if(m_tail.size() > 0){
    ptr = &m_tail.back();
  }

  return ptr;
}

Expression::ConstIteratorType Expression::tailConstBegin() const noexcept{
  return m_tail.cbegin();
}

Expression::ConstIteratorType Expression::tailConstEnd() const noexcept{
  return m_tail.cend();
}

Expression apply(const Atom & op, const std::vector<Expression> & args, const Environment & env){
  // head must be a symbol
  if(!op.isSymbol()){
    throw SemanticError("Error during evaluation: procedure name not symbol");
  }
  // must map to a proc
  if(!env.is_proc(op)){
    throw SemanticError("Error during evaluation: symbol does not name a procedure");
  }
  // map from symbol to proc
  Procedure proc = env.get_proc(op);
  //applying lambda
  return proc(args);
}

Expression Expression::handle_lookup(const Atom & head, const Environment & env){
    if(head.isSymbol()){ // if symbol is in env return value
      if(env.is_exp(head)){
          Expression temp = env.get_exp(head);
          return env.get_exp(head);
      }else if(head.asSymbol() == "list") {
          std::list<Expression> list;
          return Expression(list);
      }
      else{
	throw SemanticError("Error during evaluation: unknown symbol");
      }
    }
    else if(head.isNumber()){
      return Expression(head);
    }
    else if(head.isString()) {
        return Expression(head);
    }
    else{
      throw SemanticError("Error during evaluation: Invalid type in terminal expression");
    }
}

Expression Expression::handle_begin(Environment & env){
  
  if(m_tail.size() == 0){
    throw SemanticError("Error during evaluation: zero arguments to begin");
  }

  // evaluate each arg from tail, return the last
  Expression result;
  for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
    result = it->eval(env);
  }
  
  return result;
}


Expression Expression::handle_define(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to define");
  }
  
  // tail[0] must be symbol
  if(!m_tail[0].isHeadSymbol()){
    throw SemanticError("Error during evaluation: first argument to define not symbol");
  }

  // but tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();
  if((s == "define") || (s == "begin")){
    throw SemanticError("Error during evaluation: attempt to redefine a special-form");
  }
  
  if(env.is_proc(m_head)){
    throw SemanticError("Error during evaluation: attempt to redefine a built-in procedure");
  }
	
  // eval tail[1]
  Expression result = m_tail[1].eval(env);
  if(env.is_exp(m_head)){
    throw SemanticError("Error during evaluation: attempt to redefine a previously defined symbol");
  }
  //and add to env
  env.add_exp(m_tail[0].head(), result);
  
  return result;
}

Expression Expression::handle_list(Environment &env) {
    Expression result(m_head);
    result.m_head.tagAtom();
    for(auto e = m_tail.begin(); e != m_tail.end(); ++e)
        result.m_list.push_back(e->eval(env));
    return result;
}

Expression Expression::handle_lambda() {
    if(m_tail.size() != 2)
        throw SemanticError("Error during evaluation: invalid number of arguments to lambda");
    Expression result(m_head);
    result.m_head.markLambda();
    //add each parameter to vector of expressions, which is a needed for a procedure.
    result.m_list.push_back(m_tail[0].head());
    for(auto e = m_tail[0].tailConstBegin(); e != m_tail[0].tailConstEnd(); ++e) {
        result.m_list.push_back(*e);
    }
    //add the expression to m_tail
    result.m_tail.push_back(m_tail[1]);
    return result;
}

Expression Expression::eval_lambda(const Atom & op, const std::vector<Expression> & args, const Environment & env) {
    Environment pocketenv = env;
    Expression lfunc = pocketenv.get_exp(op);
    if(args.size() != lfunc.listSize())
        throw SemanticError("Error during lambda evaluation: wrong number of arguments.");
    int argCnt = 0;
    for(auto e = lfunc.listConstBegin(); e != lfunc.listConstEnd(); ++e ) {
        Atom a = e->head();
        pocketenv.add_exp(a, args[argCnt]);
        argCnt++;
    }
    Expression result = lfunc.m_tail[0].eval(pocketenv);;
    //need to copy properties here
    for(auto e = lfunc.mapConstBegin(); e != lfunc.mapConstEnd(); ++e)
        result.properties.emplace(e->first, e->second);
    return result;
}

Expression Expression::handle_apply(Environment &env) {
    if(m_tail.size() != 2)
        throw SemanticError("Error: invalid number of arguments to apply");
    //evaluate the first and second arguments of apply, make sure m_tail[0] is procedure/lambda
    Expression pdr = m_tail[0];
    if(pdr.m_tail.size() != 0)
        throw SemanticError("Error: first argument to apply is not a procedure.");
    if(!env.is_proc(pdr.head()) && !(env.get_exp(pdr.head()).isHeadLambda()))
        throw SemanticError("Error: first argument to apply is not a procedure.");
    //make sure m_tail[1] is a list
    Expression lst = m_tail[1].eval(env);
    if(!lst.isHeadList())
        throw SemanticError("Error: second argument to apply is not a list");
    //copy the list of values into a vector of arguments for easier translation
    std::vector<Expression> args;
    for(auto e = lst.listConstBegin(); e != lst.listConstEnd(); ++e)
        args.push_back(*e);
    //now evaluate as a procedure if possible
    Expression result;
    if(env.is_proc(pdr.head())) {
        Procedure proc = env.get_proc(pdr.head());
        result = proc(args);
    }
    //now evaluate as a lambda if possible
    if(env.get_exp(pdr.head()).isHeadLambda()) {
        result = eval_lambda(pdr.head(), args, env);
    }
    return result;
}

Expression Expression::handle_map(Environment &env) {
    if(m_tail.size() != 2)
        throw SemanticError("Error: invalid number of arguments to map");
    //evaluate the first and second arguments of apply, make sure m_tail[0] is procedure/lambda
    Expression pdr = m_tail[0];
    if(pdr.m_tail.size() != 0)
        throw SemanticError("Error: first argument to map is not a procedure.");
    if(!env.is_proc(pdr.head()) && !(env.get_exp(pdr.head()).isHeadLambda()))
        throw SemanticError("Error: first argument to map is not a procedure.");
    //make sure m_tail[1] is a list
    Expression lst = m_tail[1].eval(env);
    if(!lst.isHeadList())
        throw SemanticError("Error: second argument to map is not a list");
    //copy the list of values into a vector of arguments for easier translation
    std::vector<Expression> args;
    for(auto e = lst.listConstBegin(); e != lst.listConstEnd(); ++e)
        args.push_back(*e);
    std::list<Expression> results;
    //now evaluate as a procedure if possible
    if(env.is_proc(pdr.head())) {
        Procedure proc = env.get_proc(pdr.head());
        for(auto e = args.begin(); e != args.end(); e++) {
            std::vector<Expression> procargs;
            procargs.push_back(*e);
            results.push_back(proc(procargs));
        }
    }
    //now evaluate as a lambda if possible
    if(env.get_exp(pdr.head()).isHeadLambda()) {
        for(auto e = args.begin(); e != args.end(); e++) {
            std::vector<Expression> procargs;
            procargs.push_back(*e);
            results.push_back(eval_lambda(pdr.head(), procargs, env));
        }
    }
    //base case return blank expression
    return Expression(results);
}

Expression Expression::property_set(Environment & env) {
    //not sure if the pocketenv is needed
    Environment pocketenv = env;
    if(m_tail.size() != 3)
        throw SemanticError("Error: Wrong number of arguments to set-property.");
    //String as first argument, key
    if(!m_tail[0].isHeadString())
        throw SemanticError("Error: First Argument is not a String");
    Expression key = m_tail[0];
    //any argument second, value, evaluate this
    Expression value = m_tail[1].eval(pocketenv);
    //Expression as the third argument
    Expression result = m_tail[2].eval(env);
    result.set_prop(key, value);
    return result;
}

Expression Expression::property_get(Environment & env) {
    if(m_tail.size() != 2)
        throw SemanticError("Error: wrong number of arguments to get-property.");
    if(!m_tail[0].isHeadString())
        throw SemanticError("Error: first argument not string in get-property.");
    Expression key = m_tail[0];
    Expression value = m_tail[1];
    return env.get_exp(value.head()).get_prop(key, env.get_exp(value.head()));
}

void Expression::set_prop(const Expression & key, const Expression & value) {
    if(!key.isHeadString())
        throw SemanticError("Error: key is not an expression of type String.");
    if(properties.find(key.head().asString()) != properties.end()) {
        auto result = properties.find(key.head().asString());
        result->second = value;
    } else {
        properties.emplace(key.head().asString(), value);
    }
}

Expression Expression::get_prop(const Expression & key, const Expression & value) {
    Expression result;
    if(key.isHeadString()) {
        auto result = value.properties.find(key.head().asString());
        if((result != value.properties.end()))
            return result->second;
    }
    return result;
}

// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env){
  if(m_tail.empty()){
    return handle_lookup(m_head, env);
  }
  // handle begin special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "begin"){
    return handle_begin(env);
  }
  // handle define special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "define"){
    return handle_define(env);
  }
  // handle list special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "list") {
      return handle_list(env);
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "lambda") {
      return handle_lambda();
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "apply") {
      return handle_apply(env);
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "map") {
      return handle_map(env);
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "set-property") {
      return property_set(env);
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "get-property") {
      return property_get(env);
  }
  // else attempt to treat as procedure
  else{ 
    std::vector<Expression> results;
    for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it)
      results.push_back(it->eval(env));
    if(env.get_exp(m_head).head().isLambda()) {
        return eval_lambda(m_head, results, env);
    } else return apply(m_head, results, env);
  }
}

std::ostream & operator<<(std::ostream & out, const Expression & exp){
    if(exp.isHeadNone()) { out << exp.head(); return out;}
    out << "(";
    if (exp.isHeadList()) {
        for (auto e = exp.listConstBegin(); e != exp.listConstEnd(); ++e) {
            if (e != exp.listConstEnd() && e != exp.listConstBegin()) {
                out << " ";
            }
            out << *e;
        }
    }
    else if (exp.isHeadLambda()) {
        out << "(";
        for (auto e = exp.listConstBegin(); e != exp.listConstEnd(); ++e) {
            if (e != exp.listConstEnd() && e != exp.listConstBegin()) {
                out << " ";
            }
            out << *e;
        }
        out << ")";
    }
    else {
        out << exp.head();
    }
    for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
        if (e != exp.tailConstEnd() || e != exp.tailConstBegin()) {
            out << " ";
        }
        out << *e;
    }
    out << ")";
    return out;
}

bool Expression::operator==(const Expression & exp) const noexcept{
  bool result = (m_head == exp.m_head);
  result = result && (m_tail.size() == exp.m_tail.size());
  if(result){
    for(auto lefte = m_tail.begin(), righte = exp.m_tail.begin();
	(lefte != m_tail.end()) && (righte != exp.m_tail.end());
	++lefte, ++righte){
      result = result && (*lefte == *righte);
    }
  }
  return result;
}

bool operator!=(const Expression & left, const Expression & right) noexcept{
  return !(left == right);
}
