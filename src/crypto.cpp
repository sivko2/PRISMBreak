
#include "crypto.hpp"


namespace pronic { namespace prism { // NAMESPACE

                              
const char* Crypto::AES_KEY = "*****put_your_sym_key_here******";
const char* Crypto::AES_IV = "1n1tV3ctpr1smbrk";


Crypto::Crypto(Configurator *_config)
{
	config = _config;

	int rc = SB_SUCCESS;

	rc = hu_GlobalCtxCreateDefault(&context);
	qDebug() << "hu_GlobalCtxCreateDefault RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RegisterSbg56(context);
	qDebug() << "hu_RegisterSbg56 RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RegisterSystemSeed(context);
	qDebug() << "hu_RegisterSystemSeed RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_InitSbg56(context);
	qDebug() << "hu_InitSbg56 RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RngDrbgCreate(HU_DRBG_HASH, 256, FALSE, 0, NULL, NULL, &randomContext,
			context);
	qDebug() << "hu_RngDrbgCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_AESParamsCreate(SB_AES_CBC, SB_AES_128_BLOCK_BITS, randomContext, NULL, &aesParams, context);
	qDebug() << "hu_AESParamsCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS, (const unsigned char*)AES_KEY, &aesKey, context);
	qDebug() << "hu_AESKeySet RC: " << rc << " " << Crypto::getErrorText(rc);

}


Crypto::~Crypto()
{
	int rc = SB_SUCCESS;

	rc = hu_AESKeyDestroy(aesParams, &aesKey, context);
	qDebug() << "hu_AESKeyDestroy RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_AESParamsDestroy(&aesParams, context);
	qDebug() << "hu_AESParamsDestroy RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAParamsDestroy(&rsaParams, context);
	qDebug() << "hu_RSAParamsDestroy RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RngDrbgDestroy(&randomContext, context);
	qDebug() << "hu_RngDrbgCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_GlobalCtxDestroy(&context);
	qDebug() << "hu_GlobalCtxDestroy RC: " << rc << " " << Crypto::getErrorText(rc);
}


void Crypto::setAsymKeys()
{
	int rc = SB_SUCCESS;

	rc = hu_RngCreate(config->getInitValues().length(), (const unsigned char *)(config->getInitValues().constData()),
			NULL, NULL, NULL, &rsaRandomContext, context);
	qDebug() << "hu_RngCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAParamsCreate(1024, rsaRandomContext, NULL, &rsaParams, context);
	qDebug() << "hu_RSAParamsCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	QByteArray eArray = QByteArray::fromBase64(config->getE().toAscii());
	qDebug() << "E len: " << eArray.length() << " " << (const unsigned char *)eArray.constData() << " " << config->getE();

	QByteArray nArray = QByteArray::fromBase64(config->getN().toAscii());
	qDebug() << "N len: " << nArray.length() << " " << (const unsigned char *)nArray.constData() << " " << config->getN();

	QByteArray dArray = QByteArray::fromBase64(config->getD().toAscii());
	qDebug() << "D len: " << dArray.length() << " " << (const unsigned char *)dArray.constData() << " " << config->getD();

	qDebug() << "SET ASYM KEYS BEFORE: " << privateKey << " " << publicKey;
	rc = hu_RSAKeySet(rsaParams, eArray.length(), (const unsigned char *)eArray.constData(),
			nArray.length(), (const unsigned char *)nArray.constData(),
			dArray.length(), (const unsigned char *)dArray.constData(),
			0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL,
			&privateKey, &publicKey, context);
	qDebug() << "SET ASYM KEYS After: " << privateKey << " " << publicKey;
}


void Crypto::createAsymKeys(const QString &initValue)
{
	int rc = SB_SUCCESS;

	rc = hu_RngCreate(initValue.length(), (const unsigned char *)initValue.constData(), NULL, NULL, NULL,
			&rsaRandomContext, context);
	qDebug() << "hu_RngCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAParamsCreate(1024, rsaRandomContext, NULL, &rsaParams, context);
	qDebug() << "hu_RSAParamsCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAKeyGen(rsaParams, 0, NULL, &privateKey, &publicKey, context);
	qDebug() << "hu_RSAKeyCreate RC: " << rc << " " << Crypto::getErrorText(rc);
}


QString Crypto::exportOwnPublicKey()
{
	return "{\"N\":\"" + config->getN() + "\",\"E\":\"" + config->getE() + "\"}";
}


QString Crypto::exportKeys(const QString &password)
{
	int rc = SB_SUCCESS;

	QString pwdKey = password;

	if (password.length() > 32)
	{
		pwdKey = pwdKey.mid(0, 32);
	}

	while (pwdKey.length() < 32)
	{
		pwdKey.append("*");
	}
	qDebug() << "Password key: " << pwdKey;

	rc = hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS, (const unsigned char*)pwdKey.constData(), &passwordKey, context);
	qDebug() << "hu_AESKeySet RC: " << rc << " " << Crypto::getErrorText(rc);
	qDebug() << config->getN() << " + " << config->getE() << " + " << config->getD();

	return "{\"STATUS\":\"" + encryptAESImpl(QString("OK"), passwordKey) + "\",\"N\":\"" +
			encryptAESImpl(config->getN(), passwordKey) + "\",\"E\":\"" +
			encryptAESImpl(config->getE(), passwordKey) + "\",\"D\":\"" +
			encryptAESImpl(config->getD(), passwordKey) + "\"}";
}


