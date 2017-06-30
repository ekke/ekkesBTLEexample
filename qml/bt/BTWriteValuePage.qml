// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"
import org.ekkescorner.btle 1.0

Page {
    id: btWriteValuePage
    property MyBluetoothDeviceInfo deviceInfo
    property MyBluetoothServiceInfo serviceInfo
    property MyBluetoothCharacteristic characteristic
    property bool withResponse: false
    property bool asHex: false
    focus: true
    property string name: "BTWriteValuePage"

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

    Pane {
        topPadding: 16
        leftPadding: 16
        rightPadding: 16
        width: appWindow.width - 12

        ColumnLayout {
            anchors.right: parent.right
            anchors.left: parent.left

            LabelTitle {
                bottomPadding: 12
                text: btWriteValuePage.asHex? qsTr("Please enter HEX Value") : qsTr("Please enter ASCII String Value")
            }

            TextAreaRow{
                id: writeTextRow
                text: ""

                onTextChanged: {
                    //
                }
                onDone: {
                    console.log("D O N E")
                    if(asHex) {
                        serviceInfo.writeCharacteristicAsHex(characteristic, writeTextRow.text, withResponse)
                        appWindow.showToast(qsTr("Write as HEX: %1").arg(writeTextRow.text))
                    } else {
                        serviceInfo.writeCharacteristicAsString(characteristic, writeTextRow.text, withResponse)
                        appWindow.showToast(qsTr("Write String: %1").arg(writeTextRow.text))
                    }


                    navPane.popOnePage()
                }
            } // writeTextRow
        }
    }



    Component.onDestruction: {
        cleanup()
    }

    // called immediately after Loader.loaded
    function init() {
        console.log("Init done from btWriteValuePage")
        writeTextRow.textField.forceActiveFocus()
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from btWriteValuePage")
    }

} // btWriteValuePage
