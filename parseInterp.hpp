#ifndef PARSEINTERP_HPP
#define PARSEINTERP_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "interpreter.hpp"
#include "startup_config.hpp"
#include "tsQueue.hpp"

typedef tsQueue<std::string> parseQueue;
typedef tsQueue<Expression> resultQueue;

void error(const std::string & err_str){
    std::cerr << "Error: " << err_str << std::endl;
}

class parseInterp {
public:
    parseInterp(parseQueue *parseQ, resultQueue *resultQ) {
        pQ = parseQ;
        rQ = resultQ;
    }
    void operator()() const {
        //load startup file
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
        //keep thread alive
        while(1) {
            std::string line;
            pQ->wait_and_pop(line);
            std::istringstream expression(line);
            if(!interp.parseStream(expression)){
                error("Invalid Expression. Could not parse.");
            }
            else{
                try{
                    Expression exp = interp.evaluate();
                    rQ->push(exp);
                }
                catch(const SemanticError & ex){
                    std::cerr << ex.what() << std::endl;
                }
            }
        }
    }
    void reset() {
        
    }
private:
    parseQueue * pQ;
    resultQueue * rQ;
};

#endif
