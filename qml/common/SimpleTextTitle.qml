// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

ToolBar {
    id: titleToolBar
    property alias text: titleLabel.text

    RowLayout {
        focus: false
        spacing: 6
        anchors.fill: parent
        LabelTitle {
            id: titleLabel
            text: "ekke"
            leftPadding: 16
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: textOnPrimary
        }
        ToolButton {
            focusPolicy: Qt.NoFocus
            Image {
                id: buttonImage
                anchors.centerIn: parent
                source: "qrc:/images/"+iconOnPrimaryFolder+"/more_vert.png"
            }
            onClicked: {
                optionsMenu.open()   
            }
            Menu {
                id: optionsMenu
                modal:true
                dim: false
                closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape
                x: parent.width - width
                transformOrigin: Menu.TopRight
                MenuItem {
                    text: isDarkTheme? qsTr("Light Theme") : qsTr("Dark Theme")
                    onTriggered: {
                        themePalette = myApp.swapThemePalette()
                    }
                }
                MenuSeparator {}
                MenuItem {
                    text: qsTr("Select Primary Color")
                    onTriggered: {
                        popup.selectAccentColor = false
                        popup.open()
                    }
                }
                MenuSeparator {}
                MenuItem {
                    text: qsTr("Select Accent Color")
                    onTriggered: {
                        popup.selectAccentColor = true
                        popup.open()
                    }
                }
                MenuSeparator {}
                MenuItemWithIcon {
                    itemText: qsTr("Back")
                    imageName: "arrow_back.png"
                    onTriggered: {
                        //
                    }
                } // menu item back workaround QTBUG-61581
                onAboutToHide: {
                    appWindow.resetFocus()
                }
            } // end optionsMenu

        } // end ToolButton
    } // end RowLayout
} // end ToolBar


