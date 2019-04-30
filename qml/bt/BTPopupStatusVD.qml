// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"

Popup {
    id: thePopup
    property var statusVDMap: ({})
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.elevation: 8
    x: (appWindow.width - implicitWidth)
    implicitHeight: 200
    implicitWidth: appWindow.width*.9
    ColumnLayout {
        width: parent.width
        RowLayout {
            Layout.leftMargin: 16
            LabelTitle {
                Layout.preferredWidth: 1
                text: qsTr("Status VD Data")
                color: primaryColor
            }
        } // title row
        HorizontalListDividerL{}
        RowLayout {
            Layout.leftMargin: 16
            LabelSubheading {
                Layout.preferredWidth: 1
                text: qsTr("Status")
                color: primaryColor
            }
            LabelSubheading {
                id: statusText
                Layout.preferredWidth: 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        } // status row
        RowLayout {
            Layout.leftMargin: 16
            LabelSubheading {
                Layout.preferredWidth: 1
                text: qsTr("Timestamp")
                color: primaryColor
            }
            LabelSubheading {
                id: timestampText
                Layout.preferredWidth: 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        } // timestamp row
        RowLayout {
            Layout.leftMargin: 16
            LabelSubheading {
                Layout.preferredWidth: 1
                text: qsTr("Version")
                color: primaryColor
            }
            LabelSubheading {
                id: versionText
                Layout.preferredWidth: 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        } // timestamp row
    } // col

    ButtonFlat {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.rightMargin: 6
        textColor: accentColor
        text: qsTr("Done")
        onClicked: {
            thePopup.close()
        }
    }

    onOpened: {
        appWindow.backKeyfreezed = true
        statusText.text = statusVDMap.Status
        timestampText.text = statusVDMap.Timestamp
        versionText.text = statusVDMap.Version
    }
    onClosed: {
        appWindow.backKeyfreezed = false
    }
}// thePopup
