#include "qlistclient.h"
#include <QElapsedTimer>
#include <QSharedPointer>
#include <QDebug>

#include <QListRequest>

typedef QSharedPointer<QElapsedTimer> QReqTimerPtr;

QT_USE_NAMESPACE

QListClient::QListClient(QObject *parent) :
    QObject(parent)
{
	m_requestCounter = 0;
	this->setPort(8080);
	this->setIpAddress("127.0.0.1");
	connect(&m_wsSocket, &QWebSocket::binaryMessageReceived, this, &QListClient::on_binaryMessageReceived);
}

QDeferred<QString> QListClient::start()
{
	QDeferred<QString> retDef;
	// check if connected
	if (this->isRunning())
	{
		retDef.reject(tr("Client is currently running, please stop it first"));
		return retDef;
	}
	// check ip address and port
	
	QString strFullAddress = QString("ws://%1:%2").arg(m_serverIpAddress).arg(m_serverPort);
	QUrl url(strFullAddress);
	if (!url.isValid())
	{
		retDef.reject(tr("Invalid configuration : %1").arg(url.toDisplayString()));
		return retDef;
	}
	// disconnect old
	this->disconnectConnsStart();
	// connect to relevant signals
	m_listConnsStart <<
	connect(&m_wsSocket, &QWebSocket::connected, this, 
	[this, retDef]() mutable {
		// disconnect first
		this->disconnectConnsStart();
		// resolve promise
		retDef.resolve(tr("success"));
		// handle disconnected by unexpected reasons
		m_listConnsStart <<
		connect(&m_wsSocket, &QWebSocket::disconnected, this, 
		[this]() mutable {
			emit this->disconnected();
			// disconnect first
			this->disconnectConnsStart();
			// clear pending requests
			this->clearPendingRequests();
		});
	});
	m_listConnsStart <<
	connect(&m_wsSocket, &QWebSocket::disconnected, this, 
	[this, retDef]() mutable {
		// disconnect first
		this->disconnectConnsStart();
		// reject promise
		retDef.reject(tr("connection error : %1").arg(m_wsSocket.errorString()));
	});
	m_listConnsStart <<
	connect(&m_wsSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
	[this, retDef]() mutable {
		// disconnect first
		this->disconnectConnsStart();
		// reject promise
		retDef.reject(tr("connection error : %1").arg(m_wsSocket.errorString()));
	});
	// actually start
    m_wsSocket.open(url);
	// return promise
	return retDef;
}

QDeferred<QString> QListClient::stop()
{
	QDeferred<QString> retDef;
	// success if already disconnected
	if (!this->isRunning())
	{
		retDef.resolve("success");
		return retDef;
	}
	// disconnect old stop signals
	this->disconnectConnsStop();
	// disconnect old start signals
	this->disconnectConnsStart();
	// connect to relevant signals
	m_listConnsStop <<
	connect(&m_wsSocket, &QWebSocket::disconnected, this, 
	[this, retDef]() mutable {
		emit this->disconnected();
		// disconnect first
		this->disconnectConnsStop();
		// resolve promise
		retDef.resolve(tr("success"));	
		// clear pending requests
		this->clearPendingRequests();
	});
	m_listConnsStop <<
	connect(&m_wsSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
	[this, retDef]() mutable {
		// disconnect first
		this->disconnectConnsStop();
		// reject promise
		retDef.reject(tr("socket error : %1").arg(m_wsSocket.errorString()));
		// clear pending requests
		this->clearPendingRequests();
	});
	// actually stop
	m_wsSocket.close();
	// return promise
	return retDef;
}

bool QListClient::isRunning() const
{
	return m_wsSocket.state() != QAbstractSocket::UnconnectedState;
}

bool QListClient::isConnected() const
{
	return m_wsSocket.state() == QAbstractSocket::ConnectedState;
}

quint16 QListClient::port() const
{
	return m_serverPort;
}

