#ifndef GENERALSCANMANAGER_HPP
#define GENERALSCANMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"
#include "addimatmapping.hpp"

class GeneralScanManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString barcodeValue READ getBarcodeValue NOTIFY barcodeValueChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool scanNotificationsActive READ getScanNotificationsActive WRITE setScanNotificationsActive NOTIFY scanNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)

public:
    explicit GeneralScanManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    QString getBarcodeValue() const;
    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getScanNotificationsActive() const;
    void setScanNotificationsActive(bool isActive);
    bool getHasDevice() const;

    QString getSettingsFavoriteAddress() const;
    void setSettingsFavoriteAddress(QString address);

    Q_INVOKABLE
    void setCurrentDevice(MyBluetoothDeviceInfo* myDevice);

    Q_INVOKABLE
    MyBluetoothDeviceInfo* currentDevice();

    Q_INVOKABLE
    bool isCurrentDeviceConnected();

    Q_INVOKABLE
    void prepareServices();

    Q_INVOKABLE
    void updateScanValue();

    Q_INVOKABLE
    void startScanNotifications();

    Q_INVOKABLE
    void stopScanNotifications();


signals:
    void barcodeValueChanged();
    void scanNotificationsActiveChanged();
    void featuresPreparedChanged();
    void settingsChanged();

    void hasDeviceChanged();

public slots:
    void onScanCharacteristicsDone();
    void onDisconnect();
    void onBarcodeChanged();
    void onScanSubscriptionsChanged();

private:
    BluetoothManager* mBluetoothManager;

    MyBluetoothDeviceInfo* mDeviceInfo;
    bool mDeviceIsConnected;

    MyBluetoothServiceInfo* mScanService;
    bool mScanServiceAvailable;
    bool mScanServiceConnected;
    MyBluetoothCharacteristic* mBarcode;
    bool mBarcodeAvailable;
    QString mBarcodeValue;

    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mScanNotificationsActive;
    bool mHasDevice;

    // SETTINGS
    QVariantMap mGeneralScanSettingsMap;
    QString mSettingsFavoriteAddress;
    void updateSettings();
};

#endif // GENERALSCANMANAGER_HPP
