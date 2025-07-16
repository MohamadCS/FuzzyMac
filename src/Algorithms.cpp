#include "FuzzyMac/Algorithms.hpp"
#include <QDebug>
#include <cctype>

int fuzzyScore(const QString& cand, const QString& query) {
    int sub_seq_score = 0;
    std::size_t pos = 0;

    auto lower_cand = cand.toLower();

    int q = 0;
    int c = 0;
    int prefix_score = 0;

    while (q < query.size() && c < cand.size()) {
        if (query[q] != lower_cand[c]) {
            break;
        }

        c++;
        q++;
        prefix_score++;
    }

    while (q < query.size() && c < cand.size()) {
        if (query[q] == lower_cand[c]) {
            sub_seq_score += 1;
            ++q;
        }
        ++c;
    }

    if (q != query.size()) {
        return -1;
    }

    return sub_seq_score + prefix_score * 10;
}
