
#include "applicationui.hpp"


using namespace bb::cascades;
using namespace bb::system;


namespace pronic { namespace prism { // NAMESPACE


// Constructor
ApplicationUI::ApplicationUI(Application *app) : QObject(app)
{
	// Store pointer to app
	qDebug() << "########## Application init";
	application = app;

	// Create and initialize instances of helper classes
	payment = new Payment();
	config = new Configurator();
	crypto = new Crypto(config);
	util = new Util(crypto);
	pim = new PIM(crypto, util);
	invokator = new Invokator(crypto, config, payment);

	bool ok;

	ok = QObject::connect(application, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
	if (!ok)
	{
		qDebug() << "Error connecting signal aboutToQuit()";
	}

	ok = QObject::connect(invokator->getInvokeManager(), SIGNAL(invoked(const bb::system::InvokeRequest&)),
            this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));
	if (!ok)
	{
		qDebug() << "Error connecting signal invoked()";
	}

	ok = QObject::connect(invokator->getInvokeManager(),
            SIGNAL(cardResizeRequested(const bb::system::CardResizeMessage&)),
            this, SLOT(resized(const bb::system::CardResizeMessage&)));
	if (!ok)
	{
		qDebug() << "Error connecting signal cardResizeRequested()";
	}

	ok = QObject::connect(invokator->getInvokeManager(),
            SIGNAL(cardPooled(const bb::system::CardDoneMessage&)), this,
            SLOT(pooled(const bb::system::CardDoneMessage&)));
	if (!ok)
	{
		qDebug() << "Error connecting signal cardPooled()";
	}

	// Check startup mode to properly start app
    switch (invokator->getInvokeManager()->startupMode())
    {
    	// Launched as app from icon
		case ApplicationStartupMode::LaunchApplication:
			startupMode = "LaunchApp";
			qDebug() << "Launch app";
			initMainUI();
			break;

		// Launched as app
		case ApplicationStartupMode::InvokeApplication:
			startupMode = "InvokeApp";
			qDebug() << "Invoke app";
			break;

		// Launches as card
		case ApplicationStartupMode::InvokeCard:
			startupMode = "InvokeCard";
			qDebug() << "Invoke card";
			break;
    }
}


// Destructor
void ApplicationUI::onAboutToQuit()
{
	qDebug() << "onAboutToQuit()";
	if (pim)
	{
		delete pim;
	}

	if (invokator)
	{
		delete invokator;
	}

	if (util)
	{
		delete util;
	}

	if (crypto)
	{
		delete crypto;
	}

	if (config)
	{
		delete config;
	}

	if (payment)
	{
		delete payment;
	}
}


// Sleeps for defined number of millisecs
void ApplicationUI::sleep(int ms)
{
	PrismThread::msleep(ms);
}


// Hanbdling card/app invoke
void ApplicationUI::handleInvoke(const InvokeRequest& request)
{
	// init
	crypto->setAsymKeys();
	config->read();
	payment->checkPurchases(false); // check locally

	// Read invokation data
    source = QString::fromLatin1("%1 (%2)").arg(request.source().installId()).
    		arg(request.source().groupId());
    qDebug() << "##### SOURCE: " << source;
    target = request.target();
    qDebug() << "TARGET: " << target;
    action = request.action();
    qDebug() << "ACTION: " << action;
    mimeType = request.mimeType();
    qDebug() << "MIME: " << mimeType;
    uri = request.uri().toString();
    qDebug() << "URI: " << uri;
    data = QString::fromUtf8(request.data());
    qDebug() << "DATA: " << data;

    // Target is viewer
    if (target == "si.pronic.PRISMBreak4")
    {
    	qDebug() << "Invoking: message viewer - data length: " << data.length();
    	if (data.isNull() || data.length() != 1)
    	{
    		initViewUI(true);
    	}
    	else
    	{
    		initViewUI(false);
    	}
    }
    // Target is key importer
    else if (target == "si.pronic.PRISMBreakKey4")
    {
    	qDebug() << "Invoking: key importer";
        initKeyImportUI(data);
    }
    // Target is composer
    else if (target == "si.pronic.PRISMBreakCompose4")
    {
    	qDebug() << "Invoking: composer";
    	if (uri.compare("") == 0 && data.compare("") == 0)
    	{
    		initComposerUI("", "", false, true);
    	}
    	else if (uri.startsWith("pim:"))
    	{
    		initComposerUI("", "", false, true);
    	}
    	else
    	{
    		initComposerUI(data, "", true, false);
    	}
    }
    // Target is key share
    else if (target == "si.pronic.PRISMBreakShare4")
    {
    	qDebug() << "Invoking: composer via share";
   		initComposerUI(data, uri, false, true);
    }

    emit requestChanged();
}


void ApplicationUI::initMainUI()
{
	// Active card
	qDebug() << "initMainUI";
	QmlDocument *qmlCover = QmlDocument::create("asset:///PrismCover.qml").parent(this);
	qDebug() << "Cover QML created";

	if (!qmlCover->hasErrors())
	{
	    Container *coverContainer = qmlCover->createRootObject<Container>();
	    SceneCover *sceneCover = SceneCover::create().content(coverContainer);
	    Application::instance()->setCover(sceneCover);
	}

	// Delete temporary files
	util->deleteTempFiles();
	qDebug() << "Temp files deleted";

	// Sleep to show logo
	PrismThread::msleep(200);
	qDebug() << "Slept for 200ms";

	qDebug() << "Password: " << config->getPassword();
	// If there is no stored password then go through setup process
    if (config->getPassword() == 0)
    {
    	qDebug() << "Going welcome";
    	showWelcome();
    }
    // Otherwise show main screen
    else
    {
    	crypto->setAsymKeys();
    	qDebug() << "Going main";
    	showMain();
    }
}


// Show setup wizard screen
void ApplicationUI::showWelcome()
{
	qDebug() << "##### Showing welcome";

	AbstractPane *previousScene = Application::instance()->scene();
	if (previousScene)
	{
		previousScene->setParent(NULL);
	}

	QmlDocument *qml = QmlDocument::create("asset:///WelcomePane.qml").parent(this);
	qml->setContextProperty("app", this);
	qml->setContextProperty("crypto", crypto);
	qml->setContextProperty("util", util);
	qml->setContextProperty("config", config);
	qml->setContextProperty("invokator", invokator);
	qml->setContextProperty("pim", pim);
	qml->setContextProperty("payment", payment);
	AbstractPane *root = qml->createRootObject<AbstractPane>();
	Application::instance()->setScene(root);

	if (previousScene)
	{
		previousScene->deleteLater();
	}
}


// Show main screen
void ApplicationUI::showMain()
{
	QSettings settings;
	udsUtil = new UDSUtil();
	udsUtil->initialize();
	if (!settings.value("hub").toBool())
	{
		udsUtil->addAccount("PRISM Break", "PRISM Break Secure Email", "pronic.si", "si.pronic.PRISMBreak",
				"acc_u.png", "acc_u.png", "acc.png", "", true, UDS_ACCOUNT_TYPE_OTHER);
		udsUtil->addAccActionData("PRISM Break", "bb.action.COMPOSE", "Compose",
				"si.pronic.PRISMBreakCompose4", "card.composer", "compose.png", "hub/prismbreak", UDS_PLACEMENT_BAR);
		settings.setValue("hub", true);
		udsUtil->addCategory("Messages", "PRISM Break");
		udsUtil->addItem("Notification", "Check email accounts for secured messages", "plain/text",
				"itemMarkUnread.png", true, "None", QDateTime::currentDateTimeUtc().toMSecsSinceEpoch(), 1,
				true, "Messages", "PRISM Break");
	}

	qDebug() << "##### Showing main";

	AbstractPane *previousScene = Application::instance()->scene();
	if (previousScene)
	{
		previousScene->setParent(NULL);
	}
	qDebug() << "Previous scene removed";

	QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
	qDebug() << "Main QML created";
	qml->setContextProperty("app", this);
	qml->setContextProperty("crypto", crypto);
	qml->setContextProperty("util", util);
	qml->setContextProperty("config", config);
	qml->setContextProperty("invokator", invokator);
	qml->setContextProperty("pim", pim);
	qml->setContextProperty("payment", payment);
	qDebug() << "Context properties set";
	AbstractPane *root = qml->createRootObject<AbstractPane>();
	Application::instance()->setScene(root);
	qDebug() << "Scene set";

	if (previousScene)
	{
		previousScene->deleteLater();
	}
}


// Show message viewer card
void ApplicationUI::initViewUI(bool secured)
{
	// Showing message viewer
    QmlDocument *qml = QmlDocument::create("asset:///MessageView.qml");
    qml->setContextProperty("app", this);
    qml->setContextProperty("crypto", crypto);
    qml->setContextProperty("util", util);
    qml->setContextProperty("config", config);
    qml->setContextProperty("invokator", invokator);
	qml->setContextProperty("pim", pim);
	qml->setContextProperty("payment", payment);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    root->setProperty("secured", secured ? 2 : 1);
    Application::instance()->setScene(root);
}


// Show public key importer card
void ApplicationUI::initKeyImportUI(const QString &pubKey)
{
	qDebug() << "##### Importing: " << pubKey;
	QString key = pubKey.mid(11);
	qDebug() << "##### Key: " << key;
	crypto->setPubKey(key);
	qDebug() << "##### Imported";

    QmlDocument *qml = QmlDocument::create("asset:///KeyImport.qml");
    qml->setContextProperty("app", this);
    qml->setContextProperty("crypto", crypto);
    qml->setContextProperty("util", util);
    qml->setContextProperty("config", config);
    qml->setContextProperty("invokator", invokator);
	qml->setContextProperty("pim", pim);
	qml->setContextProperty("payment", payment);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    Application::instance()->setScene(root);
}


// Show composer card
void ApplicationUI::initComposerUI(const QString &data, const QString &uri, bool isJson, bool secured)
{
	if (payment != NULL)
	{
		delete payment;
	}
	payment = new Payment();
	if (config != NULL)
	{
		delete config;
	}
	config = new Configurator();
	JsonDataAccess jda;

    QmlDocument *qml = QmlDocument::create("asset:///Composer.qml");
    qml->setContextProperty("app", this);
    qml->setContextProperty("crypto", crypto);
    qml->setContextProperty("util", util);
    qml->setContextProperty("config", config);
    qml->setContextProperty("invokator", invokator);
	qml->setContextProperty("pim", pim);
	qml->setContextProperty("payment", payment);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    root->setProperty("secured", secured ? 2 : 1);
    Application::instance()->setScene(root);

    if (isJson)
    {
		QMap<QString,QVariant> map = jda.loadFromBuffer(data).toMap();
		int type = map.value("TYPE").toInt();

		if (type == Invokator::FORWARD)
		{
			if (payment->isPurchased())
			{
				root->setProperty("msgText", QString("\n\n[Forwarded from ").append(map.value("FROM").toString()).
						append("]\n\n").append(map.value("TEXT").toString()));
			}
			root->setProperty("securityLabel", "Low Security");
			root->setProperty("userEmail", map.value("FROM").toString());
			root->setProperty("fileName", map.value("FILENAME").toString());
		}
		else if (type == Invokator::REPLY)
		{
			if (payment->isPurchased())
			{
				root->setProperty("msgText", QString("\n\n[Replying to ").append(map.value("TO").toString()).
						append("]\n\n").append(map.value("TEXT").toString()));
			}
			root->setProperty("toText", map.value("TO").toString());
			root->setProperty("key", map.value("PUBKEY").toString());
			root->setProperty("attEnabled", false);
			root->setProperty("toActionEnabled", false);
			root->setProperty("securityLabel", "High Security");
			root->setProperty("userEmail", map.value("FROM").toString());
		}
    }
    else
    {
    	if (uri.length() > 0)
    	{
    		if (payment->isPurchased())
    		{
    			root->setProperty("attachment", QString("." + uri.mid(21)));
    		}
    	}
    	else
    	{
    		root->setProperty("msgText", QString(data));
    	}
    }
	root->setProperty("readyToWork", true);
}


// When card is being closed
void ApplicationUI::cardDone(const QString& msg)
{
    CardDoneMessage message;
    message.setData(msg);
    message.setDataType("text/plain");
    message.setReason("Success!");
    qDebug() << "##### Closing card...";
    invokator->getInvokeManager()->sendCardDone(message);
}


void ApplicationUI::resized(const bb::system::CardResizeMessage&)
{
	qDebug() << "##### Resized!";
    emit statusChanged();
}


void ApplicationUI::pooled(const bb::system::CardDoneMessage& doneMessage)
{
	qDebug() << "##### Pooled!";
    emit statusChanged();
    emit requestChanged();
}


QString ApplicationUI::getStartupMode() const
{
    return startupMode;
}


QString ApplicationUI::getSource() const
{
    return source;
}


QString ApplicationUI::getTarget() const
{
    return target;
}


QString ApplicationUI::getAction() const
{
    return action;
}


QString ApplicationUI::getMimeType() const
{
    return mimeType;
}


QString ApplicationUI::getUri() const
{
    return uri;
}


QString ApplicationUI::getData() const
{
    return data;
}


QString ApplicationUI::getStatus() const
{
    return status;
}


} } // NAMESPACE