QString Crypto::createPackage(int type, const QString &body, const QString &from,
		const QString &sharePubKey, const QString &recepient, const QString &attachment,
		const QString &fileName)
{
	QString retVal;
	qDebug() << "Create package - type: " << type;
	qDebug() << "Create package - body: " << body;
	qDebug() << "Create package - from: " << from;
	qDebug() << "Create package - to: " << recepient;
	qDebug() << "Create package - share pub key: " << sharePubKey;

	QByteArray pk(exportOwnPublicKey().toUtf8());

	if (type == 0)
	{
		retVal.append("{\"STATUS\":\"");
		retVal.append(encryptAESPwd(QString("OK").toUtf8(), recepient));
		retVal.append("\",\"TYPE\":\"0\",\"VER\":\"1\",\"PUBKEY\":\"");
		retVal.append(toHex(pk));
		retVal.append("\",\"FROM\":\"");
		retVal.append(from);
		retVal.append("\",\"TO\":\"");
		retVal.append(recepient);
		retVal.append("\",\"BODY\":\"");
		retVal.append(encryptAESPwd(body, recepient));
		retVal.append("\",\"BINARY\":[");
		if (attachment.length() > 0)
		{
			retVal.append("{\"DATA\":\"");
			retVal.append(encryptAESPwd(attachment, recepient));
			retVal.append("\",\"FILENAME\":\"");
			retVal.append(encryptAESPwd(fileName, recepient));
			retVal.append("\"}");
		}
		retVal.append("]}");
		qDebug() << "Low security package: " << retVal;
	}
	else
	{
		setPubKey(sharePubKey);
		retVal.append("{\"STATUS\":\"");
		retVal.append(encryptRSA(QString("OK")));
		retVal.append("\",\"XSTATUS\":\"");
		retVal.append(encryptRSAOwn(QString("OK")));
		retVal.append("\",\"TYPE\":\"1\",\"VER\":\"1\",\"PUBKEY\":\"");
		retVal.append(toHex(pk));
		retVal.append("\",\"FROM\":\"");
		retVal.append(from);
		retVal.append("\",\"TO\":\"");
		retVal.append(recepient);
		retVal.append("\",\"BODY\":\"");
		retVal.append(encryptRSA(body));
		retVal.append("\",\"XBODY\":\"");
		retVal.append(encryptRSAOwn(body));
		retVal.append("\",\"BINARY\":[");
		if (attachment.length() > 0)
		{
			retVal.append("{\"DATA\":\"");
			retVal.append(encryptRSA(attachment));
			retVal.append("\",\"FILENAME\":\"");
			retVal.append(encryptRSA(fileName));
			retVal.append("\"}");
		}
		retVal.append("],");
		retVal.append("\"XBINARY\":[");
		if (attachment.length() > 0)
		{
			retVal.append("{\"DATA\":\"");
			retVal.append(encryptRSAOwn(attachment));
			retVal.append("\",\"FILENAME\":\"");
			retVal.append(encryptRSAOwn(fileName));
			retVal.append("\"}");
		}
		retVal.append("]}");
		qDebug() << "Hi security package: " << retVal;
	}

	return retVal;
}


int Crypto::importKeys(const QString &json, const QString &password)
{
	int rc = SB_SUCCESS;

	QString pwdKey = password;

	if (password.length() > 32)
	{
		pwdKey = pwdKey.mid(0, 32);
	}

	while (pwdKey.length() < 32)
	{
		pwdKey.append("*");
	}
	qDebug() << "Password key: " << pwdKey;

	rc = hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS, (const unsigned char*)pwdKey.constData(), &passwordKey, context);
	qDebug() << "hu_AESKeySet RC: " << rc << " " << Crypto::getErrorText(rc);
	qDebug() << config->getN() << " + " << config->getE() << " + " << config->getD();

	JsonDataAccess jda;

	QMap<QString,QVariant> map = jda.loadFromBuffer(json).toMap();
	qDebug() << "STATUS: " << map.value("STATUS").toString() << " " << decryptAESImpl(map.value("STATUS").toString(), passwordKey);
	if (QString::compare(decryptAESImpl(map.value("STATUS").toString(), passwordKey), "OK") == 0)
	{
		config->setN(decryptAESImpl(map.value("N").toString(), passwordKey));
		config->setE(decryptAESImpl(map.value("E").toString(), passwordKey));
		config->setD(decryptAESImpl(map.value("D").toString(), passwordKey));

		setAsymKeys();
		return 0;
	}
	return -1;
}


QString Crypto::getOwnPublicKeyURL()
{
	return toHex(exportOwnPublicKey().toUtf8());
}


void Crypto::importForeignPublicKey(const QString &json)
{
	JsonDataAccess jda;
	qDebug() << "JSON: " << json;
	QMap<QString,QVariant> map = jda.loadFromBuffer(json).toMap();
	QString n = map.value("N").toString();
	QString e = map.value("E").toString();

	int rc = SB_SUCCESS;

	QByteArray eArray = QByteArray::fromBase64(e.toAscii());
	qDebug() << "E len: " << eArray.length() << " " << (const unsigned char *)eArray.constData();

	QByteArray nArray = QByteArray::fromBase64(n.toAscii());
	qDebug() << "N len: " << nArray.length() << " " << (const unsigned char *)nArray.constData();

	qDebug() << "Objects: " << &rsaParams << " " << &foreignPublicKey << " " << &context;

	rc = hu_RngCreate(config->getInitValues().length(), (const unsigned char *)(config->getInitValues().constData()),
			NULL, NULL, NULL, &rsaRandomContext, context);
	qDebug() << "hu_RngCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAParamsCreate(1024, rsaRandomContext, NULL, &rsaParams, context);
	qDebug() << "hu_RSAParamsCreate RC: " << rc << " " << Crypto::getErrorText(rc);

	rc = hu_RSAKeySet(rsaParams, eArray.length(), (const unsigned char *)eArray.constData(),
			nArray.length(), (const unsigned char *)nArray.constData(),
			0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL,
			NULL, &foreignPublicKey, context);
	qDebug() << "hu_RSAKeySet RC: " << rc << " " << Crypto::getErrorText(rc);
}


QByteArray Crypto::getRandomBytes(QByteArray &buffer)
{
	int rc = SB_SUCCESS;

	rc = hu_RngGetBytes(randomContext, buffer.length(), (unsigned char *)buffer.data(), context);
	qDebug() << "getRandomBytes RC: " << rc << " " << Crypto::getErrorText(rc);

	return buffer;
}


