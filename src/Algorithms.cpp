#include "../include/App/Algorithms.hpp"
#include "wx/arrstr.h"
#include <algorithm>
#include <cctype>



static char flipChar(char c) {
    if (c >= 'a' && c <='z') {
        return 'A' + c - 'a';
    } else if (c >= 'A' && c <='Z'){
        return 'a' + c - 'A';
    } else {
        return c; 
    }
}

static int prefixScore(const wxString& cand,const std::string& query) {
    const auto N = std::min(cand.size(), query.size());

    int score = 0;
    for(int i = 0; i < N; ++i){
        if(cand[i] == query[i]) {
            score++;
        } else {
            return score;
        }
    }

    return score;
}

int fuzzyScore(const wxString& cand, const std::string& query) {
    int score = 0;
    std::size_t pos = 0;

    for(auto ch : query) {
        pos = cand.find(ch, pos);

        if(pos == std::string::npos) {
            return -1;
        }
        ++score;
        ++pos;
    }

    score += prefixScore(cand, query);

    return score;
}




