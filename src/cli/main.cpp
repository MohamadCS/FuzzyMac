#include "nlohmann/json.hpp"
#include "shared/Data.hpp"

#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "CLI/CLI11.hpp"

using json = nlohmann::json;

struct Args {
    bool preview = false;
    std::string sep = "\n";
    std::string title;
    std::string mode;
    std::string format;
};

std::unique_ptr<CLI::App> createCliTool(Args& args) {
    auto app = std::make_unique<CLI::App>("FuzzyMac client");

    app->add_flag("--preview", args.preview, "Enable previews for existing paths");
    app->add_option("--sep", args.sep, "List entries seperator")->default_val("\n");
    app->add_option("--title", args.title, "Prompt title")->default_val("");
    app->add_option("--mode", args.mode, "input/find")->default_val("find");
    app->add_option("--format", args.format, "Format of search filtering and label's text")->default_val("");

    return app;
}

std::string readStdin() {
    if (isatty(STDIN_FILENO)) {
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
}

void fillJsonObj(json& json_file, const Args& args) {
    json_file["stdin"] = readStdin();
    json_file["args"]["preview"] = args.preview;
    json_file["args"]["title"] = args.title;
    json_file["args"]["separator"] = args.sep;
    json_file["args"]["mode"] = args.mode;
    json_file["args"]["format"] = args.format;
}

int main(int argc, char* argv[]) {

    Args args;
    auto app = createCliTool(args);

    CLI11_PARSE(*app, argc, argv);

    json json_file;
    fillJsonObj(json_file, args);

    std::string json_message = json_file.dump();

    /// Communicate with server

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, server_path.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    if (write(sock, json_message.c_str(), json_message.size()) < 0) {
        perror("write");
        close(sock);
        return 1;
    }

    // Read simple response
    char buffer[1024];
    ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << buffer;
    }

    close(sock);
}
