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

#ifndef UDSUTIL_HPP_
#define UDSUTIL_HPP_

#include <QObject>
#include <QDateTime>
#include <bb/pim/unified/unified_data_source.h>
#include <bb/pim/account/Account.hpp>


namespace pronic { namespace prism { // NAMESPACE


class UDSUtil: public QObject
{
	Q_OBJECT

public:

	UDSUtil();
    void initialize();
    void initGenerators();
    virtual ~UDSUtil();

    int getItemId();
    QVariant getItem(QString sourceId, QString catName, QString accName);
    void replaceItem(QVariantMap itemMap, QString catName, QString accName);
    QVariantMap categories(QString accName);
    QVariantList& items();

    bool reloadHub();
    void resetReloadHub();
    bool removeAllAccounts();

    Q_INVOKABLE bool addAccount(QString accName, QString displayName, QString serverName, QString target,
    						QString icon, QString lockedIcon, QString composeIcon, QString desc,
    						bool compose, uds_account_type_t type);

    Q_INVOKABLE bool updateAccount(QString oldAccName, QString accName, QString target,
    								QString icon, QString lockedIcon, QString composeIcon, QString desc,
    								bool compose, uds_account_type_t type);
    Q_INVOKABLE bool removeAccount(QString accName);
    Q_INVOKABLE bool restoreAccount(QString accName, uds_account_key_t accId);
    Q_INVOKABLE uds_account_key_t getAccountId(int index);
    Q_INVOKABLE QString& getAccountName(int index);

    Q_INVOKABLE int addCategory(QString catName, QString accName);
    Q_INVOKABLE bool updateCategory(QString catName, QString newCatName, QString accName);
    Q_INVOKABLE bool removeCategory(QString catName, QString accName);
    Q_INVOKABLE bool restoreCategory(QString accName, QString catName, uds_category_key_t catId);
    Q_INVOKABLE bool restoreCategories(QString accName, QVariantMap categoriesMap);

    Q_INVOKABLE uds_category_key_t getCategoryId(QString accName, int index);
    Q_INVOKABLE QString& getCategoryName(QString accName, int index);

    Q_INVOKABLE int addItem(QString itmName, QString subject, QString mimeType, QString icon, bool read, QString syncId,
    					  long long timestamp, unsigned int contextState, bool notify,
    					  QString catName, QString accName);
    Q_INVOKABLE bool updateItem(QString srcID, QString itmName, QString subject, QString mimeType, QString icon, bool read, QString syncId,
    						  long long timestamp, unsigned int contextState, bool notify,
    						  QString catName, QString accName);
    Q_INVOKABLE bool removeItem(QString itmName, QString catName, QString accName);
    Q_INVOKABLE bool restoreLastItemId(int lastItemId);
    Q_INVOKABLE void restoreItems(QList<QVariant> items, QString accName);

    int addAccActionData(QString accName, QString action, QString title,
            QString invtarget, QString invtargettype, QString imgsource,
            QString mimetype,int placement);Q_INVOKABLE
    int updateAccActionData(QString accName, QString action, QString title,
            QString invtarget, QString invtargettype, QString imgsource,
            QString mimetype,int placement);Q_INVOKABLE
    int addItmActionData(QString accName, QString action, QString title,
            QString invtarget, QString invtargettype, QString imgsource,
            QString mimetype,int placement);Q_INVOKABLE
    int updateItmActionData(QString accName, QString action, QString title,
            QString invtarget, QString invtargettype, QString imgsource,
            QString mimetype,int placement);

private:

    bb::pim::account::Account createAccount();

    uds_context_t udsHandle;

    int accIdGenerator;
    int catIdGenerator;
    int itmIdGenerator;

    bool async;
    bool bIsRegistrationSuccess;
    bool bReloadHub;

    QMap<QString, int> mapAccount;
    QMap<QString, QMap<QString, int> > mapCategory;
    QMap<QString, QMap<QString, QMap<QString, QVariantMap> > > mapItem;
    QMap<QString, QMap<QString, QString> > mapAction;
    QMap<QString, QMap<QString, QString> > mapItemAction;

    QList<QVariant> itemMaps;

    char _assetPath[256];
};


} } // NAMESPACE


#endif /* UDSUTIL_HPP_ */
