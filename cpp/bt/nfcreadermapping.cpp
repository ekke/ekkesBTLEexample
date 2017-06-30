#include "nfcreadermapping.hpp"

NfcReaderMapping::NfcReaderMapping(QObject *parent) : QObject(parent)
{

}

void NfcReaderMapping::setData(QString tagId, QString userName)
{
    bool valuesChanged = false;
    if(tagId != mTagId) {
        mTagId = tagId;
        valuesChanged = true;
    }
    if(userName != mUserName) {
        mUserName = userName;
        valuesChanged = true;
    }
    if(valuesChanged) {
        emit mappingChanged();
    }
}

QString NfcReaderMapping::getTagId() const
{
    return mTagId;
}

QString NfcReaderMapping::getUserName() const
{
    return mUserName;
}

void NfcReaderMapping::setTagId(QString newValue)
{
    if(newValue != mTagId) {
        mTagId = newValue;
        emit mappingChanged();
    }
}

void NfcReaderMapping::setUserName(QString newValue)
{
    if(newValue != mUserName) {
        mUserName = newValue;
        emit mappingChanged();
    }
}

QVariantMap NfcReaderMapping::toMap()
{
    QVariantMap map;
    map.insert("tagId", mTagId);
    map.insert("userName", mUserName);
    return map;

}

void NfcReaderMapping::fromMap(QVariantMap map)
{
    mTagId = map.value("tagId").toString();
    mUserName = map.value("userName").toString();
}
