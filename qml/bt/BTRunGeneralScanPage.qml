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
    id: btRunGeneralScanPage
    property MyBluetoothDeviceInfo deviceInfo
    property bool isSearchRunning: discoverDevicesPopup.isActive

    focus: true
    property string name: "BTRunGeneralScanPage"

    header: Pane {
        leftPadding: 24
        rightPadding: 8
        Material.elevation: 4
        RowLayout {
            width: appWindow.width - 32
            Column {
                Layout.fillWidth: true
                Row {
                    spacing: 6
                    IconActive {
                        imageName: "barcode.png"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    LabelHeadline {
                        text: deviceInfo? deviceInfo.deviceName : qsTr("no device")
                        color: primaryColor
                        wrapMode: Label.WordWrap
                        maximumLineCount: 2
                        elide: Label.ElideRight
                        anchors.verticalCenter: parent.verticalCenter
                    } // name
                    ButtonFlat {
                        visible: !deviceInfo
                        text: qsTr("Search Device")
                        textColor: accentColor
                        textAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            btRunGeneralScanPage.searchDevice()
                        }
                    } // search devices button
                } // first row
                Row {
                    visible: deviceInfo
                    Column {
                        width: appWindow.width - 32 - 160 - 12
                        anchors.verticalCenter: parent.verticalCenter
                        LabelSubheading {
                            visible: Qt.platform.os != "ios" && !appWindow.isSmallDevice
                            width: parent.width
                            text: deviceInfo? deviceInfo.deviceAddress : ""
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        } // address
                        LabelBody {
                            visible: Qt.platform.os == "ios" || appWindow.isSmallDevice
                            width: parent.width
                            text: deviceInfo? deviceInfo.deviceAddress : ""
                            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 3
                            elide: Label.ElideRight
                        } // address
                        LabelBody {
                            width: parent.width
                            text: deviceInfo? deviceInfo.controllerStateDescription : ""
                            wrapMode: Label.WordWrap
                            maximumLineCount: 2
                            elide: Label.ElideRight
                        } // address
                    }
                    // retry if services hang in discovering
                    ButtonFlat {
                        visible: deviceInfo
                        width: 160
                        text: scanManager.featuresPrepared? qsTr("Disconnect") : qsTr("Initialize")
                        textColor: accentColor
                        textAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: 12
                        onClicked: {
                            if(scanManager.featuresPrepared) {
                                btRunGeneralScanPage.showDisconnectMessage = false
                                btRunGeneralScanPage.disconnectFromDevice()
                            } else {
                                // search the device, connect and create expected services if needed
                                btRunGeneralScanPage.searchDevice()
                            }
                        }
                    } // connect disconnect button
                } // 2nd row
            }// header main col
            DotMarker {
                width: 12
                height: 12
                color: deviceInfo? (deviceInfo.deviceIsConnected? "green":"red") : "transparent"
                anchors.rightMargin: 0
                anchors.bottom: undefined
                anchors.bottomMargin: undefined
                anchors.top: parent.top
            } // indicator top-right at row
        } // header row layout
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
                    ItemDelegate {
                        id: btSettingsMenuButton
                        //anchors.top: parent.top
                        //anchors.right: parent.right
                        Layout.fillWidth: true
                        visible: scanManager.featuresPrepared || (deviceInfo && deviceInfo.controllerState >=3)
                        focusPolicy: Qt.NoFocus
                        Image {
                            width: 48
                            height: 48
                            anchors.left: parent.left
                            source: "qrc:/images/extra/ring_scanner.png"
                        }
                        Image {
                            opacity: 0.6
                            anchors.right: parent.right
                            source: "qrc:/images/black/more_vert.png"
                        }
                        onClicked: {
                            openSettingsMenu()
                        }
                    } // btSettingsMenuButton
                } // battery level
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Barcode")
                        color: primaryColor
                    }
                    LabelSubheading {
                        id: theBarcodeValueField
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.barcodeValue.length >0 ? scanManager.barcodeValue : qsTr("no barcode")
                    }
                } // key
                RowLayout {
                    visible: scanManager.scanNotificationsActive
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 12
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: " "
                        color: primaryColor
                    }
                    ProgressBar {
                        id: commandProgressBar
                        Layout.preferredWidth: 3
                        Layout.fillWidth: true
                        leftPadding: 16
                        rightPadding: 10
                        indeterminate: true
                    }
                }
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("Manufacturer")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.manufacturerNameValue
                    }
                } // manufacturer
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("Model")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.modelNumberValue
                    }
                } // model number
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("S/N")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.serialNumberValue
                    }
                } // serial number
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("Hardware Revision")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.hardwareRevisionValue
                    }
                } // hw revision
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("Firmware Revision")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.firmwareRevisionValue
                    }
                } // fw revision
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("Software Revision")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.softwareRevisionValue
                    }
                } // sw revision
                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelBodySecondary {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        wrapMode: Text.WrapAnywhere
                        text: qsTr("System Id")
                    }
                    LabelBody {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        leftPadding: 16
                        rightPadding: 10
                        wrapMode: Text.WrapAnywhere
                        text:scanManager.systemIdValue
                    }
                } // sw revision
            } // main column
        } // root pane
    } // flickable

    FloatingActionButton {
        id: startScanNotificationsButton
        visible: scanManager.featuresPrepared
        backgroundColor: primaryColor
        imageSource: scanManager.scanNotificationsActive? "qrc:/images/"+iconOnPrimaryFolder+"/stop.png" : "qrc:/images/"+iconOnPrimaryFolder+"/play.png"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            if(scanManager.scanNotificationsActive) {
                scanManager.stopScanNotifications()
            } else {
                scanManager.startScanNotifications()
            }
        }
    } // FloatingActionButton

    // C O N N E C T - D I S C O N N E C T
    function disconnectFromDevice() {
        deviceInfo.disconnectFromDevice()
    }
    ReconnectYesNo {
        id: reconnectDialog
        parent: rootPane
        text: deviceInfo? qsTr("%1 is disconnected.\nTry to reconnect now ?\n\nCheck Device is ON, Battery OK\nBluetooth is On\n").arg(deviceInfo.deviceName) : "Reconnect"
        onIsYesChanged: {
            if(isYes) {
                isYes = false
                searchDevice()
            }
        }
    } // reconnectDialog
    // if disconnecting using the Button we don't want a dialog asking for reconnect
    property bool showDisconnectMessage: true
    function onDeviceDisconnected() {
        if(!showDisconnectMessage) {
            showDisconnectMessage = true
            return
        }
        // got the signal from elsewhere, so we ask user for reconnect
        // if this is our current tab
        if(appWindow.isBarcodeCurrentTab() && !isSearchRunning) {
            reconnectDialog.open()
        }
    }
    Connections {
        target: deviceInfo
        onDisconnected: onDeviceDisconnected()
    }

    // S E A R C H, DISCOVER, CONNECT, PREPARE
    Timer {
        id: reconnectDelayTimer
        interval: 1000
        repeat: false
        onTriggered: {
            deviceInfo.connectToDeviceAgain()
        }
    } // reconnectDelayTimer
    function startReconnectDelayTimer() {
        reconnectDelayTimer.start()
    }
    Timer {
        id: waitServicesDiscoveredTimer
        interval: 1000
        repeat: false
        onTriggered: {
            console.log("retry prepare services")
            scanManager.prepareServices()
        }
    } // waitServicesDiscoveredTimer
    function startWaitServicesDiscoveredTimer() {
        waitServicesDiscoveredTimer.start()
    }
    function onRetryDeviceConnectChanged() {
        if(!discoverDevicesPopup.isActive) {
            return
        }
        if(!deviceInfo.retryDeviceConnect) {
            console.log("we don't want to retry now")
            return
        }
        // check if user clicks STOP button
        if(discoverDevicesPopup.stopClicked) {
            discoverDevicesPopup.stopClicked = false
            discoverDevicesPopup.close()
            return
        }
        console.log("RETRY detected: YES #"+deviceInfo.retryCounter)
        discoverDevicesPopup.infoText = qsTr("Try to connect again: %1 of 5").arg(deviceInfo.retryCounter)
        discoverDevicesPopup.from = 0
        discoverDevicesPopup.to = 30
        discoverDevicesPopup.max = 60
        discoverDevicesPopup.showStopButton = true
        discoverDevicesPopup.startTimer()
        startReconnectDelayTimer()
    }
    Connections {
        target: deviceInfo
        onRetryDeviceConnectChanged: onRetryDeviceConnectChanged()
    }
    function isSearchRunning() {
        return discoverDevicesPopup.isActive
    }

    BTScanPopup {
        id: discoverDevicesPopup
        property bool isActive: false
        text: qsTr("Search Device, connect, prepare Services. Please wait ...")
        textTimeout: qsTr("Waited too long for end of discovering devices")
        imageSource: "qrc:/images/extra/scan.png"
        from: 0
        to: 30
        max: 45
        showStopButton: true
        function onDeviceDiscoveryActiveChanged() {
            console.log("DeviceDiscoveryActive changed: "+ btManager.deviceDiscoveryActive)
            if(!btManager.deviceDiscoveryActive) {
                if(btManager.devicesList.length === 0) {
                    // Close this Popup
                    discoverDevicesPopup.close()
                    appWindow.showInfo(qsTr("No Devices found. Switch Devices Off/On or Settings Bluetooth Off/On to clear cache."))
                    return
                }
                // we discovered devices - check if favorite device is there
                deviceInfo = btManager.searchDevice(scanManager.settingsFavoriteAddress)
                scanManager.setCurrentDevice(deviceInfo)
                if(scanManager.settingsFavoriteAddress.length > 0 && !scanManager.hasDevice) {
                    // Close this Popup
                    discoverDevicesPopup.close()
                    appWindow.showInfo(qsTr("%1 Devices found, but not Device %2.").arg(btManager.devicesList.length).arg(scanManager.settingsFavoriteAddress))
                    return
                }
                // we found the already known device
                // now connect and create expected services
                showStopButton = false
                deviceInfo.createExpectedServices()
                return
            }
        } // onDeviceDiscoveryActiveChanged
        function onCreateServicesActiveChanged() {
            if(!deviceInfo.createServicesActive) {
                // services are created
                if(deviceInfo.controllerState == 4) {
                    // all services discovered
                    scanManager.prepareServices()
                } else if(deviceInfo.controllerState == 3) {
                    // Attention: on some devices (Samsung Xvover 3 getting some errors 133 while connecting)
                    // later on those devices hang a while before getting state 4 (all services discovered)
                    // instead getting state 3 (discovering) - this can cause a timing problem and missing the correct state
                    // we wait a short moment and try again - this normaly fixes this
                    startWaitServicesDiscoveredTimer()
                }
            }
        } // onCreateServicesActiveChanged
        function onFeaturesPreparedChanged() {
            // Close this Popup
            discoverDevicesPopup.close()
        }

        Connections {
            target: discoverDevicesPopup.isActive? scanManager : null
            onFeaturesPreparedChanged: discoverDevicesPopup.onFeaturesPreparedChanged()
        }

        Connections {
            target: discoverDevicesPopup.isActive? deviceInfo : null
            onCreateServicesActiveChanged: discoverDevicesPopup.onCreateServicesActiveChanged()
        }
        Connections {
            target:discoverDevicesPopup.isActive? btManager : null
            onDeviceDiscoveryActiveChanged: discoverDevicesPopup.onDeviceDiscoveryActiveChanged()
        }
        onOpened: {
            isActive = true
            infoText = ""
            if(!scanManager.hasDevice) {
                btManager.startDeviceDiscovery()
            } else {
                if(deviceInfo.controllerState == 4 && !scanManager.featuresPrepared) {
                    // all services discovered
                    scanManager.prepareServices()
                } else {
                    deviceInfo.createExpectedServices()
                }
            }
        } // on opened
        onStopClickedChanged: {
            if(stopClicked) {
                btManager.stopDeviceDiscovery()
            }
        } // on stop clicked
        onClosed: {
            isActive = false
            showStopButton = true
        } // on closed
    }// discoverDevicesPopup
    // search device, connect and create expected services if known device
    function searchDevice() {
        // reset to initial values
        discoverDevicesPopup.from = 0
        discoverDevicesPopup.to = 30
        discoverDevicesPopup.max = 45
        discoverDevicesPopup.stopClicked = false
        console.log("search device")
        if(scanManager.hasDevice) {
            console.log("We already have a device")
            deviceInfo = scanManager.currentDevice()
            if(!scanManager.featuresPrepared) {
                discoverDevicesPopup.open()
            }
            return
        }
        // do we have a favorite ?
        if(scanManager.settingsFavoriteAddress.length > 0) {
            console.log("searching..."+scanManager.settingsFavoriteAddress)
            // search favorite in already scanned devices
            deviceInfo = btManager.searchDevice(scanManager.settingsFavoriteAddress)
            scanManager.setCurrentDevice(deviceInfo)
            if(!scanManager.hasDevice) {
                // not found - try to scan new
                btManager.searchAddress = scanManager.settingsFavoriteAddress
            }
            discoverDevicesPopup.open()
        } else {
            appWindow.gotoDeviceList()
        }
    } // searchDevice

    //  S E R V I C E S   and   C H A R A C T E R I S T I C S
    // Notifications new or empty Keys
    function onBarcodeValueChanged() {
        if(scanManager.barcodeValue.length > 0) {
            console.log("Barcode Value changed: "+ scanManager.barcodeValue)
            // theBarcodeValueField.text should be set from property
        } else {
            console.log("Barcode Value EMPTY ")
        }
    }
    Connections {
        target: scanManager
        onBarcodeValueChanged: onBarcodeValueChanged()
    }
    // autostart notifications when all is prepared
    function onFeaturesPreparedChanged() {
        if(scanManager.featuresPrepared) {
            if(!scanManager.scanNotificationsActive) {
                scanManager.startScanNotifications()
            }
        }
    }
    Connections {
        target: scanManager
        onFeaturesPreparedChanged: onFeaturesPreparedChanged()
    }
    // S E T T I N G S
    BTSettingsGeneralScan {
        id: mySettings
    }
    function openSettingsMenu() {
        btSettingsMenu.open()
    }
    Menu {
        id: btSettingsMenu
        modal:true
        dim: false
        closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape
        parent: btSettingsMenuButton
        x: parent.width - width
        transformOrigin: Menu.TopRight
        MenuItemWithIcon {
            itemText: qsTr("Settings")
            imageName: "settings.png"
            onTriggered: {
                mySettings.open()
            }
        }
        MenuSeparator {}
        MenuItemWithIcon {
            itemText: qsTr("Disconnect")
            imageName: "remove.png"
            onTriggered: {
                btRunGeneralScanPage.showDisconnectMessage = false
                btRunGeneralScanPage.disconnectFromDevice()
            }
        }
        onAboutToShow: {
            appWindow.modalMenuOpen = true
        }
        onAboutToHide: {
            appWindow.modalMenuOpen = false
            appWindow.resetFocus()
        }
    } // btSettingsMenu

    function onHasDeviceChanged() {
        deviceInfo = scanManager.currentDevice()
    }
    Connections {
        target: scanManager
        onHasDeviceChanged: btRunGeneralScanPage.onHasDeviceChanged()
    }

    // I N I T   and   C L E A N U P
    // called immediately after Loader.loaded
    function init() {
        console.log("Init from btRunGeneralScanPage")
    }
    // don't forget to adjust main.qml replaceDestination() !
    function destinationChanged() {
        console.log("DESTINATION changed to btRunGeneralScanPage")
        // coming from scanner a device can be stored at appWindow
        // if there's currently no or another device used by this controlle, we overwrite with the one from appWindow
        if (appWindow.currentBarcodeDeviceInfo && (btRunGeneralScanPage.deviceInfo !== appWindow.currentBarcodeDeviceInfo)) {
            console.log("scanner device not the same")
            scanManager.setCurrentDevice(appWindow.currentBarcodeDeviceInfo)
            btRunGeneralScanPage.deviceInfo = appWindow.currentBarcodeDeviceInfo
        } else if(!appWindow.currentBarcodeDeviceInfo &&!scanManager.hasDevice && btRunGeneralScanPage.deviceInfo) {
            // if scanner rebuilds all devices new we must delete the current one
            appWindow.showToast(qsTr("current device removed"))
            btRunGeneralScanPage.deviceInfo = null
        }
        searchDevice()
    }

    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from btRunGeneralScanPage")
    }

} // btRunGeneralScanPage
