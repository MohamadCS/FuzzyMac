#include "FuzzyMac/Utils.hpp"

#include "exprtk/exprtk.hpp"

#include <QString>
#include <QStringList>
#include <iostream>
#include <wordexp.h>

QString convertToReadableFileSize(qint64 size) {
    constexpr qint64 KB = 1024;
    constexpr qint64 MB = 1024 * KB;
    constexpr qint64 GB = 1024 * MB;
    constexpr qint64 TB = 1024 * GB;

    if (size < KB) {
        return QString::number(size) + " B";
    } else if (size < MB) {
        return QString::number(size / double(KB), 'f', 2) + " KB";
    } else if (size < GB) {
        return QString::number(size / double(MB), 'f', 2) + " MB";
    } else if (size < TB) {
        return QString::number(size / double(GB), 'f', 2) + " GB";

    } else {
        return QString::number(size / double(TB), 'f', 2) + " TB";
    }
}

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