void QListClient::setPort(const quint16 & port)
{
	m_serverPort = port;
}

QString QListClient::ipAddress() const
{
	return m_serverIpAddress;
}

void QListClient::setIpAddress(const QString & ipAddress)
{
	QString cleanIpAddress = ipAddress.trimmed();
	cleanIpAddress.remove("ws://", Qt::CaseInsensitive);
	m_serverIpAddress = cleanIpAddress;
}

QDeferred<QString, qint64> QListClient::getName()
{
	QDeferred<QString, qint64> retDef;
	// check if connected
	if (!this->isConnected())
	{
		retDef.reject("not connected", 0);
		return retDef;
	}
	// create request
	QListRequest req;
	quint32 reqId = ++m_requestCounter;
	req.setId(reqId);
	req.setType(QListRequestType::GET_NAME);
	//qDebug() << "[GET_NAME] " << req;
	// measure time
	QReqTimerPtr pTimer = QReqTimerPtr(new QElapsedTimer);
	// store in pending requests
	QPendingReq pendingReq;
	pendingReq
	.fail([this, retDef](QListResponse res, QString strError) mutable {
		Q_UNUSED(res);
		retDef.reject(strError, 0);
	})
	.done([this, retDef, pTimer](QListResponse res, QString strSuccess) mutable {
		Q_UNUSED(strSuccess);
		retDef.resolve(res.clientName(), pTimer->elapsed());
	});
	Q_ASSERT(!m_pendingRequests.contains(reqId));
	m_pendingRequests[reqId] = pendingReq;
	// actually send request
	pTimer->start();
	m_wsSocket.sendBinaryMessage(req.toByteArray());
	// return promise
	return retDef;
}

QDeferred<QUserMap, QString, qint64> QListClient::getAllUsers()
{
	QDeferred<QUserMap, QString, qint64> retDef;
	// check if connected
	if (!this->isConnected())
	{
		retDef.reject(QUserMap(), "not connected", 0);
		return retDef;
	}
	// create request
	QListRequest req;
	quint32 reqId = ++m_requestCounter;
	req.setId(reqId);
	req.setType(QListRequestType::GET_ALL_USERS);
	//qDebug() << "[GET_ALL_USERS] " << req;
	// measure time
	QReqTimerPtr pTimer = QReqTimerPtr(new QElapsedTimer);
	// store in pending requests
	QPendingReq pendingReq;
	pendingReq
	.fail([this, retDef](QListResponse res, QString strError) mutable {
		Q_UNUSED(res);
		retDef.reject(QUserMap(), strError, 0);
	})
	.done([this, retDef, pTimer](QListResponse res, QString strSuccess) mutable {
		Q_UNUSED(strSuccess);
		QUserMap usersMap;
		for (quint32 i = 0; i < res.elementCount(); i++)
		{
			QListElement elem = res.mutableElementAt(i);
			usersMap[elem.id()] = {
				elem.name(),
				elem.clientName()
			};
		}
		retDef.resolve(usersMap, "success", pTimer->elapsed());
	});
	Q_ASSERT(!m_pendingRequests.contains(reqId));
	m_pendingRequests[reqId] = pendingReq;
	// actually send request
	pTimer->start();
	m_wsSocket.sendBinaryMessage(req.toByteArray());
	// return promise
	return retDef;
}

