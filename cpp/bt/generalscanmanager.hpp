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
    // BT DEVICE INFO SERVICE
    Q_PROPERTY(QString manufacturerName READ getManufacturerName NOTIFY manufacturerNameChanged)
    Q_PROPERTY(QString modelNumber READ getModelNumber NOTIFY modelNumberChanged)
    Q_PROPERTY(QString serialNumber READ getSerialNumber NOTIFY serialNumberChanged)
    Q_PROPERTY(QString hardwareRevision READ getHardwareRevision NOTIFY hardwareRevisionChanged)
    Q_PROPERTY(QString firmwareRevision READ getFirmwareRevision NOTIFY firmwareRevisionChanged)
    Q_PROPERTY(QString softwareRevision READ getSoftwareRevision NOTIFY softwareRevisionChanged)
    Q_PROPERTY(QString systemId READ getSystemId NOTIFY systemIdChanged)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)

public:
    explicit GeneralScanManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    QString getBarcodeValue() const;
    QString getManufacturerName() const;
    QString getModelNumber() const;
    QString getSerialNumber() const;
    QString getHardwareRevision() const;
    QString getFirmwareRevision() const;
    QString getSoftwareRevision() const;
    QString getSystemId() const;

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
    void manufacturerNameChanged();
    void modelNumberChanged();
    void serialNumberChanged();
    void hardwareRevisionChanged();
    void firmwareRevisionChanged();
    void softwareRevisionChanged();
    void systemIdChanged();

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
    QString mManufacturerName;
    QString mModelNumber;
    QString mSerialNumber;
    QString mHardwareRevision;
    QString mFirmwareRevision;
    QString mSoftwareRevision;
    QString mSystemId;


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
