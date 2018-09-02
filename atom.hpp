/*! \file atom.hpp
Defines the Atom type and associated functions.
 */
#ifndef ATOM_HPP
#define ATOM_HPP

#include "token.hpp"

/*! \class Atom
\brief A variant type that may be a Number or Symbol or the default type None.

This class provides value semantics.
*/
class Atom {
public:

  /// Construct a default Atom of type None
  Atom();

  /// Construct an Atom of type Number with value
  Atom(double value);

  /// Construct an Atom of type Symbol named value
  Atom(const std::string & value);
    
  /// Construct an Atom of type complex with two doubles
  Atom(double real, double image);

  /// Construct an Atom directly from a Token
  Atom(const Token & token);

  /// Copy-construct an Atom
  Atom(const Atom & x);

  /// Assign an Atom
  Atom & operator=(const Atom & x);

  /// Atom destructor
  ~Atom();

  /// predicate to determine if an Atom is of type None
  bool isNone() const noexcept;

  /// predicate to determine if an Atom is of type Number
  bool isNumber() const  noexcept;

  /// predicate to determine if an Atom is of type Symbol
  bool isSymbol() const noexcept;
    
  /// predicate to determine if an Atom is of type Complex
  bool isComplex() const noexcept;

  /// value of Atom as a number, return 0 if not a Number
  double asNumber() const noexcept;
    
  double getComReal() const noexcept;
  double getComImag() const noexcept;

  /// value of Atom as a number, returns empty-string if not a Symbol
  std::string asSymbol() const noexcept;
    
  /// value of Atom as complex, returns empty-string if not a complex
  std::string asComplex() const noexcept;

  /// equality comparison based on type and value
  bool operator==(const Atom & right) const noexcept;

private:

  // internal enum of known types
  // Milestone - 0 : added ComplexKind
  enum Type {NoneKind, NumberKind, SymbolKind, ComplexKind};

  // track the type
  Type m_type;

    //struct for complex number
    struct complexNumber {
        double real;
        double imaginary;
    };
    
  // values for the known types. Note the use of a union requires care
  // when setting non POD values (see setSymbol)
  union {
    double numberValue;
    complexNumber cm;
    std::string stringValue;
  };
    
  // helper to set type and value of Number
  void setNumber(double value);

  // helper to set type and value of Symbol
  void setSymbol(const std::string & value);
    
  // helper to set type and value of Complex
    void setComplex(double real, double image);
};

/// inequality comparison for Atom
bool operator!=(const Atom &left, const Atom & right) noexcept;

/// output stream rendering
std::ostream & operator<<(std::ostream & out, const Atom & a);

#endif
