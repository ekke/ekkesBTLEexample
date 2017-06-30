// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import org.ekkescorner.btle 1.0

import "../pages"
import "../common"

Page {
    id: navPage
    property alias depth: navPane.depth
    property string name: "DevicesNavPage"
    // index to get access to Loader (Destination)
    property int myIndex: index

    StackView {
        id: navPane
        property var deviceTypeImages: [
            "block.png",
            "chart.png",
            "form.png",
            "key.png",
            "tag.png",
            "signal.png",
            "tag.png"
        ]
        anchors.fill: parent
        property string name: "DevicesNavPane"
        focus: true

        initialItem: LEDevicesListPage{
            id: initialItem
        }

        Loader {
            id: servicesListPageLoader
            property MyBluetoothDeviceInfo deviceInfo
            active: false
            visible: false
            source: "LEServicesListPage.qml"
            onLoaded: {
                item.deviceInfo = servicesListPageLoader.deviceInfo
                navPane.push(item)
                item.init()
            }
        }

        Loader {
            id: characteristicListPageLoader
            property MyBluetoothDeviceInfo deviceInfo
            property MyBluetoothServiceInfo serviceInfo
            active: false
            visible: false
            source: "LECharacteristicListPage.qml"
            onLoaded: {
                item.deviceInfo = characteristicListPageLoader.deviceInfo
                item.serviceInfo = characteristicListPageLoader.serviceInfo
                navPane.push(item)
                item.init()
            }
        }

        Loader {
            id: characteristicDetailPageLoader
            property MyBluetoothDeviceInfo deviceInfo
            property MyBluetoothServiceInfo serviceInfo
            property MyBluetoothCharacteristic characteristic
            active: false
            visible: false
            source: "LECharacteristicDetailPage.qml"
            onLoaded: {
                item.deviceInfo = characteristicDetailPageLoader.deviceInfo
                item.serviceInfo = characteristicDetailPageLoader.serviceInfo
                item.characteristic = characteristicDetailPageLoader.characteristic
                navPane.push(item)
                item.init()
            }
        }

        Loader {
            id: writeValuePageLoader
            property MyBluetoothDeviceInfo deviceInfo
            property MyBluetoothServiceInfo serviceInfo
            property MyBluetoothCharacteristic characteristic
            property bool withResponse: false
            property bool asHex: false
            active: false
            visible: false
            source: "BTWriteValuePage.qml"
            onLoaded: {
                item.deviceInfo = writeValuePageLoader.deviceInfo
                item.serviceInfo = writeValuePageLoader.serviceInfo
                item.characteristic = writeValuePageLoader.characteristic
                item.withResponse = writeValuePageLoader.withResponse
                item.asHex = writeValuePageLoader.asHex
                navPane.push(item)
                item.init()
            }
        }

        function pushServices(deviceInfo) {
            servicesListPageLoader.deviceInfo = deviceInfo
            servicesListPageLoader.active = true
        }

        function pushCharacteristics(deviceInfo, serviceInfo) {
            characteristicListPageLoader.deviceInfo = deviceInfo
            characteristicListPageLoader.serviceInfo = serviceInfo
            characteristicListPageLoader.active = true
        }

        function pushCharacteristicDetail(deviceInfo, serviceInfo, characteristic) {
            characteristicDetailPageLoader.deviceInfo = deviceInfo
            characteristicDetailPageLoader.serviceInfo = serviceInfo
            characteristicDetailPageLoader.characteristic = characteristic
            characteristicDetailPageLoader.active = true
        }

        function pushWriteValue(deviceInfo, serviceInfo, characteristic, withResponse, asHex) {
            writeValuePageLoader.deviceInfo = deviceInfo
            writeValuePageLoader.serviceInfo = serviceInfo
            writeValuePageLoader.characteristic = characteristic
            writeValuePageLoader.withResponse = withResponse
            writeValuePageLoader.asHex = asHex
            writeValuePageLoader.active = true
        }

        function isThisPageOnTop(myName) {
            return navPane.top.name === myName
        }

        function findPage(pageName) {
            var targetPage = find(function(item) {
                return item.name == pageName;
            })
            if(targetPage) {
                return targetPage.StackView.index
            } else {
                console.log("Page not found in StackView: "+pageName)
                return -1
            }
        }
        function backToPage(targetStackIndex) {
            for (var i=depth-1; i > targetStackIndex; i--) {
                popOnePage()
            }
        }

        function backToRootPage() {
            for (var i=depth-1; i > 0; i--) {
                popOnePage()
            }
        }

        function popOnePage() {
            var page = pop()
            if(page.name == "ServicesListPage") {
                servicesListPageLoader.active = false
                return
            }
            if(page.name == "CharacteristicsListPage") {
                characteristicListPageLoader.active = false
                return
            }
            if(page.name == "CharacteristicDetailPage") {
                characteristicDetailPageLoader.active = false
                return
            }
            if(page.name == "BTWriteValuePage") {
                writeValuePageLoader.active = false
                return
            }
        } // popOnePage

        function openDevice(labelField, deviceInfo) {
            deviceMenu.parent = labelField
            deviceMenu.deviceInfo = deviceInfo
            deviceMenu.open()
        }
        Menu {
            id: deviceMenu
            // workaround set closePolicy https://bugreports.qt.io/browse/QTBUG-59670
            closePolicy: Popup.NoAutoClose
            property MyBluetoothDeviceInfo deviceInfo
            MenuItemWithIcon {
                enabled: deviceMenu.deviceInfo? deviceMenu.deviceInfo.deviceType > 0 : false
                itemText: deviceMenu.deviceInfo && deviceMenu.deviceInfo.deviceType> 0? qsTr("Select") : qsTr("unknown Type")
                imageName: deviceMenu.deviceInfo? navPane.deviceTypeImages[deviceMenu.deviceInfo.deviceType] : ""
                onTriggered: {
                    switch (deviceMenu.deviceInfo.deviceType) {
                    case 1:
                        // check if there's another device already connected
                        if(heartRateManager.hasDevice) {
                            if(heartRateManager.getSettingsFavoriteAddress !== deviceMenu.deviceInfo.deviceAddress) {
                                if(heartRateManager.isCurrentDeviceConnected()) {
                                    appWindow.showInfo(qsTr("There's another HeartRate device connected.\nPlease disconnect before selecting another one"))
                                    return
                                }
                            }
                        }
                        appWindow.gotoHeartRate(deviceMenu.deviceInfo)
                        break
                    case 2:
                        // appWindow.gotoWeight(deviceMenu.deviceInfo)
                        break
                    case 3:
                        // check if there's another device already connected
                        if(lockManager.hasDevice) {
                            if(lockManager.getSettingsFavoriteAddress !== deviceMenu.deviceInfo.deviceAddress) {
                                if(lockManager.isCurrentDeviceConnected()) {
                                    appWindow.showInfo(qsTr("There's another Addimat Waiter Lock connected.\nPlease disconnect before selecting another one"))
                                    return
                                }
                            }
                        }
                        appWindow.gotoLock(deviceMenu.deviceInfo)
                        break
                    case 4:
                        // appWindow.gotoBeacon(deviceMenu.deviceInfo)
                        break
                    case 5:
                        // appWindow.gotoPegelMeter(deviceMenu.deviceInfo)
                        break
                    case 6:
                        // check if there's another device already connected
                        if(nfcManager.hasDevice) {
                            if(nfcManager.getSettingsFavoriteAddress !== deviceMenu.deviceInfo.deviceAddress) {
                                if(nfcManager.isCurrentDeviceConnected()) {
                                    appWindow.showInfo(qsTr("There's another NFC Reader connected.\nPlease disconnect before selecting another one"))
                                    return
                                }
                            }
                        }
                        appWindow.gotoNfc(deviceMenu.deviceInfo)
                        break
                    }
                } // triggered
            } // menu item goto
            MenuSeparator {}
            MenuItemWithIcon {
                itemText: deviceMenu.deviceInfo? qsTr("Explore Services") : ""
                imageName: "settings.png"
                onTriggered: {
                    navPane.pushServices(deviceMenu.deviceInfo)
                }
            } // menu item explore
            //menu item mark as type
            MenuSeparator {}
            MenuItemWithIcon {
                itemText: deviceMenu.deviceInfo? (deviceMenu.deviceInfo.deviceIsConnected? qsTr("Disconnect now") : qsTr("Connect now")) : ""
                imageName: deviceMenu.deviceInfo? (deviceMenu.deviceInfo.deviceIsConnected? "clear.png":"done.png") : ""
                onTriggered: {
                    if(deviceMenu.deviceInfo.deviceIsConnected) {
                        initialItem.disconnectFromDevice(deviceMenu.deviceInfo)
                    } else {
                        initialItem.connectToDevice(deviceMenu.deviceInfo)
                    }
                }
            } // menu item connect disconnect
            MenuSeparator {}
            MenuItemWithIcon {
                itemText: qsTr("Back")
                imageName: "arrow_back.png"
                onTriggered: {
                    //
                }
            } // menu item explore
            onClosed: {
                deviceMenu.parent = initialItem
            }
        } // menu

    } // navPane

    function destinationAboutToChange() {
        // nothing
    }

    // triggered from BACK KEYs:
    // a) Android system BACK
    // b) Back Button from TitleBar
    function goBack() {
        // check if goBack is allowed
        //
        navPane.popOnePage()
    }

    Component.onDestruction: {
        cleanup()
    }

    function init() {
        console.log("INIT DevicesNavPane")
        initialItem.init()
    }
    function cleanup() {
        console.log("CLEANUP DevicesNavPane")
    }

} // navPage
