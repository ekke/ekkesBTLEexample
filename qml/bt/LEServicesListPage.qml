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
    id: servicesListPage
    property MyBluetoothDeviceInfo deviceInfo
    focus: true
    property string name: "ServicesListPage"

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
            LabelHeadline {
                id: deviceNameLabel
                text: deviceInfo.deviceName
                color: accentColor
            }
            Rectangle {
                width: 16
                height: 16
                radius: 8
                color: deviceInfo.deviceIsConnected? "green":"red"
                anchors.right: parent.right
                anchors.top: parent.top
            }
        }
    } // headerpane

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
        // bottomMargin: 40
        // QList<MyBluetoothServiceInfo*>
        model: deviceInfo.servicesList
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
            id: serviceLoader
            // define Components inside Loader to enable direct access to ListView functions and modelData
            sourceComponent: serviceRowComponent

            // LIST ROW DELEGTE
            Component {
                id: serviceRowComponent

                ItemDelegate {
                    id: rowDelegate
                    width: appWindow.width
                    implicitHeight: Math.max(64+24, contentItem.implicitHeight+12+12)
                    focusPolicy: Qt.NoFocus
                    spacing: 12
                    leftPadding: spacing*2
                    rightPadding: spacing + 12
                    onClicked: {
                        if(model.modelData.serviceUuid === "0x1812") {
                            // needs android.permission.BLUETOOTH_PRIVILEGED
                            appWindow.showInfo(qsTr("Human Interface Device not supported yet"))
                            return
                        }
                        navPane.pushCharacteristics(deviceInfo, model.modelData)
                    }


                    //                    FloatingActionMiniButton {
                    //                        id: rightItem
                    //                        imageSource: "qrc:/images/"+iconOnPrimaryFolder+"/settings.png"
                    //                        showShadow: true
                    //                        anchors.right: parent.right
                    //                        anchors.rightMargin: 12
                    //                        //anchors.top: undefined
                    //                        anchors.verticalCenter: parent.verticalCenter
                    //                        onClicked: {
                    //                            if(model.modelData.serviceUuid === "0x1812") {
                    //                                // needs android.permission.BLUETOOTH_PRIVILEGED
                    //                                appWindow.showInfo(qsTr("Human Interface Device not supported yet"))
                    //                                return
                    //                            }
                    //                            navPane.pushCharacteristics(deviceInfo, model.modelData)
                    //                        } // clicked
                    //                    }
//                    CharButton {
//                        id: rightItem
//                        anchors.right: parent.right
//                        anchors.rightMargin: 12
//                        anchors.verticalCenter: parent.verticalCenter
//                        character: "N"
//                        onClicked: {
//                            console.log("KLICKI")
//                        }
//                    }

                    // CONTENT
                    contentItem: Column {
                        width: appWindow.width - parent.leftPadding - parent.rightPadding
                        LabelHeadline {
                            width: parent.width
                            text: model.modelData.serviceName.length ? model.modelData.serviceName : "???"
                            color: primaryColor
                            wrapMode: Label.WordWrap
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        } // label
                        LabelBody {
                            width: parent.width
                            text: model.modelData.serviceUuid
                            wrapMode: Label.WordWrap
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        }
                        LabelBody {
                            width: parent.width
                            bottomPadding: 6
                            text: model.modelData.serviceType
                            wrapMode: Label.WordWrap
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        }
                    }
                    // dividers below each delegate
                    Rectangle {
                        y: parent.height
                        width: parent.width
                        height: 1
                        opacity: dividerOpacity
                        color: dividerColor
                    }
                } // rowDelegate
            } // serviceRowComponent

        } // serviceLoader

        ScrollIndicator.vertical: ScrollIndicator { }

    } // end listView

    Component.onDestruction: {
        cleanup()
    }

    FloatingActionButton {
        id: searchServicesButton
        visible: deviceInfo.servicesList.length === 0
        imageSource: "qrc:/images/"+iconOnAccentFolder+"/refresh.png"
        backgroundColor: accentColor
        z: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 10
        anchors.bottom: parent.bottom
        onClicked: {
            discoverServicesPopup.infoText = ""
            discoverServicesPopup.open()
        }
    }

    BTScanPopup {
        id: discoverServicesPopup
        property bool isActive: false
        text: qsTr("Discover %1 for Services - please wait ...").arg(deviceInfo.deviceName)
        textTimeout: qsTr("Waited too long for end of discovering services")
        infoText: ""
        imageSource: "qrc:/images/extra/scan.png"
        from: 0
        to: 30
        max: 45
        onOpened: {
            isActive = true
            infoText = ""
            deviceInfo.discoverServices()
        }
        onClosed: {
            isActive = false
        }
    }// discoverServicesPopup

    function onServicesDiscoveryActiveChanged() {
        if(!discoverServicesPopup.isActive) {
            return
        }
        console.log("ServicesDiscoveryActive changed: "+ deviceInfo.servicesDiscoveryActive)
        if(!deviceInfo.servicesDiscoveryActive) {
            discoverServicesPopup.close()
        }
    }
    Connections {
        target: deviceInfo
        onServicesDiscoveryActiveChanged: servicesListPage.onServicesDiscoveryActiveChanged()
    }
    Timer {
        id: restartDelayTimer
        interval: 1000
        repeat: false
        onTriggered: {
            deviceInfo.connectToDeviceAgain()
        }
    } // restartDelayTimer
    function onRetryDeviceConnectChanged() {
        if(!discoverServicesPopup.isActive) {
            return
        }
        if(!deviceInfo.retryDeviceConnect) {
            console.log("RESET detected: NO")
            return
        }
        if(discoverServicesPopup.stopClicked) {
            discoverServicesPopup.stopClicked = false
            discoverServicesPopup.close()
            return
        }
        console.log("RESET detected: YES "+deviceInfo.retryCounter)
        discoverServicesPopup.infoText = qsTr("Try to connect again: %1 of 5").arg(deviceInfo.retryCounter)
        discoverServicesPopup.from = 0
        discoverServicesPopup.to = 30
        discoverServicesPopup.max = 60
        discoverServicesPopup.showStopButton = true
        discoverServicesPopup.startTimer()
        restartDelayTimer.start()
    }
    Connections {
        target: deviceInfo
        onRetryDeviceConnectChanged: servicesListPage.onRetryDeviceConnectChanged()
    }
    // called immediately after Loader.loaded
    function init() {
        console.log("Init from servicesListPage")
        if(deviceInfo.servicesList.length === 0) {
            console.log("ZERO Services")
            discoverServicesPopup.open()
        }
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from servicesListPage")
    }
} // end servicesListPage
