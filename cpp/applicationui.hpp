// ekke (Ekkehard Gentz) @ekkescorner
#ifndef APPLICATIONUI_HPP
#define APPLICATIONUI_HPP

#include <QObject>
#include <QtQml>

#include "bt/bluetoothmanager.hpp"
#include "bt/heartratemanager.hpp"
#include "bt/waiterlockmanager.hpp"
#include "bt/nfcreadermanager.hpp"
#include "bt/generalscanmanager.hpp"
#include "bt/feitiancardreadermanager.hpp"

class ApplicationUI : public QObject
{
    Q_OBJECT

public:
     ApplicationUI(QObject *parent = 0);

     Q_INVOKABLE
     QStringList swapThemePalette();

     Q_INVOKABLE
     QStringList defaultThemePalette();

     Q_INVOKABLE
     QStringList primaryPalette(const int paletteIndex);

     Q_INVOKABLE
     QStringList accentPalette(const int paletteIndex);

     Q_INVOKABLE
     QStringList defaultPrimaryPalette();

     Q_INVOKABLE
     QStringList defaultAccentPalette();

     void addContextProperty(QQmlContext* context);

signals:

public slots:
     void onAboutToQuit();
     void onApplicationStateChanged(Qt::ApplicationState applicationState);

private:
     bool mIsDarkTheme;
     int mPrimaryPalette;
     int mAccentPalette;

     BluetoothManager* mBluetoothManager;
     HeartRateManager* mHeartRateManager;
     WaiterLockManager* mWaiterLockManager;
     NfcReaderManager* mNfcReaderManager;
     GeneralScanManager* mGeneralScanManager;
     FeitianCardReaderManager* mFeitianCardReaderManager;

};

#endif // APPLICATIONUI_HPP
