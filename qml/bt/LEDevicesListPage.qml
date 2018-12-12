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
    id: devicesListPage
    focus: true
    property string name: "DevicesListPage"

    property MyBluetoothDeviceInfo selectedDeviceInfo

    bottomPadding: 6
    topPadding: 6
    // set to 0 to see whole row highlightes
    leftPadding: 0
    rightPadding: 0

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
        bottomMargin: 60
        // QList<MyBluetoothDeviceInfo*>
        model: btManager.devicesList
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
            id: deviceLoader
            // define Components inside Loader to enable direct access to ListView functions and modelData
            sourceComponent: deviceRowComponent

            // LIST ROW DELEGTE
            Component {
                id: deviceRowComponent

                ItemDelegate {
                    id: rowDelegate
                    width: appWindow.width
                    implicitHeight: Math.max(64+24, contentItem.implicitHeight+12+12)
                    focusPolicy: Qt.NoFocus
                    spacing: 12
                    leftPadding: leftItem.width + spacing + 10
                    rightPadding: spacing
                    onClicked: {
                        if(model.modelData.deviceIsConnected) {
                            showDisconnectMessage = false
                        }
                        navPane.openDevice(labelDeviceName,model.modelData)
                    }
                    // left item
                    Column {
                        id: leftItem
                        x: 10
                        y: 12
                        Rectangle {
                            id: outerRssi
                            width: isSmallDevice? 16: 24
                            height: 60
                            color: "grey"
                            Rectangle {
                                anchors.bottom: parent.bottom
                                color: "green"
                                width: isSmallDevice? 16: 24
                                height: Math.min(60,(60+model.modelData.deviceRssi+40))
                            }
                        }
                        LabelCaption {
                            bottomPadding: 12
                            text: model.modelData.deviceRssi
                            horizontalAlignment: Text.horizontalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    Rectangle {
                        id: connectIndicator
                        width: 16
                        height: 16
                        radius: 8
                        color: model.modelData.deviceIsConnected? "green":"red"
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.rightMargin: 8
                        anchors.topMargin: 8
                    }

                    // CONTENT
                    contentItem: Row {
                        width: appWindow.width - parent.leftPadding - parent.rightPadding
                        Column {
                            id: textColumn
                            width: parent.width-120-12
                            Row {
                                width: parent.width
                                IconActive {
                                    visible: model.modelData.deviceType > 0
                                    imageName: navPane.deviceTypeImages[model.modelData.deviceType]
                                }
                                LabelHeadline {
                                    id: labelDeviceName
                                    leftPadding: model.modelData.deviceType > 0? 2 : 0
                                    width: model.modelData.deviceType > 0? parent.width - 26 : parent.width
                                    text: model.modelData.deviceName.length ? model.modelData.deviceName : "???"
                                    font.pixelSize: isSmallDevice? fontSizeTitle : fontSizeHeadline
                                    color: primaryColor
                                    wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                                    maximumLineCount: 2
                                    elide: Label.ElideRight
                                } // name
                            }
                            LabelSubheading {
                                width: parent.width
                                text: model.modelData.controllerStateDescription
                            }
                            LabelBody {
                                bottomPadding: 6
                                width: parent.width
                                text: model.modelData.deviceAddress
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            }

                        }
                        ButtonFlat {
                            anchors.bottom: textColumn.bottom
                            width: 120
                            text: model.modelData.deviceIsConnected? qsTr("Disconnect") : qsTr("Connect")
                            textColor: model.modelData.deviceIsConnected? accentColor : primaryColor
                            textAlignment: Text.AlignRight
                            onClicked: {
                                if(model.modelData.deviceIsConnected) {
                                    showDisconnectMessage = false
                                    devicesListPage.disconnectFromDevice(model.modelData)
                                } else {
                                    devicesListPage.connectToDevice(model.modelData)
                                }
                            }
                        } // connect disconnect button
                    }
                    // dividers below each delegate
                    Rectangle {
                        y: parent.height
                        width: parent.width
                        height: 1
                        opacity: dividerOpacity
                        color: dividerColor
                    }
                    // CONNECTIONS
                    property bool showDisconnectMessage: true
                    function onDeviceDisconnected() {
                        if(!showDisconnectMessage) {
                            showDisconnectMessage = true
                            return
                        }
                        devicesListPage.askForReconnect(model.modelData)
                    }
                    Connections {
                        target: model.modelData
                        onDisconnected: onDeviceDisconnected()
                    }
                    function onDeviceConnected() {
                        devicesListPage.checkConnect(model.modelData)
                    }
                    Connections {
                        target: model.modelData
                        onConnected: onDeviceConnected()
                    }
                } // rowDelegate
            } // deviceRowComponent

        } // deviceLoader

        ScrollIndicator.vertical: ScrollIndicator { }

    } // end listView

    Component.onDestruction: {
        cleanup()
    }

    // C O N N E C T - D I S C O N N E C T
    // functions called from List Row Delegate Buttons
    function connectToDevice(deviceInfo) {
        selectedDeviceInfo = deviceInfo
        connectToDevicePopup.open()
    }
    function disconnectFromDevice(deviceInfo) {
        selectedDeviceInfo = deviceInfo
        selectedDeviceInfo.disconnectFromDevice()
    }

    //  D I S C O V E R   D E V I C E S
    FloatingActionButton {
        id: discoverDevicesButton
        imageSource: "qrc:/images/"+iconOnPrimaryFolder+"/add.png"
        backgroundColor: primaryColor
        z: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 10
        anchors.bottom: parent.bottom
        onClicked: {
            discoverDevicesPopup.open()
        }
    }
    FloatingActionMiniButton {
        id: clearDevicesButton
        imageSource: "qrc:/images/"+iconOnAccentFolder+"/clear.png"
        backgroundColor: accentColor
        z: 1
        anchors.left: discoverDevicesButton.right
        anchors.leftMargin: 24
        anchors.bottomMargin: 10+24
        anchors.bottom: parent.bottom
        onClicked: {
            clearDevicesList()
        }
    }

    function clearDevicesList() {
        appWindow.resetCurrentDevices()
        btManager.clearDevices()
    }

    // see also inside other managers to search specific device
    BTScanPopup {
        id: discoverDevicesPopup
        text: qsTr("Discovering Bluetooth LE Devices - please wait ...")
        textTimeout: qsTr("Waited too long for end of discovering devices")
        imageSource: "qrc:/images/extra/scan.png"
        from: 0
        to: 15
        max: 20
        showStopButton: true
        function onDeviceDiscoveryActiveChanged() {
            console.log("DeviceDiscoveryActive changed: "+ btManager.deviceDiscoveryActive)
            if(!btManager.deviceDiscoveryActive) {
                discoverDevicesPopup.close()
                if(btManager.devicesList.length === 0) {
                    if(appWindow.isDeviceListCurrentTab()) {
                        appWindow.showInfo(qsTr("No Devices found. Switch Devices Off/On or Settings Bluetooth Off/On to clear cache."))
                    }
                }
            }
        }
        Connections {
            target: btManager
            onDeviceDiscoveryActiveChanged: discoverDevicesPopup.onDeviceDiscoveryActiveChanged()
        }
        onOpened: {
            btManager.startDeviceDiscovery()
        }
        onStopClickedChanged: {
            if(stopClicked) {
                stopClicked = false
                btManager.stopDeviceDiscovery()
            }
        }
        onClosed: {
            showStopButton = true
        }
    }// discoverDevicesPopup

    function onDeviceDiscoveryErrorReceived(errorMessage) {
        appWindow.showInfo(errorMessage)
    }
    Connections {
        target: btManager
        onDeviceDiscoveryErrorReceived: devicesListPage.onDeviceDiscoveryErrorReceived(errorMessage)
    }

    // R E C O N N E C T   T O   D E V I C E
    // signal connected in list row delegate !
    // ask for Reconnect if YES pop all pages using data from deviceInfo
    function checkConnect(deviceInfo) {
        if(connectToDevicePopup.isActive) {
            if(deviceInfo === selectedDeviceInfo) {
                connectToDevicePopup.close()
            }
        }
    }
    function askForReconnect(deviceInfo) {
        selectedDeviceInfo = deviceInfo
        // got the signal from elsewhere, so we ask user for reconnect
        // if this is our current tab
        if(appWindow.isDeviceListCurrentTab()) {
            reconnectDialog.open()
        }
    }
    ReconnectYesNo {
        id: reconnectDialog
        parent: rootPane
        text: selectedDeviceInfo? qsTr("%1 is disconnected.\nReconnect to %1 now ?\n\nCheck Device is ON, Battery OK\nBluetooth is On\n").arg(selectedDeviceInfo.deviceName) : "Reconnect"
        onIsYesChanged: {
            if(isYes) {
                isYes = false
                // perhaps check if deviceInfo of Pages on stack == selectedDeviceInfo
                navPane.backToRootPage()
                connectToDevicePopup.open()
            }
        }
    }
    BTScanPopup {
        id: connectToDevicePopup
        property bool isActive: false
        text: selectedDeviceInfo? qsTr("Connecting to %1 - please wait ...").arg(selectedDeviceInfo.deviceName) : ""
        textTimeout: qsTr("Waited too long for end of connecting to device")
        infoText: ""
        imageSource: "qrc:/images/extra/scan.png"
        from: 0
        to: 30
        max: 45
        onOpened: {
            isActive = true
            infoText = ""
            selectedDeviceInfo.connectToDevice()
        }
        onClosed: {
            isActive = false
        }
    }// connectToDevicePopup
    Timer {
        id: restartDelayTimer
        interval: 1000
        repeat: false
        onTriggered: {
            selectedDeviceInfo.connectToDeviceAgain()
        }
    } // restartDelayTimer
    function onRetryDeviceConnectChanged() {
        if(!connectToDevicePopup.isActive) {
            return
        }

        if(!selectedDeviceInfo.retryDeviceConnect) {
            console.log("RESET detected: NO")
            return
        }
        if(connectToDevicePopup.stopClicked) {
            connectToDevicePopup.stopClicked = false
            connectToDevicePopup.close()
            return
        }
        console.log("RESET detected: YES "+selectedDeviceInfo.retryCounter)
        connectToDevicePopup.infoText = qsTr("Try to connect again: %1 of 5").arg(selectedDeviceInfo.retryCounter)
        connectToDevicePopup.from = 0
        connectToDevicePopup.to = 30
        connectToDevicePopup.max = 60
        connectToDevicePopup.showStopButton = true
        connectToDevicePopup.startTimer()
        restartDelayTimer.start()
    }
    Connections {
        target: selectedDeviceInfo
        onRetryDeviceConnectChanged: devicesListPage.onRetryDeviceConnectChanged()
    }

    ///////////////
    // called immediately after Loader.loaded
    function init() {
        console.log("Init from devicesListPage")
        if(btManager.devicesList.length === 0) {
            console.log("ZERO Devices")
            discoverDevicesPopup.open()
        }
    }
    Component.onCompleted: {
        console.log("Component created devicesListPage")
    }

    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from devicesListPage")
    }
} // end devicesListPage
