#ifndef QLISTCLIENT_H
#define QLISTCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QMap>
#include <QHash>

#include <QDeferred>

#include <QListResponse>

struct QUserEntryInfo
{
	QString userName;
	QString clientName;
};

typedef QMap<quint32, QUserEntryInfo> QUserMap;

typedef QDeferred<QListResponse, QString> QPendingReq;
typedef QHash<quint32, QPendingReq> QPendingReqsMap;

class QListClient : public QObject
{
    Q_OBJECT
public:
    explicit QListClient(QObject *parent = nullptr);

	QDeferred<QString> start();
	QDeferred<QString> stop();
	bool               isRunning() const;
	bool               isConnected() const;

	quint16 port() const;
	void    setPort(const quint16 &port);

	QString ipAddress() const;
	void    setIpAddress(const QString &ipAddress);

	QDeferred<QString , qint64         > getName();
	QDeferred<QUserMap, QString, qint64> getAllUsers();
	QDeferred<QUserMap, QString, qint64> getNewUser();
	QDeferred<QString , qint64         > insertUser(const quint32 &userId, const QString &userName);

signals:
	void disconnected();

private slots:
    void on_binaryMessageReceived(QByteArray message);

private:
    QWebSocket      m_wsSocket;
	quint16         m_serverPort;
	QString         m_serverIpAddress;
    QUrl            m_url;
	quint32         m_requestCounter;
	QPendingReqsMap m_pendingRequests;

	QList<QMetaObject::Connection> m_listConnsStart;
	QList<QMetaObject::Connection> m_listConnsStop;

	void disconnectConnsStart();
	void disconnectConnsStop();
	void clearPendingRequests();
};

#endif // QLISTCLIENT_H
