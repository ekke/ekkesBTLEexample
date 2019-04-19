#ifndef FEITIANCARDREADERMANAGER_HPP
#define FEITIANCARDREADERMANAGER_HPP

#include <QObject>
#include "bluetoothmanager.hpp"

class FeitianCardReaderManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cardDataValue READ getCardDataValue NOTIFY cardDataValueChanged)
    Q_PROPERTY(bool featuresPrepared READ getFeaturesPrepared WRITE setFeaturesPrepared NOTIFY featuresPreparedChanged)
    Q_PROPERTY(bool cardNotificationsActive READ getCardNotificationsActive WRITE setCardNotificationsActive NOTIFY cardNotificationsActiveChanged)
    Q_PROPERTY(bool hasDevice READ getHasDevice NOTIFY hasDeviceChanged)

    // common settings
    Q_PROPERTY(QString settingsFavoriteAddress READ getSettingsFavoriteAddress WRITE setSettingsFavoriteAddress NOTIFY settingsChanged)
    Q_PROPERTY(QString settingsFavoriteName READ getSettingsFavoriteName WRITE setSettingsFavoriteName NOTIFY settingsNameChanged)


public:
    explicit FeitianCardReaderManager(QObject *parent = nullptr);

    void init(BluetoothManager* bluetoothManager);

    QString getCardDataValue() const;

    bool getFeaturesPrepared() const;
    void setFeaturesPrepared(bool isPrepared);
    bool getCardNotificationsActive() const;
    void setCardNotificationsActive(bool isActive);
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
    void startCardNotifications();

    Q_INVOKABLE
    void stopCardNotifications();

    Q_INVOKABLE
    void resetFoundDevices();

    Q_INVOKABLE
    void addToFoundDevices(MyBluetoothDeviceInfo* deviceInfo);

    Q_INVOKABLE
    QList<QObject*> foundDevices();

    Q_INVOKABLE
    void doPowerOn();

    Q_INVOKABLE
    void doPowerOff();

    Q_INVOKABLE
    void doSelectFile();

    Q_INVOKABLE
    void doReadBinaryStatusVD();

    Q_INVOKABLE
    void doReadBinaryPersonalData();

    Q_INVOKABLE
    void doReadBinaryInsuranceData();

signals:
    void cardDataValueChanged();
    void cardIN();
    void cardOUT();
    void readATRSuccess();
    void readATRWrong(const QString message, const QString parseATRUrl);
    void appSelectedSuccess();
    void appSelectedFailed(const QString& message, const QString apduResponseInfoUrl, const QString apduResponse);
    void statusVDSuccess(const QVariantMap statusVDMap);
    void statusVDFailed(const QString message, const QString apduResponseInfoUrl, const QString apduResponse);
    void personalDataSuccess(const QVariantMap pdMap);
    void personalDataFailed(const QString message, const QString apduResponseInfoUrl, const QString apduResponse);

    void cardNotificationsActiveChanged();
    void featuresPreparedChanged();
    void settingsChanged();
    void settingsNameChanged();

    void hasDeviceChanged();

    void foundDevicesCounter(const int devicesCounter);

public slots:
    void onCardCharacteristicsDone();

    void onDisconnect();
    void onCardDataChanged();
    void onCardSubscriptionsChanged();

private:
    BluetoothManager* mBluetoothManager;

    // list of MyBluetoothDeviceInfo*
    QList<QObject*> mFoundDevices;

    MyBluetoothDeviceInfo* mDeviceInfo;
    bool mDeviceIsConnected;

    MyBluetoothServiceInfo* mCardService;
    bool mCardServiceAvailable;
    bool mCardServiceConnected;
    MyBluetoothCharacteristic* mCardData;
    bool mCardDataAvailable;
    QString mCardDataValue;

    MyBluetoothCharacteristic* mWriteData;
    bool mWriteDataAvailable;
    QString mWriteDataValue;

    void checkIfAllPrepared();
    bool mFeaturesPrepared;
    bool mCardNotificationsActive;
    bool mHasDevice;

    // ID for Commands sent to SmartCard
    // (Counter - HEX)
    QString mCurrentIdHex;
    void calculateNextIdHex();
    // currently running Command
    QString mRunningCommand;
    // currently running APDU
    QString mRunningAPDU;
    // read binary can be split into some chunk of data
    // first data contains the data length attribute
    // so we must know if the first part was processed
    bool mFirstResponseProcessed;
    int mExpectedLength;
    void resetCommand();
    // processing commands
    void processPowerOn(const QString &hexData);
    void processSelectFiles(const QString &hexData);
    void processReadBinaryStatusVD(const QString &hexData);
    void doReadBinaryNext();
    void processReadBinaryPersonalData(const QString &hexData);

    // complete (concatanated) data
    QString mCurrentData;

    // SETTINGS
    QVariantMap mFeitianCardReaderSettingsMap;
    QString mSettingsFavoriteAddress;
    QString mSettingsFavoriteName;
    void updateSettings();



};

#endif // FEITIANCARDREADERMANAGER_HPP
