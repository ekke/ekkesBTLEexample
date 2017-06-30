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
    id: characteristicsListPage
    property MyBluetoothDeviceInfo deviceInfo
    property MyBluetoothServiceInfo serviceInfo
    focus: true
    property string name: "CharacteristicsListPage"

    bottomPadding: 6
    topPadding: 6
    // set to 0 to see whole row highlightes
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



    // LIST VIEW
    ListView {
        id: listView
        focus: true
        clip: true
        spacing: 1 // spacing between delegates
        // highlight: Rectangle {color: Material.listHighlightColor }
        currentIndex: -1
        anchors.fill: parent
        // setting the margin to be able to scroll the list above the FAB to use the Switch on last row
        bottomMargin: 48
        // QList<MyBluetoothServiceInfo*>
        model: serviceInfo.characteristicList
        // Divider on Top of first Row
        data: Rectangle {
            parent: listView.contentItem // automatically follows the content
            y: -1 // move divider above first delegate
            width: parent.width
            height: 1
            opacity: dividerOpacity
            color: dividerColor
        }

        // important: use Loader to avoid errors because of https://bugreports.qt.io/browse/QTBUG-49224
        delegate:

            Loader {
            id: characteristicLoader
            // define Components inside Loader to enable direct access to ListView functions and modelData
            sourceComponent: characteristicRowComponent

            // LIST ROW DELEGTE
            Component {
                id: characteristicRowComponent

                ItemDelegate {
                    id: rowDelegate
                    width: appWindow.width
                    // implicitHeight: Math.max(64+24, contentItem.implicitHeight+12+12)
                    implicitHeight: contentItem.implicitHeight + 12 + 12
                    focusPolicy: Qt.NoFocus
                    spacing: 6
                    leftPadding: 24
                    rightPadding: 24
                    onClicked: {
                        navPane.pushCharacteristicDetail(deviceInfo, serviceInfo, model.modelData)
                    }
                    HorizontalListDivider {
                        anchors.bottom: parent.bottom
                    }
// doesnt work yet: we must propagate device disconnect to services and characteristics
//                    DotMarker {
//                        width: 10
//                        height: 10
//                        color: model.modelData.characteristicIsValid? "green":"red"
//                        anchors.right: parent.right
//                        anchors.bottom: undefined
//                        anchors.bottomMargin: undefined
//                        anchors.top: parent.top
//                        anchors.topMargin: 12
//                    }

                    // CONTENT
                    // using Column doesn't work well if headline has more then 1 line
                    contentItem: Column {
                        //Layout.bottomMargin: 6
                        width: appWindow.width - parent.leftPadding - parent.rightPadding
                        LabelTitle {
                            width: parent.width
                            text: model.modelData.characteristicName.length ? model.modelData.characteristicName : "???"
                            color: primaryColor
                            wrapMode: Label.WordWrap
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        } // label
                        Row {
                            width: parent.width
                            LabelSubheading {
                                width: parent.width/3
                                text: "UUID"
                            }
                            LabelSubheading {
                                width: parent.width/3*2
                                text: model.modelData.characteristicUuid
                                wrapMode: Label.WrapAnywhere
                                maximumLineCount: 2
                                elide: Label.ElideRight
                            }
                        }
                        Row {
                            width: parent.width
                            LabelSubheading {
                                width: parent.width/3
                                text: "Handle"
                            }
                            LabelSubheading {
                                width: parent.width/3*2
                                text: model.modelData.characteristicHandle
                                wrapMode: Label.WrapAnywhere
                                maximumLineCount: 2
                                elide: Label.ElideRight
                            }
                        }
                        Row {
                            visible: model.modelData.characteristicPermission.length > 0
                            width: parent.width
                            LabelSubheading {
                                width: parent.width/3
                                text: "Permissions"
                            }
                            LabelSubheading {
                                width: parent.width/3*2
                                text: model.modelData.characteristicPermission
                                wrapMode: Label.WrapAnywhere
                                maximumLineCount: 2
                                elide: Label.ElideRight
                            }
                        }
                        Row {
                            width: parent.width
                            LabelSubheading {
                                width: parent.width/3
                                text: "Value"
                            }
                            LabelSubheading {
                                width: parent.width/3*2
                                text: model.modelData.characteristicValue
                                wrapMode: Label.WrapAnywhere
                            }
                        }
                    }
                } // rowDelegate
            } // characteristicRowComponent

        } // characteristicLoader

        ScrollIndicator.vertical: ScrollIndicator { }

    } // end listView

    Component.onDestruction: {
        cleanup()
    }

    FloatingActionButton {
        id: searchCharacteristicsButton
        visible: serviceInfo.characteristicList.length === 0
        imageSource: "qrc:/images/"+iconOnAccentFolder+"/refresh.png"
        backgroundColor: accentColor
        z: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 10
        anchors.bottom: parent.bottom
        onClicked: {
            scanCharacteristicsPopup.infoText = ""
            scanCharacteristicsPopup.open()
        }
    }

    BTScanPopup {
        id: scanCharacteristicsPopup
        text: qsTr("Scanning %1-%2 for Characteristics - please wait ...").arg(deviceInfo.deviceName).arg(serviceInfo.serviceName)
        textTimeout: qsTr("Waited too long for end of scanning Characteristics")
        infoText: ""
        imageSource: "qrc:/images/extra/scan.png"
        from: 0
        to: 30
        max: 45
        onOpened: {
            infoText = ""
            serviceInfo.connectToService()
        }
    } // scanCharacteristicsPopup
    //
    function onCharacteristicsDone() {
        scanCharacteristicsPopup.close()
    }
    Connections {
        target: serviceInfo
        onCharacteristicsDone: characteristicsListPage.onCharacteristicsDone()
    }
    //
    function onServiceInvalid() {
        scanCharacteristicsPopup.close()
        appWindow.showInfo(qsTr("Couldn't scan Characteristics: Service is invalid. Device connected ?."))
    }
    Connections {
        target: serviceInfo
        onServiceInvalid: characteristicsListPage.onServiceInvalid()
    }
    //
    function onServiceDiscovering() {
        scanCharacteristicsPopup.close()
        appWindow.showInfo(qsTr("Couldn't scan Characteristics: Service is already discovering. Wait and try again."))
    }
    Connections {
        target: serviceInfo
        onServiceDiscovering: characteristicsListPage.onServiceDiscovering()
    }

    // called immediately after Loader.loaded
    function init() {
        console.log("Init from characteristicsListPage")
        if(serviceInfo.characteristicList.length === 0) {
            console.log("ZERO Characteristics")
            scanCharacteristicsPopup.open()
        }
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from characteristicsListPage")
    }
} // end characteristicsListPage
