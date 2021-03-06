#include "atom.hpp"

#include <sstream>
#include <cctype>
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>

Atom::Atom(): m_type(NoneKind) {}

Atom::Atom(double value){
    setNumber(value);
}

Atom::Atom(double real, double image) {
    setComplex(real, image);
}

Atom::Atom(std::complex<double> complexNumber) {
    setComplex(real(complexNumber), imag(complexNumber));
}

Atom::Atom(const Token & token): Atom() {
  // is token a number?
  double temp;
  std::istringstream iss(token.asString());
  if(iss >> temp){
    // check for trailing characters if >> succeeds
    if(iss.rdbuf()->in_avail() == 0){
      setNumber(temp);
    }
  }
  //assume symbol and check if first character is a digit
  else if(!std::isdigit(token.asString()[0])) {
      if(token.asString().back() == '"') {
          setString(token.asString());
      } else {
          setSymbol(token.asString());
      }
  }
}

Atom::Atom(const std::string & value): Atom() {
    if(value.back() == '"') {
        setString(value);
    } else setSymbol(value);
}

Atom::Atom(const Atom & x): Atom(){
  if(x.isNumber()){
    setNumber(x.numberValue);
  }
  else if(x.isSymbol()){
    setSymbol(x.stringValue);
  }
  else if(x.isComplex()) {
      setComplex(x.getComReal(), x.getComImag());
  }
  else if(x.isString()) {
      setString(x.sstring);
  }
}

Atom & Atom::operator=(const Atom & x) {
  if(this != &x){
    if(x.m_type == NoneKind){
      m_type = NoneKind;
    }
    else if(x.m_type == NumberKind){
      setNumber(x.numberValue);
    }
    else if(x.m_type == SymbolKind){
      setSymbol(x.stringValue);
    }
    else if(x.m_type == ComplexKind) {
        setComplex(x.getComReal(), x.getComImag());
    }
    else if(x.m_type == StringKind) {
        setString(x.sstring);
    }
  }
  return *this;
}
  
Atom::~Atom() {
  // we need to ensure the destructor of the symbol string is called
  if(m_type == SymbolKind){
    stringValue.~basic_string();
  } else if(m_type == StringKind) {
      sstring.~basic_string();
    }
}

bool Atom::isNone() const noexcept {
  return m_type == NoneKind;
}

bool Atom::isNumber() const noexcept {
  return m_type == NumberKind;
}

bool Atom::isSymbol() const noexcept {
  return m_type == SymbolKind;
}

bool Atom::isComplex() const noexcept {
    return m_type == ComplexKind;
}

bool Atom::isString() const noexcept {
    return m_type == StringKind;
}

void Atom::setComplex(double real, double image) {
    m_type = ComplexKind;
    complexNumber = std::complex<double>(real, image);
}

void Atom::setNumber(double value) {
  m_type = NumberKind;
  numberValue = value;
}

void Atom::setSymbol(const std::string & value) {
  // we need to ensure the destructor of the symbol string is called
  if(m_type == SymbolKind){
    stringValue.~basic_string();
  }
  m_type = SymbolKind;
  // copy construct in place
  new (&stringValue) std::string(value);
}

void Atom::setString(const std::string & value) {
    if(m_type == StringKind){
        sstring.~basic_string();
    }
    m_type = StringKind;
    // copy construct in place
    new (&sstring) std::string(value);
    if(sstring.back() == '"') {
        sstring.pop_back();
    }
}

double Atom::getComImag() const noexcept {
    return imag(complexNumber);
}

double Atom::getComReal() const noexcept {
    return real(complexNumber);
}

double Atom::asNumber() const noexcept {
  return (m_type == NumberKind) ? numberValue : 0.0;  
}

std::string Atom::asSymbol() const noexcept{
  std::string result;
  if(m_type == SymbolKind) {
    result = stringValue;
  }
  return result;
}

std::string Atom::asString() const noexcept {
    std::string result;
    if(m_type == StringKind) {
        result = sstring;
    }
    return result;
}

std::complex<double> Atom::asComplex() const noexcept {
    return (m_type == ComplexKind) ? complexNumber : 0.0;
}

bool Atom::operator==(const Atom & right) const noexcept{
  if(m_type != right.m_type) return false;
  switch(m_type){
  case NoneKind:
    if(right.m_type != NoneKind) return false;
    break;
  case NumberKind:
    {
      if(right.m_type != NumberKind) return false;
      double dleft = numberValue;
      double dright = right.numberValue;
      double diff = fabs(dleft - dright);
      if(std::isnan(diff) ||
	 (diff > std::numeric_limits<double>::epsilon())) return false;
    }
    break;
  case SymbolKind:
    {
      if(right.m_type != SymbolKind) return false;
      return stringValue == right.stringValue;
    }
    break;
      case StringKind:
      {
          if(right.m_type != StringKind) return false;
          return stringValue == right.sstring;
      }
          break;
      case ComplexKind: {
          if(right.m_type != ComplexKind) return false;
          double dleft = getComReal();
          double dright = right.getComReal();
          double diff = fabs(dleft - dright);
          if(std::isnan(diff) || (diff > std::numeric_limits<double>::epsilon())) return false;
          //check imaginary as well
          double ileft = getComImag();
          double iright = right.getComImag();
          double iiff = fabs(ileft - iright);
          if(std::isnan(iiff) || (iiff > std::numeric_limits<double>::epsilon())) return false;
      }
    break;
  default:
    return false;
  }

  return true;
}

bool operator!=(const Atom & left, const Atom & right) noexcept{
  
  return !(left == right);
}

std::ostream & operator<<(std::ostream & out, const Atom & a){
  if(a.isNumber()){
    out << a.asNumber();
  } else if(a.isSymbol()){
    out << a.asSymbol();
  } else if(a.isComplex()) {
    out << a.getComReal() << "," << a.getComImag();
  } else if(a.isString()) {
    out << a.asString();
  } else if(a.isNone()) {
      out << "NONE";
  }
  return out;
}
