#include "FuzzyMac/ParseConfig.hpp"




QString buildStyleSheet(const toml::table& config, const std::string& widget_name) {
    const std::map<std::string, QString> styles = {
        {"query_input",
         QString(R"(
                                    QLineEdit {
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        background: %4;
                                        border : none;
                                        padding: 12px;
                                    })")
             .arg(get<std::string>(config, {"colors", "query_input", "selection_background"}))
             .arg(get<std::string>(config, {"colors", "query_input", "selection"}))
             .arg(get<std::string>(config, {"colors", "query_input", "text"}))
             .arg(get<std::string>(config, {"colors", "query_input", "background"}))},
        {"results_list",
         QString(R"(
                                    QListWidget {
                                        background: %4;
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        border: none;
                                        padding: 0px;
                                    })")
             .arg(get<std::string>(config, {"colors", "results_list", "selection_background"}))
             .arg(get<std::string>(config, {"colors", "results_list", "selection"}))
             .arg(get<std::string>(config, {"colors", "results_list", "text"}))
             .arg(get<std::string>(config, {"colors", "results_list", "background"}))},
    };
    return styles.at(widget_name);
}




