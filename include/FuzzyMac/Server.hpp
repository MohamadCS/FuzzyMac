// LocalServer.h
#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <functional>

class Server : public QObject {
    Q_OBJECT
public:
    Server(MainWindow* win, std::function<void()> disconnectHandler);
    ~Server();

    void startServer(const QString& serverName);

    QLocalSocket* getCurrentClient() const;

    void dropConnection();

private slots:
    void handleNewConnection();
    void handleReadyRead();

private:
    QLocalServer* server;
    QLocalSocket* current_client;
    std::function<void()> disconnectHandler;
    MainWindow* win;
};
