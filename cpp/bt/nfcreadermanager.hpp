#ifndef NFCREADERMANAGER_HPP
#define NFCREADERMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"
#include "nfcreadermapping.hpp"

class NfcReaderManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int batteryLevelValue READ getBatteryLevelValue NOTIFY batteryLevelValueChanged)
    Q_PROPERTY(QString tagIdValue READ getTagIdValue NOTIFY tagIdValueChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool tagNotificationsActive READ getTagNotificationsActive WRITE setTagNotificationsActive NOTIFY tagNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)

    Q_PROPERTY(QVariant mappingsList READ getMappings NOTIFY mappingsUpdated)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelInfo READ getSettingsBatteryLevelInfo NOTIFY settingsChanged)
    Q_PROPERTY(int settingsBatteryLevelWarning READ getSettingsBatteryLevelWarning NOTIFY settingsChanged)
    // specific

public:
    explicit NfcReaderManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    int getBatteryLevelValue() const;
    QString getTagIdValue() const;
    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getTagNotificationsActive() const;
    void setTagNotificationsActive(bool isActive);
    bool getHasDevice() const;

    QString getSettingsFavoriteAddress() const;
    void setSettingsFavoriteAddress(QString address);
    int getSettingsBatteryLevelInfo() const;
    int getSettingsBatteryLevelWarning() const;

    QVariant getMappings();
    void clearMappings();

    Q_INVOKABLE
    NfcReaderMapping* createOrGetMapping(QString tagId);

    Q_INVOKABLE
    void triggerMappingsDataUpdated();

    Q_INVOKABLE
    bool removeMapping(QString tagId);

    Q_INVOKABLE
    NfcReaderMapping* findFromMapping(QString tagId);

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
    void updateTagValue();

    Q_INVOKABLE
    void startTagNotifications();

    Q_INVOKABLE
    void stopTagNotifications();

    Q_INVOKABLE
    MyBluetoothCharacteristic* getBatteryLevel();

    Q_INVOKABLE
    void doBuzzer(QString buzzerValue);

    Q_INVOKABLE
    void doLED(QString ledValue);

signals:
    void batteryLevelValueChanged();
    void tagIdValueChanged();
    void tagNotificationsActiveChanged();
    void featuresPreparedChanged();

    void mappingsUpdated();

    void settingsChanged();

    void hasDeviceChanged();

public slots:
    void onBatteryCharacteristicsDone();
    void onTagCharacteristicsDone();
    void onDisconnect();

    void onBatteryLevelChanged();
    void onTagIdChanged();
    void onTagSubscriptionsChanged();


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

    MyBluetoothServiceInfo* mTagService;
    bool mTagServiceAvailable;
    bool mTagServiceConnected;
    MyBluetoothCharacteristic* mTagId;
    bool mTagIdAvailable;
    QString mTagIdValue;
    QString mCurrentId;

    MyBluetoothCharacteristic* mWriteData;
    bool mWriteDataAvailable;
    QString mWriteDataValue;

    QList<QObject*> mMappings;
    QVariantList mappingsToVariantList();
    void mappingsFromVariantList(QVariantList& theList);

    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mTagNotificationsActive;
    bool mHasDevice;

    // SETTINGS
    QVariantMap mNfcReaderSettingsMap;
    QString mSettingsFavoriteAddress;
    int mSettingsBatteryLevelInfo;
    int mSettingsBatteryLevelWarning;
    void updateSettings();

};

#endif // NFCREADERMANAGER_HPP
