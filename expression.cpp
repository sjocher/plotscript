#include "expression.hpp"

#include <sstream>
#include <list>
#include <iostream>
#include <unordered_map>
#include <iomanip>
#include <cmath>

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
    if(a.m_head.isTagged()) {
        m_head.tagAtom();
    } else {
        m_head.deTag();
    }
    if(a.m_head.isLambda()) {
        m_head.markLambda();
    } else {
        m_head.deMarkLambda();
    }
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
      if(a.m_head.isTagged()) {
          m_head.tagAtom();
      } else {
          m_head.deTag();
      }
      if(a.m_head.isLambda()) {
          m_head.markLambda();
      } else {
          m_head.deMarkLambda();
      }
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
    for(auto e = m_tail.begin(); e != m_tail.end(); ++e) {
        Expression evaled = e->eval(env);
        result.m_list.push_back(evaled);
    }
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
    Expression result = lfunc.m_tail[0].eval(pocketenv);
    //need to copy properties here
    for(auto e = lfunc.properties.begin(); e != lfunc.properties.end(); ++e)
        result.properties.emplace(e->first, e->second);
    //also need to copy list here
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

void Expression::populatePoints(std::list<Expression> &list, const Expression & exp) {
    for(auto e = exp.m_list.begin(); e != exp.m_list.end(); ++e) {
        Expression a(*e);
        list.push_back(a);
    }
}

void Expression::findMaxMinPoints(double &AL, double &AU, double &OL, double &OU, const std::list<Expression> points) {
    //x min and max values
    for(auto e = points.begin(); e !=  points.end(); ++e) {
        Expression pt = *e;
        double x = pt.listConstBegin()->head().asNumber();
        double y = std::next(pt.listConstBegin())->head().asNumber();
        if(x < AL) AL = x;
        if(x > AU) AU = x;
        if(y < OL) OL = y;
        if(y > OU) OU = y;
    }
}

Expression Expression::makePExpression(const double x, const double y) {
    Atom ax(x);
    Atom ay(y);
    Expression xp(ax);
    Expression yp(ay);
    std::list<Expression> pointform;
    pointform.push_back(xp);
    pointform.push_back(yp);
    Expression result(pointform);
    result.set_prop(Expression(Atom("object-name\"")), Expression(Atom("point\"")));
    return result;
}

Expression Expression::makeLine(const double x1, const double y1, const double x2, const double y2) {
    Expression point1 = makePExpression(x1, y1);
    Expression point2 = makePExpression(x2, y2);
    std::list<Expression> l1;
    l1.push_back(point1);
    l1.push_back(point2);
    Expression line(l1);
    line.set_prop(Expression(Atom("object-name\"")), Expression(Atom("line\"")));
    line.set_prop(Expression(Atom("thickness\"")), Expression(Atom(0)));
    return line;
}

std::list<Expression> Expression::makeGrid(const double xscale, const double yscale, const double AL, const double AU, const double OL, const double OU) {
    std::list<Expression> lines;
    bool x = true;
    bool y = true;
    if(OU < 0 || OL > 0) x = false;
    if(AU < 0 || AL > 0) y = false;
    Expression bottom = makeLine(AL * xscale, -(OL*yscale), (AL * xscale) + N, -(OL*yscale));
    Expression top = makeLine(AL * xscale, -(OU*yscale), (AL * xscale) + N, -(OU*yscale));
    Expression right = makeLine((AU * xscale), -(OL*yscale), (AU * xscale), -(OU*yscale));
    Expression left = makeLine((AL * xscale), -(OL*yscale), (AL * xscale), -(OU*yscale));
    if(x) {
        Expression abscissa = makeLine(-(AL * xscale), 0, -(AL * xscale) - N, 0);
        lines.push_back(abscissa);
    }
    if(y) {
        Expression ordinate = makeLine(0, -(OL * yscale), 0, -(OL * yscale) - N);
        lines.push_back(ordinate);
    }
    lines.push_back(bottom);
    lines.push_back(top);
    lines.push_back(left);
    lines.push_back(right);
    return lines;
}

