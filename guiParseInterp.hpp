#ifndef GUIPARSEINTERP_HPP
#define GUIPARSEINTERP_HPP
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <atomic>
#include <thread>
#include "interpreter.hpp"
#include "tsQueue.hpp"

typedef tsQueue<QString> parseQueue;
typedef tsQueue<Expression> resultQueue;
typedef tsQueue<std::string> messageQueue;

class guiParseInterp {
public:
    guiParseInterp() {}
    void startThread(messageQueue *mQ, parseQueue *pQ, resultQueue *rQ, std::atomic_bool *solved, Interpreter * interp) {
        pool.emplace_back(std::thread(&guiParseInterp::gpI, this, mQ, pQ, rQ, solved, interp));
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
    void gpI(messageQueue *mQ, parseQueue *pQ, resultQueue *rQ, std::atomic_bool *solved, Interpreter * interp) {
        while(1) {
            QString line;
            pQ->wait_and_pop(line);
            std::istringstream expression(line.toStdString());
            if(line.toStdString() == "%%%%%") return;
            if(!interp->parseStream(expression)){
                mQ->push("Error: Could not parse.");
                solved->store(false);
            }
            else{
                try{
                    Expression exp = interp->evaluate();
                    rQ->push(exp);
                    solved->store(true);
                    exp = Expression();
                }
                catch(const SemanticError & ex){
                    std::string error = ex.what();
                    mQ->push(error);
                    solved->store(false);
                }
            }
        }
    }
};
#endif
