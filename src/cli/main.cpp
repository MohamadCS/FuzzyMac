#include <QCoreApplication>
#include <QDebug>
#include <QLocalSocket>
#include <QTextStream>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    QLocalSocket socket;
    QString serverName = "fuzzymac_socket"; // same as server

    socket.connectToServer(serverName);
    if (!socket.waitForConnected(5000)) { // wait up to 5s for server
        qCritical() << "Failed to connect to server:" << socket.errorString();
        return 1;
    }

    // Read all stdin (supports piping)
    QTextStream in(stdin);
    QString input = in.readAll();
    QByteArray message = input.toUtf8();

    // Send to server
    socket.write(message);
    socket.flush();

    // Wait for server to reply (blocks until ready)
    if (socket.waitForReadyRead(-1)) { // -1 = wait indefinitely
        QByteArray response = socket.readAll();
        QTextStream out(stdout);
        out << response;
        out.flush();
    } else {
        qWarning() << "No response from server";
    }

    socket.disconnectFromServer();
    return 0;
}
