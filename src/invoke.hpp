#ifndef INVOKE_HPP_
#define INVOKE_HPP_


#include <QVariantMap>
#include <QVariantList>
#include <QTemporaryFile>
#include <bb/cascades/Application>
#include <bb/cascades/InvokeQuery>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/PpsObject>
#include <bb/system/CardDoneMessage>
#include "crypto.hpp"
#include "config.hpp"
#include "payment.hpp"


using namespace bb::cascades;
using namespace bb::system;


namespace pronic { namespace prism { // NAMESPACE


class Invokator : public QObject
{
	Q_OBJECT

	public:

		Invokator(Crypto *_crypto, Configurator *_config, Payment *_payment);
		virtual ~Invokator() {};

	    static const int NEW;
	    static const int REPLY;
	    static const int FORWARD;

		Q_INVOKABLE void invokePermissions();
		Q_INVOKABLE void invokeAccounts();
		Q_INVOKABLE void invokeViewer(const QString &uri);
		Q_INVOKABLE void invokeComposer(int type, const QString &text, const QString &from,
				const QString &to, const QString &pubKey, const QString &fileName);
		Q_INVOKABLE void invokeKeyImport(const QString &data);
		Q_INVOKABLE void invokeMoreApps();
		Q_INVOKABLE void invokeFacebookPage(const QString &pageName);
		Q_INVOKABLE void invokeTwitterProfile(const QString &profile);
		Q_INVOKABLE void shareEmail(const QString &fileName);
		Q_INVOKABLE void shareBBM(const QString &fileName);
		Q_INVOKABLE void sharePublicKey();
		Q_INVOKABLE void sharePublicKey2();
		Q_INVOKABLE void sharePublicKeyViaNFC();

		InvokeManager* getInvokeManager();

		InvokeManager* invokeManager;

		Q_INVOKABLE void closeCard();

	private Q_SLOTS:

		void onCardDone(const bb::system::CardDoneMessage &message);
		void onMessageCardDone(const bb::system::CardDoneMessage &message);

	private:

		Crypto *crypto;
		Configurator *config;
		Payment *payment;

};


} } // NAMESPACE


#endif /* INVOKE_HPP_ */