QString Crypto::encryptRSAImpl(QString text, sb_PublicKey &key)
{
	QByteArray in(text.toUtf8());
	qDebug() << "##### IN length: " << in.length();
	pad(in, 128);

	int rc = SB_SUCCESS;

	rc = hu_RSAPublicEncrypt(rsaParams, key, (const unsigned char *)in.constData(),
			(unsigned char *)in.data(), context);
	qDebug() << "hu_RSAPublicEncrypt RC: " << rc << " " << Crypto::getErrorText(rc);

	if (rc != SB_SUCCESS)
	{
		return "";
	}

	return toHex(in);
}


QString Crypto::decryptRSAImpl(QString cipher, sb_PrivateKey &key)
{
	QByteArray in;

	if (!fromHex(cipher, in))
	{
		return "";
	}
	qDebug() << "##### DECRYPT IN length: " << in.length();

	int rc = SB_SUCCESS;

	rc = hu_RSAPrivateDecrypt(rsaParams, key, (const unsigned char *)in.constData(),
			(unsigned char *)in.data(), context);
	qDebug() << "hu_RSAPPrivateDecrypt RC: " << rc << " " << Crypto::getErrorText(rc);

	if (rc != SB_SUCCESS)
	{
		return "";
	}

	if (removePadding(in, 128))
	{
		QString plainText = QString::fromUtf8(in.constData(), in.length());
		return plainText;
	}

	return "";
}


QString Crypto::encryptRSA(QString text)
{
	return encryptRSAImpl(text, foreignPublicKey);
}


QString Crypto::decryptRSA(QString cipher)
{
	return decryptRSAImpl(cipher, privateKey);
}


QString Crypto::encryptRSAOwn(QString text)
{
	return encryptRSAImpl(text, publicKey);
}


QString Crypto::encryptAESPwd(QString text, QString init)
{
	int rc = SB_SUCCESS;

	QString pwdKey = init;

	if (init.length() > 32)
	{
		pwdKey = pwdKey.mid(0, 32);
	}

	while (pwdKey.length() < 32)
	{
		pwdKey.append("*");
	}
	qDebug() << "Init key: " << pwdKey;

	rc = hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS, (const unsigned char*)pwdKey.constData(), &passwordKey, context);
	qDebug() << "hu_AESKeySet RC: " << rc << " " << Crypto::getErrorText(rc);
	qDebug() << config->getN() << " + " << config->getE() << " + " << config->getD();

	return encryptAESImpl(text, passwordKey);
}


QString Crypto::encryptAES(QString text)
{
	return encryptAESImpl(text, aesKey);
}


QString Crypto::encryptAESExport(QString text)
{
	return encryptAESImpl(text, passwordKey);
}


QString Crypto::encryptAESImpl(QString text, sb_Key &key)
{
	QByteArray in(text.toUtf8());
	pad(in, 32);
	QByteArray out(in.length(), 0);

	int rc = SB_SUCCESS;

	rc = hu_AESEncryptMsg(aesParams, key, SB_AES_128_BLOCK_BYTES, (const unsigned char*)AES_IV, in.size(),
			(const unsigned char *)in.constData(), (unsigned char *)out.data(), context);
	qDebug() << "hu_AESEncryptMsg RC: " << rc << " " << Crypto::getErrorText(rc);

	if (rc == SB_SUCCESS)
	{
		return toHex(out);
	}

	return "";
}


QString Crypto::decryptAESPwd(QString cipher, QString init)
{
	int rc = SB_SUCCESS;

	QString pwdKey = init;

	if (init.length() > 32)
	{
		pwdKey = pwdKey.mid(0, 32);
	}

	while (pwdKey.length() < 32)
	{
		pwdKey.append("*");
	}
	qDebug() << "Init key: " << pwdKey;

	rc = hu_AESKeySet(aesParams, SB_AES_256_KEY_BITS, (const unsigned char*)pwdKey.constData(), &passwordKey, context);
	qDebug() << "hu_AESKeySet RC: " << rc << " " << Crypto::getErrorText(rc);
	qDebug() << config->getN() << " + " << config->getE() << " + " << config->getD();

	return decryptAESImpl(cipher, passwordKey);
}


QString Crypto::decryptAES(QString cipher)
{
	return decryptAESImpl(cipher, aesKey);
}


QString Crypto::decryptAESExport(QString cipher)
{
	return decryptAESImpl(cipher, passwordKey);
}


QString Crypto::decryptAESImpl(QString cipher, sb_Key &key)
{
	QByteArray in;

	if (!fromHex(cipher, in))
	{
		return "";
	}

	QByteArray out(in.length(), 0);

	int rc = SB_SUCCESS;
	rc = hu_AESDecryptMsg(aesParams, key, SB_AES_128_BLOCK_BYTES, (const unsigned char*)AES_IV, in.length(),
			(const unsigned char *) in.constData(), (unsigned char *) out.data(), context);
	qDebug() << "hu_AESDecryptMsg RC: " << rc << " " << Crypto::getErrorText(rc);

	if (rc == SB_SUCCESS)
	{
		if (removePadding(out, 32))
		{
			QString plainText = QString::fromUtf8(out.constData(), out.length());
			return plainText;
		}
	}

	return "";
}


QString Crypto::getPubKey()
{
	return pubKey;
}



void Crypto::setPubKey(const QString &key)
{
	qDebug() << "Starting setPubKey";
	pubKey = key;
	qDebug() << "Global pub key set";
	importForeignPublicKey(fromHexToQString(key));
	qDebug() << "Foreign pub key imported";
}


