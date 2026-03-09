#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WebSocketServer : public QObject
{
        Q_OBJECT
public:
    explicit WebSocketServer(quint16 port, bool debug = false, QObject *parent = Q_NULLPTR);
    ~WebSocketServer();

    void senddata(QWebSocket *pClient, QByteArray data);
    void sendall(QByteArray data);


Q_SIGNALS:
    void closed();

signals:
    void signal_web_new_client( QWebSocket *pSocket );
    void signal_web_message( QWebSocket *pSocket, QJsonObject val);

private Q_SLOTS:
    void onNewConnection();
    void parse_message(QWebSocket *pClient, QString data);
    void processTextMessage(QString message);
    void socketDisconnected();
    QByteArray get_alive();

private:
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket *> m_clients;
        bool m_debug;
        void check_void();
};

#endif
