#include "qlistresponse.h"
#include "qlistproto.pb.h"

#include <QDebug>


QListResponseData::QListResponseData() :
	res(new ListResponse)
{
	//qDebug() << "QListResponseData : Data constructor (default)";
}

QListResponseData::QListResponseData(const QListResponseData & other) :
	QSharedData(other),
	res(new ListResponse(*other.res))
{
	//qDebug() << "QListResponseData : Data constructor (copy)";
}

QListResponseData::~QListResponseData()
{
	//qDebug() << "QListRequestData : Data destructor";
	delete res;
}

// ------------------------------------------------------------------

QListResponse::QListResponse() :
	data(new QListResponseData)
{

}

QListResponse::QListResponse(const QListResponse & other) :
	data(other.data)
{

}

QListResponse QListResponse::FromByteArray(const QByteArray & bytes)
{
	QListResponse newRes;
	newRes.fromByteArray(bytes);
	return newRes;
}

quint32 QListResponse::requestId() const
{
	return data->res->request_id();
}

QListResponse & QListResponse::setRequestId(const quint32 & id)
{
	data->res->set_request_id(id);
	return *this;
}

QListResponseResult QListResponse::result() const
{
	return static_cast<QListResponseResult>(data->res->result_info());
}

QListResponse & QListResponse::setResult(const QListResponseResult & result)
{
	data->res->set_result_info(static_cast<ListResponse::Result>(result));
	return *this;
}

QString QListResponse::clientName() const
{
	return QString::fromStdString(data->res->client_name());
}

QListResponse & QListResponse::setClientName(const QString & clientName)
{
	data->res->set_client_name(clientName.toStdString());
	return *this;
}

quint32 QListResponse::elementCount() const
{
	return static_cast<quint32>(data->res->elements_size());
}

QListElement QListResponse::appendMutableElement()
{
	ListElement * elem = data->res->add_elements();
	return QListElement(elem);
}

QListElement QListResponse::mutableElementAt(const quint32 & index)
{
	Q_ASSERT_X(index < this->elementCount(), "QListResponse::mutableElementAt", "Index out pf range.");
	if (index >= this->elementCount())
	{
		return QListElement();
	}
	ListElement * elem = data->res->mutable_elements(static_cast<int>(index));
	return QListElement(elem);
}

void QListResponse::removeElementAt(const quint32 & index)
{
	Q_ASSERT_X(index < this->elementCount(), "QListResponse::removeElementAt", "Index out pf range.");
	if (index >= this->elementCount())
	{
		return;
	}
	auto elems = data->res->mutable_elements();
	elems->erase(elems->begin() + index);
}

void QListResponse::clearElements()
{
	data->res->clear_elements();
}

QByteArray QListResponse::toByteArray() const
{
	int bSize = data->res->ByteSize();
	QByteArray bytes;
	bytes.resize(bSize);
	data->res->SerializeToArray(bytes.data(), bytes.size());
	return bytes;
}

void QListResponse::fromByteArray(const QByteArray & bytes)
{
	data->res->ParseFromArray(bytes.data(), bytes.size());
}

QString QListResponse::debugString() const
{
	return QString::fromStdString(data->res->DebugString());
}

QDebug operator<<(QDebug debug, const QListResponse & response)
{
	QDebugStateSaver saver(debug);
	debug.nospace() << response.debugString();
	return debug;
}
