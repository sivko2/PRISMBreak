/*
 * Copyright (c) 2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QDir>
#include <bb/pim/account/Account.hpp>
#include <bb/pim/account/AccountService>
#include <bb/pim/account/Provider>
#include <bb/pim/account/Result>

#include "uds.hpp"


using namespace bb::pim::account;


namespace pronic { namespace prism { // NAMESPACE


UDSUtil::UDSUtil() :
       accIdGenerator(0), catIdGenerator(0), itmIdGenerator(0), async(false),
       bIsRegistrationSuccess(false), bReloadHub(false) {

	int retVal = -1;
    int serviceId = 0;
    int status = 0;

    if ((retVal = uds_init(&udsHandle, async)) == 0) {
        qDebug() << "UDSUtil: uds_init: successful\n";
        const char* serviceURL = "pronic_prismbreak_service";
        const char* libPath = "";

        // determine absolute path for hub assets
        QString tmpPath = QDir::current().absoluteFilePath("data");
    	qDebug() << "tmpPath: " << tmpPath;
        tmpPath = tmpPath.replace("/data", "/public/hubassets/");
    	qDebug() << "tmpPath: " << tmpPath;
        tmpPath = tmpPath.replace("/accounts/1000/appdata", "/apps");
    	qDebug() << "tmpPath: " << tmpPath;
        strcpy(_assetPath, (char *)tmpPath.toAscii().data());
    	qDebug() << "assetPath: " << _assetPath;

        if ((retVal = uds_register_client(udsHandle, serviceURL, libPath,
                _assetPath)) != 0) {
        	qDebug() << "UDSUtil: uds_register_client call failed with error " << retVal;
        }

        // not sure if this is better than the check below
//        if (retVal == UDS_REGISTRATION_NEW) {
//        	bReloadHub = true;
//        	qDebug() << "UDSUtil: uds_register_client call return code indicates Hub reload required.";
//        }

        retVal = uds_wait_for_response(udsHandle, 300000);
        if (async) {
            retVal = uds_get_response(udsHandle);
            if (retVal == 0) {
                serviceId = uds_get_service_id(udsHandle);
                status = uds_get_service_status(udsHandle);
            }
        } else {
            if (retVal == 0) {
                serviceId = uds_get_service_id(udsHandle);
                status = uds_get_service_status(udsHandle);
            }
        }
        qDebug() << "UDSUtil: uds_register_client call successful with " << serviceId << " as serviceId and " << status << " as status\n";
        if (retVal == 0)
            bIsRegistrationSuccess = true;

        if (status == UDS_REGISTRATION_NEW) {
        	bReloadHub = true;
        	qDebug() << "UDSUtil: uds_get_service_status call return code indicates Hub reload required.";
        }

        initGenerators();
    }
}

UDSUtil::~UDSUtil() {
}

void UDSUtil::initialize() {

	qDebug() << "UDSUtil: initialize ";
}

void UDSUtil::initGenerators() {
    accIdGenerator = 0;
    catIdGenerator = 0;
    itmIdGenerator = 0;
}


int UDSUtil::getItemId() {
	int itemId = itmIdGenerator;

    itmIdGenerator++;

    return itemId;
}

QVariant UDSUtil::getItem(QString sourceId, QString catName, QString accName)
{
	return mapItem[accName][catName][sourceId];
}

void UDSUtil::replaceItem(QVariantMap itemMap, QString catName, QString accName) {
	int index = 0;

	for(index = 0; index < itemMaps.size(); index++) {
		QVariantMap item = itemMaps.at(index).toMap();
		if (item["sourceId"].toString() == itemMap["sourceId"].toString()) {
			itemMaps.takeAt(index);
			itemMaps << itemMap;
	        mapItem[accName][catName][itemMap["sourceId"].toString()] = itemMap;
			break;
		}
	};
}

QVariantMap UDSUtil::categories(QString accName)
{
	QVariantMap categoriesMap;

    QMap<QString, int>::iterator it = mapCategory[accName].begin();
    while (it != mapCategory[accName].end()) {
        qDebug() << "UDSUtil: categories: " << it.key() << ":" << it.value();
    	categoriesMap[it.key()] = it.value();
        ++it;
    }

    qDebug() << "UDSUtil: categories: size: " << categoriesMap.size();

	return categoriesMap;
}

QList<QVariant>& UDSUtil::items()
{
    return itemMaps;
}

bool UDSUtil::reloadHub() {
    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: reloadHub: not registered yet\n";
        return false;
    }

    return bReloadHub;
}
void UDSUtil::resetReloadHub() {
    bReloadHub = false;
}

bool UDSUtil::removeAllAccounts() {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: removeAllAccounts: not registered yet\n";
        return retVal;
    }

    QMap<QString, int>::iterator i = mapAccount.begin();
    while (i != mapAccount.end()) {
        if (0 != (retVal = uds_account_removed(udsHandle, i.value()))) {
            qDebug() << "UDSUtil: removeAllAccounts: uds_account_removed failed with error %d\n" << retVal;
        }
        if (async) {
            retVal = uds_get_response(udsHandle);
        }
        i = mapAccount.erase(i);
    }

    return retVal;
}

bool UDSUtil::addAccount(QString accName, QString displayName, QString serverName, QString target,
						QString icon, QString lockedIcon, QString composeIcon, QString desc,
						bool compose, uds_account_type_t type)
{
    bool retVal = false;
    int retCode = -1;
    char accountName[255];
    char targetName[255];
    char iconName[255];
    char lockedIconName[255];
    char composeIconName[255];
    char description[255];
    QVariantMap itemMap;

    strcpy(accountName, accName.toAscii().data());
    strcpy(targetName, target.toAscii().data());
    strcpy(iconName, icon.toAscii().data());
    strcpy(lockedIconName, lockedIcon.toAscii().data());
    strcpy(composeIconName, composeIcon.toAscii().data());
    strcpy(description, desc.toAscii().data());

    //bool retVal = true;
    //int retCode;


    Account act;
	AccountService accountService;
	QString s;
	char *p = 0;
	//bool enterprise = false;


	QList<Account> allAccounts;
	do {
		allAccounts = accountService.accounts();
		qDebug() <<  "# accounts: " << allAccounts.length();

		if (allAccounts.length() > 0) {
		   for(int index = 0; index < allAccounts.length(); index++) {
			   Account account = allAccounts.at(index);

			   //char *accountName = (char *)account.displayName().toAscii().constData();
			   qDebug() << "UDSUtil: addAccount: account " << index << " : " << accountName << account.isServiceSupported(Service::Messages) << " : " << account.provider().id();

			   if (account.isServiceSupported(Service::Messages) && account.displayName() == displayName && account.provider().id() == "external") {
				   accIdGenerator = account.id();
				   qDebug() << "UDSUtil: addAccount: found existing account " << accIdGenerator;
			   }
			}
		}

		// in case service does not start up right away, retry until it becomes available
		//usleep(5000);
	} while (allAccounts.length() == 0);

	if (accIdGenerator == 0) {
		QString providerId("external");  // this maps to the filename of the provider's json file
		const Provider provider = accountService.provider(providerId);
		Account account(provider);
		account.setExternalData(true);
		account.setSettingsValue("server", serverName);
		// expose more of these settings at a later time
		//account.setSettingsValue("smtp_server","smtp.gmail.com");
		//account.setSettingsValue("password","walla123");
		//account.setSettingsValue("email_provider","gmail");
		account.setDisplayName(displayName);
		Result r = accountService.createAccount(provider.id(), account);
		if (!r.isSuccess())
		{
			s = r.message();
			p = s.toAscii().data();
			qDebug() << "account create failed !"<<s<<p<<account.isValid();
			act = Account();
		}
		else
		{
			// Was successful, so lets get the account id
			AccountKey id = account.id();
			qDebug() << "account created with id "<<id<<account.isValid();
			act = account;
		}

		if( act.isValid() ){
			accIdGenerator = act.id();
		}else{
			accIdGenerator = 0;
		}
    }

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (mapAccount.contains(accountName)) {
        qDebug() << "UDSUtil: addAccount: account " << accountName << " already exists\n";
        return retVal;
    }

    uds_account_data_t *accountData = uds_account_data_create();
    uds_account_data_set_id(accountData,accIdGenerator);
    uds_account_data_set_name(accountData,accountName);
    uds_account_data_set_icon(accountData, iconName);
    uds_account_data_set_locked_icon(accountData,lockedIconName);
    uds_account_data_set_description(accountData, description);
    uds_account_data_set_compose_icon(accountData, composeIconName);
    uds_account_data_set_supports_compose(accountData,compose);
    uds_account_data_set_type(accountData, type);
    uds_account_data_set_target_name(accountData, targetName);

    if (0 != (retCode = uds_account_added(udsHandle, accountData))) {
        qDebug() << "UDSUtil: uds_account_added failed with error " << retCode << " for account: " << accName << "\n";
        retVal = false;
    } else {
        mapAccount[accName] = uds_account_data_get_id(accountData);
        qDebug() << "UDSUtil: account: " << accName << ": added, id" << QString::number(accIdGenerator);
        accIdGenerator++;
        retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    //populateAccountSelector();
    uds_account_data_destroy(accountData);

    return retVal;
}

bool UDSUtil::updateAccount(QString oldAccName, QString accName, QString target,
							QString icon, QString lockedIcon, QString composeIcon, QString desc,
							bool compose, uds_account_type_t type)
{
	bool retVal = false;
	char oldAccountName[255];
	char accountName[255];
	char targetName[255];
	char iconName[255];
	char lockedIconName[255];
	char composeIconName[255];
	char description[255];
	QVariantMap itemMap;

	strcpy(oldAccountName, oldAccName.toAscii().data());
	strcpy(accountName, accName.toAscii().data());
	strcpy(targetName, target.toAscii().data());
	strcpy(iconName, icon.toAscii().data());
	strcpy(lockedIconName, lockedIcon.toAscii().data());
	strcpy(composeIconName, composeIcon.toAscii().data());
	strcpy(description, desc.toAscii().data());


    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(oldAccName)) {
        qDebug() << "UDSUtil: account: " << oldAccName << " does not exist\n";
        return retVal;
    }

    if (mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << "already exists\n";
        return retVal;
    }

    uds_account_data_t *accountData = uds_account_data_create();
    uds_account_data_set_id(accountData,accIdGenerator);
    uds_account_data_set_name(accountData,accountName);
    uds_account_data_set_icon(accountData, iconName);
    uds_account_data_set_locked_icon(accountData,lockedIconName);
    uds_account_data_set_description(accountData, description);
    uds_account_data_set_compose_icon(accountData, composeIconName);
    uds_account_data_set_supports_compose(accountData,compose);
    uds_account_data_set_type(accountData, type);
    uds_account_data_set_target_name(accountData, targetName);

    if (0 != uds_account_updated(udsHandle, accountData)) {
        qDebug() << "UDSUtil: uds_account_updated failed with error " << retVal << " for account: " << accName << "\n";
        retVal = false;
    } else {
        mapAccount[accName] = uds_account_data_get_id(accountData);
        QMap<QString, int> tempCat;
        tempCat = mapCategory.take(oldAccName);
        mapCategory[accName] = tempCat;
        QMap<QString, QMap<QString, QVariantMap> > tempItem;
        tempItem = mapItem.take(oldAccName);
        mapItem[accName] = tempItem;
        qDebug() << "UDSUtil: account: " << accName << " updated";
        retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    uds_account_data_destroy(accountData);

    return retVal;
}

bool UDSUtil::removeAccount(QString accName) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account  " << accName << " does not exist\n";
        return retVal;
    }

    if (0 != (uds_account_removed(udsHandle,
                    mapAccount.take(accName)))) {
        qDebug() << "UDSUtil: uds_account_removed for " << " + accName "
        		"+ " << " failed with error " << retVal << "\n";
        retVal = false;
    } else {
    	qDebug() << "UDSUtil: account " << accName << " removed";
    	retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    //populateAccountSelector();

    return retVal;
}

bool UDSUtil::restoreAccount(QString accName, uds_account_key_t accId) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    accIdGenerator = accId;
    mapAccount[accName] = accId;

    return retVal;
}

uds_account_key_t UDSUtil::getAccountId(int index)  {
    return mapAccount.values().at(index);
}

QString& UDSUtil::getAccountName(int index) {
    return (QString&)mapAccount.keys().at(index);
}

int UDSUtil::addCategory(QString catName, QString accName) {
    int retVal = -1;
    int retCode = 0;
    QByteArray bytes = catName.toAscii();
    char *categoryName = bytes.data();

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << " does not exist\n";
        return retVal;
    }

    if (mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " + catName + " already exists\n";
        return retVal;
    }

    catIdGenerator++;
    uds_category_data_t *category = uds_category_data_create();
    uds_category_data_set_id(category, catIdGenerator);
    uds_category_data_set_parent_id(category, 0);
    uds_category_data_set_account_id(category, mapAccount[accName]);
    uds_category_data_set_name(category, categoryName);

    if (0 != (retCode = uds_category_added(udsHandle, category))) {
        qDebug() << "UDSUtil: uds_category_added failed for  " << catName << " with error " << retCode << "\n";
        retVal = -1;
    } else {
        mapCategory[accName][catName] = uds_category_data_get_id(category);
        qDebug() << "UDSUtil: category: " << catName << ": added";
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
        if (retVal == 0) {
            retVal = catIdGenerator;
        } else {
        	retVal = 0;
        }
    }
    uds_category_data_destroy(category);

    return retVal;
}

bool UDSUtil::updateCategory(QString oldCatName, QString catName,
        QString accName) {
    bool retVal = true;
    int retCode = 0;
    QByteArray bytes = catName.toAscii();
    char *newCategoryName = bytes.data();

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account  " << accName << " does not exist\n";
        return retVal;
    }

    if (!mapCategory[accName][oldCatName]) {
        qDebug() << "UDSUtil: category " << oldCatName << " does not exist\n";
        return retVal;
    }

    if (mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " + catName + " already exists\n";
        return retVal;
    }

    uds_category_data_t *category = uds_category_data_create();
    uds_category_data_set_id(category,  mapCategory[accName].take(oldCatName));
    uds_category_data_set_parent_id(category, 0);
    uds_category_data_set_account_id(category, mapAccount[accName]);
    uds_category_data_set_name(category, newCategoryName);


    if (0 != (retCode = uds_category_updated(udsHandle, category))) {
        qDebug() << "UDSUtil: uds_category_updated failed for " << oldCatName << " with error "<< retCode << "\n";
        retVal = false;
    } else {
        mapCategory[accName][catName] = uds_category_data_get_id(category);
        QMap<QString, QVariantMap> tempItem;
        tempItem = mapItem[accName].take(oldCatName);
        mapItem[accName][newCategoryName] = tempItem;

        qDebug() << "UDSUtil: category: " << oldCatName << ": updated";

        retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    return retVal;
}

bool UDSUtil::removeCategory(QString catName, QString accName) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << " does not exist\n";
        return retVal;
    }

    if (!mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " << catName << "does not exist\n";
        return retVal;
    }

    if (0
            != (uds_category_removed(udsHandle, mapAccount[accName],
                    mapCategory[accName].take(catName)))) {
    	qDebug() << "UDSUtil: uds_category_removed failed with error " << retVal << "\n";
    	retVal = false;
    } else {
    	qDebug() << "UDSUtil: category: " << catName << ": removed";
    	retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    return retVal;
}

Q_INVOKABLE bool UDSUtil::restoreCategories(QString accName, QVariantMap categoriesMap) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    mapCategory[accName].clear();

    int intValue = 0;
    QVariantMap::iterator it = categoriesMap.begin();
    while (it != categoriesMap.end()) {
    	intValue = it.value().toInt();
        mapCategory[accName][it.key()] = intValue;
    	if (intValue > catIdGenerator) {
    		catIdGenerator = intValue;
    	}
        ++it;
    }

    return retVal;
}

Q_INVOKABLE bool UDSUtil::restoreCategory(QString accName, QString catName, uds_category_key_t catId) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    catIdGenerator = catId;
    mapCategory[accName][catName] = catId;

    return retVal;
}

uds_category_key_t UDSUtil::getCategoryId(QString accName, int index)  {
    return mapCategory[accName].values().at(index);
}

QString& UDSUtil::getCategoryName(QString accName, int index) {
    return (QString&)mapCategory[accName].keys().at(index);
}

int UDSUtil::addItem(QString itmName, QString subject, QString mimeType, QString icon, bool read, QString syncId,
					  long long timestamp, unsigned int contextState, bool notify,
					  QString catName, QString accName) {
    int retVal = -1;
    int retCode = -1;
    char sourceId[255];
    char itemName[255];
    char subjectDesc[255];
    char mime[255];
    char iconName[255];
    char syncIdS[255];
    QVariantMap itemMap;

    strcpy(itemName, itmName.toAscii().data());
    strcpy(subjectDesc, subject.toAscii().data());
    strcpy(mime, mimeType.toAscii().data());
    strcpy(iconName, icon.toAscii().data());
    strcpy(syncIdS, syncId.toAscii().data());

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << " does not exist\n";
        return retVal;
    }

    if (!mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " << catName << " does not exist\n";
        return retVal;
    }

    itmIdGenerator++;
    itoa(itmIdGenerator, sourceId, 10);
    QString sourceId1 = QString::fromStdString(sourceId);

    if (mapItem[accName][catName].contains(sourceId1)) {
        qDebug() << "UDSUtil: item " << sourceId1 << " already exists\n";
        return retVal;
    }

    uds_inbox_item_data_t *inbox_item = uds_inbox_item_data_create();

    qDebug() << "UDSUtil: item " << itemName << " : " << subjectDesc << " : " << iconName << " : " << mime << " : " << mapAccount[accName] << " : " << mapCategory[accName][catName] << " : " << sourceId << "\n";

    uds_inbox_item_data_set_name(inbox_item, itemName);
    uds_inbox_item_data_set_description(inbox_item, subjectDesc);
    uds_inbox_item_data_set_icon(inbox_item,iconName);
    uds_inbox_item_data_set_mime_type(inbox_item,mime);
    uds_inbox_item_data_set_user_data(inbox_item, sourceId);
    uds_inbox_item_data_set_unread_count(inbox_item, read ? 0 : 1);
    uds_inbox_item_data_set_total_count(inbox_item,1);
    uds_inbox_item_data_set_source_id(inbox_item,sourceId);
    uds_inbox_item_data_set_sync_id(inbox_item, (char *)syncIdS);
    uds_inbox_item_data_set_account_id(inbox_item, mapAccount[accName]);
    uds_inbox_item_data_set_category_id(inbox_item, mapCategory[accName][catName]);
    uds_inbox_item_data_set_timestamp(inbox_item,timestamp);
    uds_inbox_item_data_set_context_state(inbox_item, contextState);
    uds_inbox_item_data_set_notification_state(inbox_item,notify);

    itemMap["name"] = itmName;
    itemMap["description"] = subject;
    itemMap["mimeType"] = mimeType;
    itemMap["icon"] = icon;
    itemMap["readCount"] = read ? 0 : 1;
    itemMap["totalCount"] = 1;
    itemMap["syncId"] = syncId;
    itemMap["timestamp"] = timestamp;
    itemMap["contextState"] = contextState;
    itemMap["notify"] = notify;
    itemMap["sourceId"] = sourceId;
    itemMap["userData"] = sourceId;
    itemMap["accountId"] =  mapAccount[accName];
    itemMap["categoryId"] = mapCategory[accName][catName];

    if (0 != (retCode = uds_item_added(udsHandle, inbox_item))) {
        qDebug() << "UDSUtil: uds_item_added failed for " << itmName << " with error "<< retCode << "\n";
        retVal = 0;
    } else {
    	itemMaps << itemMap;
        mapItem[accName][catName][sourceId1] = itemMap;

        qDebug() << "UDSUtil: item: " << itemMap << ": added";
        retVal = itmIdGenerator;

        int index = 0;
    	for(index = 0; index < itemMaps.size(); index++) {
    		QVariantMap item = itemMaps.at(index).toMap();
    		if (item["sourceId"].toString() == itemMap["sourceId"].toString()) {
    			qDebug()  << "UDSUtil: item from cache: " << item["sourceId"].toString() << " : " << item;
    		}
    	};
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
        if (retVal == 0) {
            retVal = itmIdGenerator;
        } else {
        	retVal = 0;
        }
    }
    uds_inbox_item_data_destroy(inbox_item);

    return retVal;
}

bool UDSUtil::updateItem(QString srcId, QString itmName, QString subject, QString mimeType, QString icon, bool read, QString syncId,
						  long long timestamp, unsigned int contextState, bool notify,
						  QString catName, QString accName) {
    bool retVal = true;
    int retCode = -1;
    char sourceId[255];
    char itemName[255];
    char subjectDesc[255];
    char mime[255];
    char iconName[255];
    char syncIdS[255];

    strcpy(itemName, itmName.toAscii().data());
    strcpy(subjectDesc, subject.toAscii().data());
    strcpy(mime, mimeType.toAscii().data());
    strcpy(iconName, icon.toAscii().data());
    strcpy(syncIdS, syncId.toAscii().data());
    strcpy(sourceId, srcId.toAscii().data());

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << " does not exist\n";
        return retVal;
    }

    if (!mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " << catName << " does not exist\n";
        return retVal;
    }

    if (!mapItem[accName][catName].contains(srcId)) {
        qDebug() << "(Warning) UDSUtil: item " << srcId << " does not exist\n";
        //return retVal;
    }

    QByteArray bytes = srcId.toAscii();
    QVariantMap itemMap = mapItem[accName][catName].take(srcId);

    qDebug() << "UDSUtil: updateItem: item " << itemName << " : " << subjectDesc << " : " << iconName << " : " << mime << " : " << mapAccount[accName] << " : " << mapCategory[accName][catName] << " : " << srcId << "\n";

    uds_inbox_item_data_t *inbox_item = uds_inbox_item_data_create();

    uds_inbox_item_data_set_name(inbox_item, itemName);
    uds_inbox_item_data_set_description(inbox_item, subjectDesc);
    uds_inbox_item_data_set_icon(inbox_item,iconName);
    uds_inbox_item_data_set_mime_type(inbox_item,mime);
    uds_inbox_item_data_set_user_data(inbox_item, sourceId);
    uds_inbox_item_data_set_unread_count(inbox_item, read ? 0 : 1);
    uds_inbox_item_data_set_total_count(inbox_item,1);
    uds_inbox_item_data_set_source_id(inbox_item,sourceId);
    uds_inbox_item_data_set_sync_id(inbox_item, (char *)syncIdS);
    uds_inbox_item_data_set_account_id(inbox_item, mapAccount[accName]);
    uds_inbox_item_data_set_category_id(inbox_item, mapCategory[accName][catName]);
    uds_inbox_item_data_set_timestamp(inbox_item,timestamp);
    uds_inbox_item_data_set_context_state(inbox_item, contextState);
    uds_inbox_item_data_set_notification_state(inbox_item,notify);

    itemMap["name"] = itmName;
    itemMap["description"] = subject;
    itemMap["mimeType"] = mimeType;
    itemMap["icon"] = icon;
    itemMap["readCount"] = read ? 0 : 1;
    itemMap["totalCount"] = 1;
    itemMap["syncId"] = syncId;
    itemMap["timestamp"] = timestamp;
    itemMap["contextState"] = contextState;
    itemMap["notify"] = notify;
    itemMap["sourceId"] = srcId;
    itemMap["userData"] = srcId;
    itemMap["accountId"] =  mapAccount[accName];
    itemMap["categoryId"] = mapCategory[accName][catName];

    if (0 != (retCode = uds_item_updated(udsHandle, inbox_item))) {
        qDebug() << "UDSUtil: uds_item_updated failed for " << itmName << " with error " << retCode << "\n";
        retVal = false;
    } else {
        qDebug() << "UDSUtil: item " << srcId << " updated";

        replaceItem(itemMap, catName, accName);

        int index = 0;
    	for(index = 0; index < itemMaps.size(); index++) {
    		QVariantMap item = itemMaps.at(index).toMap();
    		if (item["sourceId"].toString() == itemMap["sourceId"].toString()) {
    			qDebug()  << "UDSUtil: item from cache: " << item["sourceId"].toString() << " : " << item;
    		}
    	};

        retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }
    uds_inbox_item_data_destroy(inbox_item);

    return retVal;
}

bool UDSUtil::removeItem(QString srcId, QString catName, QString accName) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        qDebug() << "UDSUtil: account " << accName << " does not exist\n";
        return retVal;
    }

    if (!mapCategory[accName][catName]) {
        qDebug() << "UDSUtil: category " << catName << " does not exist\n";
        return retVal;
    }

    if (!mapItem[accName][catName].contains(srcId)) {
        qDebug() << "UDSUtil: item " << srcId << " does not exist\n";
        return retVal;
    }

    QVariantMap itemMap = mapItem[accName][catName].take(srcId);
    QByteArray bytes = srcId.toAscii();

    if (0 != (uds_item_removed(udsHandle, mapAccount[accName], bytes.data()))) {
    	qDebug() << "UDSUtil: uds_item_removed for " << srcId << " failed with error " << retVal << "\n";
    	retVal = false;
    } else {
    	qDebug() << "UDSUtil: item: " << srcId << ": removed";

    	int removeCount = mapItem[accName][catName].remove(srcId);

    	qDebug() << "UDSUtil: " << removeCount << " items removed";

    	int index = 0;
    	for(index = 0; index < itemMaps.size(); index++) {
    		QVariantMap item = itemMaps.at(index).toMap();
    		if (item["sourceId"].toString() == itemMap["sourceId"].toString()) {
    			itemMaps.removeAt(index);
    			break;
    		}
    	};

    	retVal = true;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }

    return retVal;
}

bool UDSUtil::restoreLastItemId(int lastItemId) {
    bool retVal = true;

    if (!bIsRegistrationSuccess) {
        qDebug() << "UDSUtil: not registered yet\n";
        return retVal;
    }

    itmIdGenerator = lastItemId;

    return retVal;
}

void UDSUtil::restoreItems(QVariantList replaceItems, QString accName)
{
	itemMaps = replaceItems;

	foreach(QVariant item, itemMaps) {
		QVariantMap itemMap = item.toMap();
		QString itemCatName("");

	    QMap<QString, int>::iterator it = mapCategory[accName].begin();
	    while (it != mapCategory[accName].end()) {
	    	if (it.value() == itemMap["categoryId"]) {
	    		itemCatName = it.key();
	    		break;
	    	}
	    }

		mapItem[accName][itemCatName][itemMap["sourceId"].toString()] = itemMap;
	}
}

int UDSUtil::addAccActionData(QString accName1, QString action1,
        QString title1, QString invtarget1, QString invtargettype1,
        QString imgsource1, QString mimetype1, int placement) {

    int retVal = -1;
    //AccountActionData aItem;
    QByteArray bytes1 = action1.toAscii();
    char *action = bytes1.data();
    QByteArray bytes2 = invtarget1.toAscii();
    char *invtarget = bytes2.data();
    QByteArray bytes3 = invtargettype1.toAscii();
    char *invtargettype = bytes3.data();
    QByteArray bytes4 = imgsource1.toAscii();
    char *imgsource = bytes4.data();
    QByteArray bytes5 = mimetype1.toAscii();
    char *mimetype = bytes5.data();
    QByteArray bytes6 = title1.toAscii();
    char *title = bytes6.data();

    if (!bIsRegistrationSuccess) {
        printf("not registered yet\n");
        return retVal;
    }

    if (!mapAccount.contains(accName1)) {
        printf("account does not exist\n");
        return retVal;
    }

    if (mapAction[accName1].contains(action1)) {
        printf("action already exists\n");
        return retVal;
    }

    uds_account_action_data_t * account_action = uds_account_action_data_create();

    uds_account_action_data_set_action(account_action,action);
    uds_account_action_data_set_target(account_action,invtarget);
    uds_account_action_data_set_type(account_action,invtargettype);
    uds_account_action_data_set_title(account_action,title);
    uds_account_action_data_set_mime_type(account_action,mimetype);
    uds_account_action_data_set_image_source(account_action,imgsource);
    uds_account_action_data_set_placement(account_action,(uds_placement_type_t)placement);

    if (0 != (retVal = uds_register_account_action(udsHandle,
                    mapAccount[accName1], account_action))) {
        printf("uds_register_account_action failed with error %d\n", retVal);
    } else {
        mapAction[accName1][action1] = action1;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }
    uds_account_action_data_destroy(account_action);

    return retVal;

}

int UDSUtil::updateAccActionData(QString accName1, QString action1,
        QString title1, QString invtarget1, QString invtargettype1,
        QString imgsource1, QString mimetype1, int placement) {
    bool retVal = 0;
    QByteArray bytes1 = action1.toAscii();
    char *action = bytes1.data();
    QByteArray bytes2 = invtarget1.toAscii();
    char *invtarget = bytes2.data();
    QByteArray bytes3 = invtargettype1.toAscii();
    char *invtargettype = bytes3.data();
    QByteArray bytes4 = imgsource1.toAscii();
    char *imgsource = bytes4.data();
    QByteArray bytes5 = mimetype1.toAscii();
    char *mimetype = bytes5.data();
    QByteArray bytes6 = title1.toAscii();
    char *title = bytes6.data();

    if (!bIsRegistrationSuccess) {
        printf("not registered yet\n");
        return retVal;
    }

    if (!mapAccount.contains(accName1)) {
        printf("account does not exist\n");
        return retVal;
    }

    if (!mapAction[accName1].contains(action1)) {
        printf("action does not exist\n");
        return retVal;
    }

    uds_account_action_data_t * account_action = uds_account_action_data_create();
    uds_account_action_data_set_action(account_action,action);
    uds_account_action_data_set_target(account_action,invtarget);
    uds_account_action_data_set_type(account_action,invtargettype);
    uds_account_action_data_set_title(account_action,title);
    uds_account_action_data_set_image_source(account_action,imgsource);
    uds_account_action_data_set_mime_type(account_action,mimetype);
    uds_account_action_data_set_placement(account_action,(uds_placement_type_t)placement);

    if (0 != (retVal = uds_update_account_action(udsHandle,
                    mapAccount[accName1], account_action))) {
        printf("uds_update_account_action failed with error %d\n", retVal);
    } else {
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }
    uds_account_action_data_destroy(account_action);

    return retVal;
}

int UDSUtil::addItmActionData(QString accName, QString action,
        QString title, QString invtarget, QString invtargettype,
        QString imgsource, QString mimetype, int placement) {

    int retVal = -1;

    char actionName[255];
    char actionTitle[255];
    char invokeTarget[255];
    char invokeTargetType[255];
    char imageSource[255];
    char mime[255];

    strcpy(actionName, action.toAscii().data());
    strcpy(actionTitle, title.toAscii().data());
    strcpy(invokeTarget, invtarget.toAscii().data());
    strcpy(invokeTargetType, invtargettype.toAscii().data());
    strcpy(imageSource, imgsource.toAscii().data());
    strcpy(mime, mimetype.toAscii().data());

    if (!bIsRegistrationSuccess) {
        printf("not registered yet\n");
        return retVal;
    }

    if (!mapAccount.contains(accName)) {
        printf("account does not exist\n");
        return retVal;
    }

    if (mapItemAction[accName].contains(action)) {
        printf("action already exists\n");
        return retVal;
    }

    uds_item_action_data_t *item_action = uds_item_action_data_create();
    uds_item_action_data_set_action(item_action,actionName);
    uds_item_action_data_set_target(item_action,invokeTarget);
    uds_item_action_data_set_type(item_action,invokeTargetType);
    uds_item_action_data_set_title(item_action,actionTitle);
    uds_item_action_data_set_image_source(item_action,imageSource);
    uds_item_action_data_set_mime_type(item_action,mime);
    uds_item_action_data_set_placement(item_action,(uds_placement_type_t)placement);
    uds_item_action_data_set_context_mask(item_action,1);

    if (0 != (retVal = uds_register_item_context_action(udsHandle,
                    mapAccount[accName], item_action))) {
        printf("uds_register_item_context_action failed with error %d\n",
                retVal);
    } else {
        mapItemAction[accName][action] = action;
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }
    uds_item_action_data_destroy(item_action);

    return retVal;

}

int UDSUtil::updateItmActionData(QString accName1, QString action1,
        QString title1, QString invtarget1, QString invtargettype1,
        QString imgsource1, QString mimetype1, int placement) {
    int retVal = -1;
    QByteArray bytes1 = action1.toAscii();
    char *action = bytes1.data();
    QByteArray bytes2 = invtarget1.toAscii();
    char *invtarget = bytes2.data();
    QByteArray bytes3 = invtargettype1.toAscii();
    char *invtargettype = bytes3.data();
    QByteArray bytes4 = imgsource1.toAscii();
    char *imgsource = bytes4.data();
    QByteArray bytes5 = mimetype1.toAscii();
    char *mimetype = bytes5.data();
    QByteArray bytes6 = title1.toAscii();
    char *title = bytes6.data();

    if (!bIsRegistrationSuccess) {
        printf("not registered yet\n");
        return retVal;
    }

    if (!mapAccount.contains(accName1)) {
        printf("account does not exist\n");
        return retVal;
    }

    if (!mapItemAction[accName1].contains(action1)) {
        printf("action does not exist\n");
        return retVal;
    }

    uds_item_action_data_t *item_action = uds_item_action_data_create();
    uds_item_action_data_set_action(item_action,action);
    uds_item_action_data_set_target(item_action,invtarget);
    uds_item_action_data_set_type(item_action,invtargettype);
    uds_item_action_data_set_title(item_action,title);
    uds_item_action_data_set_image_source(item_action,imgsource);
    uds_item_action_data_set_mime_type(item_action,mimetype);
    uds_item_action_data_set_placement(item_action,(uds_placement_type_t)placement);
    uds_item_action_data_set_context_mask(item_action,1);

    if (0 != (retVal = uds_update_item_context_action(udsHandle,
                    mapAccount[accName1], item_action))) {
        printf("uds_update_item_context_action failed with error %d\n", retVal);
    } else {
    }

    if (async) {
        retVal = uds_get_response(udsHandle);
    }
    uds_item_action_data_destroy(item_action);

    return retVal;
}


} } // NAMESPACE

