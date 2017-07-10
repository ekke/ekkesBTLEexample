// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"
import "../popups"
import org.ekkescorner.btle 1.0

Page {
    id: characteristicDetailPage
    property MyBluetoothDeviceInfo deviceInfo
    property MyBluetoothServiceInfo serviceInfo
    property MyBluetoothCharacteristic characteristic
    focus: true
    property string name: "CharacteristicDetailPage"

    bottomPadding: 6
    topPadding: 6
    leftPadding: 0
    rightPadding: 0

    header: Pane {
        leftPadding: 24
        rightPadding: 8
        Material.elevation: 4
        RowLayout {
            width: appWindow.width - 32
            Column {
                Layout.fillWidth: true
                LabelHeadline {
                    text: deviceInfo.deviceName
                    color: accentColor
                    wrapMode: Label.WordWrap
                    maximumLineCount: 2
                    elide: Label.ElideRight
                }
                LabelTitle {
                    text: serviceInfo.serviceName
                    color: accentColor
                    wrapMode: Label.WordWrap
                    maximumLineCount: 2
                    elide: Label.ElideRight
                }
                LabelTitle {
                    text: characteristic.characteristicName
                    color: accentColor
                    wrapMode: Label.WordWrap
                    maximumLineCount: 2
                    elide: Label.ElideRight
                }
            }// header col
            Rectangle {
                width: 16
                height: 16
                radius: 8
                color: deviceInfo.deviceIsConnected? "green":"red"
                anchors.right: parent.right
                anchors.top: parent.top
            }
        } // header row
    } // header pane

    Flickable {
        id: flickable
        contentHeight: root.implicitHeight + 60
        anchors.fill: parent
        Pane {
            id: root
            anchors.fill: parent
            ColumnLayout {
                Layout.fillWidth: true
                anchors.right: parent.right
                anchors.left: parent.left
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("UUID")
                        color: primaryColor
                    }
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:characteristic.characteristicUuid
                    }
                } // uuid
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Is Valid")
                        color: primaryColor
                    }
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        text:characteristic.characteristicIsValid? qsTr("Yes") : qsTr("No")
                    }
                } // is Valid
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Handle")
                        color: primaryColor
                    }
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WordWrap
                        text:characteristic.characteristicHandle
                    }
                } // handle
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Value")
                        color: primaryColor
                    }
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:characteristic.displayValue
                    }
                } // value
                RowLayout {
                    visible: characteristic.displayCurrentValue.length > 0
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Current")
                        color: primaryColor
                    }
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:characteristic.displayCurrentValue
                    }
                } // value
                LabelTitle {
                    visible: characteristic.characteristicPermission.length > 0
                    text: qsTr("Permissions")
                    color: accentColor
                }
                RowLayout {
                    visible: characteristic.characteristicPermission.length > 0
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        id: labelPermission
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: 1
                        text: characteristic.characteristicPermission
                        color: primaryColor
                        wrapMode: Text.WordWrap
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: 3
                        CharButton {
                            Layout.leftMargin: 12
                            visible: characteristic.characteristicPermission.includes("Read") && !notifyButton.subscribed
                            character: "R"
                            showShadow: true
                            backgroundColor: accentColor
                            textColor: textOnAccent
                            size: 42
                            onClicked: {
                                serviceInfo.readCharacteristic(characteristic)
                            }
                        }
                        CharButton {
                            id: notifyButton
                            Layout.leftMargin: 12
                            visible: characteristic.characteristicPermission.includes("Notify")
                            character: characteristic.notifyIsRunning? "X" : "N"
                            showShadow: true
                            backgroundColor: accentColor
                            textColor: textOnAccent
                            size: 42
                            onClicked: {
                                if(characteristic.notifyIsRunning) {
                                    serviceInfo.unSubscribeNotifications(characteristic)
                                } else {
                                    serviceInfo.subscribeNotifications(characteristic)
                                }
                            }
                        }
                        CharButton {
                            Layout.leftMargin: 12
                            visible: characteristic.characteristicPermission.includes("Write")
                            character: "W"
                            size: 42
                            onClicked: {
                                stringHexMenu.open()
                            }
                            Menu {
                                id: stringHexMenu
                                modal:true
                                dim: false
                                closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape
                                MenuItem {
                                    text: qsTr("Write as ASCII String")
                                    onClicked: {
                                        var withResponse = true
                                        if(characteristic.characteristicPermission.includes("WriteNoResp")) {
                                            withResponse = false
                                        }
                                        navPane.pushWriteValue(deviceInfo, serviceInfo, characteristic,withResponse, false)
                                    }
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: qsTr("Write as HEX String")
                                    onClicked: {
                                        var withResponse = true
                                        if(characteristic.characteristicPermission.includes("WriteNoResp")) {
                                            withResponse = false
                                        }
                                        navPane.pushWriteValue(deviceInfo, serviceInfo, characteristic,withResponse, true)
                                    }
                                }
                                MenuSeparator {}
                                MenuItemWithIcon {
                                    itemText: qsTr("Back")
                                    imageName: "arrow_back.png"
                                    onTriggered: {
                                        //
                                    }
                                } // menu item back workaround QTBUG-61581
                                onAboutToShow: {
                                    appWindow.modalMenuOpen = true
                                }
                                onAboutToHide: {
                                    appWindow.modalMenuOpen = false
                                    appWindow.resetFocus()
                                }
                            }
                        }
                        CharButton {
                            Layout.leftMargin: 12
                            visible: characteristic.characteristicPermission.includes("Indicate")
                            character: "I"
                            size: 42
                        }
                        CharButton {
                            Layout.leftMargin: 12
                            visible: characteristic.characteristicPermission.includes("Broadcast")
                            character: "B"
                            size: 42
                        }
                    }
                } // permissions
                RowLayout {
                    visible: notifyButton.visible
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignVCenter
                        text: qsTr("Collect Values")
                        color: primaryColor
                    }
                    Switch {
                        Layout.alignment: Qt.AlignLeft
                        checked:characteristic.collectData
                        onCheckedChanged: {
                            characteristic.collectData = checked
                        }
                    }
                } // value
                HorizontalListDivider {
                    visible: characteristic.characteristicDescriptorsSize > 0
                }
                LabelTitle {
                    visible: characteristic.characteristicDescriptorsSize > 0
                    text: qsTr("Descriptors")
                    color: accentColor
                }
                Repeater {
                    id: descriptorsRepeater
                    model: characteristic.descriptorList
                    ItemDelegate {
                        Layout.fillWidth: true
                        topPadding: 0
                        leftPadding: 16
                        rightPadding: 16
                        focusPolicy: Qt.NoFocus
                        spacing: 12
                        onClicked: {
                            //
                        }
                        // CONTENT
                        contentItem: ColumnLayout {
                            LabelSubheading {
                                text: model.modelData.descriptorName
                            }
                            LabelSubheading {
                                text: "UUID "+model.modelData.descriptorUuid
                                wrapMode: Label.WrapAnywhere
                            }
                            LabelSubheading {
                                text: "Handle "+model.modelData.descriptorHandle
                            }
                            LabelSubheading {
                                text: model.modelData.descriptorValue
                                wrapMode: Label.WrapAnywhere
                            }
                            HorizontalListDivider{}
                        } // content row
                    } // descriptor delegate

                } // descriptors repeater

            } // main column
        } // root

    } // flickable

    function onOperationError() {
        console.log("Operation Error: " + characteristic.characteristicName)
        if(!deviceInfo.deviceIsConnected) {
            // handled from disconnect state
            return
        }

        // This should not happen ! from docs:
        // An invalid characteristic object is not associated with any service (default-constructed)
        // or the associated service is no longer valid due to a disconnect from the underlying Bluetooth Low Energy device,
        // for example. Once the object is invalid it cannot become valid anymore.
        appWindow.showInfo(qsTr("Read, Write, Notify: Operation cannot be done. Getting OperationError from QLowEnergyService.\nIs %1 connected ?").arg(deviceInfo.deviceName))
    }
    Connections {
        target: serviceInfo
        onOperationError: characteristicDetailPage.onOperationError()
    }
    // called immediately after Loader.loaded
    function init() {
        console.log("Init from characteristicDetailPage")
        //        if(serviceInfo.characteristicList.length === 0) {
        //            console.log("ZERO Characteristics")
        //            scanCharacteristicsPopup.open()
        //        }
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from characteristicDetailPage")
    }
} // characteristicDetailPage
