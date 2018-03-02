
#include "config.hpp"


namespace pronic { namespace prism { // NAMESPACE


Configurator::Configurator()
{
	read();
}


Configurator::~Configurator()
{
}


void Configurator::read()
{
//	qDebug() << "Password null?: " << settings.value("password").isNull();
	if (!settings.value("password").isNull())
	{
		password = settings.value("password").toString();
		qDebug() << "##### Password read: " << settings.value("password").toString() << "/" << password;
	}

//	qDebug() << "E null?: " << settings.value("e").isNull();
	if (!settings.value("e").isNull())
	{
		e = settings.value("e").toString();
//		qDebug() << "E read: " << settings.value("e").toString() << "/" << e;
	}

//	qDebug() << "N null?: " << settings.value("n").isNull();
	if (!settings.value("n").isNull())
	{
		n = settings.value("n").toString();
//		qDebug() << "N read: " << settings.value("n").toString() << "/" << n;
	}

//	qDebug() << "D null?: " << settings.value("d").isNull();
	if (!settings.value("d").isNull())
	{
		d = settings.value("d").toString();
//		qDebug() << "D read: " << settings.value("d").toString() << "/" << d;
	}

//	qDebug() << "InitValues null?: " << settings.value("initValues").isNull();
	if (!settings.value("initValues").isNull())
	{
		initValues = settings.value("initValues").toString();
		qDebug() << "InitValues read: " << settings.value("initValues").toString() << "/" << initValues;
	}

}


void Configurator::write()
{
		settings.setValue("password", password);
		settings.setValue("e", e);
		settings.setValue("n", n);
		settings.setValue("d", d);
		settings.setValue("initValues", initValues);
		qDebug() << "Settings saved";
}


QString Configurator::getPassword()
{
//	read();
	return password;
}


void Configurator::setPassword(QString _password)
{
	password = _password;
	settings.setValue("password", password);
	write();
}


QString Configurator::getE()
{
//	read();
	return e;
}


void Configurator::setE(QString _e)
{
	e = _e;
	settings.setValue("e", e);
	write();
}


QString Configurator::getN()
{
//	read();
	return n;
}


void Configurator::setN(QString _n)
{
	n = _n;
	settings.setValue("n", n);
	write();
}


QString Configurator::getD()
{
//	read();
	return d;
}


void Configurator::setD(QString _d)
{
	d = _d;
	settings.setValue("d", d);
	write();
}


QString Configurator::getInitValues()
{
//	read();
	return initValues;
}


void Configurator::setInitValues(QString _initValues)
{
	initValues = _initValues;
	settings.setValue("initValues", initValues);
	write();
}


} } // NAMESPACE
