#ifndef UTIL_HPP_
#define UTIL_HPP_


#include <QObject>
#include <QDateTime>
#include <QtGlobal>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QDataStream>
#include <QtCore/QtCore>
#include <bb/PackageInfo>
#include <bb/device/HardwareInfo>
#include <bb/device/DisplayInfo>
#include <bb/platform/PlatformInfo>
#include "crypto.hpp"


using namespace bb::platform;
using namespace bb::device;


namespace pronic { namespace prism { // NAMESPACE


class Util : public QObject
{
	Q_OBJECT

public:

	Util(Crypto *_crypto);
	~Util();

	Q_PROPERTY(QString hardwareInfo READ getHardwareInfo);
	Q_PROPERTY(QString hardwareNumber READ getHardwareNumber);
	Q_PROPERTY(int screenWidth READ getScreenWidth);
	Q_PROPERTY(int screenHeight READ getScreenHeight);
	Q_PROPERTY(QString version READ getVersion);
	Q_PROPERTY(QString osVersion READ getOsVersion);
	Q_PROPERTY(QString pin READ getPIN);

	Q_INVOKABLE QString getTimeInMillis();

	Q_INVOKABLE void saveFile(const QString &fileName, const QString &value);
	Q_INVOKABLE void saveBinary(const QString &fileName, const QString &value);
	Q_INVOKABLE QString loadFile(const QString &fileName);
	Q_INVOKABLE QString loadBinary(const QString &fileName);
	Q_INVOKABLE void deleteFile(const QString &fileName);
	Q_INVOKABLE void saveTemp(const QString &fileName, const QString &value);
	Q_INVOKABLE QString loadTemp(const QString &fileName);

	Q_INVOKABLE void saveKeys(const QString &value, const QString &filename);
	Q_INVOKABLE QString loadKeys(const QString &filename);

	void deleteTempFiles();

	QString getHardwareInfo();
	QString getHardwareNumber();
	int getScreenWidth();
	int getScreenHeight();
	QString getVersion();
	QString getOsVersion();
    QString getPIN();

private:

	QString hardwareInfo;
	QString hardwareNumber;
	QString version;
	int screenWidth;
	int screenHeight;
	QString osVersion;
	QString pin;

	Crypto *crypto;

};


} } // NAMESPACE


#endif
