// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import "../common"

Popup {
    id: popup
    modal: true
    closePolicy: Popup.NoAutoClose
    property alias text: popupLabel.text
    property bool isYes: false
    property  string deviceImageSource
    property int deviceImageSize: popup.width / 4
    Material.elevation: 8
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    implicitHeight: isLandscape? 280 : 360
    implicitWidth: isLandscape? 360 : 280
    parent: Overlay.overlay

    Item {
        id: deviceImageItem
        visible: deviceImageSource.length
        anchors.top: popup.top
        anchors.left: popup.left
        width: deviceImageSize
        height: deviceImageSize
        Image {
            anchors.fill: parent
            source: deviceImageSource
        } // theScannerImage
    } // deviceImageItem


    ColumnLayout {
        width: parent.width - 32
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: deviceImageSource.length ? (isLandscape? parent.top : deviceImageItem.bottom) : parent.top
        spacing: 10
        RowLayout {
            LabelSubheading {
                id: popupLabel
                topPadding: 20
                leftPadding: deviceImageSource.length && isLandscape ? deviceImageSize + 24 : 24
                rightPadding: 8
                text: ""
                color: popupTextColor
                // horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            } // popupLabel
        } // row label
        RowLayout {
            spacing: 20
            Item {
                Layout.preferredWidth: 1
                Layout.fillWidth: true
            }
            ButtonFlat {
                Layout.preferredWidth: 1
                text: qsTr("No")
                textColor: accentColor
                onClicked: {
                    popup.isYes = false
                    popup.close()
                }
            }
            ButtonFlat {
                Layout.preferredWidth: 1
                text: qsTr("Yes")
                textColor: primaryColor
                onClicked: {
                    popup.isYes = true
                    popup.close()
                }
            }
        } // row layout
    }


    // workaround https://bugreports.qt.io/browse/QTBUG-59670
    onOpened: {
        appWindow.modalPopupActive = true

    }
    onClosed: {
        appWindow.modalPopupActive = false
    }
}
