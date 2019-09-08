#ifndef QLISTCLIENTWIDGET_H
#define QLISTCLIENTWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include <QListClient>

namespace Ui {
class QListClientWidget;
}

class QListClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QListClientWidget(QWidget *parent = nullptr);
    ~QListClientWidget();

	// table headers
	enum class Headers
	{
		UserId     = 0,
		UserName   = 1,
		ClientName = 2,
		Invalid    = 3
	};
	Q_ENUM(Headers)

private slots:
    void on_pushButtonConnect_clicked();

    void on_pushButtonInsert_clicked();

private:
    Ui::QListClientWidget *ui;
	QListClient           m_client;
	QStandardItemModel    m_modelUsers;
	QSortFilterProxyModel m_proxyUsers;
	quint32               m_pendingNewUsers;
	static const quint32 maxPendingNewUsers;

	void clearConnectedWidgets();
	void setEnabledConnectedWidgets(const bool &enabled);
	void checkDisconnected();
	void insertUser(const quint32 &userId, const QUserEntryInfo &userInfo);
	void keepSubscriptionAlive();

	static QString checkValidServerAddress(
		const QString &strAddress,
	    quint16 &iPort,
		QString &ipAddress
	);
};

#endif // QLISTCLIENTWIDGET_H
