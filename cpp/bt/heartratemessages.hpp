#ifndef HEARTRATEMESSAGES_HPP
#define HEARTRATEMESSAGES_HPP

#include <QObject>

class HeartRateMessages : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int heartRateBpm READ getHeartRateBpm NOTIFY messageChanged)
    Q_PROPERTY(int sensorContact READ getSensorContact NOTIFY messageChanged)
    Q_PROPERTY(int energyJoule READ getEnergyJoule NOTIFY messageChanged)

public:
    explicit HeartRateMessages(QObject *parent = 0);

    void setData(int heartRateBpm, int sensorContact, int energyJoule);
    int getHeartRateBpm() const;
    int getSensorContact() const;
    int getEnergyJoule() const;

signals:
    void messageChanged();

public slots:

private:
    int mHeartRateBpm;
    int mSensorContact;
    int mEnergyJoule;
};

#endif // HEARTRATEMESSAGES_HPP
