#ifndef PARSEINTERP_HPP
#define PARSEINTERP_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <atomic>
#include <thread>
#include "interpreter.hpp"
#include "startup_config.hpp"
#include "tsQueue.hpp"

typedef tsQueue<std::string> parseQueue;
typedef tsQueue<Expression> resultQueue;

void error(const std::string & err_str) {
    std::cerr << "Error: " << err_str << std::endl;
}

void loadStartup(Interpreter *interp) {
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
}

class parseInterp {
public:
    parseInterp() {}
    void startThread(parseQueue *pQ, resultQueue *rQ, std::atomic_bool *solved, Interpreter * interp) {
        pool.emplace_back(std::thread(&parseInterp::pI, this, pQ, rQ, solved, interp));
    }
    int size() {
        return pool.size();
    }
    void joinAll() {
        for(auto &t : pool) {
            if(t.joinable())
                t.join();
        }
        pool.clear();
    }
private:
    std::vector<std::thread> pool;
    void pI(parseQueue *pQ, resultQueue *rQ, std::atomic_bool *solved, Interpreter * interp) {
        //load startup file
        loadStartup(interp);
        //keep thread alive
        while(1) {
            std::string line;
            pQ->wait_and_pop(line);
            if(line == "%%%%%") return;
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
};
#endif
