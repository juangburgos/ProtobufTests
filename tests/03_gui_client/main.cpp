#include "qlistclientwidget.h"
#include <QApplication>
#include <QDebug>

#include <QFile>
#include <QTextStream>
#include <QSharedPointer>
#include <QFileSystemWatcher>
typedef QSharedPointer<QFileSystemWatcher> QWatcherPtr;

void setStyleSheet(QApplication &a, const QString &strPath, const bool &subscribe = false)
{
	// set stylesheet
	QFile f(strPath);
	if (!f.exists())
	{
		qDebug() << "[ERROR] Unable to set stylesheet," << strPath << "file not found.";
	}
	else
	{
		// set stylesheet
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		a.setStyleSheet(ts.readAll());
		f.close();
		// subscribe to changes (only once)
		if (!subscribe)
		{
			return;
		}
		QWatcherPtr watcher = QWatcherPtr(new QFileSystemWatcher);
		watcher->addPath(strPath);
		QObject::connect(watcher.data(), &QFileSystemWatcher::fileChanged, &a,
			[&a, watcher, strPath]()
		{
			setStyleSheet(a, strPath, false);
		});
	}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// set stylesheet and subscribe to changes
	//setStyleSheet(a, "./qlistclientwidget.qss", true);
	setStyleSheet(a, ":qlistclientwidget.qss", true);

    QListClientWidget w;
    w.show();
	w.showFullScreen();

    return a.exec();
}
