#include "catch.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>

#include "semantic_error.hpp"
#include "interpreter.hpp"
#include "expression.hpp"
#include "startup_config.hpp"

void error(const std::string & err_str){
    std::cerr << "Error: " << err_str << std::endl;
}
Expression run(const std::string & program){
    Interpreter interp;
    std::ifstream startup(STARTUP_FILE);
    if(!interp.parseStream(startup)) {
        error("Invalid Startup. Could not parse.");
    } else {
        try {
            Expression exp = interp.evaluate();
        } catch (const SemanticError & ex){
            std::cerr << ex.what() << std::endl;
        }
    }
    
  std::istringstream iss(program);
    
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  Expression result;
  REQUIRE_NOTHROW(result = interp.evaluate());

  return result;
}

TEST_CASE( "Test Interpreter parser with expected input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r)))";

  std::istringstream iss(program);
 
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == true);
}

TEST_CASE( "Test Interpreter parser with numerical literals", "[interpreter]" ) {

  std::vector<std::string> programs = {"(1)", "(+1)", "(+1e+0)", "(1e-0)"};
  
  for(auto program : programs){
    std::istringstream iss(program);
 
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == true);
  }

  {
    std::istringstream iss("(define x 1abc)");
    
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with truncated input", "[interpreter]" ) {

  {
    std::string program = "(f";
    std::istringstream iss(program);
  
    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
  
  {
    std::string program = "(begin (define r 10) (* pi (* r r";
    std::istringstream iss(program);

    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with extra input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r))) )";
  std::istringstream iss(program);

  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with single non-keyword", "[interpreter]" ) {

  std::string program = "hello";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty input", "[interpreter]" ) {

  std::string program;
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty expression", "[interpreter]" ) {

  std::string program = "( )";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with bad number string", "[interpreter]" ) {

  std::string program = "(1abc)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with incorrect input. Regression Test", "[interpreter]" ) {

  std::string program = "(+ 1 2) (+ 3 4)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter result with literal expressions", "[interpreter]" ) {
  
  { // Number
    std::string program = "(4)";
    Expression result = run(program);
    REQUIRE(result == Expression(4.));
  }

  { // Symbol
    std::string program = "(pi)";
    Expression result = run(program);
    REQUIRE(result == Expression(atan2(0, -1)));
  }

}

TEST_CASE( "Test Interpreter result with simple procedures (add)", "[interpreter]" ) {

  { // add, binary case
    std::string program = "(+ 1 2)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(3.));
  }
  
  { // add, 3-ary case
    std::string program = "(+ 1 2 3)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(6.));
  }

  { // add, 6-ary case
    std::string program = "(+ 1 2 3 4 5 6)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(21.));
  }
}
  
