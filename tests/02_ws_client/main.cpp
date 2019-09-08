#include <QCoreApplication>
#include <QVector>
#include <QTimer>
#include <QDebug>

#include <QConsoleListener>
#include <QListClient>

#include <algorithm>
#include <ctime>  
#include <cstdlib>

void printMenu()
{
	qInfo() << "client commands";
	qInfo() << "menu               : prints this menu";
	qInfo() << "status             : prints the client status";
	qInfo() << "start              : starts the client";
	qInfo() << "stop               : stops the client and resets the client list";
	qInfo() << "ip=<value>         : sets the ip address to <value> (only if client stopped)";
	qInfo() << "port=<value>       : sets the port to <value> (only if client stopped)";
	qInfo() << "name               : requests the client name";
	qInfo() << "insert=<id>,<name> : requests to insert a user in the server with userId <id> and userName <name>";
	qInfo() << "users              : requests all users currently available in the server";
	qInfo() << "new                : requests the server to notify of a new user as soon as it is available";
	qInfo() << "routine            : starts inserting users from 1 to 250 every second, stops when finished";
	qInfo() << "quit               : quits the client application";
}

void printClientStatus(const QListClient &client)
{
	qInfo() << "running :" << client.isRunning();
	qInfo() << "port    :" << client.port();
}

void startClient(QListClient &client)
{
	client.start()
	.fail([](const QString &strError) {
		qInfo() << "ERROR client failed to connect, " << strError;
	})
	.done([](const QString &strSuccess) {
		Q_UNUSED(strSuccess);
		qInfo() << "client connected";
	});
}

void stopClient(QListClient &client)
{
	client.stop()
	.fail([](const QString &strError) {
		qInfo() << "ERROR client failed to disconnect, " << strError;
	})
	.done([](const QString &strSuccess) {
		Q_UNUSED(strSuccess);
		qInfo() << "client disconnected";
	});
}

void setClientIp(QListClient &client, const QString &strLine)
{
	QStringList strParts = strLine.split("=");
	if (strParts.count() < 2)
	{
		qInfo() << "ERROR : invalid ip value";
		return;
	}
	QString strIp = strParts.at(1).trimmed();
	// check if running
	if (client.isRunning())
	{
		qInfo() << "ERROR : cannot set port while client is running, please stop first";
		return;
	}
	// set ip address
	client.setIpAddress(strIp);
	qInfo() << "client ip address set to " << strIp;
}

void setClientPort(QListClient &client, const QString &strLine)
{
	QStringList strParts = strLine.split("=");
	if (strParts.count() < 2)
	{
		qWarning() << "ERROR : invalid port value"; qWarning() << "";
		return;
	}
	QString strPort = strParts.at(1).trimmed();
	bool    bOk     = false;
	quint32 iPort   = strPort.toUInt(&bOk);
	if (!bOk)
	{
		qInfo() << "ERROR : invalid port value <" << strPort << ">";
		return;
	}
	// check if running
	if (client.isRunning())
	{
		qInfo() << "ERROR : cannot set port while client is running, please stop first";
		return;
	}
	// set port
	client.setPort(iPort);
	qInfo() << "client port set to " << iPort;
}

void getClientName(QListClient &client)
{
	client.getName()
	.fail([](const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		qCritical() << "ERROR get client name request failed, " << strError;
	})
	.done([](const QString &strName, const qint64 &msElapsed) {
		qInfo() << "server gave client name =" << strName;
		qInfo() << "request time =" << msElapsed << "msec";
	});
}

void insertUser(QListClient &client, const QString &strLine)
{
	QStringList strParts = strLine.split("=");
	if (strParts.count() < 2)
	{
		qWarning() << "ERROR : invalid insert command format"; qWarning() << "";
		return;
	}
	QStringList strUserInfo = strParts.at(1).trimmed().split(",");
	if (strUserInfo.count() < 2)
	{
		qWarning() << "ERROR : invalid insert command format"; qWarning() << "";
		return;
	}
	QString strUserId = strUserInfo.at(0).trimmed();
	bool    bOk       = false;
	quint32 userId    = strUserId.toUInt(&bOk);
	if (!bOk)
	{
		qInfo() << "ERROR : invalid userId value <" << strUserId << ">"; qInfo() << "";
		return;
	}
	QString strUserName = strUserInfo.at(1).trimmed();
	if (strUserName.isEmpty())
	{
		qInfo() << "ERROR : invalid userName, cannot be empty"; qInfo() << "";
		return;
	}
	client.insertUser(userId, strUserName)
	.fail([userId, strUserName](const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		qCritical() << "ERROR insert user = " << userId << "," << strUserName << "failed, " << strError;
	})
	.done([userId, strUserName](const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		qInfo() << "success inserting user in server =" << userId << "," << strUserName;
		qInfo() << "request time =" << msElapsed << "msec";
	});
}

void getAllUsers(QListClient &client)
{
	client.getAllUsers()
	.fail([](const QUserMap &usersMap, const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(usersMap);
		Q_UNUSED(msElapsed);
		qCritical() << "ERROR get all users request failed, " << strError;
	})
	.done([](const QUserMap &usersMap, const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		QString strUsers;
		for (auto it = usersMap.begin(); it != usersMap.end(); ++it)
		{
			const quint32        id   = it.key();
			const QUserEntryInfo info = it.value();
			strUsers += QString("%1, %2, %3\n")
				.arg(id)
				.arg(info.userName)
				.arg(info.clientName);
		}
		qInfo() << "server users :";
		qInfo().noquote() << strUsers;
		qInfo() << "request time =" << msElapsed << "msec";
	}, Qt::QueuedConnection);
}

