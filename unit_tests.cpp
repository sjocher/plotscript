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
