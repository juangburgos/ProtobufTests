#ifndef QLISTREQUEST_H
#define QLISTREQUEST_H

#include <QSharedData>
#include <QByteArray>
#include <QString>

#include <QListElement>

QT_FORWARD_DECLARE_CLASS(ListRequest)

class QListRequestData : public QSharedData
{
	friend class QListRequest;
public:
	QListRequestData();
	QListRequestData(const QListRequestData &other);
	~QListRequestData();

private:
	ListRequest * req;
};

typedef QSharedDataPointer<QListRequestData> QListRequestDataPtr;

class QListRequest
{
public:
    QListRequest();
	QListRequest(const QListRequest &other);

	enum Type {
		NULL_TYPE     = 0,
		GET_NAME      = 1,
		GET_ALL_USERS = 2,
		GET_NEW_USER  = 3,
		INSERT_USER   = 4
	};
	typedef QListRequest::Type QListRequestType;

	static QListRequest FromByteArray(const QByteArray &bytes);

	quint32          id() const;
	QListRequest &   setId(const quint32 &id);

	QListRequestType type() const;
	QListRequest &   setType(const QListRequestType &type);

	QListElement     mutableElement() const;
	void             copyElementData(const QListElement &other);
	void             clearElement();

	QByteArray       toByteArray() const;
	void             fromByteArray(const QByteArray &bytes);

	QString          debugString() const;

private:
	QListRequestDataPtr data;
};

typedef QListRequest::Type QListRequestType;

QDebug operator<<(QDebug debug, const QListRequest &request);

#endif // QLISTREQUEST_H