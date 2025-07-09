#include "FuzzyMac/Algorithms.hpp"
#include <QDebug>
#include <algorithm>
#include <cctype>

static char flipChar(char c) {
    if (c >= 'a' && c <= 'z') {
        return 'A' + c - 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return 'a' + c - 'A';
    } else {
        return c;
    }
}

static int prefixScore(const std::string& cand, const std::string& query) {
    const auto N = std::min(cand.size(), query.size());

    int score = 0;
    for (int i = 0; i < N; ++i) {
        if (cand[i] == query[i]) {
            score++;
        } else {
            return score;
        }
    }

    return score;
}

static std::string toLower(const std::string& str) {
    std::string res;
    res.resize(str.size());

    for (int i = 0; i < str.size(); ++i) {
        res[i] = (str[i] >= 'A' && str[i] <= 'Z') ? str[i] - 'A' + 'a' : str[i];
    }

    return res;
}

int fuzzyScore(const std::string& cand, const std::string& query) {
    int score = 0;
    std::size_t pos = 0;

    std::string lower_cand = toLower(cand);


    for (auto ch : query) {
        pos = lower_cand.find(ch, pos);

        if (pos == std::string::npos) {
            return -1;
        }
        ++score;
        ++pos;
    }

    score += prefixScore(lower_cand, query);

    return score;
}
