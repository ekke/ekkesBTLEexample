#ifndef HEARTRATEMANAGER_HPP
#define HEARTRATEMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"
#include "heartratemessages.hpp"

class HeartRateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int batteryLevelValue READ getBatteryLevelValue NOTIFY batteryLevelValueChanged)
    Q_PROPERTY(bool batteryLevelNotificationsActive READ getBatteryLevelNotificationsActive WRITE setBatteryLevelNotificationsActive NOTIFY batteryLevelNotificationsActiveChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool messageReadNotificationsActive READ getMessageReadNotificationsActive WRITE setMessageReadNotificationsActive NOTIFY messageReadNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)

    Q_PROPERTY(QVariant messagesList READ getMessages NOTIFY messagesUpdated)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelInfo READ getSettingsBatteryLevelInfo NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelWarning READ getSettingsBatteryLevelWarning NOTIFY settingsChanged)


public:
    explicit HeartRateManager(QObject *parent = 0);

    void init(BluetoothManager* bluetoothManager);

    int getBatteryLevelValue() const;
    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getMessageReadNotificationsActive() const;
    void setMessageReadNotificationsActive(bool isActive);
    bool getBatteryLevelNotificationsActive() const;
    void setBatteryLevelNotificationsActive(bool isActive);
    bool getHasDevice() const;

    QString getSettingsFavoriteAddress() const;
    void setSettingsFavoriteAddress(QString address);
    int getSettingsBatteryLevelInfo() const;
    int getSettingsBatteryLevelWarning() const;

    Q_INVOKABLE
    void setBatteryValues(int info, int warning);

    QVariant getMessages();

    Q_INVOKABLE
    void clearMessages();

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
    void updateMessageValue();

    Q_INVOKABLE
    void startMessageReadNotifications();

    Q_INVOKABLE
    void stopMessageReadNotifications();

    Q_INVOKABLE
    void startBatteryLevelNotifications();

    Q_INVOKABLE
    void stopBatteryLevelNotifications();

    Q_INVOKABLE
    MyBluetoothCharacteristic* getBatteryLevel();

signals:
    void batteryLevelValueChanged();
    void messageValueChanged();
    void messageReadNotificationsActiveChanged();
    void batteryLevelNotificationsActiveChanged();

    void featuresPreparedChanged();

    void messagesUpdated();

    void settingsChanged();

    void hasDeviceChanged();

public slots:
    void onBatteryCharacteristicsDone();
    void onMessageCharacteristicsDone();

    void onDisconnect();

    void onBatteryLevelChanged();
    void onMessageChanged();
    void onMessageSubscriptionsChanged();
    void onBatterySubscriptionsChanged();

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

    MyBluetoothServiceInfo* mMessageService;
    bool mMessageServiceAvailable;
    bool mMessageServiceConnected;
    MyBluetoothCharacteristic* mMessage;
    bool mMessageAvailable;
    QList<QObject*> mMessages;
    int mLastHeartRate;

    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mMessageReadNotificationsActive;
    bool mBatteryLevelNotificationsActive;
    bool mHasDevice;

    // SETTINGS
    QVariantMap mHeartRateSettingsMap;
    QString mSettingsFavoriteAddress;
    int mSettingsBatteryLevelInfo;
    int mSettingsBatteryLevelWarning;
    void updateSettings();

};

#endif // HEARTRATEMANAGER_HPP