void Crypto::storeAsymKeys()
{
	int rc = SB_SUCCESS;

	size_t eLen;
	size_t nLen;
	size_t dLen;
	size_t pLen;
	size_t qLen;
	size_t dModPLen;
	size_t dModQLen;
	size_t qInvLen;

	rc = hu_RSAKeyGet(rsaParams, privateKey, publicKey, &eLen, NULL, &nLen, NULL, &dLen, NULL, &pLen,
			NULL, &qLen, NULL, &dModPLen, NULL, &dModQLen, NULL, &qInvLen, NULL, context);

	unsigned char *e = (unsigned char*)malloc(eLen * sizeof(unsigned char));
	unsigned char *n = (unsigned char*)malloc(nLen * sizeof(unsigned char));
	unsigned char *d = (unsigned char*)malloc(dLen * sizeof(unsigned char));
	unsigned char *p = (unsigned char*)malloc(pLen * sizeof(unsigned char));
	unsigned char *q = (unsigned char*)malloc(qLen * sizeof(unsigned char));
	unsigned char *dModPm1 = (unsigned char*)malloc(dModPLen * sizeof(unsigned char));
	unsigned char *dModQm1 = (unsigned char*)malloc(dModQLen * sizeof(unsigned char));
	unsigned char *qInvModP = (unsigned char*)malloc(qInvLen * sizeof(unsigned char));

	rc = hu_RSAKeyGet(rsaParams, privateKey, publicKey, &eLen, e, &nLen, n, &dLen, d, &pLen,
			p, &qLen, q, &dModPLen, dModPm1, &dModQLen, dModQm1, &qInvLen, qInvModP, context);

	qDebug() << eLen << " " << e << " " << nLen << " " << n << " " << dLen << " " << d <<
			" " << pLen << " " << p << " " << qLen << " " << q << " " << dModPLen << " " << dModPm1 << " "
			<< " " << dModQLen << " " << dModQm1 << " " << qInvLen << " " << qInvModP;

	QByteArray eArray((char *)e, eLen);
	qDebug() << "STEP 1" ;
	QString eStr(eArray.toBase64());
	qDebug() << "STEP 2" << eStr;
	config->setE(eStr);
	qDebug() << "STEP 3 " << config->getE();

	QByteArray nArray((char *)n, nLen);
	qDebug() << "STEP 1";
	QString nStr(nArray.toBase64());
	qDebug() << "STEP 2" << nStr;
	config->setN(nStr);
	qDebug() << "STEP 3 " << config->getN();

	QByteArray dArray((char *)d, dLen);
	qDebug() << "STEP 1";
	QString dStr(dArray.toBase64());
	qDebug() << "STEP 2" << dStr;
	config->setD(dStr);
	qDebug() << "STEP 3 " << config->getD();

	free(e);
	free(n);
	free(d);
	free(p);
	free(q);
	free(dModPm1);
	free(dModQm1);
	free(qInvModP);
}


void Crypto::pad(QByteArray &in, int pad)
{
	qDebug() << "##### Pad: pad " << pad << ", in length " << in.length() << ", modula " <<
			(in.length() % pad) << ", padLength " << (pad - (in.length() % pad));
	int padLength = pad - (in.length() % pad);
	for (int i = 0; i < padLength; ++i)
	{
		in.append((char)padLength);
	}
}


bool Crypto::removePadding(QByteArray &out, int pad)
{
	qDebug() << "##### Out length: " << out.length();
	char paddingLength = out[out.length() - 1];
	int paddingLen = (int)paddingLength;
	if (paddingLen == -128)
	{
		paddingLen = 128;
	}
	qDebug() << "##### Padding length: " << paddingLen;
	if (paddingLen < 1 || paddingLen > pad)
	{
		qDebug() << "##### Padding length < 1 or > " << pad;
		out.clear();
		return false;
	}
	if (paddingLen > out.length())
	{
		qDebug() << "##### Padding length > " << out.length();
		out.clear();
		return false;
	}
	qDebug() << "##### Removing padding";
	out.remove(out.length() - paddingLen, paddingLen);
	return true;
}


QString Crypto::toHex(const QByteArray &in)
{
	static char hexChars[] = "0123456789abcdef";

	const char *c = in.constData();
	QString toReturn;
	for (int i = 0; i < in.length(); ++i)
	{
		toReturn += hexChars[(c[i] >> 4) & 0xf];
		toReturn += hexChars[(c[i]) & 0xf];
	}

	return toReturn;
}


bool Crypto::fromHex(const QString in, QByteArray &toReturn)
{
	QString temp(in);
	temp.replace(" ","");
	temp.replace(":","");
	temp.replace(".","");
	QByteArray content(temp.toLocal8Bit());

	const char *c(content.constData());

	if (content.length() == 0 || ((content.length() % 2) != 0))
	{
		return false;
	}

	for (int i = 0; i < content.length(); i += 2)
	{
		char a = c[i];
		char b = c[i + 1];
		a = nibble(a);
		b = nibble(b);
		if (a < 0 || b < 0)
		{
			toReturn.clear();
			return false;
		}
		toReturn.append((a << 4) | b);
	}
	return true;
}


QString Crypto::fromHexToQString(const QString &in)
{
	QByteArray toReturn;
	fromHex(in, toReturn);
	return QString::fromUtf8(toReturn.constData(), toReturn.length());
}


char Crypto::nibble(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	else if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	return -1;
}

QMap<int,QString> Crypto::errors;

QString Crypto::getErrorText(int error)
{
	if (errors.size() == 0)
	{
		buildErrors();
	}

	if (errors.contains(error))
	{
		return QString("%1 (%2)").arg(errors[error]).arg(error);
	}

	return QString("[Unknown error: %1]").arg(error);
}

