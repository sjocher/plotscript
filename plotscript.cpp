#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "tsQueue.hpp"
#include "parseInterp.hpp"

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


// A REPL is a repeated read-eval-print loop
void repl(std::thread *thread){
    std::atomic_bool kernalRunning(false);
    std::atomic_bool solved(false);
    parseQueue pQ;
    resultQueue rQ;
    parseInterp pI(&pQ, &rQ, &kernalRunning, &solved);
    //*thread = std::thread(pI);
    while(!std::cin.eof()){
        prompt();
        std::string line = readline();
        if(line.empty()) continue;
        if(line.front() == '%') {
            if(line == "%start") {
                if(!kernalRunning) {
                    kernalRunning.store(true);
                    *thread = std::thread(pI);
                }
                continue;
            } else if (line == "%stop") {
                kernalRunning.store(false);
                if(thread->joinable()) {
                    pQ.push(line);
                    thread->join();
                }
                continue;
            } else if (line == "%reset") {
                if(thread->joinable()) {
                    pQ.push(line);
                    thread->join();
                    *thread = std::thread(pI);
                }
                continue;
            }
        }
        if(kernalRunning) {
            pQ.push(line);
            Expression exp;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if(solved) {
                rQ.try_pop(exp);
                std::cout << exp << std::endl;
            } else {
                continue;
            }
            //weird fix but w/e
        } else {
            error("interpreter kernel not running");
        }
    }
}

int main(int argc, char *argv[]) {
    std::thread newThread;
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
        repl(&newThread);
    }
    if(newThread.joinable()) {
        newThread.join();
    }
    return EXIT_SUCCESS;
}
