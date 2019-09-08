#include "qlistserver.h"
#include <QWebSocketServer>
#include <QWebSocket>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#include <QListRequest>
#include <QListResponse>

QT_USE_NAMESPACE

QListServer::QListServer(QObject *parent) :
    QObject(parent),
    m_wsServer(new QWebSocketServer(QStringLiteral("QListServer"), QWebSocketServer::NonSecureMode, this))
{
	this->setPort(8080);
}

QListServer::~QListServer()
{
	this->stop();
}

void QListServer::on_newConnection()
{
    QWebSocket *pSocket = m_wsServer->nextPendingConnection();
	if (!pSocket)
	{
		return;
	}
	// assign name to new client conneciton
	Q_ASSERT(!m_hashClients.contains(pSocket));
	// name is peer ip address and port
	m_hashClients[pSocket] = {
		QString("%1:%2")
			.arg(pSocket->peerAddress().toString())
			.arg(pSocket->peerPort()),
		QPendingReqsQueue()
	};
	// connect to client signals
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &QListServer::on_binaryMessageReceived);
    connect(pSocket, &QWebSocket::disconnected         , this, &QListServer::on_disconnected         );
}

bool QListServer::start(const bool &autoStop/* = false*/, const int timeout/* = 60000*/)
{
	if (this->isRunning())
	{
		return true;
	}
	bool success = m_wsServer->listen(QHostAddress::Any, m_serverPort);
	if (!success)
	{
		return false;
	}
	connect(m_wsServer, &QWebSocketServer::newConnection, this, &QListServer::on_newConnection);
	// start auto-stop timer
	if (autoStop)
	{
		QObject::disconnect(m_connTimer);
		m_timerTimeout.setInterval(timeout);
		m_connTimer = 
		QObject::connect(&m_timerTimeout, &QTimer::timeout, this, 
		[this, timeout]() {
			//qDebug() << "[AUTO_STOP] No activity in" << timeout << "msecs";
			QString strFileName = this->stop();
			emit this->autoStopped(strFileName);
		});
		m_timerTimeout.start();
	}
	return true;
}

QString QListServer::stop()
{
	// stop timer
	QObject::disconnect(m_connTimer);
	m_timerTimeout.stop();
	// exit if already stopped
	if (!this->isRunning())
	{
		return QString();
	}
	// stop listening
	m_wsServer->close();
	// flush users to disk
	QString strFileName = this->flush();
	// clear clients
	auto clients = m_hashClients.keys();
	for (auto client : clients)
	{
		// reject all pending requests
		while(!m_hashClients[client].pendingReqs.isEmpty())
		{
			QPendingReq pendingReq = m_hashClients[client].pendingReqs.dequeue();
			pendingReq.reject(0, QUserEntryInfo());
		}
		// trigger remove and delete by disconnecting
		client->close();
	}
	// clear users
	m_mapUsers.clear();
	// return flush file name
	return strFileName;
}

bool QListServer::isRunning() const
{
	return m_wsServer->isListening();
}

quint16 QListServer::port() const
{
	return m_serverPort;
}

void QListServer::setPort(const quint16 & port)
{
	m_serverPort = port;
}

QString QListServer::flush() const
{
	QString strSaveFile = QString("users_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
	// save to file
	QFile file(strSaveFile);
	if (file.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		// write
		QTextStream stream(&file);
		stream << this->csvUsers();
	}
	else
	{
		strSaveFile = tr("could not open file %1").arg(strSaveFile);
	}
	// close file
	file.close();
	// return file name
	return strSaveFile;
}

QString QListServer::csvClients() const
{
	QString csvClients;
	for (auto it = m_hashClients.begin(); it != m_hashClients.end(); ++it)
	{
		const QString name = it.value().clientName;
		csvClients += QString("%1\n")
			.arg(name);
	}
	return csvClients;
}

QString QListServer::csvUsers() const
{
	QString csvUsers;
	for (auto it = m_mapUsers.begin(); it != m_mapUsers.end(); ++it)
	{
		const quint32        id   = it.key();
		const QUserEntryInfo info = it.value();
		csvUsers += QString("%1, %2, %3\n")
			.arg(id)
			.arg(info.userName)
			.arg(info.clientName);
	}
	return csvUsers;
}

void QListServer::on_binaryMessageReceived(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	Q_CHECK_PTR(pClient);
	// ignore request if for some reason client has been removed from map
	// e.g. a request is still processing when a disconnection occurs
	if (!m_hashClients.contains(pClient))
	{
		return;
	}
	// parse request
	QListRequest req = QListRequest::FromByteArray(message);
	// handle request type
	QListRequestType reqType = req.type();
	switch (reqType)
	{
	case QListRequestType::GET_NAME:
		this->getName(req, pClient);
		break;
	case QListRequestType::GET_ALL_USERS:
		this->getAllUsers(req, pClient);
		break;
	case QListRequestType::GET_NEW_USER:
		this->getNewUser(req, pClient);
		break;
	case QListRequestType::INSERT_USER:
		this->insertUser(req, pClient);
		break;
	default:
		break;
	}
	// restart timer
	if (m_timerTimeout.isActive())
	{
		m_timerTimeout.stop();
		m_timerTimeout.start();
	}
}

void QListServer::on_disconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	Q_CHECK_PTR(pClient);
	Q_ASSERT(m_hashClients.contains(pClient));
	m_hashClients.remove(pClient);
    pClient->deleteLater();
}

