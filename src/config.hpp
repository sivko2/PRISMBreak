#ifndef CONFIG_HPP_
#define CONFIG_HPP_


#include <QtCore/QtCore>
#include <QString>
#include <QVariant>
#include <QSettings>


namespace pronic { namespace prism { // NAMESPACE


class Configurator : public QObject
{
	Q_OBJECT

	public:

		Configurator();
		~Configurator();

		Q_PROPERTY(QString password READ getPassword WRITE setPassword);
		Q_PROPERTY(QString e READ getE WRITE setE);
		Q_PROPERTY(QString n READ getN WRITE setN);
		Q_PROPERTY(QString d READ getD WRITE setD);
		Q_PROPERTY(QString initValues READ getInitValues WRITE setInitValues);

		Q_INVOKABLE void read();
		Q_INVOKABLE void write();

		QString getPassword();
		void setPassword(QString);

		QString getE();
		void setE(QString);

		QString getN();
		void setN(QString);

		QString getD();
		void setD(QString);

		QString getInitValues();
		void setInitValues(QString);

	private:

		QString password;

		QString e;
		QString n;
		QString d;

		QString initValues;

		QSettings settings;
};


} } // NAMESPACE


#endif
