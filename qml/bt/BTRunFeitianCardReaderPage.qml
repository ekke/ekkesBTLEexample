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
    id: btRunFeitianCardReaderPage
    property MyBluetoothDeviceInfo deviceInfo
    property bool isSearchRunning: discoverDevicesPopup.isActive

    focus: true
    property string name: "BTRunFeitianCardReaderPage"

    property bool cardAvailable: false
    property bool cardSupported: false
    property bool cardPowerOn: false
    property bool cardAppSelected: false
    property bool cardReadStatus: false
    property bool cardReadPersonalData: false

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
                        imageName: "smartcard.png"
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
                            btRunFeitianCardReaderPage.searchDevice()
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
                        text: cardReaderManager.featuresPrepared? qsTr("Disconnect") : qsTr("Initialize")
                        textColor: accentColor
                        textAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: 12
                        onClicked: {
                            if(cardReaderManager.featuresPrepared) {
                                btRunFeitianCardReaderPage.showDisconnectMessage = false
                                btRunFeitianCardReaderPage.disconnectFromDevice()
                            } else {
                                // search the device, connect and create expected services if needed
                                btRunFeitianCardReaderPage.searchDevice()
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
        contentHeight: root.implicitHeight + 64
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
                        visible: cardReaderManager.featuresPrepared || (deviceInfo && deviceInfo.controllerState >=3)
                        focusPolicy: Qt.NoFocus
                        Item {
                            width: 200
                            height: 106
                            anchors.left: parent.left
                            Image {
                                id: theCardReaderImage
                                anchors.fill: parent
                                source: cardAvailable? "qrc:/images/extra/feitian_egk.png" : "qrc:/images/extra/feitian.png"
                            }
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
                    Layout.topMargin: btSettingsMenuButton.visible? 42 : 0
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Reader working")
                        color: primaryColor
                    }
                    DotMarker {
                        visible: !cardReaderManager.cardNotificationsActive
                        color: "red"
                    }
                    ProgressBar {
                        visible: cardReaderManager.cardNotificationsActive
                        id: commandProgressBar
                        Layout.preferredWidth: 1
                        Layout.fillWidth: true
                        leftPadding: 16
                        rightPadding: 10
                        indeterminate: true
                    }
                } // progress

                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Card supported")
                        color: primaryColor
                    }
                    DotMarker {
                        color: cardSupported? "green":"red"
                    }
                } // card supported

                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Power On ?")
                        color: primaryColor
                    }
                    DotMarker {
                        color: cardPowerOn? "green":"red"
                    }
                } // Power on or off

                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Card App selected")
                        color: primaryColor
                    }
                    DotMarker {
                        color: cardAppSelected? "green":"red"
                    }
                } // Card app selected

                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Read Status Data")
                        color: primaryColor
                    }
                    DotMarker {
                        color: cardReadStatus? "green":"red"
                    }
                } // Card read status

                RowLayout {
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 1
                        text: qsTr("Read Personal Data")
                        color: primaryColor
                    }
                    DotMarker {
                        color: cardReadPersonalData? "green":"red"
                    }
                } // Card read personal data


                HorizontalListDivider{}
                RowLayout {
                    Layout.leftMargin: 6
                    Layout.rightMargin: 16
                    LabelSubheading {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: 1
                        text: qsTr("Protocol")
                        color: primaryColor
                    }
                    ButtonFlat {
                        visible: theCardDataValueField.text.length
                        Layout.alignment: Qt.AlignVCenter
                        text: qsTr("Clear Protocol")
                        textColor: accentColor
                        onClicked: {
                            theCardDataValueField.text = ""
                        }
                    }
                } // card data label
                RowLayout {
                    Layout.leftMargin: 6
                    Layout.rightMargin: 6
                    LabelBody {
                        id: theCardDataValueField
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredWidth: 3
                        wrapMode: Text.WrapAnywhere
                    }
                } // card data protocol

            } // main column
        } // root pane
    } // flickable

    FloatingActionMiniButton {
        id: startCardNotificationsButton
        visible: cardReaderManager.featuresPrepared
        backgroundColor: primaryColor
        imageSource: cardReaderManager.cardNotificationsActive? "qrc:/images/"+iconOnPrimaryFolder+"/stop.png" : "qrc:/images/"+iconOnPrimaryFolder+"/play.png"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            if(cardReaderManager.cardNotificationsActive) {
                cardReaderManager.stopCardNotifications()
            } else {
                cardReaderManager.startCardNotifications()
            }
        }
    } // FloatingActionButton
    // TODO protocol delete button

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
        if(appWindow.isCardReaderCurrentTab() && !isSearchRunning) {
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
            cardReaderManager.prepareServices()
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
                deviceInfo = btManager.searchDevice(cardReaderManager.settingsFavoriteAddress)
                cardReaderManager.setCurrentDevice(deviceInfo)
                if(cardReaderManager.settingsFavoriteAddress.length > 0 && !cardReaderManager.hasDevice) {
                    // Close this Popup
                    discoverDevicesPopup.close()
                    appWindow.showInfo(qsTr("%1 Devices found, but not Device %2.").arg(btManager.devicesList.length).arg(cardReaderManager.settingsFavoriteAddress))
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
                    cardReaderManager.prepareServices()
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
            target: discoverDevicesPopup.isActive? cardReaderManager : null
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
            if(!cardReaderManager.hasDevice) {
                btManager.startDeviceDiscovery()
            } else {
                if(deviceInfo.controllerState == 4 && !cardReaderManager.featuresPrepared) {
                    // all services discovered
                    cardReaderManager.prepareServices()
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
        if(cardReaderManager.hasDevice) {
            console.log("We already have a device")
            deviceInfo = cardReaderManager.currentDevice()
            if(!cardReaderManager.featuresPrepared) {
                discoverDevicesPopup.open()
            }
            return
        }
        // do we have a favorite ?
        if(cardReaderManager.settingsFavoriteAddress.length > 0) {
            console.log("searching..."+cardReaderManager.settingsFavoriteAddress)
            // search favorite in already scanned devices
            deviceInfo = btManager.searchDevice(cardReaderManager.settingsFavoriteAddress)
            cardReaderManager.setCurrentDevice(deviceInfo)
            if(!cardReaderManager.hasDevice) {
                // not found - try to scan new
                btManager.searchAddress = cardReaderManager.settingsFavoriteAddress
            }
            discoverDevicesPopup.open()
        } else {
            appWindow.gotoDeviceList()
        }
    } // searchDevice

    PopupYesNo {
        property string theUrl
        id: popupWrongWithUrl
        onClosed: {
            if(isYes) {
                Qt.openUrlExternally(theUrl)
            }
        }
    } // popupWrongCard

    //  S E R V I C E S   and   C H A R A C T E R I S T I C S
    // Notifications new or changed data
    function onCardDataValueChanged() {
        if(cardReaderManager.cardDataValue.length > 0) {
            if(theCardDataValueField.text.length) {
                theCardDataValueField.text = theCardDataValueField.text + "\n" + cardReaderManager.cardDataValue
            } else {
                theCardDataValueField.text = cardReaderManager.cardDataValue
            }
        } else {
            console.log("CardData Value EMPTY ")
        }
    }
    function onCardIn() {
        cardAvailable = true
        cardReaderManager.doPowerOn()
    }
    function onCardout() {
        cardReadPersonalData = false
        cardReadStatus = false
        cardAppSelected = false
        cardPowerOn = false
        cardSupported = false
        cardAvailable = false
    }
    function onReadATRSuccess() {
        cardSupported = true
        cardPowerOn = true
    }
    function onReadATRWrong(message, parseATRUrl) {
        cardSupported = false
        cardPowerOn = true
        if(parseATRUrl.length) {
            popupWrongWithUrl.text = qsTr("This Card Type is not implemented yet - you need an eGK G2.\nDo you want to see detailed Infos about the currently inserted Card ?")
            popupWrongWithUrl.theUrl = parseATRUrl
            popupWrongWithUrl.isYes = false
            popupWrongWithUrl.open()
        } else {
            if(message.length) {
                appWindow.showInfo(message)
            }
        }
    }
    function onAppSelectedSuccess() {
        cardAppSelected = true
    }
    function onAppSelectedFailed(message, apduResponseInfoUrl, apduResponse) {
        cardAppSelected = false
        if(apduResponseInfoUrl.length) {
            popupWrongWithUrl.text = message + qsTr("\nDo you want to see detailed Infos about the Response Code %1 ?").arg(apduResponse)
            popupWrongWithUrl.theUrl = apduResponseInfoUrl
            popupWrongWithUrl.isYes = false
            popupWrongWithUrl.open()
        } else {
            if(message.length) {
                appWindow.showInfo(message)
            }
        }
    }
    BTPopupStatusVD {
        id: popupStatusVD
    }

    function onStatusVDSuccess(statusVDMap) {
        cardReadStatus = true
        console.log("STATUS VD TIMESTAMP: "+statusVDMap.Timestamp)
        popupStatusVD.statusVDMap = statusVDMap
    }
    function onStatusVDFailed(message, apduResponseInfoUrl, apduResponse) {
        popupStatusVD.statusVDMap = ({})
        cardReadStatus = false
        if(apduResponseInfoUrl.length) {
            popupWrongWithUrl.text = message + qsTr("\nDo you want to see detailed Infos about the Response Code %1 ?").arg(apduResponse)
            popupWrongWithUrl.theUrl = apduResponseInfoUrl
            popupWrongWithUrl.isYes = false
            popupWrongWithUrl.open()
        } else {
            if(message.length) {
                appWindow.showInfo(message)
            }
        }
    }
    function onPersonalDataSuccess(pdMap) {
        cardReadPersonalData = true
        console.log("TEST XML SIZE: "+pdMap.XMLBytes)
    }
    function onPersonalDataFailed(message, apduResponseInfoUrl, apduResponse) {
        // cardPersonalDataMap = ({})
        cardReadPersonalData = false
        if(apduResponseInfoUrl.length) {
            popupWrongWithUrl.text = message + qsTr("\nDo you want to see detailed Infos about the Response Code %1 ?").arg(apduResponse)
            popupWrongWithUrl.theUrl = apduResponseInfoUrl
            popupWrongWithUrl.isYes = false
            popupWrongWithUrl.open()
        } else {
            if(message.length) {
                appWindow.showInfo(message)
            }
        }
    }
    Connections {
        target: cardReaderManager
        onCardDataValueChanged: onCardDataValueChanged()
        onCardIN: onCardIn()
        onCardOUT: onCardout()
        onReadATRSuccess: onReadATRSuccess()
        onReadATRWrong: onReadATRWrong(message, parseATRUrl)
        onAppSelectedSuccess: onAppSelectedSuccess()
        onAppSelectedFailed: onAppSelectedFailed(message, apduResponseInfoUrl, apduResponse)
        onStatusVDSuccess: onStatusVDSuccess(statusVDMap)
        onStatusVDFailed: onStatusVDFailed(message, apduResponseInfoUrl, apduResponse)
        onPersonalDataSuccess: onPersonalDataSuccess(pdMap)
        onPersonalDataFailed: onPersonalDataFailed(message, apduResponseInfoUrl, apduResponse)
    }
    // autostart notifications when all is prepared
    function onFeaturesPreparedChanged() {
        if(cardReaderManager.featuresPrepared) {
            if(!cardReaderManager.cardNotificationsActive) {
                cardReaderManager.startCardNotifications()
            }
        }
    }
    Connections {
        target: cardReaderManager
        onFeaturesPreparedChanged: onFeaturesPreparedChanged()
    }
    // S E T T I N G S
    BTSettingsFeitianCardReader {
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
        MenuItemWithIcon {
            enabled: btRunFeitianCardReaderPage.cardReadStatus
            itemText: qsTr("Status Data")
            imageName: "info.png"
            onTriggered: {
                popupStatusVD.open()
            }
        }
        MenuItemWithIcon {
            enabled: btRunFeitianCardReaderPage.cardReadPersonalData
            itemText: qsTr("Person Data")
            imageName: "person.png"
            onTriggered: {
                // TODO
            }
        }

        MenuSeparator {}
        MenuItemWithIcon {
            itemText: qsTr("Disconnect")
            imageName: "remove.png"
            onTriggered: {
                btRunFeitianCardReaderPage.showDisconnectMessage = false
                btRunFeitianCardReaderPage.disconnectFromDevice()
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
        deviceInfo = cardReaderManager.currentDevice()
    }
    Connections {
        target: cardReaderManager
        onHasDeviceChanged: btRunFeitianCardReaderPage.onHasDeviceChanged()
    }

    // I N I T   and   C L E A N U P
    // called immediately after Loader.loaded
    function init() {
        console.log("Init from btRunFeitianCardReaderPage")
    }
    // don't forget to adjust main.qml replaceDestination() !
    function destinationChanged() {
        console.log("DESTINATION changed to btRunFeitianCardReaderPage")
        // coming from scanner a device can be stored at appWindow
        // if there's currently no or another device used by this controlle, we overwrite with the one from appWindow
        if (appWindow.currentCardReaderDeviceInfo && (btRunFeitianCardReaderPage.deviceInfo !== appWindow.currentCardReaderDeviceInfo)) {
            console.log("scanner device not the same")
            cardReaderManager.setCurrentDevice(appWindow.currentCardReaderDeviceInfo)
            btRunFeitianCardReaderPage.deviceInfo = appWindow.currentCardReaderDeviceInfo
        } else if(!appWindow.currentCardReaderDeviceInfo &&!cardReaderManager.hasDevice && btRunFeitianCardReaderPage.deviceInfo) {
            // if scanner rebuilds all devices new we must delete the current one
            appWindow.showToast(qsTr("current device removed"))
            btRunFeitianCardReaderPage.deviceInfo = null
        }
        console.log("now search the device")
        searchDevice()
    }

    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from btRunFeitianCardReaderPage")
    }

} // btRunFeitianCardReaderPage
