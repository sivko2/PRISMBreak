
#include "pim.hpp"


//using namespace bb::cascades;


namespace pronic { namespace prism { // NAMESPACE


PIM::PIM(Crypto *_crypto, Util *_util)
{
	crypto = _crypto;
	util = _util;
	contactService = new ContactService;
	messageService = new MessageService;
	accountService = new AccountService;

/*	bool res = QObject::connect(messageService,
			SIGNAL(messageAdded(bb::pim::account::AccountKey,
					bb::pim::message::ConversationKey,
					bb::pim::message::MessageKey)),
			this,
			SLOT(onMessageAdded(bb::pim::account::AccountKey,
					bb::pim::message::ConversationKey,
					bb::pim::message::MessageKey)));
	Q_ASSERT(res);

	res = QObject::connect(messageService,
			SIGNAL(messageRemoved(bb::pim::account::AccountKey,
					bb::pim::message::ConversationKey,
					bb::pim::message::MessageKey,
					QString)),
			this,
			SLOT(onMessageRemoved(bb::pim::account::AccountKey,
					bb::pim::message::ConversationKey,
					bb::pim::message::MessageKey,
					QString)));
	Q_ASSERT(res);

	Q_UNUSED(res);*/
}


PIM::~PIM()
{
	delete crypto;
	delete contactService;
	delete messageService;
	delete accountService;
}


int PIM::findContact(const QString &from)
{
	ContactSearchFilters options;
	options.setSearchValue(from);
	options.setIncludeAttribute(AttributeKind::Email);

	QList<Contact>contacts = contactService->searchContacts(options);
	qDebug() << "Find contact - list size: " << contacts.length();
	if (contacts.length() == 0)
	{
		return 0;
	}

	return contacts.at(0).id();
}


int PIM::checkContact(int contactId, const QString &key)
{
	Contact primaryContact = contactService->contactDetails(contactId);
	qDebug() << "Contact selected: " + primaryContact.id();

	QList<Contact> contactList = contactService->mergedContacts(contactId);
	contactList.insert(0, primaryContact);
	qDebug() << "Merged contacts: " << contactList.length();

	int retVal = 0;

	for (int i = 0; i < contactList.length(); i++)
	{
		Contact contact = contactList.at(i);
		qDebug() << "Merged contact ID: " << contact.id();

		if (contact.id())
		{
			QList<ContactAttribute> list = contact.attributes();
			qDebug() << "Contact attr list size: " << list.length();

			for (int j = 0; j < list.length(); j++)
			{
				ContactAttribute attr = list.at(j);
				qDebug() << "IN Attr value: " << attr.value() << " * " << attr.kind() << " " << attr.subKind();
				if (attr.kind() == AttributeKind::Note && attr.subKind() == AttributeSubKind::Other)
				{
					if (attr.value().startsWith("prismkey://"))
					{
						if (attr.value().compare(key) == 0)
						{
							if (retVal < 2)
							{
							    retVal = 1; // at least one value found that is the same
							}
						}
						else
						{
							retVal = 2; // at least one value found that is different
						}
					}
				}
			}
		}
	}

	return retVal; // no prismkey found in any contact
}


void PIM::updateContact(int contactId, const QString &key)
{
	Contact primaryContact = contactService->contactDetails(contactId);

	QList<Contact> contactList = contactService->mergedContacts(contactId);
	contactList.insert(0, primaryContact);
	qDebug() << "Merged contacts: " << contactList.length();

	for (int i = 0; i < contactList.length(); i++)
	{
		Contact contact = contactList.at(i);
		qDebug() << "Merged contact ID: " << contact.id();

		if (contact.id())
		{
			ContactBuilder builder = contact.edit();

			QList<ContactAttribute> list = contact.attributes();
			for (int j = 0; j < list.length(); j++)
			{
				ContactAttribute attr = list.at(j);
				if (attr.kind() == AttributeKind::Note && attr.subKind() == AttributeSubKind::Other)
				{
					if (attr.value().startsWith("prismkey://"))
					{
						builder.deleteAttribute(attr);
						break;
					}
				}
			}

			builder.addAttribute(ContactAttributeBuilder().setKind(AttributeKind::Note).
					setSubKind(AttributeSubKind::Other).setValue(key));
			contact = contactService->updateContact(contact);
			list = contact.attributes();
			qDebug() << "Contact attr list size: " << list.length();

			for (int j = 0; j < list.length(); j++)
			{
				ContactAttribute attr = list.at(j);
				qDebug() << "OUT Attr value: " << attr.value() << " * " << attr.kind() << " " << attr.subKind();
			}
		}
	}
}


QVariantList PIM::listEmails(int contactId)
{
	QVariantList retList;

	Contact primaryContact = contactService->contactDetails(contactId);
	qDebug() << "Contact selected: " + primaryContact.id();

	QList<Contact> contactList = contactService->mergedContacts(contactId);
	contactList.insert(0, primaryContact);
	qDebug() << "Merged contacts: " << contactList.length();

	for (int i = 0; i < contactList.length(); i++)
	{
		Contact contact = contactList.at(i);
		qDebug() << "Merged contact ID: " << contact.id();

		if (contact.id())
		{
			QList<ContactAttribute> list = contact.emails();
			qDebug() << "Emails list size: " << list.length();

			for (int j = 0; j < list.length(); j++)
			{
				ContactAttribute attr = list.at(j);
				qDebug() << attr.value();
				retList << attr.value();
			}
		}
	}

	return retList;
}


QString PIM::getName(int contactId)
{
	Contact primaryContact = contactService->contactDetails(contactId);
	QString name = " ";

	if (primaryContact.id())
	{
		ContactBuilder builder = primaryContact.edit();

		QList<ContactAttribute> list = primaryContact.attributes();
		for (int j = 0; j < list.length(); j++)
		{
			ContactAttribute attr = list.at(j);
			if (attr.kind() == AttributeKind::Name &&
					(attr.subKind() == AttributeSubKind::NameGiven ||
							attr.subKind() == AttributeSubKind::NameSurname))
			{
				name = name + attr.value() + " ";
			}
		}
	}

	if (name.compare(" ") == 0)
	{
		name = "";
	}

	return name;
}


QString PIM::getPublicKey(int contactId)
{
	Contact primaryContact = contactService->contactDetails(contactId);

	QList<Contact> contactList = contactService->mergedContacts(contactId);
	contactList.insert(0, primaryContact);
	qDebug() << "Merged contacts: " << contactList.length();

	for (int i = 0; i < contactList.length(); i++)
	{
		Contact contact = contactList.at(i);
		qDebug() << "Merged contact ID: " << contact.id();

		if (contact.id())
		{
			ContactBuilder builder = contact.edit();

			QList<ContactAttribute> list = contact.attributes();
			for (int j = 0; j < list.length(); j++)
			{
				ContactAttribute attr = list.at(j);
				if (attr.kind() == AttributeKind::Note && attr.subKind() == AttributeSubKind::Other)
				{
					if (attr.value().startsWith("prismkey://"))
					{
						return attr.value().mid(11);
					}
				}
			}
		}
	}

	return "";
}


QVariantList PIM::listAccounts()
{
	QVariantList retList;

/*	QList<Provider> plist = accountService->providers();
	for (int i = 0; i < plist.length(); i++)
	{
		Provider p = plist.at(i);
		qDebug() << "Provider ID: " << p.id() << " " <<
				p.name() << " " << p.isSocial();
	}*/

	QList<Account> list = accountService->accounts(Service::Messages);
	for (int i = 0; i < list.length(); i++)
	{
		Account account = list.at(i);
		qDebug() << "Account ID: " << account.id() << " " <<
				account.displayName() << " " << account.provider().id()
				 << " " << account.provider().name();

/*		QList<QString> settingsList = account.provider().settingsKeys();
		for (int j = 0; j < settingsList.length(); j++)
		{
			QString key = settingsList.at(j);
			qDebug() << "Settings key: " << key ;
		}*/
		qDebug() << "Email provider address: " << account.settingsProperty("email_address").toString();

		if (account.provider().id().compare("imapemail") == 0 ||
				account.provider().id().compare("popemail") == 0 ||
				account.provider().id().compare("activesync") == 0)
		{
			QVariantMap map;
			map.insert("id", account.id());
			map.insert("name", account.displayName());
			map.insert("email", account.settingsProperty("email_address").toString());
			retList.append(map);
		}
	}

	return retList;
}


QVariantList PIM::listEmails()
{
	QVariantList retList;

	QList<Account> list = accountService->accounts(Service::Messages);
	for (int i = 0; i < list.length(); i++)
	{
		Account account = list.at(i);
		qDebug() << "Account ID: " << account.id() << " " <<
				account.displayName() << " " << account.provider().id()
				 << " " << account.provider().name();

		qDebug() << "Email provider address: " << account.settingsProperty("email_address").toString();

		if (account.provider().id().compare("imapemail") == 0 ||
				account.provider().id().compare("popemail") == 0 ||
				account.provider().id().compare("activesync") == 0)
		{
			retList.append(account.settingsProperty("email_address").toString());
		}
	}

	return retList;
}


void PIM::sendEmail(int accountId, const QString &recepient, const QString &msg)
{
	MessageContact recipient = MessageContact(1, MessageContact::To,
			recepient, recepient);
	MessageBuilder *msgBuilder = MessageBuilder::create(accountId);
	msgBuilder->subject("PRISM Break Secure Email Message");
	msgBuilder->body(MessageBody::PlainText,
			QByteArray("Hi!\n\nI am sending you a secret message.\n\nYou need to download a free PRISM Break Secure Email application to read it.\n\nYou can find it at http://appworld.blackberry.com/webstore/content/49133888 and purchase a full version later.\n\n\n"));
	msgBuilder->addRecipient(recipient);
	qDebug() << "Send email MSG: " << msg;
	util->saveFile("./shared/misc/prism/message.prism", msg);
	Attachment att("text/plain", "message.prism", QUrl::fromLocalFile("./shared/misc/prism/message.prism"));
	bool ok;
	msgBuilder->addAttachment(att, &ok);
	qDebug() << "Attached? " << ok;
	Message message;
	message = *msgBuilder;
	messageService->send(accountId, message);
}


QVariantList PIM::listMessages()
{
	qDebug() << "List messages";
	QVariantList retList;

	QList<Account> list = accountService->accounts(Service::Messages);
	qDebug() << "Acount list size: " << list.length();

	for (int j = 0; j < list.length(); j++)
	{
		Account account = list.at(j);
		Provider provider = account.provider();

		if (account.provider().id().compare("imapemail") == 0 ||
				account.provider().id().compare("popemail") == 0 ||
				account.provider().id().compare("activesync") == 0)
		{
			MessageSearchFilter filter;
			filter.addSearchCriteria(SearchFilterCriteria::Subject, "PRISM Break Secure Email Message");
			filter.setLimit(1000);

//			MessageFilter filter;
//			filter.insert(MessageFilter::Quantity ,QVariant("1000") );

			QList<Message> results = messageService->searchLocal(account.id(), filter);
//			QList<Message> results = messageService->messages(account.id(), filter);
			qDebug() << "Account: " << account.id() << " Messages list size: " << results.length();

			for (int i = 0; i < results.length(); i++)
			{
				Message message = results.at(i);
				message = messageService->message(account.id(), message.id());
				qDebug() << "Subject: " << message.subject() << " Account name: " << account.displayName() <<
						" Body: " << message.body(MessageBody::PlainText).plainText() <<
						" Att count: " << message.attachmentCount();
				QString init = message.recipientAt(0).address();

				if ((message.subject().trimmed().compare("PRISM Break Secure Email Message") == 0) &&
						(message.attachmentCount() == 1))
				{
					Attachment att = message.attachments().at(0);
					qDebug() << "Attachment name: " << att.name() << " ID: "<< att.id() << " File: " << att.path().toString();
					QByteArray data = att.data();
					qDebug() << "Attachment data size: " << data.length();

					if (data.length() == 0)
					{
						messageService->downloadAttachment(account.id(), message.id(), att.id());
						continue;
					}

					QString json(data);
					qDebug() << "JSON: " << json;

					JsonDataAccess jda;
					QMap<QString,QVariant> map = jda.loadFromBuffer(json).toMap();
					qDebug() << "Map size: " <<  map.keys().length();
					for (int k = 0; k < map.keys().length(); k++)
					{
						qDebug() << "*" << map.keys().at(k);
					}
					qDebug() << "Contains key TYPE: " << map.contains("TYPE");

					if (map.contains("TYPE"))
					{
						QString type = map.value("TYPE").toString();
						qDebug() << "Type: " << type;
						QString xstatus;
						QString status = map.value("STATUS").toString();
						QString body = map.value("BODY").toString();
						qDebug() << "Body encrypted: " << body;

						QVariantMap retMap;

						if (type.compare("0") == 0)
						{
							status = crypto->decryptAESPwd(status, init);
							qDebug() << "Status sym: " << status;
						}
						else if (type.compare("1") == 0)
						{
							xstatus = map.value("XSTATUS").toString();;
							xstatus = crypto->decryptRSA(xstatus);
							status = crypto->decryptRSA(status);
							qDebug() << "Status asym: " << status << " " << xstatus;
						}

						if (status.compare("OK") == 0 || xstatus.compare("OK") == 0)
						{
							QDateTime msgTimestamp =  message.deviceTimestamp();
							msgTimestamp.setTimeSpec(Qt::UTC);
							retMap.insert("id", message.id());
							retMap.insert("accountId", message.accountId());
							retMap.insert("account", account.displayName());
							retMap.insert("provider", provider.name());
							qDebug() << "Account name: " << account.displayName();
							retMap.insert("timestamp", msgTimestamp.toLocalTime().toString());
							retMap.insert("dateTime", msgTimestamp.toLocalTime());
							retMap.insert("inbound", message.isInbound());
							retMap.insert("from", map.value("FROM").toString());
							retMap.insert("to", map.value("TO").toString());

							retMap.insert("uri", att.path().toString());
							retMap.insert("security", type);
							retMap.insert("hasAtt", map.value("BINARY").toList().size() > 0);

/*							if (type.compare("0") == 0)
							{
								body = crypto->decryptAESPwd(body, init);
								qDebug() << "Body sym: " << body;
							}
							else if (type.compare("1") == 0)
							{
								body = crypto->decryptRSA(body);
								qDebug() << "Body asym: " << body;
							}

							retMap.insert("body", body);*/
							retList.append(retMap);
						}
					}
				}
			}
		}
	}

	qDebug() << "The End ";

	return retList;
}


QVariantMap PIM::getMessage(int accountId, int messageId)
{
	QVariantMap retMap;
	qDebug() << "GET MESSAGE";

	Account account = accountService->account(accountId);
	qDebug() << "ACCOUNT NAME " << account.displayName();
	Provider provider = account.provider();
	qDebug() << "PROVIDER NAME " << provider.name();

	if (account.provider().id().compare("imapemail") == 0 ||
			account.provider().id().compare("popemail") == 0 ||
			account.provider().id().compare("activesync") == 0)
	{
		Message message = messageService->message(account.id(), message.id());
		QString init = message.recipientAt(0).address();

		if ((message.subject().trimmed().compare("PRISM Break Secure Email Message") == 0) &&
				(message.attachmentCount() == 1))
		{
			Attachment att = message.attachments().at(0);
			qDebug() << "Attachment name: " << att.name() << " ID: "<< att.id() << " File: " << att.path().toString();
			QByteArray data = att.data();
			qDebug() << "Attachment data size: " << data.length();
			QString json(data);
			qDebug() << "JSON: " << json;

			JsonDataAccess jda;
			QMap<QString,QVariant> map = jda.loadFromBuffer(json).toMap();
			qDebug() << "Map size: " <<  map.keys().length();
			for (int k = 0; k < map.keys().length(); k++)
			{
				qDebug() << "*" << map.keys().at(k);
			}
			qDebug() << "Contains key TYPE: " << map.contains("TYPE");

			if (map.contains("TYPE"))
			{
				QString type = map.value("TYPE").toString();
				qDebug() << "Type: " << type;
				QString status = map.value("STATUS").toString();
				QString body = map.value("BODY").toString();
				qDebug() << "Body encrypted: " << body;

				if (type.compare("0") == 0)
				{
					status = crypto->decryptAESPwd(status, init);
					qDebug() << "Status sym: " << status;
				}
				else if (type.compare("1") == 0)
				{
					status = crypto->decryptRSA(status);
					qDebug() << "Status asym: " << status;
				}

				if (status.compare("OK") == 0)
				{
					QDateTime msgTimestamp =  message.deviceTimestamp();
					msgTimestamp.setTimeSpec(Qt::UTC);
					retMap.insert("id", message.id());
					retMap.insert("init", init);
					retMap.insert("accountId", message.accountId());
					retMap.insert("account", account.displayName());
					retMap.insert("provider", provider.name());
					qDebug() << "Account name: " << account.displayName();
					retMap.insert("timestamp", msgTimestamp.toLocalTime().toString());
					retMap.insert("dateTime", msgTimestamp.toLocalTime());
					retMap.insert("inbound", message.isInbound());
					if (message.isInbound())
					{
						retMap.insert("from", message.recipientAt(0).address());
						retMap.insert("to", account.settingsProperty("email_address").toString());
					}
					else
					{
						retMap.insert("from", account.settingsProperty("email_address").toString());
						retMap.insert("to", message.recipientAt(0).address());
					}
					retMap.insert("uri", att.path().toString());
//							map.insert("json", json);
					retMap.insert("security", type);

					if (type.compare("0") == 0)
					{
						body = crypto->decryptAESPwd(body, init);
						qDebug() << "Body sym: " << body;
					}
					else if (type.compare("1") == 0)
					{
						body = crypto->decryptRSA(body);
						qDebug() << "Body asym: " << body;
					}

					retMap.insert("body", body);
				}
			}
		}
	}

	return retMap;
}


void PIM::onMessageAdded(bb::pim::account::AccountKey accountKey,
		bb::pim::message::ConversationKey conversationKey,
		bb::pim::message::MessageKey messageKey)
{
	qDebug() << "******** ON MSG ADDED ******** " << accountKey << " " << messageKey;

	Account account = accountService->account(accountKey);
	qDebug() << "ACCOUNT NAME " << account.displayName();
	Provider provider = account.provider();
	qDebug() << "PROVIDER NAME " << provider.name();

	if (account.provider().id().compare("imapemail") == 0 ||
			account.provider().id().compare("popemail") == 0 ||
			account.provider().id().compare("activesync") == 0)
	{
		Message message = messageService->message(accountKey, messageKey);
		QString init = message.recipientAt(0).address();

		if ((message.subject().trimmed().compare("PRISM Break Secure Email Message") == 0) &&
				(message.attachmentCount() == 1))
		{
			Attachment att = message.attachments().at(0);
			qDebug() << "Attachment name: " << att.name() << " ID: "<< att.id() << " File: " << att.path().toString();
			QByteArray data = att.data();
			qDebug() << "Attachment data size: " << data.length();
			QString json(data);
			qDebug() << "JSON: " << json;

			JsonDataAccess jda;
			QMap<QString,QVariant> map = jda.loadFromBuffer(json).toMap();
			qDebug() << "Map size: " <<  map.keys().length();
			for (int k = 0; k < map.keys().length(); k++)
			{
				qDebug() << "*" << map.keys().at(k);
			}
			qDebug() << "Contains key TYPE: " << map.contains("TYPE");

			if (map.contains("TYPE"))
			{
				QString type = map.value("TYPE").toString();
				qDebug() << "Type: " << type;
				QString status = map.value("STATUS").toString();
				QString body = map.value("BODY").toString();
				qDebug() << "Body encrypted: " << body;

				if (type.compare("0") == 0)
				{
					status = crypto->decryptAESPwd(status, init);
					qDebug() << "Status sym: " << status;
				}
				else if (type.compare("1") == 0)
				{
					status = crypto->decryptRSA(status);
					qDebug() << "Status asym: " << status;
				}

				if (status.compare("OK") == 0)
				{
					emit postMessageAdded(accountKey, messageKey);
				}
			}
		}
	}
}


void PIM::onMessageRemoved(bb::pim::account::AccountKey accountKey,
		bb::pim::message::ConversationKey conversationKey,
		bb::pim::message::MessageKey messageKey,
		QString sourceId)
{
	qDebug() << "******** ON MSG REMOVED ******** " << accountKey << " " << messageKey;
	emit postMessageRemoved(accountKey, messageKey);
}


} } // NAMESPACE