void getNewUser(QListClient &client)
{
	client.getNewUser()
	.fail([](const QUserMap &usersMap, const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(usersMap);
		Q_UNUSED(msElapsed);
		qCritical() << "ERROR get new user request failed, " << strError;
	})
	.done([](const QUserMap &usersMap, const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		Q_ASSERT(usersMap.count() == 1);
		const quint32        id   = usersMap.firstKey();
		const QUserEntryInfo info = usersMap.first();
		QString strUser = QString("%1, %2, %3\n")
			.arg(id)
			.arg(info.userName)
			.arg(info.clientName);
		qInfo() << "server new user :";
		qInfo().noquote() << strUser;
		qInfo() << "request time =" << msElapsed << "msec";
	});
}

const quint32 maxNumOfUsers = 250;
QVector<quint32> listUserNumbers;
void runRoutine(QListClient &client);

void startRoutine(QListClient &client)
{
	// check if already started
	if (!listUserNumbers.isEmpty())
	{
		qInfo() << "ERROR routine already started, missing" << listUserNumbers.count() << "users to insert";
		return;
	}
	std::srand(unsigned(std::time(0)));
	// create array 1 to 250
	quint32 userNum = 0;
	listUserNumbers.resize(maxNumOfUsers);
	std::generate(listUserNumbers.begin(), listUserNumbers.end(),
	[&userNum]() {
		return ++userNum;
	});
	// make random
	std::random_shuffle(listUserNumbers.begin(), listUserNumbers.end(),
	[](quint32 i) {
		return std::rand() % i;
	});
	// actually start the routine
	runRoutine(client);
}

void runRoutine(QListClient &client)
{
	// check if already finished
	if (listUserNumbers.isEmpty())
	{
		qInfo() << "finished routine, all 250 user inserts have been issued";
		stopClient(client);
		return;
	}
	// insert
	quint32 userId      = listUserNumbers.takeFirst();
	QString strUserName = QString("user_%1").arg(userId);
	client.insertUser(userId, strUserName)
	.fail([&client, userId, strUserName](const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		qCritical() << "ERROR insert user = " << userId << "," << strUserName << "failed," << strError;
		if (strError.contains("rejected", Qt::CaseInsensitive))
		{
			qInfo() << "missing" << listUserNumbers.count() << "users to insert";
			// again after a second
			QTimer::singleShot(1000,
				[&client]() {
				runRoutine(client);
			});
			return;
		}
		qCritical() << "ERROR insert 250 users routine failed, missed" << listUserNumbers.count();
		listUserNumbers.clear();
	})
	.done([&client, userId, strUserName](const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		qInfo() << "success inserting user in server =" << userId << "," << strUserName;
		qInfo() << "request time =" << msElapsed << "msec";
		qInfo() << "missing" << listUserNumbers.count() << "users to insert";
		// again after a second
		QTimer::singleShot(1000, 
		[&client]() {
			runRoutine(client);		
		});
	});
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QConsoleListener console;
	QListClient client;

	// listen to console input
	QObject::connect(&console, &QConsoleListener::newLine, 
	[&a, &client](const QString &strNewLine) {
		QString strLine = strNewLine.trimmed();
		// menu
		if (strLine.compare("menu", Qt::CaseInsensitive) == 0)
		{
			printMenu();
		}
		else if (strLine.compare("status", Qt::CaseInsensitive) == 0)
		{
			printClientStatus(client);
		}
		else if (strLine.compare("start", Qt::CaseInsensitive) == 0)
		{
			startClient(client);
		}
		else if (strLine.compare("stop", Qt::CaseInsensitive) == 0)
		{
			stopClient(client);
		}
		else if (strLine.startsWith("ip", Qt::CaseInsensitive))
		{
			setClientIp(client, strLine);
		}
		else if (strLine.startsWith("port", Qt::CaseInsensitive))
		{
			setClientPort(client, strLine);
		}
		else if (strLine.compare("name", Qt::CaseInsensitive) == 0)
		{
			getClientName(client);
		}
		else if (strLine.startsWith("insert", Qt::CaseInsensitive))
		{
			insertUser(client, strLine);
		}
		else if (strLine.compare("users", Qt::CaseInsensitive) == 0)
		{
			getAllUsers(client);
		}
		else if (strLine.compare("new", Qt::CaseInsensitive) == 0)
		{
			getNewUser(client);
		}
		else if (strLine.compare("routine", Qt::CaseInsensitive) == 0)
		{
			startRoutine(client);
		}
		else if (strLine.compare("quit", Qt::CaseInsensitive) == 0)
		{
			client.stop()
			.fail([&a](const QString &strError) {
				qInfo() << "ERROR client failed to disconnect, " << strError;
				qInfo() << "goodbye";
				a.quit();
			})
			.done([&a](const QString &strSuccess) {
				Q_UNUSED(strSuccess);
				qInfo() << "goodbye";
				a.quit();
			});
		}
		else
		{
			qInfo() << "ERROR : unknown command <" << strLine << ">";
		}
		// extra line at the end
		qInfo() << "";
	});

	// print menu initially
	printMenu();
	qInfo() << "";

    return a.exec();
}
