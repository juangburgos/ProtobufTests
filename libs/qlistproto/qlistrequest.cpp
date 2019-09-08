#include "qlistrequest.h"
#include "qlistproto.pb.h"

#include <QDebug>

QListRequestData::QListRequestData() :
	req(new ListRequest)
{
	//qDebug() << "QListRequestData : Data constructor (default)";
}

QListRequestData::QListRequestData(const QListRequestData & other) :
	QSharedData(other),
	req(new ListRequest(*other.req))
{
	//qDebug() << "QListRequestData : Data constructor (copy)";
}

QListRequestData::~QListRequestData()
{
	//qDebug() << "QListRequestData : Data destructor";
	delete req;
}

// ------------------------------------------------------------------

QListRequest::QListRequest() :
	data(new QListRequestData)
{

}

QListRequest::QListRequest(const QListRequest & other) :
	data(other.data)
{

}

QListRequest QListRequest::FromByteArray(const QByteArray & bytes)
{
	QListRequest newReq;
	newReq.fromByteArray(bytes);
	return newReq;
}

quint32 QListRequest::id() const
{
	return data->req->request_id();
}

QListRequest & QListRequest::setId(const quint32 & id)
{
	data->req->set_request_id(id);
	return *this;
}

QListRequestType QListRequest::type() const
{
	return static_cast<QListRequestType>(data->req->request_type());
}

QListRequest & QListRequest::setType(const QListRequestType & type)
{
	data->req->set_request_type(static_cast<ListRequest::Type>(type));
	return *this;
}

QListElement QListRequest::mutableElement() const
{
	return QListElement(data->req->mutable_element());
}

void QListRequest::copyElementData(const QListElement & other)
{
	*data->req->mutable_element() = *other.data->elem;
}

void QListRequest::clearElement()
{
	data->req->clear_element();
}

QByteArray QListRequest::toByteArray() const
{
	int bSize = data->req->ByteSize();
	QByteArray bytes;
	bytes.resize(bSize);
	data->req->SerializeToArray(bytes.data(), bytes.size());
	return bytes;
}

void QListRequest::fromByteArray(const QByteArray & bytes)
{
	data->req->ParseFromArray(bytes.data(), bytes.size());
}

QString QListRequest::debugString() const
{
	return QString::fromStdString(data->req->DebugString());
}

QDebug operator<<(QDebug debug, const QListRequest & request)
{
	QDebugStateSaver saver(debug);
	debug.nospace() << request.debugString();
	return debug;
}
