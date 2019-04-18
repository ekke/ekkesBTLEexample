// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.2
import "../common"

Popup {
    id: popup
    modal: true
    closePolicy: Popup.NoAutoClose
    property alias text: popupLabel.text
    property bool isYes: false
    Material.elevation: 8
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    implicitHeight: 200
    implicitWidth: parent.width * .9
    ColumnLayout {
        width: parent.width - 32
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: 20
        RowLayout {
            LabelSubheading {
                id: popupLabel
                topPadding: 20
                leftPadding: 8
                rightPadding: 8
                text: ""
                color: popupTextColor
                horizontalAlignment: Text.AlignHCenter
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
                id: noButton
                Layout.preferredWidth: 1
                text: qsTr("No")
                textColor: accentColor
                onClicked: {
                    popup.isYes = false
                    popup.close()
                }
            }
            ButtonFlat {
                id: yesButton
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
