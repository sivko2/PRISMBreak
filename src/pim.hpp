#ifndef PIM_HPP_
#define PIM_HPP_


#include <bb/pim/account/AccountService>
#include <bb/pim/account/Account>
#include <bb/pim/account/Service>
#include <bb/pim/account/Provider>
#include <bb/pim/account/Property>
#include <bb/pim/message/MessageService>
#include <bb/pim/message/Attachment>
#include <bb/pim/message/MessageContact>
#include <bb/pim/message/MessageBody>
#include <bb/pim/message/Message>
#include <bb/pim/message/MessageBuilder>
#include <bb/pim/message/MessageFilter>
#include <bb/pim/message/MessageSearchFilter>
#include <bb/pim/message/SearchFilterCriteria>
#include <bb/pim/contacts/ContactService>
#include <bb/pim/contacts/Contact>
#include <bb/pim/contacts/ContactAttribute>
#include <bb/pim/contacts/ContactBuilder>
#include <bb/pim/contacts/ContactAttributeBuilder>
#include <bb/pim/contacts/ContactSearchFilters>
#include <bb/data/JsonDataAccess>
#include <QVariantList>
#include <QList>
#include <QVariantMap>
#include <QVariant>
#include <QMap>
#include <QVariantList>
#include <QByteArray>
#include <QUrl>
#include "crypto.hpp"
#include "util.hpp"


//using namespace bb::cascades;
using namespace bb::pim::contacts;
using namespace bb::pim::message;
using namespace bb::pim::account;


namespace pronic { namespace prism { // NAMESPACE


class PIM : public QObject
{
	Q_OBJECT

	public:

		PIM(Crypto *_crypto, Util *_util);
		~PIM();

		Q_INVOKABLE int checkContact(int contactId, const QString &key);
		Q_INVOKABLE void updateContact(int contactId, const QString &key);
		Q_INVOKABLE QVariantList listEmails(int contactId);
		Q_INVOKABLE QString getPublicKey(int contactId);
		Q_INVOKABLE QString getName(int contactId);
		Q_INVOKABLE void sendEmail(int accountId, const QString &recepient, const QString &msg);
		Q_INVOKABLE QVariantList listAccounts();
		Q_INVOKABLE QVariantList listEmails();
		Q_INVOKABLE QVariantList listMessages();
		Q_INVOKABLE QVariantMap getMessage(int accountId, int messageId);
		Q_INVOKABLE int findContact(const QString &from);

	signals:

		void postMessageAdded(bb::pim::account::AccountKey, bb::pim::message::MessageKey);
		void postMessageRemoved(bb::pim::account::AccountKey, bb::pim::message::MessageKey);

	public slots:

		void onMessageAdded(bb::pim::account::AccountKey,
				bb::pim::message::ConversationKey,
				bb::pim::message::MessageKey);
		void onMessageRemoved(bb::pim::account::AccountKey,
				bb::pim::message::ConversationKey,
				bb::pim::message::MessageKey,
				QString);

	private:

		ContactService *contactService;
		MessageService *messageService;
		AccountService *accountService;

		Crypto *crypto;
		Util *util;

};


} } // NAMESPACE





#endif /* PIM_HPP_ */
