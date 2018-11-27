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

#include "semantic_error.hpp"
#include "tsQueue.hpp"

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
