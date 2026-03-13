#include "websocketserver.h"
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebSockets/qwebsocket.h>
#include <QtCore/QDebug>
#include <QLoggingCategory> // ign
#include <QSaveFile>
#include <qfile.h>

QT_USE_NAMESPACE
static QLoggingCategory category("websocket-server"); // ign
/**
*  Конструктор
**/
WebSocketServer::WebSocketServer(quint16 port, bool debug, QObject *parent) :
        QObject(parent),
        m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                QWebSocketServer::NonSecureMode, this)),
        m_clients(),
        m_debug(debug)
{
       
        if (m_pWebSocketServer->listen(QHostAddress::Any, port)){
                if (m_debug)
                        qDebug(category) << "WebSocketServer listening on port" << port;
                connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                        this, &WebSocketServer::onNewConnection);
                connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WebSocketServer::closed);
        }

        qDebug(category) << " 30 created"; // ign
}
void check_void(){
    qDebug(category) << " 31 check void"; // ign
}
WebSocketServer::~WebSocketServer()
{
        m_pWebSocketServer->close();
        qDebug(category) << " 34"; // ign
        qDeleteAll(m_clients.begin(), m_clients.end());
}

/**
*  Обработчик нового соединения
**/
void WebSocketServer::onNewConnection()
{
        QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

        connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
        connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);

        m_clients << pSocket;

        emit signal_web_new_client(pSocket);
}

/**
*  Отправить данные одному клиенту
**/
void WebSocketServer::senddata(QWebSocket *pClient, QByteArray data)
{
        pClient->sendTextMessage(data);
}

/**
*  Отправить данные всем клиентам
**/
void WebSocketServer::sendall(QByteArray data)
{
    QList<QWebSocket *>::iterator i;
    for (i = m_clients.begin(); i != m_clients.end(); ++i){
            senddata(*i, data);
    }
}

/**
*  Генерирует ответное json сообщение
*  проверки соединения
**/
QByteArray WebSocketServer::get_alive()
{
QJsonObject json;
QJsonObject data_obj;

        json["type"] = "keepalive";
        data_obj["status"] = "alive";
        json["data"] = data_obj;
        QJsonDocument saveDoc(json);

        return saveDoc.toJson();
}


/**
*  Разбор входящего сообщения
**/
void WebSocketServer::parse_message(QWebSocket *pClient, QString data)
{
QJsonObject data_obj;
QJsonParseError err;

    QJsonDocument json = QJsonDocument::fromJson(data.toUtf8(), &err);
    QJsonObject    obj = json.object();
    QJsonValue     val = obj.value("type");

    if(val.toString() == "keepalive"){
        senddata(pClient, get_alive());
        return;
    }

    emit signal_web_message(pClient, obj);
}

/**
** Получено новое сообщение
**/
void WebSocketServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    if (m_debug)
        qDebug(category) << "Message received:" << message;

    parse_message(pClient, message);
}

/**
*  Отключение клиента
**/
void WebSocketServer::socketDisconnected()
{
        QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
        if (m_debug)
                qDebug(category) << "socketDisconnected:" << pClient;
        if (pClient){
                m_clients.removeAll(pClient);
                pClient->deleteLater();
        }
}
