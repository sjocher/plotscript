#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "atom.hpp"
#include "environment.hpp"
#include "expression.hpp"
#include "interpreter.hpp"
#include "parse.hpp"
#include "token.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>

#include "semantic_error.hpp"
#include "tsQueue.hpp"

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

TEST_CASE("Message Queue Test","[Message Queue]") {
    {
        tsQueue<std::string> input;
        std::string test = "test";
        std::string value;
        input.push(test);
        REQUIRE(input.empty() == false);
        REQUIRE(input.try_pop(value));
        REQUIRE(input.empty() == true);
        REQUIRE(value == "test");
    }
}

TEST_CASE("Threading Tests","[Threads]") {
    {
        
    }
}