std::list<Expression> Expression::scalePoints(const std::list<Expression> points, const double xscale, const double yscale, const double OL, const double OU) {
    std::list<Expression> spoints;
    for(auto e = points.begin(); e != points.end(); ++e) {
        Expression pt = *e;
        double xpt = (((pt.listConstBegin()->head().asNumber()) * xscale));
        double ypt = -(((std::next(pt.listConstBegin()))->head().asNumber() * yscale));
        Expression newPt = makePExpression(xpt, ypt);
        Expression lolliLine;
        if(OU < 0 || OL > 0) {
            lolliLine = makeLine(xpt, ypt, xpt, -(OL*yscale));
        } else {
            lolliLine = makeLine(xpt, ypt, xpt, 0);
        }
        newPt.set_prop(Expression(Atom("object-name\"")), Expression(Atom("point\"")));
        newPt.set_prop(Expression(Atom("size\"")), Expression(Atom(dP)));
        spoints.push_back(newPt);
        spoints.push_back(lolliLine);
    }
    return spoints;
}

std::list<Expression> Expression::combineLists(const std::list<Expression> list1, const std::list<Expression> list2) {
    std::list<Expression> result;
    for(auto e = list1.begin(); e != list1.end(); ++e) {
        result.push_back(*e);
    }
    for(auto e = list2.begin(); e != list2.end(); ++e) {
        result.push_back(*e);
    }
    return result;
}

Expression Expression::dbltoString(const double num) {
    std::stringstream ss;
    ss << std::setprecision(2) << num;
    std::string string = ss.str();
    string += '"';
    return Expression(Atom(string));
}

std::list<Expression> Expression::sigpointlabels(const double AL, const double AU, const double OL, const double OU) {
    std::list<Expression> result;
    double xscale = (N / ((AU) - (AL)));
    double yscale = (N / ((OU) - (OL)));
    Expression eAL = dbltoString(AL);
    eAL.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
    eAL.set_prop(Expression(Atom("position\"")), makePExpression((AL * xscale), -(OL*yscale) + dC));
    Expression eAU = dbltoString(AU);
    eAU.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
    eAU.set_prop(Expression(Atom("position\"")), makePExpression((AL * xscale) + N, -(OL*yscale) + dC));
    Expression eOL = dbltoString(OL);
    eOL.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
    eOL.set_prop(Expression(Atom("position\"")), makePExpression((AL * xscale) - dD, -(OL*yscale)));
    Expression eOU = dbltoString(OU);
    eOU.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
    eOU.set_prop(Expression(Atom("position\"")), makePExpression((AL * xscale) - dD, -(OL*yscale) - N));
    result.push_back(eAL);
    result.push_back(eAU);
    result.push_back(eOL);
    result.push_back(eOU);
    return result;
}

Expression makeText(const std::string text) {
    std::string temp = text + '"';
    return Expression(Atom(temp));
}

std::list<Expression> Expression::handleOptions(const Expression options, const double AL, const double AU, const double OL, const double OU) {
    std::list<Expression> yes;
    double scale = 1;
    double xscale = (N / ((AU) - (AL)));
    double yscale = (N / ((OU) - (OL)));
    for(auto e = options.listConstBegin(); e != options.listConstEnd(); ++e) {
        Expression label = *e;
        std::string id = label.listConstBegin()->head().asString();
        std::string data;
        Expression result;
        if(id == "title") {
            data = std::next(label.listConstBegin())->head().asString();
            result = makeText(data);
            result.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
            result.set_prop(Expression(Atom("rotation\"")), Expression(Atom(0)));
            result.set_prop(Expression(Atom("position\"")), makePExpression(((AL + AU) / 2) * xscale, -(OU * yscale) - dA));
            yes.push_back(result);
        } else if(id == "abscissa-label") {
            data = std::next(label.listConstBegin())->head().asString();
            result = makeText(data);
            result.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
            result.set_prop(Expression(Atom("rotation\"")), Expression(Atom(0)));
            result.set_prop(Expression(Atom("position\"")), makePExpression(((AL + AU) / 2) * xscale, -(OL * yscale) + dA));
            yes.push_back(result);
        } else if(id == "ordinate-label") {
            data = std::next(label.listConstBegin())->head().asString();
            result = makeText(data);
            result.set_prop(Expression(Atom("object-name\"")), Expression(Atom("text\"")));
            result.set_prop(Expression(Atom("rotation\"")), Expression(Atom(-(M_PI/2))));
            result.set_prop(Expression(Atom("position\"")), makePExpression((AL * xscale) - dB, -((OL + OU) / 2) * yscale));
            yes.push_back(result);
        } else if(id == "text-scale") {
            scale = std::next(label.listConstBegin())->head().asNumber();
        }
    }
    std::list<Expression> result;
    for(auto e = yes.begin(); e != yes.end(); ++e) {
        Expression label = *e;
        label.set_prop(Expression(Atom("scale\"")), Expression(Atom(scale)));
        result.push_back(label);
    }
    return result;
}

