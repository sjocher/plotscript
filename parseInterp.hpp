#ifndef PARSEINTERP_HPP
#define PARSEINTERP_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <atomic>
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
    parseInterp(parseQueue *parseQ, resultQueue *resultQ, std::atomic_bool *sol, Interpreter * interpreter) {
        pQ = parseQ;
        rQ = resultQ;
        solved = sol;
        interp = interpreter;
    }
    void operator()() const {
        //load startup file
        std::ifstream startup(STARTUP_FILE);
        if(!interp->parseStream(startup)) {
            error("Invalid Startup. Could not parse.");
        } else {
            try {
                Expression exp = interp->evaluate();
            } catch (const SemanticError & ex){
                std::cerr << ex.what() << std::endl;
            }
        }
        //keep thread alive
        while(1) {
            std::string line;
            pQ->wait_and_pop(line);
            if(line == "%stop" || line == "%reset") break;
            std::istringstream expression(line);
            if(!interp->parseStream(expression)){
                error("Invalid Expression. Could not parse.");
                solved->store(false);
            }
            else{
                try{
                    Expression exp = interp->evaluate();
                    solved->store(true);
                    rQ->push(exp);
                }
                catch(const SemanticError & ex){
                    solved->store(false);
                    std::cerr << ex.what() << std::endl;
                }
            }
        }
    }
private:
    parseQueue * pQ;
    resultQueue * rQ;
    std::atomic_bool * solved;
    Interpreter * interp;
};
#endif
