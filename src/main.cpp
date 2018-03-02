
#include <bb/cascades/Application>
#include <Qt/qdeclarativedebug.h>
#include <QDebug>
#include <QTimer>
#include <bb/system/SystemCredentialsPrompt>
#include <bb/system/SystemDialog>
#include <bb/system/SystemPrompt>
#include <bb/system/SystemToast>
#include <bb/system/SystemUiButton>
#include <bb/system/SystemUiError>
#include <bb/system/SystemUiInputField>
#include <bb/system/SystemUiInputMode>
#include <bb/system/SystemUiPosition>
#include <bb/system/SystemUiResult>
#include <bb/system/SystemListDialog>
#include <bb/platform/PaymentManager>
#include <bb/platform/PurchaseReply>
#include <bb/platform/PaymentConnectionMode>
#include <bb/cascades/Pickers/ContactPicker>
#include "applicationui.hpp"


using namespace bb::cascades;
using namespace pronic::prism;
using namespace bb::cascades::pickers;


Q_DECL_EXPORT int main(int argc, char **argv)
{
	// Creating app instance
	qDebug() << "########## Starting PRISM Break";
    Application app(argc, argv);

    // Setting payment mode
    PaymentManager::setConnectionMode(PaymentConnectionMode::Production);
//    PaymentManager::setConnectionMode(PaymentConnectionMode::Test);

    // Registring classes to use in QML
    qmlRegisterType<QTimer>("bb.core", 1, 2, "QTimer");
    qmlRegisterType<ContactPicker>("bb.cascades.pickers", 1, 2, "ContactPicker");
    qmlRegisterType<bb::platform::PaymentManager>("bb.system", 1, 2, "PaymentManager");
    qmlRegisterType<bb::system::SystemUiButton>("bb.system", 1, 2, "SystemUiButton");
    qmlRegisterType<bb::system::SystemListDialog>("bb.system", 1, 2, "SystemListDialog");
    qmlRegisterType<bb::system::SystemUiInputField>("bb.system", 1, 2, "SystemUiInputField");
    qmlRegisterType<bb::system::SystemToast>("bb.system", 1, 2, "SystemToast");
    qmlRegisterType<bb::system::SystemPrompt>("bb.system", 1, 2, "SystemPrompt");
    qmlRegisterType<bb::system::SystemCredentialsPrompt>("bb.system", 1, 2, "SystemCredentialsPrompt");
    qmlRegisterType<bb::system::SystemDialog>("bb.system", 1, 2, "SystemDialog");
    qmlRegisterUncreatableType<bb::platform::PaymentConnectionMode>("bb.system", 1, 2, "PaymentConnectionMode", "");
    qmlRegisterUncreatableType<bb::platform::PurchaseReply>("bb.system", 1, 2, "PurchaseReply", "");
    qmlRegisterUncreatableType<bb::system::SystemUiError>("bb.system", 1, 2, "SystemUiError", "");
    qmlRegisterUncreatableType<bb::system::SystemUiResult>("bb.system", 1, 2, "SystemUiResult", "");
    qmlRegisterUncreatableType<bb::system::SystemUiPosition>("bb.system", 1, 2, "SystemUiPosition", "");
    qmlRegisterUncreatableType<bb::system::SystemUiInputMode>("bb.system", 1, 2, "SystemUiInputMode", "");
    qmlRegisterUncreatableType<bb::system::SystemUiModality>("bb.system", 1, 2, "SystemUiModality", "");
    qRegisterMetaType<bb::system::SystemUiResult::Type>("bb::system::SystemUiResult::Type");

    // Creating UI app instance
    qDebug() << "Creating application UI...";
    ApplicationUI *appUI = new ApplicationUI(&app);

    // Start app loop
    qDebug() << "Starting app loop";
    return Application::exec();
}
