#include <QCoreApplication>
#include <QDebug>

#include <QConsoleListener>
#include <QListServer>

void printMenu()
{
	qInfo() << "server commands";
	qInfo() << "menu         : prints this menu";
	qInfo() << "status       : prints the server status";
	qInfo() << "start        : starts the server";
	qInfo() << "stop         : stops the server and flushes the list to disk before cleaning it";
	qInfo() << "port=<value> : sets the port to the given value (only if server stopped)";
	qInfo() << "flush        : flushes the current state of the list to disk";
	qInfo() << "quit         : quits the server application";
}

void printServerStatus(const QListServer &server)
{
	qInfo() << "running :" << server.isRunning();
	qInfo() << "port    :" << server.port();
	QString clients = server.csvClients();
	if (!clients.isEmpty())
	{
		qInfo() << "clients :";
		qInfo().noquote() << clients;
	}
	else
	{
		qInfo() << "clients : none";
	}
	QString users = server.csvUsers();
	if (!users.isEmpty())
	{
		qInfo() << "users   :";
		qInfo().noquote() << users;
	}
	else
	{
		qInfo() << "users   : none";
	}
}

void startServer(QListServer &server)
{
	bool success = server.start(true);
	if (!success)
	{
		qCritical() << "ERROR : failed to start server";
	}
	else
	{
		qInfo() << "server started on port " << server.port();
		qInfo() << "server will auto-stop if there is no activity within one minute";
	}
}

void setServerPort(QListServer &server, const QString &strLine)
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
	if (server.isRunning())
	{
		qInfo() << "ERROR : cannot set port while server is running, please stop first";
		return;
	}
	// set port
	server.setPort(iPort);
	qInfo() << "server port set to " << iPort;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QConsoleListener console;
	QListServer server;

	// print message if auto-stopped
	QObject::connect(&server, &QListServer::autoStopped, 
	[](const QString &strFileName) {
		qInfo() << "server suto-stopped due to inactivity, flushed list to file" << strFileName;
	});

	// listen to console input
	QObject::connect(&console, &QConsoleListener::newLine, 
	[&a, &server](const QString &strNewLine) {
		QString strLine = strNewLine.trimmed();
		// menu
		if (strLine.compare("menu", Qt::CaseInsensitive) == 0)
		{
			printMenu();
		}
		else if (strLine.compare("status", Qt::CaseInsensitive) == 0)
		{
			printServerStatus(server);
		}
		else if (strLine.compare("start", Qt::CaseInsensitive) == 0)
		{
			startServer(server);
		}
		else if (strLine.compare("stop", Qt::CaseInsensitive) == 0)
		{
			QString fileName = server.stop();
			qInfo() << "server stopped, flushed list to file" << fileName;
		}
		else if (strLine.startsWith("port", Qt::CaseInsensitive))
		{
			setServerPort(server, strLine);
		}
		else if (strLine.compare("flush", Qt::CaseInsensitive) == 0)
		{
			QString fileName = server.flush();
			qInfo() << "server flushed list to file" << fileName;
		}
		else if (strLine.compare("quit", Qt::CaseInsensitive) == 0)
		{
			server.stop();
			qInfo() << "goodbye";
			a.quit();
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
