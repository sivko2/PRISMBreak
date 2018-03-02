#ifndef PAYMENT_HPP_
#define PAYMENT_HPP_


#include <bb/cascades/Application>
#include <bb/cascades/Window>
#include <bb/platform/PaymentManager>
#include <bb/platform/PaymentConnectionMode>
#include <bb/platform/PaymentReply>
#include <bb/platform/PurchaseReply>
#include <bb/platform/CancelSubscriptionReply>
#include <bb/platform/DigitalGoodReply>
#include <bb/platform/ExistingPurchasesReply>
#include <bb/platform/PriceReply>
#include <bb/platform/SubscriptionStatusReply>
#include <bb/platform/SubscriptionTermsReply>
#include <bb/platform/PurchaseReceipt>
#include <bb/platform/DigitalGoodState>
#include <QSettings>


using namespace bb::platform;


namespace pronic { namespace prism { // NAMESPACE


class Configurator;


class Payment : public QObject
{
	Q_OBJECT

	public:

		Payment();
		~Payment();

		Q_PROPERTY(bool purchased READ isPurchased WRITE setPurchased NOTIFY purchasedChanged);

		Q_INVOKABLE void checkPurchases(bool);

		bool isPurchased();
		void setPurchased(bool);

	public slots:

		void handleExistingPurchases();

	signals:

		void purchasedChanged();


	private:

		bool firstTime;
		bool purchased;

		PaymentManager *manager;

		QSettings settings;
};


} } // NAMESPACE


#endif