void Crypto::buildErrors()
{
	QMap<int, QString> &map(errors);

	map[0x0000] = "SB_SUCCESS";
	map[-1] = "Crypto objects are invalid";
	map[0xFFFE] = "SB_NOT_IMPLEMENTED";
	map[0xFFFF] = "SB_FAILURE";
	map[0xE101] = "SB_ERR_NULL_PARAMS";
	map[0xE102] = "SB_ERR_NULL_PARAMS_PTR";
	map[0xE103] = "SB_ERR_BAD_PARAMS";
	map[0xE104] = "SB_ERR_NULL_CONTEXT";
	map[0xE105] = "SB_ERR_NULL_CONTEXT_PTR";
	map[0xE106] = "SB_ERR_BAD_CONTEXT";
	map[0xE107] = "SB_ERR_NULL_RNG";
	map[0xE108] = "SB_ERR_NO_RNG";
	map[0xE109] = "SB_ERR_BAD_RNG_TYPE";
	map[0xE10A] = "SB_ERR_BAD_RNG_CONTEXT";
	map[0xE10B] = "SB_ERR_BAD_YIELD_CONTEXT";
	map[0xE10C] = "SB_ERR_NULL_KEY";
	map[0xE10D] = "SB_ERR_NULL_KEY_PTR";
	map[0xE10E] = "SB_ERR_BAD_KEY";
	map[0xE10F] = "SB_ERR_NULL_KEY_LEN";
	map[0xE110] = "SB_ERR_BAD_KEY_LEN";
	map[0xE111] = "SB_ERR_NULL_PRIVATE_KEY";
	map[0xE112] = "SB_ERR_BAD_PRIVATE_KEY";
	map[0xE113] = "SB_ERR_NULL_PRI_KEY_BUF";
	map[0xE114] = "SB_ERR_BAD_PRI_KEY_BUF_LEN";
	map[0xE115] = "SB_ERR_NULL_PUBLIC_KEY";
	map[0xE116] = "SB_ERR_BAD_PUBLIC_KEY";
	map[0xE117] = "SB_ERR_NULL_PUB_KEY_BUF";
	map[0xE118] = "SB_ERR_BAD_PUB_KEY_BUF_LEN";
	map[0xE119] = "SB_ERR_NULL_INPUT";
	map[0xE11A] = "SB_ERR_NULL_INPUT_LEN";
	map[0xE11B] = "SB_ERR_BAD_INPUT_LEN";
	map[0xE11C] = "SB_ERR_NULL_INPUT_BUF";
	map[0xE11D] = "SB_ERR_BAD_INPUT_BUF_LEN";
	map[0xE11E] = "SB_ERR_BAD_INPUT";
	map[0xE11F] = "SB_ERR_NULL_OUTPUT";
	map[0xE120] = "SB_ERR_NULL_OUTPUT_BUF";
	map[0xE121] = "SB_ERR_NULL_OUTPUT_BUF_LEN_PTR";
	map[0xE122] = "SB_ERR_NULL_OUTPUT_BUF_LEN";
	map[0xE123] = "SB_ERR_BAD_OUTPUT_BUF_LEN";
	map[0xE124] = "SB_ERR_NULL_ADDINFO";
	map[0xE125] = "SB_ERR_BAD_ALG";
	map[0xE126] = "SB_ERR_BAD_FLAG";
	map[0xE127] = "SB_ERR_NULL_BUFFER";
	map[0xE128] = "SB_ERR_NULL_LENGTH";
	map[0xE129] = "SB_ERR_BAD_LENGTH";
	map[0xE12A] = "SB_ERR_OVERFLOW";
	map[0xE12B] = "SB_ERR_NULL_HASH_INFO";
	map[0xE12C] = "SB_ERR_PRI_KEY_NOT_EXPORTABLE";
	map[0xE12D] = "SB_ERR_PUB_KEY_NOT_EXPORTABLE";
	map[0xE12E] = "SB_ERR_SYM_KEY_NOT_EXPORTABLE";
	map[0xE201] = "SB_ERR_NULL_EPHEM_PRI_KEY";
	map[0xE202] = "SB_ERR_BAD_EPHEM_PRI_KEY";
	map[0xE203] = "SB_ERR_NULL_EPHEM_PUB_KEY";
	map[0xE204] = "SB_ERR_BAD_EPHEM_PUB_KEY";
	map[0xE205] = "SB_ERR_NULL_REM_EPHEM_PUB_KEY";
	map[0xE206] = "SB_ERR_BAD_REM_EPHEM_PUB_KEY";
	map[0xE207] = "SB_ERR_NULL_REM_PUB_KEY";
	map[0xE208] = "SB_ERR_BAD_REM_PUB_KEY";
	map[0xE301] = "SB_ERR_NULL_SIGNATURE";
	map[0xE302] = "SB_ERR_NULL_SIGNATURE_LEN";
	map[0xE303] = "SB_ERR_BAD_SIGNATURE_LEN";
	map[0xE305] = "SB_ERR_NULL_S_VALUE";
	map[0xE306] = "SB_ERR_NULL_S_VALUE_LEN";
	map[0xE307] = "SB_ERR_BAD_S_VALUE_LEN";
	map[0xE308] = "SB_ERR_NULL_R_VALUE";
	map[0xE309] = "SB_ERR_NULL_R_VALUE_LEN";
	map[0xE30A] = "SB_ERR_BAD_R_VALUE_LEN";
	map[0xE30B] = "SB_ERR_BAD_HASH_TYPE";
	map[0xE501] = "SB_ERR_NULL_ORDER_INT";
	map[0xE502] = "SB_ERR_NULL_ORDER_INT_PTR";
	map[0xE503] = "SB_ERR_BAD_ORDER_INT";
	map[0xE504] = "SB_ERR_NULL_ECPOINT";
	map[0xE505] = "SB_ERR_NULL_ECPOINT_PTR";
	map[0xE506] = "SB_ERR_BAD_ECPOINT";
	map[0xE601] = "SB_ERR_NULL_IDLC_P";
	map[0xE602] = "SB_ERR_BAD_IDLC_P_LEN";
	map[0xE603] = "SB_ERR_BAD_IDLC_P";
	map[0xE604] = "SB_ERR_NULL_IDLC_Q";
	map[0xE605] = "SB_ERR_BAD_IDLC_Q_LEN";
	map[0xE606] = "SB_ERR_BAD_IDLC_Q";
	map[0xE607] = "SB_ERR_NULL_IDLC_G";
	map[0xE608] = "SB_ERR_BAD_IDLC_G_LEN";
	map[0xE609] = "SB_ERR_BAD_IDLC_G";
	map[0xE701] = "SB_ERR_BAD_PUB_EXP_LEN";
	map[0xE702] = "SB_ERR_NULL_RSA_N";
	map[0xE703] = "SB_ERR_BAD_RSA_N_LEN";
	map[0xE704] = "SB_ERR_BAD_RSA_N";
	map[0xE705] = "SB_ERR_NULL_RSA_E";
	map[0xE706] = "SB_ERR_BAD_RSA_E_LEN";
	map[0xE707] = "SB_ERR_BAD_RSA_E";
	map[0xE708] = "SB_ERR_NULL_RSA_D";
	map[0xE709] = "SB_ERR_BAD_RSA_D_LEN";
	map[0xE70A] = "SB_ERR_BAD_RSA_D";
	map[0xE70B] = "SB_ERR_NULL_RSA_P";
	map[0xE70C] = "SB_ERR_BAD_RSA_P_LEN";
	map[0xE70D] = "SB_ERR_BAD_RSA_P";
	map[0xE70E] = "SB_ERR_NULL_RSA_Q";
	map[0xE70F] = "SB_ERR_BAD_RSA_Q_LEN";
	map[0xE711] = "SB_ERR_BAD_RSA_Q";
	map[0xE712] = "SB_ERR_NULL_RSA_QINV";
	map[0xE713] = "SB_ERR_BAD_RSA_QINV_LEN";
	map[0xE714] = "SB_ERR_BAD_RSA_QINV";
	map[0xE715] = "SB_ERR_NULL_RSA_DP";
	map[0xE716] = "SB_ERR_BAD_RSA_DP_LEN";
	map[0xE717] = "SB_ERR_BAD_RSA_DP";
	map[0xE718] = "SB_ERR_NULL_RSA_DQ";
	map[0xE719] = "SB_ERR_BAD_RSA_DQ_LEN";
	map[0xE71A] = "SB_ERR_BAD_RSA_DQ";
	map[0xE71B] = "SB_ERR_RSA_CRT_NOT_AVAILABLE";
	map[0xE801] = "SB_ERR_BAD_MODE";
	map[0xE802] = "SB_ERR_BAD_ALGORITHM";
	map[0xE803] = "SB_ERR_BAD_KEY_PARITY";
	map[0xE804] = "SB_ERR_BAD_KEY_OPTION";
	map[0xE805] = "SB_ERR_BAD_NUM_KEYS";
	map[0xE806] = "SB_ERR_BAD_ROUNDS";
	map[0xE807] = "SB_ERR_NULL_IV";
	map[0xE808] = "SB_ERR_BAD_IV_LEN";
	map[0xE809] = "SB_ERR_WEAK_KEY";
	map[0xE80A] = "SB_ERR_BAD_BLOCK_LEN";
	map[0xE80B] = "SB_ERR_BAD_KEY_UNWRAP";
	map[0xE80C] = "SB_ERR_NO_MODE";
	map[0xE80D] = "SB_ERR_INVALID_MAC";
	map[0xE80E] = "SB_ERR_MAC_INVALID";
	map[0xE80F] = "SB_ERR_BAD_IV";
	map[0xE901] = "SB_ERR_BAD_DIGEST_LEN";
	map[0xE902] = "SB_ERR_BAD_MESSAGE_LEN";
	map[0xEA01] = "SB_ERR_RNG_BAD_DRBG_CONTEXT";
	map[0xEA02] = "SB_ERR_RNG_INVALID_HANDLE";
	map[0xEA03] = "SB_ERR_RNG_BAD_HANDLE";
	map[0xEA04] = "SB_ERR_RNG_NO_MORE_HANDLE";
	map[0xEA05] = "SB_ERR_RNG_SECURITY_STRENGTH_TOO_SMALL";
	map[0xEA06] = "SB_ERR_RNG_SECURITY_STRENGTH_NOT_SUPPORTED";
	map[0xEA07] = "SB_ERR_RNG_PREDICTIVE_RESISTANCE_NOT_SUPPORTED";
	map[0xEA08] = "SB_ERR_RNG_PERSONALIZATION_STRING_TOO_BIG";
	map[0xEA09] = "SB_ERR_RNG_ADDITIONAL_INPUT_TOO_BIG";
	map[0xEA0A] = "SB_ERR_RNG_REQUESTED_BYTES_TOO_BIG";
	map[0xEA0B] = "SB_ERR_RNG_REQUESTED_SECURITY_TOO_BIG";
	map[0xEA0C] = "SB_ERR_RNG_REQUESTED_HASH_DERIVE_TOO_BIG";
	map[0xEA0D] = "SB_ERR_RNG_RESEED_IS_REQUIRED";
	map[0xEA0E] = "SB_ERR_RNG_NULL_TIME_CALLBACK";
	map[0xEF01] = "SB_ERR_NULL_GLOBAL_CTX";
	map[0xEF02] = "SB_ERR_NULL_GLOBAL_CTX_PTR";
	map[0xEF03] = "SB_ERR_BAD_GLOBAL_CTX";
	map[0xF001] = "SB_FAIL_ALLOC";
	map[0xF002] = "SB_FAIL_KEYGEN";
	map[0xF003] = "SB_FAIL_LOCK";
	map[0xF004] = "SB_FAIL_UNLOCK";
	map[0xF005] = "SB_FAIL_NULL_PTR";
	map[0xF006] = "SB_FAIL_INVALID_PRIVATE_KEY";
	map[0xF007] = "SB_FAIL_CANNOT_LOAD_LIBRARY";
	map[0xF008] = "SB_FAIL_LIBRARY_DISABLED";
	map[0xF009] = "SB_FAIL_INTEGRITY";
	map[0xF00A] = "SB_FAIL_KAT";
	map[0xF00B] = "SB_FAIL_OPEN_FILE";
	map[0xF00C] = "SB_FAIL_READ_FILE";
	map[0xF00D] = "SB_FAIL_LIBRARY_ALREADY_INIT";
	map[0xF00E] = "SB_FAIL_LIBRARY_NOT_INIT";
	map[0xF501] = "SB_FAIL_ECIES_HMAC";
	map[0xF502] = "SB_FAIL_INVALID_SHARED_SECRET";
	map[0xF503] = "SB_FAIL_INVALID_SIGNATURE";
	map[0xF701] = "SB_FAIL_BAD_PADDING";
	map[0xF702] = "SB_FAIL_PKCS1_DECRYPT";
	map[0xFA01] = "SB_FAIL_RANDOM_GEN";
	map[0xFB01] = "SB_FAIL_DIVIDE_BY_ZERO";
	map[0xFB02] = "SB_FAIL_NO_INVERSE";
	map[0xFC01] = "SB_FAIL_NO_SOLUTION";
	map[0xFC02] = "SB_ERR_MODULUS_TOO_BIG";
	map[0xFC03] = "SB_ERR_MODULUS_TOO_SMALL";
	map[0xFE01] = "SB_FAIL_PRIME_GEN";
	map[0xFF00] = "SB_ERR_POINT_AT_INFINITY";
	map[0x3001] = "SB_ERR_NULL_PROVIDER";
	map[0x3002] = "SB_ERR_NULL_PROVIDER_PTR";
	map[0x3004] = "SB_ERR_NULL_SESSION";
	map[0x3005] = "SB_ERR_NULL_SESSION_PTR";
	map[0x3006] = "SB_ERR_BAD_SESSION";
	map[0x3007] = "SB_ERR_NOT_SUPPORTED";
	map[0x3008] = "SB_ERR_BAD_CIPHER_TYPE";
	map[0x3009] = "SB_ERR_BAD_MODE_TYPE";
	map[0x300A] = "SB_ERR_BAD_MAC_TYPE";
	map[0x300B] = "SB_ERR_BAD_ECDH_TYPE";
	map[0x300F] = "SB_ERR_BAD_PARAMETER";
	map[0x3010] = "SB_ERR_NULL_OID";
	map[0x3011] = "SB_ERR_BAD_INPUT_FORMAT";
	map[0x3012] = "SB_ERR_NULL_HANDLE";
	map[0x3013] = "SB_ERR_BAD_HANDLE";
	map[0x3014] = "SB_ERR_BAD_ALLOC_POLICY";
	map[0x3015] = "SB_ERR_ECC_NOT_SUPPORTED";
	map[0x3016] = "SB_ERR_ECC_CURVE_SECT163K1_NOT_SUPPORTED";
	map[0x3017] = "SB_ERR_ECC_CURVE_SECT163R2_NOT_SUPPORTED";
	map[0x3018] = "SB_ERR_ECC_CURVE_SECT233K1_NOT_SUPPORTED";
	map[0x3019] = "SB_ERR_ECC_CURVE_SECT233R1_NOT_SUPPORTED";
	map[0x301A] = "SB_ERR_ECC_CURVE_SECT239K1_NOT_SUPPORTED";
	map[0x301B] = "SB_ERR_ECC_CURVE_SECT283K1_NOT_SUPPORTED";
	map[0x301C] = "SB_ERR_ECC_CURVE_SECT283R1_NOT_SUPPORTED";
	map[0x301D] = "SB_ERR_ECC_CURVE_SECT409K1_NOT_SUPPORTED";
	map[0x301E] = "SB_ERR_ECC_CURVE_SECT409R1_NOT_SUPPORTED";
	map[0x301F] = "SB_ERR_ECC_CURVE_SECT571K1_NOT_SUPPORTED";
	map[0x3020] = "SB_ERR_ECC_CURVE_SECT571R1_NOT_SUPPORTED";
	map[0x3021] = "SB_ERR_ECC_CURVE_SECP160R1_NOT_SUPPORTED";
	map[0x3022] = "SB_ERR_ECC_CURVE_SECP192R1_NOT_SUPPORTED";
	map[0x3023] = "SB_ERR_ECC_CURVE_SECP224R1_NOT_SUPPORTED";
	map[0x3024] = "SB_ERR_ECC_CURVE_SECP256R1_NOT_SUPPORTED";
	map[0x3025] = "SB_ERR_ECC_CURVE_SECP384R1_NOT_SUPPORTED";
	map[0x3026] = "SB_ERR_ECC_CURVE_SECP521R1_NOT_SUPPORTED";
	map[0x3027] = "SB_ERR_ECC_CURVE_WTLS5_NOT_SUPPORTED";
	map[0x3028] = "SB_ERR_ECC_CURVE_WAPI1_NOT_SUPPORTED";
	map[0x3029] = "SB_ERR_ECC_CURVE_GBP320T1_NOT_SUPPORTED";
	map[0x302A] = "SB_ERR_ECC_CURVE_GBP320R1_NOT_SUPPORTED";
	map[0x302F] = "SB_ERR_ECC_BAD_CURVE";
	map[0x3030] = "SB_ERR_RSA_NOT_SUPPORTED";
	map[0x3031] = "SB_ERR_IDLC_NOT_SUPPORTED";
	map[0x3032] = "SB_ERR_AES_NOT_SUPPORTED";
	map[0x3033] = "SB_ERR_DES_NOT_SUPPORTED";
	map[0x3034] = "SB_ERR_ARC2_NOT_SUPPORTED";
	map[0x3035] = "SB_ERR_ARC4_NOT_SUPPORTED";
	map[0x3036] = "SB_ERR_RC5_NOT_SUPPORTED";
	map[0x3037] = "SB_ERR_AUTHENC_NOT_SUPPORTED";
	map[0x3040] = "SB_ERR_MD2_NOT_SUPPORTED";
	map[0x3041] = "SB_ERR_MD4_NOT_SUPPORTED";
	map[0x3042] = "SB_ERR_MD5_NOT_SUPPORTED";
	map[0x3043] = "SB_ERR_SHA1_NOT_SUPPORTED";
	map[0x3044] = "SB_ERR_SHA224_NOT_SUPPORTED";
	map[0x3045] = "SB_ERR_SHA256_NOT_SUPPORTED";
	map[0x3046] = "SB_ERR_SHA384_NOT_SUPPORTED";
	map[0x3047] = "SB_ERR_SHA512_NOT_SUPPORTED";
	map[0x3048] = "SB_ERR_AES_MMO_NOT_SUPPORTED";
	map[0x3050] = "SB_ERR_HMAC_MD2_NOT_SUPPORTED";
	map[0x3051] = "SB_ERR_HMAC_MD4_NOT_SUPPORTED";
	map[0x3052] = "SB_ERR_HMAC_MD5_NOT_SUPPORTED";
	map[0x3053] = "SB_ERR_HMAC_SHA1_NOT_SUPPORTED";
	map[0x3054] = "SB_ERR_HMAC_SHA224_NOT_SUPPORTED";
	map[0x3055] = "SB_ERR_HMAC_SHA256_NOT_SUPPORTED";
	map[0x3056] = "SB_ERR_HMAC_SHA384_NOT_SUPPORTED";
	map[0x3057] = "SB_ERR_HMAC_SHA512_NOT_SUPPORTED";
	map[0x3058] = "SB_ERR_MAC_XCBC_AES_NOT_SUPPORTED";
	map[0x3059] = "SB_ERR_MAC_CMAC_AES_NOT_SUPPORTED";
	map[0x3060] = "SB_ERR_RNG_NOT_SUPPORTED";
	map[0x3061] = "SB_ERR_KDF_ANSI_SHA1_NOT_SUPPORTED";
	map[0x3062] = "SB_ERR_KDF_IEEE_KDF1_SHA1_NOT_SUPPORTED";
	map[0x3063] = "SB_ERR_KDF_ANSI_SHA224_NOT_SUPPORTED";
	map[0x3064] = "SB_ERR_KDF_ANSI_SHA256_NOT_SUPPORTED";
	map[0x3065] = "SB_ERR_KDF_ANSI_SHA384_NOT_SUPPORTED";
	map[0x3066] = "SB_ERR_KDF_ANSI_SHA512_NOT_SUPPORTED";
	map[0x3067] = "SB_ERR_KDF_PKCS5_V1_MD2_NOT_SUPPORTED";
	map[0x3068] = "SB_ERR_KDF_PKCS5_V1_MD5_NOT_SUPPORTED";
	map[0x3069] = "SB_ERR_KDF_PKCS5_V1_SHA1_NOT_SUPPORTED";
	map[0x306A] = "SB_ERR_KDF_PKCS5_V2_SHA1_NOT_SUPPORTED";
	map[0x306B] = "SB_ERR_KDF_PKCS5_V2_SHA256_NOT_SUPPORTED";
	map[0x306C] = "SB_ERR_KDF_PKCS12_V1_SHA1_NOT_SUPPORTED";
	map[0x306D] = "SB_ERR_KDF_PKCS12_V1_SHA256_NOT_SUPPORTED";
	map[0x306E] = "SB_ERR_SEED_NOT_SUPPORTED";
	map[0x306F] = "SB_ERR_KDF_BAD_ALGORITHM";
	map[0x3070] = "SB_ERR_UNWRAP_FAILED";
	map[0x3071] = "SB_ERR_MULTI_DIGEST_EXCEEDED";
	map[0x3080] = "SB_ERR_IDLC_GROUP_IPSEC_1_NOT_SUPPORTED";
	map[0x3081] = "SB_ERR_IDLC_GROUP_IPSEC_2_NOT_SUPPORTED";
	map[0x3082] = "SB_ERR_IDLC_GROUP_IPSEC_5_NOT_SUPPORTED";
	map[0x3083] = "SB_ERR_IDLC_GROUP_WTLS_1_NOT_SUPPORTED";
	map[0x3084] = "SB_ERR_IDLC_GROUP_WTLS_2_NOT_SUPPORTED";
	map[0x3085] = "SB_ERR_IDLC_GROUP_IPSEC_14_NOT_SUPPORTED";
	map[0x3086] = "SB_ERR_IDLC_GROUP_IPSEC_15_NOT_SUPPORTED";
	map[0x3087] = "SB_ERR_IDLC_GROUP_IPSEC_16_NOT_SUPPORTED";
	map[0x3088] = "SB_ERR_IDLC_GROUP_IPSEC_17_NOT_SUPPORTED";
	map[0x3089] = "SB_ERR_IDLC_GROUP_IPSEC_18_NOT_SUPPORTED";
	map[0x308F] = "SB_ERR_IDLC_BAD_GROUP";
	map[0x3090] = "SB_ERR_KDF_NIST_ALT1_NOT_SUPPORTED";
	map[0x3091] = "SB_ERR_KDF_PKCS5_V2_SHA224_NOT_SUPPORTED";
	map[0x3092] = "SB_ERR_KDF_PKCS5_V2_SHA384_NOT_SUPPORTED";
	map[0x3093] = "SB_ERR_KDF_PKCS5_V2_SHA512_NOT_SUPPORTED";
	map[0x3094] = "SB_ERR_KDF_PKCS12_V1_SHA224_NOT_SUPPORTED";
	map[0x3095] = "SB_ERR_KDF_PKCS12_V1_SHA384_NOT_SUPPORTED";
	map[0x3096] = "SB_ERR_KDF_PKCS12_V1_SHA512_NOT_SUPPORTED";
	map[0x3097] = "SB_ERR_KS_NOT_SUPPORTED";
	map[0x3098] = "SB_ERR_ZMOD_CALC_NOT_SUPPORTED";
	map[0x3100] = "SB_ERR_CS_BASE";
	map[0x3200] = "SB_ERR_BS_BASE";
	map[0x3300] = "SB_ERR_PKCS11_VENDOR_BASE";
	map[0x3400] = "SB_ERR_CAC_BASE";
	map[0x3600] = "SB_ERR_OS_BASE";
	map[0x3700] = "SB_ERR_CGX_BASE";
	map[0x3800] = "SB_ERR_PKCS11_BASE";
	map[0x3A00] = "SB_ERR_I300_BASE";
	map[0x3B00] = "SB_ERR_WTP_BASE";
	map[0x3C00] = "SB_ERR_PQ_BASE";
	map[0x3D00] = "SB_ERR_ES_BASE";
};


} } // NAMESPACE
