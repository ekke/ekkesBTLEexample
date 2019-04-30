// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.5
import QtGraphicalEffects 1.0
import "../common"

Popup {
    id: thePopup
    property string vdXml: ""
    property string gvdXml: ""
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.elevation: 8
    x: (appWindow.width - implicitWidth)
    implicitHeight: appWindow.height *.95
    implicitWidth: appWindow.width*.95
    ScrollView {
        id: scrollView
        clip: true
        width: parent.width
        height: parent. height
        contentHeight: root.implicitHeight + 60
        ColumnLayout {
            id: root
            width: thePopup.implicitWidth
            RowLayout {
                Layout.leftMargin: 16
                LabelTitle {
                    Layout.preferredWidth: 1
                    text: qsTr("Personal Insurance Data (VD) as XML")
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    color: primaryColor
                }
            } // title row
            LabelBody {
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: vdXml
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            HorizontalListDividerL{}
            RowLayout {
                Layout.leftMargin: 16
                LabelTitle {
                    Layout.preferredWidth: 1
                    text: qsTr("Protected Insurance Data (GVD) as XML")
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    color: primaryColor
                }
            } // title row
            LabelBody {
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: gvdXml
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        } // col
    } // scrollView

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
