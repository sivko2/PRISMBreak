
#include "payment.hpp"


namespace pronic { namespace prism { // NAMESPACE


Payment::Payment()
{
	manager = new PaymentManager;
	const QString windowGroupId =
	    bb::cascades::Application::instance()->mainWindow()->groupId();
	manager->setWindowGroupId(windowGroupId);

	qDebug() << "##### P as Purchased read: " << settings.value("purchased");

	if (!settings.value("purchased").isNull())
	{
		purchased = settings.value("purchased").toBool();
		qDebug() << "##### Purchased read: " << settings.value("purchased") << "/" << purchased;
/*		if (!purchased)
		{
			checkPurchases(false);
		}*/
	}
	else
	{
		setPurchased(false);
		qDebug() << "##### Purchased read with null: " << settings.value("purchased") << "/" << purchased;
		checkPurchases(true);
	}

}


Payment::~Payment()
{
}


void Payment::checkPurchases(bool refresh)
{
	if (!purchased)
	{
		const ExistingPurchasesReply *reply = manager->requestExistingPurchases(refresh);

		bool success = QObject::connect(reply, SIGNAL(finished()), this,
			 SLOT(handleExistingPurchases()));

		qDebug() << "##### IS PURCHASED invoked: " << success;
	}
}


void Payment::handleExistingPurchases()
{
	ExistingPurchasesReply *reply = qobject_cast<ExistingPurchasesReply*>(sender());
	Q_ASSERT(reply);

	qDebug() << "##### LIST OF PURCHASES isFinished: " << reply->isFinished() <<
			" isError: " << reply->isError();

	if (reply->isFinished())
	{
 		if (!reply->isError())
 		{
			const QList<PurchaseReceipt> receipts = reply->purchases();
			qDebug() << "##### LIST OF PURCHASES size: " << receipts.length();

			if (receipts.isEmpty())
			{
				setPurchased(false);
				qDebug() << "##### Purchased set to false";
			}
			else
			{
				setPurchased(true);
				qDebug() << "##### Purchased set to true";
			}
		}
 		else
 		{
			setPurchased(false);
 			qDebug() << "##### ERROR WHILE PURCHASING " <<  reply->errorCode() << " " << reply->errorInfo() << " " << reply->errorText();
 		}
	}

	reply->deleteLater();
	Q_UNUSED(reply);
}


bool Payment::isPurchased()
{
	return purchased;
}


void Payment::setPurchased(bool _purchased)
{
	purchased = _purchased;
	settings.setValue("purchased", QVariant(purchased));
	emit purchasedChanged();
}


} } // NAMESPACE

