#ifndef QLISTRESPONSE_H
#define QLISTRESPONSE_H

#include <QSharedData>
#include <QByteArray>
#include <QString>

#include <QListElement>

QT_FORWARD_DECLARE_CLASS(ListResponse)

class QListResponseData : public QSharedData
{
	friend class QListResponse;
public:
	QListResponseData();
	QListResponseData(const QListResponseData &other);
	~QListResponseData();

private:
	ListResponse * res;
};

typedef QSharedDataPointer<QListResponseData> QListResponseDataPtr;

class QListResponse
{
public:
    QListResponse();
	QListResponse(const QListResponse &other);

	enum Result {
		NULL_RESULT = 0,
		SUCCESS     = 1,
		REJECTED    = 2
	};
	typedef QListResponse::Result QListResponseResult;

	static QListResponse FromByteArray(const QByteArray &bytes);

	quint32              requestId() const;
	QListResponse &      setRequestId(const quint32 &id);
						 
	QListResponseResult  result() const;
	QListResponse &      setResult(const QListResponseResult &result);

	QString              clientName() const;
	QListResponse &      setClientName(const QString &clientName);
						 
	quint32              elementCount() const;
	QListElement         appendMutableElement();
	QListElement         mutableElementAt(const quint32 &index);
	void                 removeElementAt(const quint32 &index);
	void                 clearElements();
						 
	QByteArray           toByteArray() const;
	void                 fromByteArray(const QByteArray &bytes);
						 
	QString              debugString() const;

private:
	QListResponseDataPtr data;
};

typedef QListResponse::Result QListResponseResult;

QDebug operator<<(QDebug debug, const QListResponse &response);

#endif // QLISTRESPONSE_H