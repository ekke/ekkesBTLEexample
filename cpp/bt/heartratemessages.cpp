#include "heartratemessages.hpp"

HeartRateMessages::HeartRateMessages(QObject *parent) : QObject(parent)
{
    //
}

void HeartRateMessages::setData(int heartRateBpm, int sensorContact, int energyJoule)
{
    mHeartRateBpm = heartRateBpm;
    mSensorContact = sensorContact;
    mEnergyJoule = energyJoule;
    emit messageChanged();
}

int HeartRateMessages::getHeartRateBpm() const
{
    return mHeartRateBpm;
}

int HeartRateMessages::getSensorContact() const
{
    return mSensorContact;
}

int HeartRateMessages::getEnergyJoule() const
{
    return mEnergyJoule;
}
