// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0

import "../common"
import "../popups"

Pane {
    id: homePage
    height: appWindow.height
    property string name: "HomePage"
    topPadding: 12
    Image {
        id: logoImage
        property real portraitScale: 0.8
        property real landscapeScale: 0.5
        scale: isLandscape? landscapeScale : portraitScale
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: isLandscape? 24 : (appWindow.width - width - 12)/2
        width: sourceSize.width
        height: sourceSize.height
        fillMode: Image.PreserveAspectFit
        source: "qrc:/images/extra/sensors_image.png"
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignTop
        transformOrigin: Item.TopLeft
    } // image

    ColumnLayout {
        Layout.fillWidth: true
        anchors.right: parent.right
        anchors.left: parent.left
        transform: Translate {
            x: isLandscape ? logoImage.width+24:16
            y: isLandscape? 0 : logoImage.height
        } // translate
        RowLayout {
            LabelDisplay1 {
                Layout.maximumWidth: isLandscape? appWindow.width-12-logoImage.width : appWindow.width-12
                topPadding: 12
                rightPadding: 16
                wrapMode: Text.WordWrap
                text: qsTr("ekkes BTLE Example")
            }
        }
        RowLayout {
            LabelSubheading {
                Layout.maximumWidth: isLandscape? appWindow.width-12-logoImage.width : appWindow.width-12
                topPadding: 12
                rightPadding: 16
                wrapMode: Text.WordWrap
                text: qsTr("HeartRate, Waiter Lock, NFC Reader", "Barcode Scanner")
            }
        }
        RowLayout {
            LabelSubheading {
                Layout.maximumWidth: isLandscape? appWindow.width-12-logoImage.width : appWindow.width-12
                topPadding: 6
                rightPadding: 16
                wrapMode: Text.WordWrap
                text: qsTr("Bluetooth LE Devices")
            }
        }
    } // col layout

    // emitting a Signal could be another option
    Component.onDestruction: {
        cleanup()
    }

    function fitIntoWindow() {
        var portraitWidth = Math.min(appWindow.width,appWindow.height)
        var portraitHeight = Math.max(appWindow.width,appWindow.height)-60
        var portraitWidthScale = portraitWidth / logoImage.sourceSize.width
        var portraitHeightScale = portraitHeight / logoImage.sourceSize.height
        logoImage.portraitScale = Math.min(1.0, Math.min(portraitWidthScale, portraitHeightScale) * 0.8)
        var landscapeWidth = Math.max(appWindow.width,appWindow.height)
        var landscapeHeight = Math.min(appWindow.width,appWindow.height)-80
        var landscapeWidthScale = landscapeWidth / logoImage.sourceSize.width
        var landscapeHeightScale = landscapeHeight / logoImage.sourceSize.height
        logoImage.landscapeScale = Math.min(1.0, Math.min(landscapeWidthScale, landscapeHeightScale) * 0.5)
        console.log("portraitScale: "+logoImage.portraitScale+" landscapeScale: "+logoImage.landscapeScale)
    }

    // called immediately after Loader.loaded
    function init() {
        fitIntoWindow()

        console.log("Init done from Home Page")
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from Home Page")
    }
} // flickable
