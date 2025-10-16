#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

#include <exception>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "CLI/CLI11.hpp"

using json = nlohmann::json;

struct Parser {
    bool preview = false;
    std::string sep = "\n";
    std::string title;
    std::string mode;
    std::string format;

    Parser(int argc, char** argv) {
        CLI::App app{"FuzzyMac client"};

        app.add_flag("--preview-file", preview, "Enable file previews for entries that are files");
        app.add_option("--separator", sep, "Character used to separate entries")->default_val("\n");
        app.add_option("--title", title, "Prompt title")->default_val("");
        app.add_option("--mode", mode, "Choose mode: find/input")->default_val("find");
        app.add_option("--format", format, "Specify the format of the list entries")->default_val("");

        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            throw std::runtime_error("Invalid arguments: " + std::string(e.what()));
        }
    }
};

std::string readStdin() {
    if (isatty(STDIN_FILENO)) {
        // No data piped, return empty
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
}

int main(int argc, char* argv[]) {
    std::unique_ptr<Parser> parser = nullptr;

    try {
        parser = std::make_unique<Parser>(argc, argv);
    } catch (const std::exception& e) {
        perror(std::format("Error while parsing the file {}", e.what()).c_str());
        std::cout << "";
        return -1;
    } catch (...) {
        perror("Unknown error while parsing");
        std::cout << "";
        return -1;
    }
    json json_file;

    const char* socket_path = "/tmp/fuzzymac_socket";

    std::string stdin_data = readStdin();

    json_file["stdin"] = std::move(stdin_data);
    json_file["args"]["preview"] = parser->preview;
    json_file["args"]["title"] = parser->title;
    json_file["args"]["separator"] = parser->sep;
    json_file["args"]["mode"] = parser->mode;
    json_file["args"]["format"] = parser->format;

    std::string json_message = json_file.dump();

    // --- Send to POSIX socket ---
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

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
