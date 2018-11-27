#ifndef PARSEINTERP_HPP
#define PARSEINTERP_HPP

#include "interpreter.hpp"
#include "tsQueue.hpp"

typedef tsQueue<std::string> parseQueue;
typedef tsQueue<Expression> resultQueue;

void error(const std::string & err_str){
    std::cerr << "Error: " << err_str << std::endl;
}

class parseInterp {
public:
    parseInterp(parseQueue *parseQ, resultQueue *resultQ, Interpreter *interpreter) {
        pQ = parseQ;
        rQ = resultQ;
        interp = interpreter;
    }
    void operator()() const {
        while(1) {
            std::string line;
            pQ->wait_and_pop(line);
            std::istringstream expression(line);
            if(!interp->parseStream(expression)){
                error("Invalid Expression. Could not parse.");
            }
            else{
                try{
                    Expression exp = interp->evaluate();
                    rQ->push(exp);
                }
                catch(const SemanticError & ex){
                    std::cerr << ex.what() << std::endl;
                }
            }
        }
    }
private:
    parseQueue * pQ;
    resultQueue * rQ;
    Interpreter * interp;
};

#endif
