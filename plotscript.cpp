#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "parseInterp.hpp"
#include "interrupt.hpp"

void prompt(){
    std::cout << "\nplotscript> ";
}

std::string readline() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void info(const std::string & err_str){
    std::cout << "Info: " << err_str << std::endl;
}

int eval_from_stream(std::istream & stream){
    Interpreter interp;
    if(!interp.parseStream(stream)){
        error("Invalid Program. Could not parse.");
        return EXIT_FAILURE;
    }
    else{
        try{
            Expression exp = interp.evaluate();
            std::cout << exp << std::endl;
        }
        catch(const SemanticError & ex){
            std::cerr << ex.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int eval_from_file(std::string filename){
    std::ifstream ifs(filename);
    if(!ifs){
        error("Could not open file for reading.");
        return EXIT_FAILURE;
    }
    return eval_from_stream(ifs);
}

int eval_from_command(std::string argexp){
    std::istringstream expression(argexp);
    return eval_from_stream(expression);
}

bool checkInterrupt(Interpreter *interp, parseQueue *pQ, resultQueue *rQ) {
    if (global_status_flag > 0){
        error("interpreter kernel interrupted");
        interp->reset();
        pQ->clear();
        rQ->clear();
        return true;
    }
    return false;
}

// A REPL is a repeated read-eval-print loop
void repl(){
    Interpreter interp;
    bool kernalRunning(true);
    std::atomic_bool solved(false);
    parseQueue pQ; resultQueue rQ;
    parseInterp pI;
    pI.startThread(&pQ, &rQ, &solved, &interp);
    std::string kill = "%%%%%";
    while(!std::cin.eof()){
        global_status_flag = 0;
        prompt();
        std::string line = readline();
        if (std::cin.fail() && kernalRunning) {
            //cntrl-c here
            error("interpreter kernel interrupted");
            std::cin.clear();
            line.clear();
            if(pI.size() > 0) {
                pQ.push(kill);
                pI.joinAll();
            }
            interp.reset();
            pQ.clear();
            rQ.clear();
            pI.startThread(&pQ, &rQ, &solved, &interp);
            continue;
        }
        if(line.empty()) continue;
        if(line.front() == '%') {
            if(line == "%exit") {
                pQ.push(kill);
                pI.joinAll();
                return;
            } else if(line == "%start") {
                kernalRunning = true;
                if(pI.size() == 0) {
                    pI.startThread(&pQ, &rQ, &solved, &interp);
                }
                continue;
            } else if(line == "%stop") {
                kernalRunning = false;
                if(pI.size() > 0) {
                    pQ.push(kill);
                    pI.joinAll();
                }
                continue;
            } else if(line == "%reset") {
                if(pI.size() > 0) {
                    pQ.push(kill);
                    pI.joinAll();
                }
                interp.reset();
                pI.startThread(&pQ, &rQ, &solved, &interp);
                continue;
            }
        }
        if(checkInterrupt(&interp, &pQ, &rQ)) {
            if(pI.size() > 0) {
                pQ.push(kill);
                pI.joinAll();
            }
            pI.startThread(&pQ, &rQ, &solved, &interp);
            continue;
        }
        if(kernalRunning) {
            pQ.push(line);
            Expression exp;
            if(checkInterrupt(&interp, &pQ, &rQ)) {
                if(pI.size() > 0) {
                    pQ.push(kill);
                    pI.joinAll();
                }
                pI.startThread(&pQ, &rQ, &solved, &interp);
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if(checkInterrupt(&interp, &pQ, &rQ)) {
                if(pI.size() > 0) {
                    pQ.push(kill);
                    pI.joinAll();
                }
                pI.startThread(&pQ, &rQ, &solved, &interp);
                continue;
            }
            if(solved) {
                rQ.try_pop(exp);
                std::cout << exp << std::endl;
            } else {
                continue;
            }
        } else {
            error("interpreter kernel not running");
        }
    }
    pI.joinAll();
}

int main(int argc, char *argv[]) {
    install_handler();
    if(argc == 2){
        return eval_from_file(argv[1]);
    }
    else if(argc == 3){
        if(std::string(argv[1]) == "-e"){
            return eval_from_command(argv[2]);
        }
        else{
            error("Incorrect number of command line arguments.");
        }
    }
    else{
        repl();
    }
    return EXIT_SUCCESS;
}
