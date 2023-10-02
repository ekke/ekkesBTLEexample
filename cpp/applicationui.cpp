// ekke (Ekkehard Gentz) @ekkescorner
#include "applicationui.hpp"
#include "uiconstants.hpp"

#include <QDebug>
#include <QtQml>
#include <QGuiApplication>

#if defined (Q_OS_ANDROID)
#include <QtAndroidExtras/QAndroidJniObject>
#include <jni.h>
#endif

using namespace ekke::constants;

ApplicationUI::ApplicationUI(QObject *parent) : QObject(parent),  mBluetoothManager(new BluetoothManager(this)),
    mHeartRateManager(new HeartRateManager(this)), mWaiterLockManager(new WaiterLockManager(this)),
    mNfcReaderManager(new NfcReaderManager(this)), mGeneralScanManager(new GeneralScanManager(this)),
    mFeitianCardReaderManager(new FeitianCardReaderManager(this))
{
    // default theme is light
    mIsDarkTheme = false;
    // default primary color is Indigo
    mPrimaryPalette =  4;
    // default accent color is DeepOrange
    mAccentPalette = 15;
    //
    mBluetoothManager->init();
    mHeartRateManager->init(mBluetoothManager);
    mWaiterLockManager->init(mBluetoothManager);
    mNfcReaderManager->init(mBluetoothManager);
    mGeneralScanManager->init(mBluetoothManager);
    mFeitianCardReaderManager->init(mBluetoothManager);
}

void ApplicationUI::addContextProperty(QQmlContext *context)
{
    context->setContextProperty("btManager", mBluetoothManager);
    context->setContextProperty("heartRateManager", mHeartRateManager);
    context->setContextProperty("lockManager", mWaiterLockManager);
    context->setContextProperty("nfcManager", mNfcReaderManager);
    context->setContextProperty("scanManager", mGeneralScanManager);
    context->setContextProperty("cardReaderManager", mFeitianCardReaderManager);
}

// the best way to check if we're running inside Windows Subsystem for Android:
// see https://stackoverflow.com/questions/69985356
// Build.BRAND will be "Windows"
// Build.BOARD will be "windows"
// Build.MANUFACTURER will be "Microsoft Corporation"
// Build.MODEL will be "Subsystem for Android(TM)"
bool ApplicationUI::isWindowsSubSystemForAndroid()
{
    QString deviceModel = getDeviceModel();
    if(deviceModel.isNull() || deviceModel.isEmpty()) {
        return false;
    }
    if(QString::compare(deviceModel, "Subsystem for Android(TM);Microsoft Corporation", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

// // "STH100-2;BlackBerry" -- Model;Manufacturer
QString ApplicationUI::getDeviceModel()
{
#if defined (Q_OS_ANDROID)
    QAndroidJniObject theString = QAndroidJniObject::callStaticObjectMethod<jstring>("org/ekkescorner/utils/QAndroidUtils", "getDeviceModel");
    QString theModel = theString.toString();
    qDebug() << theModel;
    return theModel;
#elif defined (Q_OS_IOS)
    return "";
#else
    return "";
#endif
}

/* Change Theme Palette */
QStringList ApplicationUI::swapThemePalette()
{
    mIsDarkTheme = !mIsDarkTheme;
    if (mIsDarkTheme) {
        return darkPalette;
    }
    return lightPalette;
}

/* Get current default Theme Palette */
QStringList ApplicationUI::defaultThemePalette()
{
    if (mIsDarkTheme) {
        return darkPalette;
    }
    return lightPalette;
}

/* Get one of the Primary Palettes */
QStringList ApplicationUI::primaryPalette(const int paletteIndex)
{
    mPrimaryPalette = paletteIndex;
    switch (paletteIndex) {
    case 0:
        return materialRed;
    case 1:
        return materialPink;
    case 2:
        return materialPurple;
    case 3:
        return materialDeepPurple;
    case 4:
        return materialIndigo;
    case 5:
        return materialBlue;
    case 6:
        return materialLightBlue;
    case 7:
        return materialCyan;
    case 8:
        return materialTeal;
    case 9:
        return materialGreen;
    case 10:
        return materialLightGreen;
    case 11:
        return materialLime;
    case 12:
        return materialYellow;
    case 13:
        return materialAmber;
    case 14:
        return materialOrange;
    case 15:
        return materialDeepOrange;
    case 16:
        return materialBrown;
    case 17:
        return materialGrey;
    default:
        return materialBlueGrey;
    }
}

/* Get one of the Accent Palettes */
QStringList ApplicationUI::accentPalette(const int paletteIndex)
{
    mAccentPalette = paletteIndex;
    int currentPrimary = mPrimaryPalette;
    QStringList thePalette = primaryPalette(paletteIndex);
    mPrimaryPalette = currentPrimary;
    // we need: primaryColor, textOnPrimary, iconOnPrimaryFolder
    return QStringList{thePalette.at(1), thePalette.at(4), thePalette.at(7)};
}

/* Get Default Primary Palette */
QStringList ApplicationUI::defaultPrimaryPalette()
{
    return primaryPalette(mPrimaryPalette);
}

/* Get Default Accent Palette */
QStringList ApplicationUI::defaultAccentPalette()
{
    return accentPalette(mAccentPalette);
}


// ATTENTION
// iOS: NO SIGNAL
// Android: SIGNAL if leaving the App with Android BACK Key
// Android: NO SIGNAL if using HOME or OVERVIEW and THEN CLOSE from there
void ApplicationUI::onAboutToQuit()
{
    qDebug() << "On About to Q U I T Signal received";
    // do something
    mBluetoothManager->onAboutToQuit();
}

void ApplicationUI::onApplicationStateChanged(Qt::ApplicationState applicationState)
{
    qDebug() << "S T A T E changed into: " << applicationState;
    if(applicationState == Qt::ApplicationState::ApplicationSuspended) {
        // do something
        return;
    }
    if(applicationState == Qt::ApplicationState::ApplicationActive) {
        // do something
    }
}