Expression Expression::discrete_plot(Environment & env) {
    if(m_tail.size() < 1)
        throw SemanticError("Error: wrong number of arguments to discrete plot");
    double AL = 999999, AU = -999999, OL = 999999, OU = -999999;
    Expression DATA = m_tail[0].eval(env);
    Expression OPTIONS = m_tail[1].eval(env);
    std::list<Expression> points;
    populatePoints(points, DATA);
    findMaxMinPoints(AL, AU, OL, OU, points);
    double xscale = (N / ((AU) - (AL)));
    double yscale = (N / ((OU) - (OL)));
    std::list<Expression> gridlines = makeGrid(xscale, yscale, AL, AU, OL, OU);
    std::list<Expression> scaledPoints = scalePoints(points, xscale, yscale, OL, OU);
    std::list<Expression> plotdata = combineLists(gridlines, scaledPoints);
    std::list<Expression> numLabels = sigpointlabels(AL, AU, OL, OU);
    plotdata = combineLists(plotdata, numLabels);
    std::list<Expression> labels = handleOptions(OPTIONS, AL, AU, OL, OU);
    plotdata = combineLists(plotdata, labels);
    Expression result(plotdata);
    return result;
}

std::vector<Expression> Expression::fillBounds(const Expression BOUNDS) {
    double low = BOUNDS.listConstBegin()->head().asNumber();
    double high = std::next(BOUNDS.listConstBegin())->head().asNumber();
    double samplesize = ((high - low) / (cM + 0));
    std::vector<Expression> result;
    for(auto i = low; i <= (high + samplesize); i += samplesize) {
        result.push_back(Expression(Atom(i)));
    }
    return result;
}

void Expression::continuousPoints(std::list<Expression> &points, const Expression FUNC, const Expression BOUNDS, Environment & env) {
    std::vector<Expression> args = fillBounds(BOUNDS);
    for(auto e : args) {
        std::vector<Expression> singlearg;
        singlearg.push_back(e);
        Expression yval = eval_lambda(FUNC.head(), singlearg, env);
        Expression pt = makePExpression(e.head().asNumber(), yval.head().asNumber());
        points.push_back(pt);
    }
}

bool checksplit(const Expression p1, const Expression p2, const Expression p3) {
    double x2 = p1.listConstBegin()->head().asNumber();
    double y2 = std::next(p1.listConstBegin())->head().asNumber();
    double x1 = p2.listConstBegin()->head().asNumber();
    double y1 = std::next(p2.listConstBegin())->head().asNumber();
    double x3 = p3.listConstBegin()->head().asNumber();
    double y3 = std::next(p3.listConstBegin())->head().asNumber();
    //x1 is the center point
    double dx21 = x2-x1;
    double dx31 = x3-x1;
    double dy21 = y2-y1;
    double dy31 = y3-y1;
    double m12 = sqrt(dx21*dx21 + dy21*dy21);
    double m13 = sqrt(dx31*dx31 + dy31*dy31);
    double theta = acos((dx21*dx31 + dy21*dy31) / (m12 * m13));
    double angle = theta * (180/M_PI);
    if(angle < 175 && !std::isnan(angle)) {
        return true;
    }
    return false;
}

Expression Expression::getLambdaYValue(const Expression x, const Expression FUNC, Environment & env) {
    std::vector<Expression> args;
    args.push_back(x);
    Expression yval = eval_lambda(FUNC.head(), args, env);
    Expression pt = makePExpression(x.head().asNumber(), (yval.head().asNumber()));
    return pt;
}

