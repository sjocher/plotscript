/*! \file expression.hpp
Defines the Expression type and assiciated functions.
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <list>

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

  /// convienience member to determine if head atom is a number
  bool isHeadNumber() const noexcept;

  /// convienience member to determine if head atom is a symbol
  bool isHeadSymbol() const noexcept;
    
  /// convienience member to determine if head atom is a complex number
  bool isHeadComplex() const noexcept;
    
  /// convienience member to determine if the head atom is the head of a list
  bool isHeadList() const noexcept;
  
  /// convienience member to determine if the list is empty
  bool isListEmpty() const noexcept {return m_list.size() == 0;}
    
  /// conveinience member to return the size of a list
  double listSize() const noexcept {return m_list.size();}

  /// Evaluate expression using a post-order traversal (recursive)
  Expression eval(Environment & env);

  /// equality comparison for two expressions (recursive)
  bool operator==(const Expression & exp) const noexcept;
  
private:

  // the head of the expression
  Atom m_head;

  // the tail list is expressed as a vector for access efficiency
  // and cache coherence, at the cost of wasted memory.
  std::vector<Expression> m_tail;
    
  // list data type to store values from list
  std::list<Expression> m_list;

  // convenience typedef
  typedef std::vector<Expression>::iterator IteratorType;
  
  // internal helper methods
  Expression handle_lookup(const Atom & head, const Environment & env);
  Expression handle_define(Environment & env);
  Expression handle_begin(Environment & env);
  Expression handle_list(Environment & env);
  Expression handle_lambda();
  Expression eval_lambda(Environment & env);
};

/// Render expression to output stream
std::ostream & operator<<(std::ostream & out, const Expression & exp);

/// inequality comparison for two expressions (recursive)
bool operator!=(const Expression & left, const Expression & right) noexcept;
  
#endif
