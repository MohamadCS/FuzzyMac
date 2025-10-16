// Server.cpp
#include "FuzzyMac/Server.hpp"
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "spdlog/spdlog.h"
#include <QDebug>
#include <functional>

Server::Server(MainWindow* win, std::function<void()> diconnectHandler)
    : QObject(win),
      server(new QLocalServer(this)),
      current_client(nullptr), // track the active connection
      win(win),
      disconnectHandler(diconnectHandler) {
    connect(server, &QLocalServer::newConnection, this, &Server::handleNewConnection);
}

Server::~Server() {
    if (current_client) {
        current_client->disconnectFromServer();
        current_client->deleteLater();
    }
    server->close();
}

void Server::startServer(const QString& server_name) {
    QLocalServer::removeServer(server_name); // remove old socket file if it exists

    if (!server->listen(server_name)) {
        qWarning() << "Unable to start server:" << server->errorString();
    } else {
        qDebug() << "Server started on" << server_name;
    }
}

void Server::handleNewConnection() {

    if (current_client) {
        current_client->disconnectFromServer(); // triggers old socket disconnected lambda
    }

    QLocalSocket* client_conn = server->nextPendingConnection();
    auto socket = client_conn;

    current_client = socket;

    connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
        win->handleNewRequest(); 
    });

    connect(socket, &QLocalSocket::disconnected, socket, [this, socket]() {
        if (current_client == socket) {
            current_client = nullptr;
        }
        disconnectHandler();
        socket->deleteLater();
    });
}

QLocalSocket* Server::getCurrentClient() const {
    return current_client;
}

void Server::dropConnection() {
    spdlog::info("Started dropping connection");
    if (!current_client) {
        return;
    }

    // Disconnect all signals from the current client
    disconnect(current_client, nullptr, this, nullptr);
    // Close the socket
    current_client->disconnectFromServer();
    current_client = nullptr;
}

void Server::handleReadyRead() {
}
