#ifndef WAITERLOCKMANAGER_HPP
#define WAITERLOCKMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"
#include "addimatmapping.hpp"
#include <QTimer>

class WaiterLockManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int batteryLevelValue READ getBatteryLevelValue NOTIFY batteryLevelValueChanged)
    Q_PROPERTY(QString keyIdValue READ getKeyIdValue NOTIFY keyIdValueChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool keyNotificationsActive READ getKeyNotificationsActive WRITE setKeyNotificationsActive NOTIFY keyNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)

    Q_PROPERTY(QVariant mappingsList READ getMappings NOTIFY mappingsUpdated)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelInfo READ getSettingsBatteryLevelInfo NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelWarning READ getSettingsBatteryLevelWarning NOTIFY settingsChanged)
    // specific


public:
    explicit WaiterLockManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    int getBatteryLevelValue() const;
    QString getKeyIdValue() const;
    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getKeyNotificationsActive() const;
    void setKeyNotificationsActive(bool isActive);
    bool getHasDevice() const;

    QString getSettingsFavoriteAddress() const;
    void setSettingsFavoriteAddress(QString address);
    int getSettingsBatteryLevelInfo() const;
    int getSettingsBatteryLevelWarning() const;

    QVariant getMappings();
    void clearMappings();

    Q_INVOKABLE
    AddimatMapping* createOrGetMapping(QString addimatKey);

    Q_INVOKABLE
    void triggerMappingsDataUpdated();

    Q_INVOKABLE
    bool removeMapping(QString addimatKey);

    Q_INVOKABLE
    AddimatMapping* findFromMapping(QString addimatKey);

    Q_INVOKABLE
    void setBatteryValues(int info, int warning);

    Q_INVOKABLE
    void setCurrentDevice(MyBluetoothDeviceInfo* myDevice);

    Q_INVOKABLE
    MyBluetoothDeviceInfo* currentDevice();

    Q_INVOKABLE
    bool isCurrentDeviceConnected();

    Q_INVOKABLE
    void prepareServices();

    Q_INVOKABLE
    void updateBatteryLevel();

    Q_INVOKABLE
    void updateKeyValue();

    Q_INVOKABLE
    void startKeyNotifications();

    Q_INVOKABLE
    void stopKeyNotifications();

    Q_INVOKABLE
    MyBluetoothCharacteristic* getBatteryLevel();

signals:
    void batteryLevelValueChanged();
    void keyIdValueChanged();
    void keyNotificationsActiveChanged();
    void featuresPreparedChanged();

    void mappingsUpdated();

    void settingsChanged();

    void hasDeviceChanged();

public slots:
    void onBatteryCharacteristicsDone();
    void onKeyCharacteristicsDone();
    void onDisconnect();

    void onBatteryLevelChanged();
    void onKeyIdChanged();
    void onKeySubscriptionsChanged();

private slots:
    void onTottleTimedOut();

private:

    BluetoothManager* mBluetoothManager;

    MyBluetoothDeviceInfo* mDeviceInfo;
    bool mDeviceIsConnected;

    MyBluetoothServiceInfo* mBatteryService;
    bool mBatteryServiceAvailable;
    bool mBatteryServiceConnected;
    MyBluetoothCharacteristic* mBatteryLevel;
    bool mBatteryLevelAvailable;
    int mBatteryLevelValue;

    MyBluetoothServiceInfo* mKeyService;
    bool mKeyServiceAvailable;
    bool mKeyServiceConnected;
    MyBluetoothCharacteristic* mKeyId;
    bool mKeyIdAvailable;
    QString mKeyIdValue;
    QString mCurrentKey;
    QTimer* mTotterTimer;

    QList<QObject*> mMappings;
    QVariantList mappingsToVariantList();
    void mappingsFromVariantList(QVariantList& theList);

    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mKeyNotificationsActive;
    bool mHasDevice;

    // SETTINGS
    QVariantMap mWaiterLockSettingsMap;
    QString mSettingsFavoriteAddress;
    int mSettingsBatteryLevelInfo;
    int mSettingsBatteryLevelWarning;
    void updateSettings();

};

#endif // WAITERLOCKMANAGER_HPP
