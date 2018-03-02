#ifndef CRYPTO_HPP_
#define CRYPTO_HPP_


#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <sbreturn.h>
#include <huctx.h>
#include <hugse56.h>
#include <huseed.h>
#include <hurandom.h>
#include <huaes.h>
#include <hursa.h>
#include <QtCore/QtCore>
#include <bb/data/JsonDataAccess>
#include "config.hpp"


using namespace bb::data;


namespace pronic { namespace prism { // NAMESPACE


class Crypto : public QObject
{
    Q_OBJECT

public:

    Crypto(Configurator*_config);
    ~Crypto();

    static const char* AES_KEY;
    static const char* AES_IV;

    QString encryptAESImpl(QString text, sb_Key &key);
    QString decryptAESImpl(QString cipher, sb_Key &key);
    Q_INVOKABLE QString encryptAESExport(QString text);
    Q_INVOKABLE QString decryptAESExport(QString cipher);
    Q_INVOKABLE QString encryptAES(QString text);
    Q_INVOKABLE QString decryptAES(QString cipher);
    Q_INVOKABLE QString encryptAESPwd(QString text, QString init);
    Q_INVOKABLE QString decryptAESPwd(QString cipher, QString init);

    QString encryptRSAImpl(QString text, sb_PublicKey &key);
    QString decryptRSAImpl(QString cipher, sb_PrivateKey &key);
    Q_INVOKABLE QString encryptRSA(QString text);
    Q_INVOKABLE QString decryptRSA(QString cipher);
    Q_INVOKABLE QString encryptRSAOwn(QString text);

    Q_INVOKABLE void createAsymKeys(const QString &initValue);
    Q_INVOKABLE void setAsymKeys();

    Q_INVOKABLE void storeAsymKeys();

    Q_INVOKABLE QString exportKeys(const QString &password);
    Q_INVOKABLE QString exportOwnPublicKey();
    Q_INVOKABLE QString getOwnPublicKeyURL();
    Q_INVOKABLE void importForeignPublicKey(const QString &json);
    Q_INVOKABLE int importKeys(const QString &json, const QString &password);

    QString toHex(const QByteArray &in);
    bool fromHex(const QString in, QByteArray &toReturn);

    Q_INVOKABLE QString getPubKey();
    void setPubKey(const QString &key);

    Q_INVOKABLE QString createPackage(int type, const QString &body, const QString &from,
    		const QString &pubKey, const QString &recepient, const QString &attachment,
    		const QString &fileName);

private:

	static QMap<int, QString> errors;
	static void buildErrors();
	static QString getErrorText(int error);

	QByteArray getRandomBytes(QByteArray &buffer);
    void pad(QByteArray &in, int pad);
    bool removePadding(QByteArray &out, int pad);
    QString fromHexToQString(const QString &in);
    char nibble(char c);

	sb_GlobalCtx context;

	sb_RNGCtx randomContext;
	sb_RNGCtx rsaRandomContext;

	sb_Params aesParams;
	sb_Key aesKey;
	sb_Key passwordKey;

	sb_Params rsaParams;
	sb_PrivateKey privateKey;
	sb_PublicKey publicKey;
	sb_PublicKey foreignPublicKey;

	QString pubKey;

	Configurator *config;
};


} } // NAMESPACE


#endif
