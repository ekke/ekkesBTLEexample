#ifndef GENERALSCANMANAGER_HPP
#define GENERALSCANMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"

class GeneralScanManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString barcodeValue READ getBarcodeValue NOTIFY barcodeValueChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool scanNotificationsActive READ getScanNotificationsActive WRITE setScanNotificationsActive NOTIFY scanNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)
    // BT DEVICE INFO SERVICE
    Q_PROPERTY(QString manufacturerNameValue READ getManufacturerNameValue NOTIFY manufacturerNameValueChanged)
    Q_PROPERTY(QString modelNumberValue READ getModelNumberValue NOTIFY modelNumberValueChanged)
    Q_PROPERTY(QString serialNumberValue READ getSerialNumberValue NOTIFY serialNumberValueChanged)
    Q_PROPERTY(QString hardwareRevisionValue READ getHardwareRevisionValue NOTIFY hardwareRevisionValueChanged)
    Q_PROPERTY(QString firmwareRevisionValue READ getFirmwareRevisionValue NOTIFY firmwareRevisionValueChanged)
    Q_PROPERTY(QString softwareRevisionValue READ getSoftwareRevisionValue NOTIFY softwareRevisionValueChanged)
    Q_PROPERTY(QString systemIdValue READ getSystemIdValue NOTIFY systemIdValueChanged)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)
    Q_PROPERTY(QString settingsFavoriteName READ getSettingsFavoriteName WRITE setSettingsFavoriteName NOTIFY settingsNameChanged)

public:
    explicit GeneralScanManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    QString getBarcodeValue() const;
    QString getManufacturerNameValue() const;
    QString getModelNumberValue() const;
    QString getSerialNumberValue() const;
    QString getHardwareRevisionValue() const;
    QString getFirmwareRevisionValue() const;
    QString getSoftwareRevisionValue() const;
    QString getSystemIdValue() const;

    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getScanNotificationsActive() const;
    void setScanNotificationsActive(bool isActive);
    bool getHasDevice() const;

    QString getSettingsFavoriteAddress() const;
    void setSettingsFavoriteAddress(QString address);

    QString getSettingsFavoriteName() const;
    void setSettingsFavoriteName(QString name);

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
    void manufacturerNameValueChanged();
    void modelNumberValueChanged();
    void serialNumberValueChanged();
    void hardwareRevisionValueChanged();
    void firmwareRevisionValueChanged();
    void softwareRevisionValueChanged();
    void systemIdValueChanged();

    void scanNotificationsActiveChanged();
    void featuresPreparedChanged();
    void settingsChanged();
    void settingsNameChanged();

    void hasDeviceChanged();

public slots:
    void onScanCharacteristicsDone();
    void onDeviceInfoCharacteristicsDone();

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

    MyBluetoothServiceInfo* mDeviceInfoService;
    bool mDeviceInfoServiceAvailable;
    bool mDeviceInfoServiceConnected;
    MyBluetoothCharacteristic* mManufacturerName;
    bool mManufacturerNameAvailable;
    QString mManufacturerNameValue;
    MyBluetoothCharacteristic* mModelNumber;
    bool mModelNumberAvailable;
    QString mModelNumberValue;
    MyBluetoothCharacteristic* mSerialNumber;
    bool mSerialNumberAvailable;
    QString mSerialNumberValue;
    MyBluetoothCharacteristic* mHardwareRevision;
    bool mHardwareRevisionAvailable;
    QString mHardwareRevisionValue;
    MyBluetoothCharacteristic* mFirmwareRevision;
    bool mFirmwareRevisionAvailable;
    QString mFirmwareRevisionValue;
    MyBluetoothCharacteristic* mSoftwareRevision;
    bool mSoftwareRevisionAvailable;
    QString mSoftwareRevisionValue;
    MyBluetoothCharacteristic* mSystemId;
    bool mSystemIdAvailable;
    QString mSystemIdValue;


    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mScanNotificationsActive;
    bool mHasDevice;

    // SETTINGS
    QVariantMap mGeneralScanSettingsMap;
    QString mSettingsFavoriteAddress;
    QString mSettingsFavoriteName;
    void updateSettings();
};

#endif // GENERALSCANMANAGER_HPP
