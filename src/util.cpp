
#include "util.hpp"


namespace pronic { namespace prism { // NAMESPACE


Util::Util(Crypto *_crypto)
{
	crypto = _crypto;

	HardwareInfo info;
	hardwareInfo = info.modelName();
	hardwareNumber = info.modelNumber();
	pin = info.pin();
	qDebug() << "HW: " << hardwareInfo << " " << hardwareNumber << " " << pin;

	bb::PackageInfo pkgInfo;
	version = pkgInfo.version();
	qDebug() << "VER: " << version;

    DisplayInfo display;
    screenWidth = display.pixelSize().width();
    screenHeight = display.pixelSize().height();
    qDebug() << "Screen: " << screenWidth << "x" << screenHeight;

    PlatformInfo platform;
    osVersion = platform.osVersion();
    qDebug() << "OS: " << osVersion;
}


Util::~Util()
{
//	deletePrismFiles();
}


QString Util::getTimeInMillis()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	qint64 timeInMillis = dateTime.currentMSecsSinceEpoch();
	QString timeInMillisString = QString::number(timeInMillis);
	qDebug() << "Time in millis: " << timeInMillisString;
	return timeInMillisString;
}


QString Util::getHardwareInfo()
{
	return hardwareInfo;
}


QString Util::getVersion()
{
	return version;
}


QString Util::getOsVersion()
{
	return osVersion;
}


QString Util::getHardwareNumber()
{
	return hardwareNumber;
}


QString Util::getPIN()
{
	return pin;
}


int Util::getScreenWidth()
{
	return screenWidth;
}


int Util::getScreenHeight()
{
	return screenHeight;
}


void Util::saveFile(const QString &fileName, const QString &value)
{
	qDebug() << "SAVE";
	QFile file(fileName);
	qDebug() << "FILE EXISTS: " << file.exists();
	if (file.exists())
	{
		deleteFile(fileName);
	}
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);
	out << value;
	file.flush();
	file.close();
	qDebug() << "SAVED";
}


void Util::saveTemp(const QString &fileName, const QString &value)
{
	saveBinary("./shared/misc/prism/" + fileName, value);
}


void Util::saveBinary(const QString &fileName, const QString &value)
{
	qDebug() << "SAVE BIN";
//	QDir dirPath = QDir("./sharewith/pim/" + fileName);
	qDebug() << "Cut file name: " << fileName;
	QDir dirPath = QDir(fileName);
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE EXISTS: " << file.exists();
	if (file.exists())
	{
		deleteFile(fileName);
	}
	file.open(QIODevice::WriteOnly);
	QByteArray outBytes;
	crypto->fromHex(value, outBytes);
	qDebug() << "Size: " << value.length() << " -> " << outBytes.length();
	file.write(outBytes);
	file.flush();
	file.close();
	qDebug() << "SAVED";
}


QString Util::loadFile(const QString &fileName)
{
	qDebug() << "LOAD";
//	QDir dirPath = QDir("./shared/misc/" + fileName);
	qDebug() << "Cut file name: " << fileName.mid(7);
	QDir dirPath = QDir(fileName.mid(7));
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE EXISTS: " << file.exists();
	if (!file.exists())
	{
		return "";
	}
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&file);
	QString value = in.readAll();
	qDebug() << "VALUE: " << value;
	file.close();
	qDebug() << "LOADED";
	return value;
}


QString Util::loadTemp(const QString &fileName)
{
	qDebug() << "LOAD temp";
	return loadBinary("./shared/misc/prism/" + fileName);
}


QString Util::loadBinary(const QString &fileName)
{
	qDebug() << "LOAD bin";
//	QDir dirPath = QDir("./shared/misc/" + fileName);
	qDebug() << "Cut file name: " << fileName;
	QDir dirPath = QDir(fileName);
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE EXISTS: " << file.exists();
	if (!file.exists())
	{
		return "";
	}
	file.open(QIODevice::ReadOnly);
	qDebug() << "File opened - size: " << file.size();
	qint64 size = file.size();
	QByteArray fileData = file.readAll();
	qDebug() << "Data read - size: " << fileData.length();

	QString value = crypto->toHex(fileData);
	qDebug() << "VALUE: " << value;
	qDebug() << "Size: " << fileData.length() << " -> " << value.length();
	file.close();
	qDebug() << "LOADED";
	return value;
}


void Util::deleteFile(const QString &fileName)
{
	qDebug() << "DELETE";
	QDir dirPath = QDir("./shared/misc/prism/" + fileName);
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE " << dirPath.absolutePath() << " EXISTS: " << file.exists();
	file.remove();
	qDebug() << "DELETED";
}


void Util::deleteTempFiles()
{
	QDir dirPath = QDir("./shared/misc/");
	dirPath.mkdir("prism");

	dirPath = QDir("./shared/misc/prism/");
	dirPath.setFilter(QDir::Files);
	QStringList entries = dirPath.entryList();

	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString fileName = *entry;
		if (fileName.compare("prism.licence") != 0)
		{
			qDebug() << "File to delete: " << fileName;
			deleteFile(fileName);
		}
	}

}


void Util::saveKeys(const QString &value, const QString &filename)
{
	qDebug() << "SAVE KEYS";
	QDir dirPath = QDir("." + filename.mid(21));
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE EXISTS: " << file.exists() << " "  << file.fileName();
	qDebug() << "FILE OPENED: " << file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);
	out << value;
	file.close();
	qDebug() << "KEYS SAVED";
}


QString Util::loadKeys(const QString &filename)
{
	qDebug() << "LOAD KEYS";
	QDir dirPath = QDir("." + filename.mid(21));
	QFile file(dirPath.absolutePath());
	qDebug() << "FILE EXISTS: " << file.exists();
	if (!file.exists())
	{
		return "";
	}
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&file);
	QString value = in.readAll();
	qDebug() << "VALUE: " << value;
	file.close();
	qDebug() << "KEYS LOADED";
	return value;
}


} } // NAMESPACE
