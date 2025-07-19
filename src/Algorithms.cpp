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


QStringList customSearch(MainWindow* win, const QString& query_, const QStringList& entries) {

    QString query = query_.toLower();

    std::vector<std::pair<int, int>> scores_per_idx{};

    // calculate the score for each entry.
    for (int i = 0; i < entries.size(); ++i) {
        int score = fuzzyScore(QFileInfo(entries[i]).fileName(), query);

        if (score >= 0) {
            scores_per_idx.push_back({score, i});
        }
    }

    // sort in decreasing order according to score
    std::sort(scores_per_idx.begin(), scores_per_idx.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    });

    QStringList res{};
    res.reserve(scores_per_idx.size());

    // fill resultslist based on the scores
    for (int i = 0; i < scores_per_idx.size(); ++i) {
        int idx = scores_per_idx[i].second;
        res.push_back(entries[idx]);
    }

    return res;
}
