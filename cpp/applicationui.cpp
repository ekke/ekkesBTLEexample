// ekke (Ekkehard Gentz) @ekkescorner
#include "applicationui.hpp"
#include "uiconstants.hpp"

#include <QDebug>
#include <QtQml>
#include <QGuiApplication>

using namespace ekke::constants;

ApplicationUI::ApplicationUI(QObject *parent) : QObject(parent),  mBluetoothManager(new BluetoothManager(this)),
    mHeartRateManager(new HeartRateManager(this)), mWaiterLockManager(new WaiterLockManager(this)),
    mNfcReaderManager(new NfcReaderManager(this))
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
}

void ApplicationUI::addContextProperty(QQmlContext *context)
{
    context->setContextProperty("btManager", mBluetoothManager);
    context->setContextProperty("heartRateManager", mHeartRateManager);
    context->setContextProperty("lockManager", mWaiterLockManager);
    context->setContextProperty("nfcManager", mNfcReaderManager);
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
        break;
    case 1:
        return materialPink;
        break;
    case 2:
        return materialPurple;
        break;
    case 3:
        return materialDeepPurple;
        break;
    case 4:
        return materialIndigo;
        break;
    case 5:
        return materialBlue;
        break;
    case 6:
        return materialLightBlue;
        break;
    case 7:
        return materialCyan;
        break;
    case 8:
        return materialTeal;
        break;
    case 9:
        return materialGreen;
        break;
    case 10:
        return materialLightGreen;
        break;
    case 11:
        return materialLime;
        break;
    case 12:
        return materialYellow;
        break;
    case 13:
        return materialAmber;
        break;
    case 14:
        return materialOrange;
        break;
    case 15:
        return materialDeepOrange;
        break;
    case 16:
        return materialBrown;
        break;
    case 17:
        return materialGrey;
        break;
    default:
        return materialBlueGrey;
        break;
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