QDeferred<QUserMap, QString, qint64> QListClient::getNewUser()
{
	QDeferred<QUserMap, QString, qint64> retDef;
	// check if connected
	if (!this->isConnected())
	{
		retDef.reject(QUserMap(), "not connected", 0);
		return retDef;
	}
	// create request
	QListRequest req;
	quint32 reqId = ++m_requestCounter;
	req.setId(reqId);
	req.setType(QListRequestType::GET_NEW_USER);
	//qDebug() << "[GET_NEW_USER] " << req;
	// measure time
	QReqTimerPtr pTimer = QReqTimerPtr(new QElapsedTimer);
	// store in pending requests
	QPendingReq pendingReq;
	pendingReq
	.fail([this, retDef](QListResponse res, QString strError) mutable {
		Q_UNUSED(res);
		retDef.reject(QUserMap(), strError, 0);
	})
	.done([this, retDef, pTimer](QListResponse res, QString strSuccess) mutable {
		Q_UNUSED(strSuccess);
		QUserMap usersMap;
		for (quint32 i = 0; i < res.elementCount(); i++)
		{
			QListElement elem = res.mutableElementAt(i);
			usersMap[elem.id()] = {
				elem.name(),
				elem.clientName()
			};
		}
		retDef.resolve(usersMap, "success", pTimer->elapsed());
	});
	Q_ASSERT(!m_pendingRequests.contains(reqId));
	m_pendingRequests[reqId] = pendingReq;
	// actually send request
	pTimer->start();
	m_wsSocket.sendBinaryMessage(req.toByteArray());
	// return promise
	return retDef;
}

QDeferred<QString, qint64> QListClient::insertUser(const quint32 & userId, const QString & userName)
{
	QDeferred<QString, qint64> retDef;
	// check if connected
	if (!this->isConnected())
	{
		retDef.reject("not connected", 0);
		return retDef;
	}
	// create request
	QListRequest req;
	quint32 reqId = ++m_requestCounter;
	req.setId(reqId);
	req.setType(QListRequestType::INSERT_USER);
	req.mutableElement()
		.setId(userId)
		.setName(userName);
	//qDebug() << "[INSERT_USER] " << req;
	// measure time
	QReqTimerPtr pTimer = QReqTimerPtr(new QElapsedTimer);
	// store in pending requests
	QPendingReq pendingReq;
	pendingReq
	.fail([this, retDef](QListResponse res, QString strError) mutable {
		Q_UNUSED(res);
		retDef.reject(strError, 0);
	})
	.done([this, retDef, pTimer](QListResponse res, QString strSuccess) mutable {
		Q_UNUSED(res);
		retDef.resolve(strSuccess, pTimer->elapsed());
	});
	Q_ASSERT(!m_pendingRequests.contains(reqId));
	m_pendingRequests[reqId] = pendingReq;
	// actually send request
	pTimer->start();
	m_wsSocket.sendBinaryMessage(req.toByteArray());
	// return promise
	return retDef;
}

void QListClient::on_binaryMessageReceived(QByteArray message)
{
	QListResponse res = QListResponse::FromByteArray(message);
	quint32 reqId = res.requestId();
	// early exit if had already cleared requests due to unexpected disconnection
	if (!m_pendingRequests.contains(reqId))
	{
		return;
	}
	QListResponseResult result = res.result();
	Q_ASSERT(result != QListResponseResult::NULL_RESULT);
	QPendingReq pendingReq = m_pendingRequests.take(reqId);
	// check result
	if (result == QListResponseResult::SUCCESS)
	{
		pendingReq.resolve(res, "success");
	}
	else
	{
		pendingReq.reject(res, result == QListResponseResult::REJECTED ? "rejected" : "unknown error");
	}
}

void QListClient::disconnectConnsStart()
{
	while (m_listConnsStart.count() > 0)
	{
		QObject::disconnect(m_listConnsStart.takeFirst());
	}
}

void QListClient::disconnectConnsStop()
{
	while (m_listConnsStop.count() > 0)
	{
		QObject::disconnect(m_listConnsStop.takeFirst());
	}
}

void QListClient::clearPendingRequests()
{
	auto listKeys = m_pendingRequests.keys();
	while (m_pendingRequests.count() > 0)
	{
		Q_ASSERT(listKeys.count() == m_pendingRequests.count());
		QPendingReq pendingReq = m_pendingRequests.take(listKeys.takeFirst());
		pendingReq.reject(QListResponse(), "unexpectedly disconnected");
	}
}
