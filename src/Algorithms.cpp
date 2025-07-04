#include "../include/App/Algorithms.hpp"

int fuzzyScore(const std::string& cand, const std::string& query) {
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
    return score;
}