std::list<Expression> Expression::makeSplitLine(const Expression p1, const Expression p2, const Expression p3, const Expression FUNC, Environment & env) {
    std::list<Expression> newPoints;
    double x1 = p1.listConstBegin()->head().asNumber();
    double x2 = p2.listConstBegin()->head().asNumber();
    double x3 = p3.listConstBegin()->head().asNumber();
    Expression split1half = getLambdaYValue(Expression(Atom(((x1+x2)/2))), FUNC, env);
    Expression split2half = getLambdaYValue(Expression(Atom(((x2+x3)/2))), FUNC, env);
    newPoints.push_back(p1);
    newPoints.push_back(split1half);
    newPoints.push_back(p2);
    newPoints.push_back(split2half);
    newPoints.push_back(p3);
    return newPoints;
}

std::list<Expression> Expression::convP2Lines(const std::list<Expression> points, const double xscale, const double yscale) {
    std::list<Expression> lines;
    for(auto e = points.begin(); e != std::prev(points.end()); ++e) {
        Expression p1 = *e;
        Expression p2 = *(std::next(e));
        Expression line = makeLine((p1.listConstBegin()->head().asNumber()) * xscale, -(std::next(p1.listConstBegin())->head().asNumber()) * yscale, (p2.listConstBegin()->head().asNumber()) * xscale, -(std::next(p2.listConstBegin())->head().asNumber())* yscale);
        lines.push_back(line);
    }
    return lines;
}

std::list<Expression> Expression::smoothedLines(const std::list<Expression> points, const Expression FUNC, Environment & env) {
    std::list<Expression> smoothies;
    for(auto e = points.begin(); e != std::prev(std::prev(points.end())); ++e) {
        Expression p1 = *e;
        Expression p2 = *(std::next(e));
        Expression p3 = *(std::next(std::next(e)));
        if(checksplit(p1, p2, p3)) {
            std::list<Expression> additionalPoints = makeSplitLine(p1, p2, p3, FUNC, env);
            for(auto i : additionalPoints)
                smoothies.push_back(i);
            e = std::next(std::next(e));
        } else {
            smoothies.push_back(p1);
            if(e == std::prev(std::prev(std::prev(points.end())))) {
                smoothies.push_back(p2);
                smoothies.push_back(p3);
            }
        }
    }
    return smoothies;
}

Expression Expression::continuous_plot(Environment & env) {
    double AL = 999999, AU = -999999, OL = 999999, OU = -999999;
    Expression FUNC = m_tail[0];
    Expression BOUNDS = m_tail[1].eval(env);
    Expression OPTIONS;
    if(m_tail.size() == 3)
        OPTIONS = m_tail[2].eval(env);
    std::list<Expression> points;
    continuousPoints(points, FUNC, BOUNDS, env);
    findMaxMinPoints(AL, AU, OL, OU, points);
    double xscale = (N / ((AU) - (AL)));
    double yscale = (N / ((OU) - (OL)));
    std::list<Expression> gridlines = makeGrid(xscale, yscale, AL, AU, OL, OU);
    std::list<Expression> smoothed = smoothedLines(points, FUNC, env);
    std::list<Expression> functionLines = convP2Lines(smoothed, xscale, yscale);
    std::list<Expression> plotdata = combineLists(gridlines, functionLines);
    std::list<Expression> numLabels = sigpointlabels(AL, AU, OL, OU);
    plotdata = combineLists(plotdata, numLabels);
    std::list<Expression> labels = handleOptions(OPTIONS, AL, AU, OL, OU);
    plotdata = combineLists(plotdata, labels);
    Expression result(plotdata);
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
  else if(m_head.isSymbol() && m_head.asSymbol() == "discrete-plot") {
      return discrete_plot(env);
  }
  else if(m_head.isSymbol() && m_head.asSymbol() == "continuous-plot") {
      return continuous_plot(env);

  }
  // else attempt to treat as procedure
  else{ 
    std::vector<Expression> results;
    for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it)
      results.push_back(it->eval(env));
    if(env.get_exp(m_head).head().isLambda()) {
        Expression result = eval_lambda(m_head, results, env);
        return result;
    } else return apply(m_head, results, env);
  }
  return Expression();
}

std::ostream & operator<<(std::ostream & out, const Expression & exp){
    //special cases for convenience
    if(exp.isHeadNone()) { out << exp.head(); return out;}
    if(exp.isHeadString()) {out << "(\"" << exp.head() << "\")"; return out;}
    //normal output
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
