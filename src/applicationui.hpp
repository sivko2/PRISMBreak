#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_


#include <QBool>
#include <QObject>
#include <QDebug>
#include <QVariantMap>
#include <bb/cascades/Application>
#include <bb/system/CardDoneMessage>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/Container>
#include <bb/cascades/SceneCover>
#include <bb/cascades/AbstractCover>
#include <bb/data/JsonDataAccess>
#include <QDateTime>
#include "crypto.hpp"
#include "util.hpp"
#include "config.hpp"
#include "invoke.hpp"
#include "pim.hpp"
#include "prismthread.hpp"
#include "payment.hpp"
#include "uds.hpp"


using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;


namespace pronic { namespace prism { // NAMESPACE


class ApplicationUI : public QObject
{
    Q_OBJECT

public:

    Q_PROPERTY(QString startupMode READ getStartupMode NOTIFY requestChanged);

    Q_PROPERTY(QString source READ getSource NOTIFY requestChanged);
    Q_PROPERTY(QString target READ getTarget NOTIFY requestChanged);
    Q_PROPERTY(QString action READ getAction NOTIFY requestChanged);
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY requestChanged);
    Q_PROPERTY(QString uri READ getUri NOTIFY requestChanged);
    Q_PROPERTY(QString data READ getData NOTIFY requestChanged);

    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged);

	ApplicationUI(bb::cascades::Application *app);
	virtual ~ApplicationUI() {}

	void initMainUI();
	void initViewUI(bool secured);
	void initComposerUI(const QString &data, const QString &uri, bool isJson, bool secured);
	void initKeyImportUI(const QString &pubKey);
	Q_INVOKABLE void showMain();
	Q_INVOKABLE void showWelcome();
	Q_INVOKABLE void sleep(int ms);

Q_SIGNALS:

	void requestChanged();
	void statusChanged();

private Q_SLOTS:

	void onAboutToQuit();
	void handleInvoke(const bb::system::InvokeRequest&);
	void resized(const bb::system::CardResizeMessage&);
	void pooled(const bb::system::CardDoneMessage&);

private:

	QString getStartupMode() const;
	QString getSource() const;
	QString getTarget() const;
	QString getAction() const;
	QString getMimeType() const;
	QString getUri() const;
	QString getData() const;
	QString getStatus() const;

	void cardDone(const QString& msg);

	Payment *payment;
	Crypto *crypto;
	Util *util;
	Invokator *invokator;
	PIM *pim;
	Configurator *config;
	UDSUtil *udsUtil;
//	uds_context_t udsHandle;

	QString startupMode;
	QString source;
	QString target;
	QString action;
	QString mimeType;
	QString uri;
	QString data;
	QString status;

	Application *application;

};


} } // NAMESPACE


#endif
