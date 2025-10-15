// LocalServer.h
#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>

class Server : public QObject {
    Q_OBJECT
public:
     Server(MainWindow* win);
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
    MainWindow* win;
};
