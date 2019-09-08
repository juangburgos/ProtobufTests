#ifndef QLISTSERVER_H
#define QLISTSERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QMap>
#include <QHash>
#include <QQueue>
#include <QTimer>

#include <QDeferred>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(QListRequest)

class QListServer : public QObject
{
    Q_OBJECT
public:
    explicit QListServer(QObject *parent = nullptr);
    ~QListServer();

	bool    start(const bool &autoStop = false, const int timeout = 60000);
	QString stop();
	bool    isRunning() const;

	quint16 port() const;
	void    setPort(const quint16 &port);

	QString flush() const;

	QString csvClients() const;
	QString csvUsers() const;

signals:
	void autoStopped(const QString &strFileName);

private slots:
    void on_newConnection();
    void on_binaryMessageReceived(QByteArray message);
    void on_disconnected();

private:
    QWebSocketServer * m_wsServer;
	quint16            m_serverPort;
	QTimer             m_timerTimeout;
	QMetaObject::Connection m_connTimer;

	struct QUserEntryInfo
	{
		QString userName;
		QString clientName;
	};

	typedef QDeferred<quint32, QUserEntryInfo> QPendingReq;
	typedef QQueue<QPendingReq> QPendingReqsQueue;

	struct QClientInfo
	{
		QString           clientName;
		QPendingReqsQueue pendingReqs;
	};

	QHash<QWebSocket*, QClientInfo   > m_hashClients;
	QMap <quint32    , QUserEntryInfo> m_mapUsers  ;

	void getName    (const QListRequest &req, QWebSocket *pClient);
	void getAllUsers(const QListRequest &req, QWebSocket *pClient);
	void getNewUser (const QListRequest &req, QWebSocket *pClient);
	void insertUser (const QListRequest &req, QWebSocket *pClient);
};

#endif //QLISTSERVER_H
