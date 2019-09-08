#ifndef QLISTELEMENT_H
#define QLISTELEMENT_H

#include <QSharedData>
#include <QByteArray>
#include <QString>

QT_FORWARD_DECLARE_CLASS(ListElement)
QT_FORWARD_DECLARE_CLASS(QListRequest)
QT_FORWARD_DECLARE_CLASS(QListResponse)

class QListElementData : public QSharedData
{
	friend class QListElement;
	friend class QListRequest;
public:
	QListElementData();
	QListElementData(ListElement * raw_elem);
	QListElementData(const QListElementData &other);
	~QListElementData();

private:
	ListElement * elem;
	bool is_owned;
};

typedef QSharedDataPointer<QListElementData> QListElementDataPtr;

class QListElement
{
	friend class QListRequest;
	friend class QListResponse;
public:
    QListElement();
	QListElement(const QListElement &other);

	static QListElement FromByteArray(const QByteArray &bytes);

	quint32        id() const;
	QListElement & setId(const quint32 &id);

	QString        name() const;
	QListElement & setName(const QString &name);

	QString        clientName() const;
	QListElement & setClientName(const QString &clientName);

	QByteArray     toByteArray() const;
	void           fromByteArray(const QByteArray &bytes);

	QListElement   clone() const;
			       
	QString        debugString() const;

private:
	QListElement(ListElement * raw_elem);

private:
	QListElementDataPtr data;
};

QDebug operator<<(QDebug debug, const QListElement &element);

#endif // QLISTELEMENT_H