TEST_CASE( "Test Interpreter special forms: begin and define", "[interpreter]" ) {

  {
    std::string program = "(define answer 42)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }

  {
    std::string program = "(begin (define answer 42)\n(answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }
  
  {
    std::string program = "(begin (define answer (+ 9 11)) (answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(20.));
  }

  {
    std::string program = "(begin (define a 1) (define b 1) (+ a b))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(2.));
  }
}

TEST_CASE( "Test a medium-sized expression", "[interpreter]" ) {

  {
    std::string program = "(+ (+ 10 1) (+ 30 (+ 1 1)))";
    Expression result = run(program);
    REQUIRE(result == Expression(43.));
  }
}

TEST_CASE( "Test arithmetic procedures", "[interpreter]" ) {

  {
    std::vector<std::string> programs = {"(+ 1 -2)",
					 "(+ -3 1 1)",
					 "(- 1)",
					 "(- 1 2)",
					 "(* 1 -1)",
					 "(* 1 1 -1)",
					 "(/ -1 1)",
					 "(/ 1 -1)"};

    for(auto s : programs){
      Expression result = run(s);
      REQUIRE(result == Expression(-1.));
    }
  }
}


TEST_CASE( "Test some semantically invalid expresions", "[interpreter]" ) {
  
  std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(- 1 1 2)", // too many arguments
				       "(define begin 1)", // redefine special form
				       }; // redefine builtin symbol
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test for exceptions from semantically incorrect input", "[interpreter]" ) {

  std::string input = R"(
(+ 1 a)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test malformed define", "[interpreter]" ) {

    std::string input = R"(
(define a 1 2)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test using number as procedure", "[interpreter]" ) {
    std::string input = R"(
(1 2 3)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE("Test Real, Imag, Mag Milestone0", "[Milestone0]") {
    {
        std::string program = "(^ I 2)";
        Expression result = run(program);
        Atom a(-1, 0);
        REQUIRE(result == Expression(a));
    }
    {
        double r = 0;
        double i = 1;
        std::string program = "(real I)";
        std::string program2 = "(imag I)";
        Expression result = run(program);
        Expression result2 = run(program2);
        REQUIRE(result == Expression(r));
        REQUIRE(result2 == Expression(i));
    }
    {
        std::string program = "(real 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(imag 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(real 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(imag 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(mag I)";
        Expression result = run(program);
        double a = 1;
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(mag 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(mag 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test Arg, Conj Milestone0", "[Milestone0]") {
    {
        std::string program = "(arg I)";
        Expression result = run(program);
        std::complex<double> a(0,1);
        REQUIRE(result == Expression(arg(a)));
    }
    {
        std::string program = "(arg 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(arg 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(conj I)";
        Expression result = run(program);
        std::complex<double> a(0,1);
        REQUIRE(result == Expression(conj(a)));
    }
    {
        std::string program = "(conj 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(conj 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test Add Milestone0", "[Milestone0]") {
    {
        std::string program = "(+ 1 1 1 1 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(5));
    }
    {
        std::string program = "(+ 1 I 1 I 1)";
        Expression result = run(program);
        std::complex<double>a(3,2);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(+ 1 z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test Mul Milestone0", "[Milestone0]") {
    {
        std::string program = "(* 1 1 1 1 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(1));
    }
    {
        std::string program = "(* 1 I 1 I 1)";
        Expression result = run(program);
        std::complex<double>a(-1,0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(* 1 z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test Subneg Milestone0", "[Milestone0]") {
    {
        std::string program = "(- 1 2)";
        Expression result = run(program);
        REQUIRE(result == Expression(-1));
    }
    {
        std::string program = "(- 1 I)";
        Expression result = run(program);
        std::complex<double>a(1,-1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(- I I)";
        Expression result = run(program);
        std::complex<double>a(0,0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(- I 1)";
        Expression result = run(program);
        std::complex<double>a(-1,1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(- I)";
        Expression result = run(program);
        std::complex<double>a(0,-1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(- 1 I I)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(- 1 I z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test Div Milestone0", "[Milestone0]") {
    {
        std::string program = "(/ 1 2)";
        Expression result = run(program);
        REQUIRE(result == Expression(0.5));
    }
    {
        std::string program = "(/ I I)";
        Expression result = run(program);
        std::complex<double>a(1,0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(/ 1 I)";
        Expression result = run(program);
        std::complex<double>a(0,-1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(/ I 1)";
        Expression result = run(program);
        std::complex<double>a(0,1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(/ I I I)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(/ I z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(/ 2)";
        Expression result = run(program);
        REQUIRE(result == Expression(0.5));
    }
    {
        std::string program = "(/ I)";
        Expression result = run(program);
        Atom a(0,-1);
        REQUIRE(result == Expression(a));
    }
}

TEST_CASE("Test SQRT Milestone0", "[Milestone0]") {
    {
        std::string program = "(sqrt 4)";
        Expression result = run(program);
        REQUIRE(result == Expression(2));
    }
    {
        std::string program = "(sqrt I)";
        Expression result = run(program);
        std::complex<double>a(sqrt(2)/2,(sqrt(2)/2));
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(sqrt I 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(sqrt z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test POW Milestone0", "[Milestone0]") {
    {
        std::string program = "(^ I 2)";
        Expression result = run(program);
        std::complex<double>a(-1, 0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(^ I I)";
        Expression result = run(program);
        std::complex<double>a(std::exp(-1 * std::atan2(0, -1) / 2), 0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(^ 1 I)";
        Expression result = run(program);
        std::complex<double>a(1, 0);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(^ 1 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(1));
    }
    {
        std::string program = "(^ z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(^ 2 2 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test ln Milestone0", "[Milestone0]") {
    {
        std::string program = "(ln 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(0));
    }
    {
        std::string program = "(ln -4)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(ln 4 4)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(ln z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test sine, cosine, tangent Milestone0", "[Milestone0]") {
    {
        std::string program = "(sin 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(sin(1)));
    }
    {
        std::string program = "(cos 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(cos(1)));
    }
    {
        std::string program = "(tan 1)";
        Expression result = run(program);
        REQUIRE(result == Expression(tan(1)));
    }
    {
        std::string program = "(sin z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(cos z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(tan z)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(sin 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(cos 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(tan 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Testing Construction of a List" , "[Milestone 1]") {
    {
        std::string program = "(list 1 2 3)";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(1));
        lists.push_back(Atom(2));
        lists.push_back(Atom(3));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(first (list 1 2 3))";
        Expression result = run(program);
        Atom a(1);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(first 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(first (list))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(first (list 1) (list 2))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(rest (list 1 2 3))";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(2));
        lists.push_back(Atom(3));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(rest (1))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(rest (list))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(rest (list 1 2) (list 3 4))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(rest (list 1))";
        Expression result = run(program);
        std::list<Expression> lists;
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(length (list 1 2 3 4))";
        Expression result = run(program);
        Atom a(4);
        REQUIRE(result == Expression(a));
    }
    {
        std::string program = "(length 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(length 1 2)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(length (list 1 2) (list 2 3))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(append (list 3) 3)";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(3));
        lists.push_back(Atom(3));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(append 3 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(join (list 1) (list 2))";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(1));
        lists.push_back(Atom(2));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(join (list 1) 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(range 0 5 1)";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(0));
        lists.push_back(Atom(1));
        lists.push_back(Atom(3));
        lists.push_back(Atom(4));
        lists.push_back(Atom(5));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(range 3 -1 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(range 0 5 -1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Testing lambda function" , "[Milestone 1]") {
    {
        std::string program = "(begin (define a 1) (define x 100) (define f (lambda (x) (begin (define b 12) (+ a b x)))) (f 2))";
        Expression result = run(program);
        Expression a(Atom(15));
        REQUIRE(result == a);
    }
    {
        std::string program = "(apply + (list 1 2 3 4))";
        Expression result = run(program);
        Expression a(Atom(10));
        REQUIRE(result == a);
    }
    {
        std::string program = "(begin (define complexAsList (lambda (x) (list (real x) (imag x)))) (apply complexAsList (list (+ 1 (* 3 I)))))";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(1));
        lists.push_back(Atom(3));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(apply + 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(apply (+ z I) (list 0))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(apply (+ z I) (list 0) (list 3))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(map (+ z I) (list 0) (list 3))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(map / (list 1 2 4))";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(1));
        lists.push_back(Atom(0.5));
        lists.push_back(Atom(0.25));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
        std::string program = "(map + 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(map 3 (list 1 2 3))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(begin (define addtwo (lambda (x y) (+ x y)) (map addtwo (list 1 2 3))))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(begin (define f (lambda (x) (sin x))) (map f (list (/ (- pi) 2) 0 (/ pi 2))))";
        Expression result = run(program);
        std::list<Expression> lists;
        lists.push_back(Atom(-1));
        lists.push_back(Atom(0));
        lists.push_back(Atom(1));
        Expression a(lists);
        REQUIRE(result == a);
    }
    {
            std::string program = "(join 3 (list 1 2 3))";
            std::istringstream iss(program);
            Interpreter interp;
            bool ok = interp.parseStream(iss);
            REQUIRE(ok == true);
            REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(join (list 1 2 3) 3 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(append 3 3 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(range 3 3 3 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(begin )";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(define )";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(define 3)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(lambda (x y z) (+ x y z))";
        Expression result = run(program);
    }
}
TEST_CASE("Milestone 2 test cases", "[Milestone 2]") {
    {
        std::string program = "(begin (define make-point (lambda (x y) (list x y))) (define make-point (set-property \"object-name\" \"point\" make-point)) (define make-point (set-property \"size\" 0 make-point)) (define make-line (lambda (p1 p2) (list p1 p2))) (define make-line (set-property \"object-name\" \"line\" make-line)) (define make-line (set-property \"thickness\" 0 make-line)) (define make-text (lambda (str) (\"str\"))) (define make-text (set-property \"object-name\" \"text\" make-text)) (define make-text (set-property \"position\" (make-point 0 0) make-text)))";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream((iss));
        REQUIRE(ok == true);
    }
    {
        std::string program = "(\"abc\")";
        Expression result = run(program);
        REQUIRE(result.head().asString() == "abc");
    }
    {
        std::string program = "(set-property 1 1 1)";
        std::istringstream iss(program);
        Interpreter interp;
        bool ok = interp.parseStream(iss);
        REQUIRE(ok == true);
        REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
    {
        std::string program = "(begin (define a (lambda (x y) (* x y))) (define a (set-property \"name\" 3 a)))";
        Expression result = run(program);
        Expression b(result);
        Expression c = b;
        REQUIRE(result == b);
        REQUIRE(c == result);
    }
    {
        std::string program = "(begin (define a (lambda (x y) (* x y))) (define a (set-property \"name\" 3 a)) (get-property \"name\" a))";
        Expression result = run(program);
        Expression b(result);
        Expression c = b;
        REQUIRE(result == b);
        REQUIRE(c == result);
    }
    {
        std::string program = "(\"ree\")";
        Expression result = run(program);
        Expression b(result);
        Expression c = b;
        REQUIRE(result == b);
        REQUIRE(c == result);
    }
    {
        std::string program = "(+ 2 I)";
        Expression result = run(program);
        Expression b(result);
        Expression c = b;
        REQUIRE(result == b);
        REQUIRE(c == result);
    }
    {
        std::string program = "(define a (make-point 0 0))";
        Expression result = run(program);
        Expression a = result;
    }
}
