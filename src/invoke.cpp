
#include "invoke.hpp"


using namespace bb::cascades;
using namespace bb::system;


namespace pronic { namespace prism { // NAMESPACE


const int Invokator::NEW = 0;
const int Invokator::REPLY = 1;
const int Invokator::FORWARD = 2;


Invokator::Invokator(Crypto *_crypto, Configurator *_config, Payment *_payment)
{
	crypto = _crypto;
	config = _config;
	payment = _payment;
	invokeManager = new InvokeManager(this);
}


InvokeManager* Invokator::getInvokeManager()
{
	return invokeManager;
}


void Invokator::closeCard()
{
	CardDoneMessage message;
	message.setData("OK");
	message.setDataType("text/plain");
	message.setReason("Success");
	invokeManager->sendCardDone(message);
//	Application::exit(0);
}


void Invokator::invokeFacebookPage(const QString &pageName)
{
	InvokeRequest request;
	request.setTarget("com.rim.bb.app.facebook");
	request.setAction("bb.action.OPEN");
	QVariantMap payload;
	payload["object_type"] = "page";
	payload["object_id"] = pageName;
	request.setMetadata(payload);
	invokeManager->invoke(request);
}


void Invokator::invokeTwitterProfile(const QString &profile)
{
	InvokeRequest request;
	request.setTarget("com.twitter.urihandler");
	request.setAction("bb.action.VIEW");
	request.setUri(profile);
	invokeManager->invoke(request);
}


void Invokator::invokeMoreApps()
{
	InvokeRequest request;
	request.setTarget("sys.appworld");
	request.setAction("bb.action.OPEN");
	request.setUri("appworld://vendor/8198");
	invokeManager->invoke(request);
}


void Invokator::invokeViewer(const QString &uri)
{
	InvokeRequest request;
	request.setTarget("si.pronic.PRISMBreak4");
	request.setAction("bb.action.OPEN");
	request.setUri(uri);
	request.setData(QString("1").toUtf8());
	invokeManager->invoke(request);

	QObject::connect(invokeManager, SIGNAL(childCardDone(const bb::system::CardDoneMessage&)),
			this, SLOT(onMessageCardDone(const bb::system::CardDoneMessage&)));
}


void Invokator::invokeAccounts()
{
	InvokeRequest request;
	request.setTarget("sys.settings.card");
	request.setAction("bb.action.OPEN");
	request.setMimeType("settings/view");
	request.setUri("settings://pim");
	invokeManager->invoke(request);
}


void Invokator::invokePermissions()
{
	InvokeRequest request;
	request.setTarget("sys.settings.card");
	request.setAction("bb.action.OPEN");
	request.setMimeType("settings/view");
	request.setUri("settings://permissions");
	invokeManager->invoke(request);
}


void Invokator::invokeKeyImport(const QString &data)
{
	InvokeRequest request;
	request.setTarget("si.pronic.PRISMBreakKey4");
	request.setAction("bb.action.OPEN");
	request.setData(data.toUtf8());
	invokeManager->invoke(request);
}


void Invokator::invokeComposer(int type, const QString &text, const QString &from,
		const QString &to, const QString &pubKey, const QString &fileName)
{
	qDebug() << "Invoke composer - filename: " << fileName;
	InvokeRequest request;
	request.setTarget("si.pronic.PRISMBreakCompose4");
	request.setAction("bb.action.COMPOSE");
	qDebug() << "Invoke composer - target and action set";

	QString data("{\"TYPE\":\"");
	data.append(QString::number(type));
	data.append("\",\"TEXT\":\"");
	data.append(text);
	data.append("\",\"FROM\":\"");
	data.append(from);
	data.append("\",\"TO\":\"");
	data.append(to);
	data.append("\",\"PUBKEY\":\"");
	data.append(pubKey.mid(11));
	data.append("\",\"FILENAME\":\"");
	data.append(fileName);
	data.append("\"}");

	request.setData(data.toUtf8());
	qDebug() << "Invoke composer - data set";
	QObject::connect(invokeManager, SIGNAL(childCardDone(const bb::system::CardDoneMessage&)),
			this, SLOT(onCardDone(const bb::system::CardDoneMessage&)));
	invokeManager->invoke(request);
}


void Invokator::shareEmail(const QString &fileName)
{
	QString homePath = QDir::home().absolutePath();
	qDebug() << "Home path: " << homePath;
	int pos = homePath.lastIndexOf('/');
	qDebug() << "Pos: " << pos;
	homePath = homePath.left(pos);
	qDebug() << "Dir: " << QString("file://" + homePath + "/shared/misc/" + fileName);

	QString theFile = QString("file://" + QDir::currentPath() + "/sharewith/pim/" + fileName);

/*	InvokeRequest request;
	request.setTarget("sys.pim.uib.email.hybridcomposer");
	request.setAction("bb.action.SHARE");
	request.setMimeType("text/plain");
	request.setUri(QString("file://" + homePath + "/shared/misc/" + fileName));
	invokeManager->invoke(request);*/

	InvokeRequest request;
	request.setTarget("sys.pim.uib.email.hybridcomposer");
	request.setAction("bb.action.COMPOSE");
	request.setMimeType("message/rfc822");

	QVariantMap payload;
	payload["subject"] = "PRISM Break Secure Email Message";
	payload["body"] = "Hi!\n\n I am sending you secret message.\n\nYou need to download a free PRISM Break application to read it.\n\nLink: http://appworld.blackberry.com/webstore/content/49133888";

	QVariantList list;
//	qDebug() << "Invoke URL PATH (encoded): "  << QString("file://" + pimDir.absolutePath() + "/" + fileName);
//	list.append(QString("file://" + homePath + "/shared/misc/" + fileName));
//	list << QString("file://" + homePath + "/shared/misc/" + fileName);
	list << QString(theFile);
	qDebug() << "L: " << list;
	payload["attachment"] = list;
	qDebug() << "PL: " << payload;

	QVariantMap data;
	data["data"] = payload;
	bool ok;
	request.setData(bb::PpsObject::encode(data, &ok));

	invokeManager->invoke(request);
}


void Invokator::onCardDone(const CardDoneMessage &message)
{
	QObject::disconnect(invokeManager, SIGNAL(childCardDone(const bb::system::CardDoneMessage&)),
			this, SLOT(onCardDone(const bb::system::CardDoneMessage&)));
	qDebug() << "##### Card done: " << message.reason();
	config->read();
	payment->checkPurchases(false);
}


void Invokator::onMessageCardDone(const CardDoneMessage &message)
{
	QObject::disconnect(invokeManager, SIGNAL(childCardDone(const bb::system::CardDoneMessage&)),
			this, SLOT(onMessageCardDone(const bb::system::CardDoneMessage&)));
	qDebug() << "##### Message card done: " << message.reason();
	config->read();
}


void Invokator::shareBBM(const QString &fileName)
{
	QString homePath = QDir::home().absolutePath();
	qDebug() << "Home path: " << homePath;
	int pos = homePath.lastIndexOf('/');
	qDebug() << "Pos: " << pos;
	homePath = homePath.left(pos);
	qDebug() << "Dir: " << QString("file://" + homePath + "/shared/misc/" + fileName);

	InvokeRequest request;
	request.setTarget("sys.bbm.sharehandler");
	request.setAction("bb.action.SHARE");
	request.setMimeType("text/plain");
	request.setUri(QString("file://" + homePath + "/shared/misc/" + fileName));
	invokeManager->invoke(request);

}


void Invokator::sharePublicKey()
{
	QString keyValue = crypto->toHex(crypto->exportOwnPublicKey().toUtf8());

	InvokeRequest request;
	request.setTarget("sys.pim.uib.email.hybridcomposer");
	request.setAction("bb.action.COMPOSE");
	request.setMimeType("text/html");

	QVariantMap payload;
	payload["subject"] = "PRISM Break Secure Email Public Key";
	payload["body"] = "Hi!\n\nThis is a message for you with my public key.\n\nClick on the link prismkey://" +
			keyValue + " to import the key and don't forget to download a free version of PRISM Break Secure " +
			"Email application to read secured messages.\n\nYou can find the application at http://appworld.blackberry.com/webstore/content/49133888 and purchase a full version later.\n\n\n";

	QVariantMap data;
	data["data"] = payload;
	bool ok;
	request.setData(bb::PpsObject::encode(data, &ok));

	invokeManager->invoke(request);

}


void Invokator::sharePublicKey2()
{
	InvokeRequest request;
	request.setAction("bb.action.SHARE");
//	request.setMimeType("text/plain");
	request.setData(QString("prismkey://" + crypto->getOwnPublicKeyURL()).toUtf8());
	invokeManager->invoke(request);
}


void Invokator::sharePublicKeyViaNFC()
{
	InvokeRequest request;
	request.setTarget("sys.NFCViewer");
	request.setAction("bb.action.SHARE");
	request.setUri(QString("prismkey://" + crypto->getOwnPublicKeyURL()));
	invokeManager->invoke(request);
}


} } // NAMESPACE
