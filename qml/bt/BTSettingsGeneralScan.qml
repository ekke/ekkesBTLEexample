// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"

Popup {
    id: thePopup
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.elevation: 8
    x: (appWindow.width - implicitWidth)
    implicitHeight: 200
    implicitWidth: 280
    LabelSubheading {
        id: label1
        leftPadding: 12
        width: parent.width - 24
        text: qsTr("Favorite Device")
        color: primaryColor
    }
    LabelSubheading {
        topPadding: 6
        id: value1
        leftPadding: 12 + 24
        width: parent.width - 48
        anchors.top: label1.bottom
        text: scanManager.settingsFavoriteAddress
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }


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
    }
    onClosed: {
        appWindow.backKeyfreezed = false
    }
}// thePopup