void QListServer::getName(const QListRequest & req, QWebSocket * pClient)
{
	//qDebug() << "[GET_NAME] " << req;
	Q_ASSERT(m_hashClients.contains(pClient));
	// form response
	QListResponse res;
	res.setRequestId(req.id())
		.setClientName(m_hashClients[pClient].clientName)
		.setResult(QListResponseResult::SUCCESS);
	// send response
	pClient->sendBinaryMessage(res.toByteArray());
}

void QListServer::getAllUsers(const QListRequest & req, QWebSocket * pClient)
{
	//qDebug() << "[GET_ALL_USERS] " << req;
	Q_ASSERT(m_hashClients.contains(pClient));
	// form response
	QListResponse res;
	res.setRequestId(req.id())
		.setResult(QListResponseResult::SUCCESS);
	// loop all users
	for (auto it = m_mapUsers.begin(); it != m_mapUsers.end(); ++it)
	{
		const quint32        id   = it.key();
		const QUserEntryInfo info = it.value();
		res.appendMutableElement()
			.setId(id)
			.setName(info.userName)
			.setClientName(info.clientName);
	}
	// send response
	pClient->sendBinaryMessage(res.toByteArray());
}

void QListServer::getNewUser(const QListRequest & req, QWebSocket * pClient)
{
	//qDebug() << "[GET_NEW_USER] " << req;
	Q_ASSERT(m_hashClients.contains(pClient));
	// get data from request
	quint32 reqId = req.id();
	// store in pending request
	QPendingReq pendingReq;
	pendingReq
	.fail([reqId, pClient](const quint32 &userId, const QUserEntryInfo &userInfo) {
		Q_UNUSED(userId);
		Q_UNUSED(userInfo);
		Q_CHECK_PTR(pClient);
		// form response
		QListResponse res;
		res.setRequestId(reqId)
			.setResult(QListResponseResult::REJECTED);
		// send response
		pClient->sendBinaryMessage(res.toByteArray());
	})
	.done([reqId, pClient](const quint32 &userId, const QUserEntryInfo &userInfo) {
		Q_CHECK_PTR(pClient);
		// form response
		QListResponse res;
		res.setRequestId(reqId)
			.setResult(QListResponseResult::SUCCESS);
		res.appendMutableElement()
			.setId(userId)
			.setName(userInfo.userName)
			.setClientName(userInfo.clientName);
		// send response
		pClient->sendBinaryMessage(res.toByteArray());
	});
	m_hashClients[pClient].pendingReqs << pendingReq;
}

void QListServer::insertUser(const QListRequest & req, QWebSocket * pClient)
{
	//qDebug() << "[INSERT_USER] " << req;
	Q_ASSERT(m_hashClients.contains(pClient));
	// get data from request
	QListElement elem     = req.mutableElement();
	quint32      userId   = elem.id();
	QString      userName = elem.name();
	// form response
	QListResponse res;
	res.setRequestId(req.id());
	// check if have user id
	if (m_mapUsers.contains(userId))
	{
		res.setResult(QListResponseResult::REJECTED);
	}
	else
	{
		res.setResult(QListResponseResult::SUCCESS);
		// insert
		QUserEntryInfo userInfo = {
			userName,
			m_hashClients[pClient].clientName
		};
		m_mapUsers[userId] = userInfo;
		// check if there are any pending requests for any client
		auto clients = m_hashClients.keys();
		for (auto client : clients)
		{
			if (!m_hashClients[client].pendingReqs.isEmpty())
			{
				QPendingReq pendingReq = m_hashClients[client].pendingReqs.dequeue();
				pendingReq.resolve(userId, userInfo);
			}
		}
	}
	// send response
	pClient->sendBinaryMessage(res.toByteArray());
}
