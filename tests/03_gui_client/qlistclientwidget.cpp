#include "qlistclientwidget.h"
#include "ui_qlistclientwidget.h"

#include <QMetaEnum>
#include <QMessageBox>

const quint32 QListClientWidget::maxPendingNewUsers = 5;

QListClientWidget::QListClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QListClientWidget)
{
    ui->setupUi(this);
	m_pendingNewUsers = 0;
	// setup params table model
	m_modelUsers.setColumnCount((int)Headers::Invalid);
	QStringList paramHeaders;
	for (int i = (int)Headers::UserId; i < (int)Headers::Invalid; i++)
	{
		paramHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelUsers.setHorizontalHeaderLabels(paramHeaders);
	// setup params sort filter
	m_proxyUsers.setSourceModel(&m_modelUsers);
	// setup params table
	ui->tableViewUsers->setModel(&m_proxyUsers);
	ui->tableViewUsers->setAlternatingRowColors(true);
	ui->tableViewUsers->horizontalHeader()->setStretchLastSection(true);
	ui->tableViewUsers->verticalHeader()->setVisible(false);
	ui->tableViewUsers->setSortingEnabled(true);
	ui->tableViewUsers->sortByColumn((int)Headers::UserId, Qt::SortOrder::AscendingOrder);
	ui->tableViewUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewUsers->setSelectionMode(QAbstractItemView::NoSelection);
	ui->tableViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// disable widgets until connected
	this->setEnabledConnectedWidgets(false);
	// subscribe to disconnection
	QObject::connect(&m_client, &QListClient::disconnected, this, &QListClientWidget::checkDisconnected);
}

QListClientWidget::~QListClientWidget()
{
    delete ui;
}

void QListClientWidget::on_pushButtonConnect_clicked()
{
	// check if connect or disconnect
	if (m_client.isRunning())
	{
		// try to stop
		m_client.stop();
		return;
	}
	// check valid server before anything
	QString strAddress = ui->lineEditServer->text().trimmed();
	quint16 iPort;
	QString ipAddress;
	QString strError = QListClientWidget::checkValidServerAddress(strAddress, iPort, ipAddress);
	if (!strError.isEmpty())
	{
		QMessageBox::critical(
			this,
			tr("Configuration Error"),
			tr("Invalid server address.\n\n%1.").arg(strError)
		);
		return;
	}
	// configure
	m_client.setIpAddress(ipAddress);
	m_client.setPort(iPort);
	// update ui
	ui->pushButtonConnect->setText(tr("Connecting"));
	ui->pushButtonConnect->setEnabled(false);
	ui->lineEditServer->setEnabled(false);
	// try to connect
	m_client.start()
	.fail([this](const QString &strError) {
		QMessageBox::critical(
			this,
			tr("Connection Error"),
			tr("Could not connect to server.\n\n%1").arg(strError)
		);
		// update ui
		ui->pushButtonConnect->setEnabled(true);
		this->checkDisconnected();
	})
	.then<QString, qint64>([this](const QString &strSuccess) {
		Q_UNUSED(strSuccess);
		// update ui
		ui->pushButtonConnect->setEnabled(true);
		ui->pushButtonConnect->setText(tr("Disconnect"));
		ui->lineEditStatus->setText(tr("Connected"));
		this->setEnabledConnectedWidgets(true);
		// ask for name
		return m_client.getName();
	})
	.fail([this](const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		QMessageBox::critical(
			this,
			tr("Request Error"),
			tr("Error Getting Client Name.\n\n%1").arg(strError)
		);
		// update ui
		this->checkDisconnected();
	})
	.then<QUserMap, QString, qint64>([this](const QString &strName, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		ui->lineEditClientName->setText(strName);
		// subscribe to new users
		this->keepSubscriptionAlive();
		// get all existing users
		return m_client.getAllUsers();
	})
	.fail([this](const QUserMap &usersMap, const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(usersMap);
		Q_UNUSED(msElapsed);
		QMessageBox::critical(
			this,
			tr("Request Error"),
			tr("Error Getting User List.\n\n%1").arg(strError)
		);
		// update ui
		this->checkDisconnected();
	})
	.done([this](const QUserMap &usersMap, const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		Q_UNUSED(msElapsed);
		// fill up table with existing users
		for (auto it = usersMap.begin(); it != usersMap.end(); ++it)
		{
			const quint32        id   = it.key();
			const QUserEntryInfo info = it.value();
			this->insertUser(id, info);
		}
	}, Qt::QueuedConnection);
}

void QListClientWidget::on_pushButtonInsert_clicked()
{
	Q_ASSERT(m_client.isRunning());
	quint32 userId      = ui->spinBoxUserId->value();
	QString strUserName = ui->lineEditUserName->text().trimmed();
	// check not empty before anything else
	if (strUserName.isEmpty())
	{
		QMessageBox::critical(
			this,
			tr("Error"),
			tr("User name cannot be empty.")
		);
		return;
	}
	// send insert request
	m_client.insertUser(userId, strUserName)
	.fail([this, userId, strUserName](const QString &strError, const qint64 &msElapsed) {
		Q_UNUSED(msElapsed);
		QMessageBox::critical(
			this,
			tr("Request Error"),
			tr("Error Inserting User %1 - %2.\n\n%3")
			.arg(userId)
			.arg(strUserName)
			.arg(strError)
		);
		// update ui
		this->checkDisconnected();
	})
	.done([this, userId, strUserName](const QString &strSuccess, const qint64 &msElapsed) {
		Q_UNUSED(strSuccess);
		QMessageBox::information(
			this,
			tr("Request Successfull"),
			tr("Success Inserting User %1 - %2.\n\nRequest took %3 msecs.")
			.arg(userId)
			.arg(strUserName)
			.arg(msElapsed)
		);
	});
}

void QListClientWidget::clearConnectedWidgets()
{
	// top
	ui->lineEditClientName->setText("");
	ui->lineEditUserName->setText("");
	ui->spinBoxUserId->setValue(0);
	// bottom
	ui->lineEditCount->setText("0");
	m_modelUsers.removeRows(0, m_modelUsers.rowCount());
}

void QListClientWidget::setEnabledConnectedWidgets(const bool & enabled)
{
	// top
	ui->lineEditClientName->setEnabled(enabled);
	ui->lineEditUserName->setEnabled(enabled);
	ui->spinBoxUserId->setEnabled(enabled);
	ui->pushButtonInsert->setEnabled(enabled);
	// bottom
	ui->lineEditCount->setEnabled(enabled);
	ui->tableViewUsers->setEnabled(enabled);
}

void QListClientWidget::checkDisconnected()
{
	// update ui
	if (!m_client.isRunning())
	{
		m_pendingNewUsers = 0;
		ui->pushButtonConnect->setEnabled(true);
		ui->lineEditServer->setEnabled(true);
		ui->pushButtonConnect->setText(tr("Connect"));
		ui->lineEditStatus->setText(tr("Disconnected"));
		this->setEnabledConnectedWidgets(false);
		this->clearConnectedWidgets();
	}
}

void QListClientWidget::insertUser(const quint32 & userId, const QUserEntryInfo & userInfo)
{
	// get parent to add rows as children
	auto parent = m_modelUsers.invisibleRootItem();
	auto row = parent->rowCount();

	// NOTE : it is much more performant to pre-allocate the entire row in advance,
	//        and calling parent->appendRow (calling parent->setChild is expensive)
	QList<QStandardItem*> listCols;
	std::generate_n(std::back_inserter(listCols), (int)Headers::Invalid,
		[]() {
		return new QStandardItem;
	});
	parent->appendRow(listCols);

	// user id column
	auto iUserId = parent->child(row, (int)Headers::UserId);
	iUserId->setText(QString("%1").arg(userId));
	// user name column
	auto iUserName = parent->child(row, (int)Headers::UserName);
	iUserName->setText(userInfo.userName);
	// client name column
	auto iClientName = parent->child(row, (int)Headers::ClientName);
	iClientName->setText(userInfo.clientName);

	// increase count
	ui->lineEditCount->setText(QString("%2").arg(parent->rowCount()));
}

void QListClientWidget::keepSubscriptionAlive()
{
	if (!m_client.isRunning())
	{
		return;
	}
	while (m_pendingNewUsers < QListClientWidget::maxPendingNewUsers)
	{
		// increase
		m_pendingNewUsers++;
		// request
		m_client.getNewUser()
		.fail([this](const QUserMap &usersMap, const QString &strError, const qint64 &msElapsed) {
			Q_UNUSED(usersMap);
			Q_UNUSED(strError);
			Q_UNUSED(msElapsed);
			if (m_client.isRunning())
			{
				// decrease
				m_pendingNewUsers--;
				// try again
				this->keepSubscriptionAlive();
				return;
			}
			this->checkDisconnected();
		})
		.done([this](const QUserMap &usersMap, const QString &strSuccess, const qint64 &msElapsed) {
			Q_UNUSED(strSuccess);
			Q_UNUSED(msElapsed);
			Q_ASSERT(usersMap.count() == 1);
			const quint32        id   = usersMap.firstKey();
			const QUserEntryInfo info = usersMap.first();
			this->insertUser(id, info);
			// decrease
			m_pendingNewUsers--;
			// try again
			this->keepSubscriptionAlive();
		});
	}
}

QString QListClientWidget::checkValidServerAddress(const QString & strAddress, quint16 & iPort, QString & ipAddress)
{
	if (strAddress.isEmpty())
	{
		return tr("Address cannot be empty");
	}
	QStringList strParts = strAddress.split(":");
	if (strParts.count() < 2)
	{
		return tr("Address must have the format 'ip_address:port'");
	}
	QString strPort = strParts.at(1).trimmed();
	bool    bOk     = false;
	iPort   = strPort.toUInt(&bOk);
	if (!bOk)
	{
		return tr("Invalid port value %1").arg(strPort);
	}
	ipAddress = strParts.at(0).trimmed();
	if (ipAddress.isEmpty())
	{
		return tr("Ip address cannot be empty");
	}
	return QString();
}

