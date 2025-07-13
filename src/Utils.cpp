#include "FuzzyMac/Utils.hpp"

#include "exprtk/exprtk.hpp"

#include <wordexp.h>
#include <iostream>

void expandPaths(std::vector<std::string>& paths) {
    for (auto& input : paths) {
        wordexp_t p;
        fs::path expanded_path;
        std::string quoted = "\"" + input + "\"";
        if (wordexp(quoted.c_str(), &p, 0) == 0) {
            if (p.we_wordc > 0) {
                input = p.we_wordv[0]; // Take the first expanded word
            }
            wordfree(&p);
        } else {
            std::cerr << "wordexp failed!" << std::endl;
        }
    }
}


std::optional<double> evalMathExp(const std::string& exp) {
    exprtk::expression<double> expression;
    exprtk::parser<double> parser;

    bool is_valid = parser.compile(exp, expression);

    if (is_valid) {
        return expression.value();
    }

    return std::nullopt;
}
