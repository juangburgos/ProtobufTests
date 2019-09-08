#include "qlistelement.h"
#include "qlistproto.pb.h"

#include <QDebug>

QListElementData::QListElementData() :
	elem(new ListElement)
{
	is_owned = true;
	//qDebug() << "QListElementData : Data constructor (default)";
}

QListElementData::QListElementData(ListElement * raw_elem) :
	elem(raw_elem)
{
	is_owned = false;
	//qDebug() << "QListElementData : Data constructor (raw)";
}

QListElementData::QListElementData(const QListElementData &other) :
	QSharedData(other),
	elem(new ListElement(*other.elem))
{
	is_owned = true;
	//qDebug() << "QListElementData : Data constructor (copy)";
}

QListElementData::~QListElementData()
{
	if (!is_owned)
	{
		//qDebug() << "QListElementData : Data destructor (raw)";
		return;
	}
	//qDebug() << "QListElementData : Data destructor (delete)";
	delete elem;
}

// ------------------------------------------------------------------

typedef QSharedDataPointer<QListElementData> QListElementPtr;

QListElement::QListElement() :
	data(new QListElementData)
{
	
}

QListElement::QListElement(const QListElement & other) :
	data(other.data)
{

}

QListElement::QListElement(ListElement * raw_elem) :
	data(new QListElementData(raw_elem))
{
	Q_CHECK_PTR(data->elem);
}

QListElement QListElement::FromByteArray(const QByteArray & bytes)
{
	QListElement newElem;
	newElem.fromByteArray(bytes);
	return newElem;
}

quint32 QListElement::id() const
{
	return data->elem->elem_id();
}

QListElement& QListElement::setId(const quint32 & id)
{
	data->elem->set_elem_id(id);
	return *this;
}

QString QListElement::name() const
{
	return QString::fromStdString(data->elem->elem_name());
}

QListElement &  QListElement::setName(const QString & name)
{
	data->elem->set_elem_name(name.toStdString());
	return *this;
}

QString QListElement::clientName() const
{
	return QString::fromStdString(data->elem->client_name());
}

QListElement & QListElement::setClientName(const QString & clientName)
{
	data->elem->set_client_name(clientName.toStdString());
	return *this;
}

QByteArray QListElement::toByteArray() const
{
	int bSize = data->elem->ByteSize();
	QByteArray bytes;
	bytes.resize(bSize);
	data->elem->SerializeToArray(bytes.data(), bytes.size());
	return bytes;
}

void QListElement::fromByteArray(const QByteArray & bytes)
{
	data->elem->ParseFromArray(bytes.data(), bytes.size());
}

QListElement QListElement::clone() const
{
	QListElement elemClone;
	*elemClone.data->elem = *this->data->elem;
	return elemClone;
}

QString QListElement::debugString() const
{
	return QString::fromStdString(data->elem->DebugString());
}

QDebug operator<<(QDebug debug, const QListElement & element)
{
	QDebugStateSaver saver(debug);
	debug.nospace() << element.debugString();
	return debug;
}
