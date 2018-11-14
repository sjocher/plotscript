/*! \file expression.hpp
Defines the Expression type and assiciated functions.
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <list>
#include <map>

#include "token.hpp"
#include "atom.hpp"

// forward declare Environment
class Environment;

/*! \class Expression
\brief An expression is a tree of Atoms.

An expression is an atom called the head followed by a (possibly empty) 
list of expressions called the tail.
 */
class Expression {
public:

  typedef std::vector<Expression>::const_iterator ConstIteratorType;
  typedef std::list<Expression>::const_iterator ConstListIteratorType;

  /// Default construct and Expression, whose type in NoneType
  Expression();

  /*! Construct an Expression with given Atom as head an empty tail
    \param atom the atom to make the head
  */
  Expression(const Atom & a);
    
  /// deep-copy construct an expression (recursive)
  Expression(const Expression & a);
    
  Expression(const std::list<Expression> & list);

  /// deep-copy assign an expression  (recursive)
  Expression & operator=(const Expression & a);

  /// return a reference to the head Atom
  Atom & head();

  /// return a const-reference to the head Atom
  const Atom & head() const;

  /// append Atom to tail of the expression
  void append(const Atom & a);

  /// return a pointer to the last expression in the tail, or nullptr
  Expression * tail();

  /// return a const-iterator to the beginning of tail
  ConstIteratorType tailConstBegin() const noexcept;

  /// return a const-iterator to the tail end
  ConstIteratorType tailConstEnd() const noexcept;
    
  /// return a const-iterator to the list beginning
  ConstListIteratorType listConstBegin() const noexcept {return m_list.cbegin();}
    
  /// return a const-iterator to the list end
  ConstListIteratorType listConstEnd() const noexcept {return m_list.cend();}
    
  /// convienience member to determine if head atom is of type none
  bool isHeadNone() const noexcept {return m_head.isNone();}

  /// convienience member to determine if head atom is a number
  bool isHeadNumber() const noexcept;

  /// convienience member to determine if head atom is a symbol
  bool isHeadSymbol() const noexcept;
    
  /// convienience member to determine if head atom is a complex number
  bool isHeadComplex() const noexcept;
    
  /// convienience member to determine if the head atom is the head of a list
  bool isHeadList() const noexcept;
    
  /// convienience member to determine if the head atom is the head of a lambda func
  bool isHeadLambda() const noexcept {return m_head.isLambda();}
    
  /// conveience member to determine if hte head atom is a string literal
  bool isHeadString() const noexcept {return m_head.isString();}
  
  /// convienience member to determine if the list is empty
  bool isListEmpty() const noexcept {return m_list.size() == 0;}
    
  /// conveinience member to return the size of a list
  double listSize() const noexcept {return m_list.size();}

  /// Evaluate expression using a post-order traversal (recursive)
  Expression eval(Environment & env);

  /// equality comparison for two expressions (recursive)
  bool operator==(const Expression & exp) const noexcept;
    
  //Property list setters and getters
  void set_prop(const Expression & key, const Expression & value);
  Expression get_prop(const Expression & key, const Expression & value);
  
private:

  // the head of the expression
  Atom m_head;

  // the tail list is expressed as a vector for access efficiency
  // and cache coherence, at the cost of wasted memory.
  std::vector<Expression> m_tail;
    
  // list data type to store values from list
  std::list<Expression> m_list;
    
  //property list
  std::map<std::string, Expression> properties;

  // convenience typedef
  typedef std::vector<Expression>::iterator IteratorType;
  
  // internal helper methods
  Expression handle_lookup(const Atom & head, const Environment & env);
  Expression handle_define(Environment & env);
  Expression handle_begin(Environment & env);
  Expression handle_list(Environment & env);
  Expression handle_lambda();
  Expression handle_apply(Environment & env);
  Expression handle_map(Environment & env);
  Expression property_get(Environment & env);
  Expression property_set(Environment & env);
  Expression discrete_plot(Environment & env);
  Expression continuous_plot(Environment & env);
  Expression eval_lambda(const Atom & op, const std::vector<Expression> & args, const Environment & env);
  void populatePoints(std::list<Expression> &list, const Expression & exp);
  void findMaxMinPoints(double &AL, double &AU, double &OL, double &OU, const std::list<Expression> points);
  Expression makePExpression(const double x, const double y);
  std::list<Expression> makeGrid(const double xscale, const double yscale, const double AL, const double AU, const double OL, const double OU);
  Expression makeLine(const double x1, const double y1, const double x2, const double y2);
  std::list<Expression> scalePoints(const std::list<Expression> points, const double xscale, const double yscale, const double OL, const double OU);
  std::list<Expression> combineLists(const std::list<Expression> list1, const std::list<Expression> list2);
  std::list<Expression> sigpointlabels(const double AL, const double AU, const double OL, const double OU);
  Expression dbltoString(const double num);
  std::list<Expression> handleOptions(const Expression options, const double AL, const double AU, const double OL, const double OU);
  std::vector<Expression> fillBounds(const Expression BOUNDS);
  void continuousPoints(std::list<Expression> &points, const Expression FUNC, const Expression BOUNDS, Environment & env);
  std::list<Expression> scaleCPoints(const std::list<Expression> points, const double xscale, const double yscale);
  std::list<Expression> convP2Lines(const std::list<Expression> points, const Expression FUNC, Environment & env, const double xscale, const double yscale);
  std::list<Expression> splitLines(const std::list<Expression> lines, const Expression FUNC, Environment & env, const double xscale, const double yscale, bool & split);
  std::list<Expression> makeSplitLine(const Expression l1, const Expression l2, const Expression FUNC, Environment & env, const double xscale, const double yscale);
  Expression getLambdaYValue(const Expression x, const Expression FUNC, Environment & env, const double xscale, const double yscale);
  //graphics scales
  double dP = 0.5;
  double dD = 2;
  double dC = 2;
  double dB = 3;
  double dA = 3;
  double N = 20;
  //double cMAX = 10;
  double cM = 50;
};

/// Render expression to output stream
std::ostream & operator<<(std::ostream & out, const Expression & exp);

/// inequality comparison for two expressions (recursive)
bool operator!=(const Expression & left, const Expression & right) noexcept;
  
#endif
