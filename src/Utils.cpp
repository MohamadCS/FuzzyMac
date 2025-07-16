#include "FuzzyMac/Utils.hpp"

#include "exprtk/exprtk.hpp"

#include <QString>
#include <QStringList>
#include <iostream>
#include <wordexp.h>

void expandPaths(QStringList& paths) {
    for (auto& input : paths) {
        wordexp_t p;
        fs::path expanded_path;
        QString quoted = "\"" + input + "\"";
        if (wordexp(quoted.toUtf8().constData(), &p, 0) == 0) {
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
