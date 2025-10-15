// Server.cpp
#include "FuzzyMac/Server.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "spdlog/spdlog.h"
#include <QDebug>

Server::Server(MainWindow* win)
    : QObject(win),
      server(new QLocalServer(this)),
      current_client(nullptr), // track the active connection
      win(win) {
    connect(server, &QLocalServer::newConnection, this, &Server::handleNewConnection);
}

Server::~Server() {
    if (current_client) {
        current_client->disconnectFromServer();
        current_client->deleteLater();
    }
    server->close();
}

void Server::startServer(const QString& serverName) {
    QLocalServer::removeServer(serverName); // remove old socket file if it exists

    if (!server->listen(serverName)) {
        qWarning() << "Unable to start server:" << server->errorString();
    } else {
        qDebug() << "Server started on" << serverName;
    }
}

void Server::handleNewConnection() {
    QLocalSocket* client_conn = server->nextPendingConnection();

    // Disconnect old client if exists
    if (current_client) {
        current_client->disconnectFromServer();
        current_client->deleteLater();
        current_client = nullptr;
        spdlog::warn("Client interrupted, closing connection");
    }

    current_client = client_conn;

    connect(client_conn, &QLocalSocket::readyRead, this, &Server::handleReadyRead);
    connect(client_conn, &QLocalSocket::disconnected, client_conn, &QLocalSocket::deleteLater);

    spdlog::info("Connected to a new client");
}

QLocalSocket* Server::getCurrentClient() const {
    return current_client;
}

void Server::dropConnection() {
    if (!current_client) {
        return;
    }

    // Disconnect all signals from the current client
    disconnect(current_client, nullptr, this, nullptr);

    // Close the socket
    current_client->disconnectFromServer();

    // Schedule deletion
    current_client->deleteLater();

    spdlog::warn("Dropped current client connection");

    current_client = nullptr;
}

void Server::handleReadyRead() {
    if (!current_client) {
        return;
    }

    win->handleNewRequest();
}
