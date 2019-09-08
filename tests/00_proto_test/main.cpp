#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QByteArray>

#include <QListElement>
#include <QListRequest>
#include <QListResponse>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// --------------------------------------------
	qDebug() << "";

	QListElement elem;
	elem.setId(77)
		.setName("juan");
	QByteArray bytesElem = elem.toByteArray();
	qDebug() << elem;
	qDebug() << bytesElem;

	QListElement elemCopy = elem;
	qDebug() << elemCopy;

	QListElement elemEdit = elem;
	elemEdit.setId(1);
	qDebug() << elemEdit;

	QListElement elemStatic = QListElement::FromByteArray(bytesElem);
	elemStatic.setName("juan burgos");
	qDebug() << elemStatic;

	// --------------------------------------------
	qDebug() << "";

	QListRequest req;
	req.setId(1);
	req.setType(QListRequestType::GET_NAME);
	QByteArray bytesReq = req.toByteArray();
	qDebug() << req;
	qDebug() << bytesReq;

	QListRequest reqCopy = req;
	qDebug() << reqCopy;

	QListRequest reqEdit = req;
	reqEdit.setId(2);
	reqEdit.setType(QListRequestType::INSERT_USER);
	qDebug() << reqEdit;

	QListRequest reqStatic = QListRequest::FromByteArray(bytesReq);
	reqStatic.setId(666);
	QListElement elemNested = reqStatic.mutableElement();
	elemNested.setName("chuchito perez");
	qDebug() << reqStatic;

	// NOTE : weirdly succeeds
	QListRequest reqError = QListRequest::FromByteArray(bytesElem);
	qDebug() << reqError;

	// --------------------------------------------
	qDebug() << "";

	QListResponse res;
	res.setRequestId(11);
	res.setResult(QListResponseResult::REJECTED);
	QListElement elemRes1 = res.appendMutableElement();
	elemRes1.setId(88);
	elemRes1.setName("Carlos");
	QByteArray bytesRes = res.toByteArray();
	qDebug() << res;
	qDebug() << bytesRes;

	QListResponse resCopy = res;
	qDebug() << resCopy;

	QListResponse resEdit = res;
	resEdit.setResult(QListResponseResult::SUCCESS);
	QListElement elemRes2 = resEdit.appendMutableElement();
	elemRes2.setId(13);
	elemRes2.setName("Arturo");
	qDebug() << resEdit;

	// NOTE : if we don't clone miguel, the accessing it after *removeElementAt* results in crash
	QListElement miguelClone = resEdit.appendMutableElement()
		.setId(21)
		.setName("Miguel")
		.clone();
	resEdit.appendMutableElement()
		.setId(22)
		.setName("Luis");
	QByteArray bytesResEdit = resEdit.toByteArray();
	qDebug() << resEdit;
	qDebug() << bytesResEdit;

	resEdit.removeElementAt(2);
	qDebug() << resEdit;
	qDebug() << miguelClone;

	QListResponse resStatic = QListResponse::FromByteArray(bytesResEdit);
	resStatic.mutableElementAt(0).setName("Juan Carlos");
	qDebug() << resStatic;

	return a.exec(); 
}
