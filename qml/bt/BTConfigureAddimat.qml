// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"
import org.ekkescorner.btle 1.0

Popup {
    id: popup

    property AddimatMapping currentAddimatMapping

    Material.elevation: 8

    x: (appWindow.width - width)/2
    y: (appWindow.height - height - 180)/2 +16
    width: Math.min(480, appWindow.width-32)
    height: Math.min(80* Math.max(lockManager.mappingsList.length, 5)+52, appWindow.height-160)
    transformOrigin: Popup.Center

    closePolicy: Popup.NoAutoClose
    modal: true

    ButtonFlat {
        id: doneButton
        z:2
        text: qsTr("Done")
        textColor: accentColor
        textAlignment: Text.AlignRight
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 12
        onClicked: {
            if(dataEntryPane.visible) {
                dataEntryPane.visible = false
                dataEntryPane.isInitialized = false
            } else {
                popup.close()
            }
        }
    }

    ButtonFlat {
        visible: dataEntryPane.visible
        z:2
        text: qsTr("Delete Mapping")
        textColor: accentColor
        textAlignment: Text.AlignRight
        anchors.bottom: parent.bottom
        anchors.right: doneButton.left
        anchors.rightMargin: 24
        onClicked: {
            lockManager.removeMapping(currentAddimatMapping.addimatKey)
            dataEntryPane.visible = false
            dataEntryPane.isInitialized = false
        }
    }

    LabelSubheading {
        visible: lockManager.mappingsList.length === 0
        anchors.centerIn: parent
        text: qsTr("No mappings available.\nUse the keys to configure")
    }


    ListView {
        id: listView
        visible: !dataEntryPane.visible
        clip: true
        focus: true
        spacing: 1

        header: Column {
            width: popup.width
            LabelTitle {
                anchors.horizontalCenter: parent.horizontalCenter
                bottomPadding: 12
                text: qsTr("Configure Key <--> User")
                color: primaryColor
            }
            HorizontalListDivider{}
        }

        model: lockManager.mappingsList

        currentIndex: -1
        anchors.fill: parent

        delegate: ItemDelegate {
            id: itemDelegate
            width: popup.width
            implicitHeight: 80
            focusPolicy: Qt.NoFocus

            onClicked: {
                currentAddimatMapping = model.modelData
                fillData()
            }
            // CONTENT
            contentItem: Column {
                anchors.left: parent.left
                anchors.right: parent.rigth
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                RowLayout {
                    width: parent.width-32
                    spacing: 0
                    LabelTitle {
                        text: model.modelData.userName
                        wrapMode: Label.WordWrap
                        maximumLineCount: 2
                        elide: Label.ElideRight
                        color: primaryColor
                    }
                } // end Row
                RowLayout {
                    width: parent.width-32
                    spacing: 0
                    LabelSubheading {
                        text: model.modelData.addimatKey
                        wrapMode: Label.WordWrap
                        maximumLineCount: 2
                        elide: Label.ElideRight
                    }
                } // end Row
                RowLayout {
                    width: parent.width-32
                    spacing: 0
                    LabelBody {
                        Layout.preferredWidth: 1
                        text: model.modelData.userPIN
                        wrapMode: Label.WordWrap
                        maximumLineCount: 2
                        elide: Label.ElideRight
                    }
                    LabelBody {
                        Layout.preferredWidth: 1
                        text: model.modelData.userId
                        wrapMode: Label.WordWrap
                        maximumLineCount: 2
                        elide: Label.ElideRight
                    }
                } // end Row
            } // content col
            // dividers below each delegate
            Rectangle {
                y: parent.height
                width: parent.width
                height: 1
                opacity: dividerOpacity
                color: dividerColor
            }
        } // end delegateItem
        ScrollIndicator.vertical: ScrollIndicator { }
    } // end listView

    Pane {
        id: dataEntryPane
        property bool isInitialized: false
        visible: false
        anchors.fill: parent
        topPadding: 12
        ColumnLayout {
            width: parent.width
            LabelTitle {
                Layout.leftMargin: 12
                text: qsTr("Key ID")
                wrapMode: Label.WordWrap
                maximumLineCount: 2
                elide: Label.ElideRight
                color: primaryColor
            }
            LabelSubheading {
                Layout.leftMargin: 36
                Layout.rightMargin: 36
                wrapMode: Label.WordWrap
                maximumLineCount: 2
                elide: Label.ElideRight
                text: currentAddimatMapping? currentAddimatMapping.addimatKey : ""
            }
            LabelSubheading {
                Layout.leftMargin: 12
                text: qsTr("User Name")
                wrapMode: Label.WordWrap
                maximumLineCount: 2
                elide: Label.ElideRight
                color: primaryColor
            } // label
            TextField {
                id: userNameField
                Layout.fillWidth: true
                Layout.leftMargin: 36
                Layout.rightMargin: 36
                onTextChanged: {
                    if(dataEntryPane.isInitialized) {
                        currentAddimatMapping.userName = text
                        lockManager.triggerMappingsDataUpdated()
                    }
                }
                Keys.onReturnPressed:  userPINField.forceActiveFocus()
            } // text
            LabelSubheading {
                Layout.leftMargin: 12
                text: qsTr("PIN")
                wrapMode: Label.WordWrap
                maximumLineCount: 2
                elide: Label.ElideRight
                color: primaryColor
            } // label
            TextField {
                id: userPINField
                Layout.fillWidth: true
                Layout.leftMargin: 36
                Layout.rightMargin: 36
                onTextChanged: {
                    if(dataEntryPane.isInitialized) {
                        currentAddimatMapping.userPIN = text
                        lockManager.triggerMappingsDataUpdated()
                    }
                }
                Keys.onReturnPressed:  userIdField.forceActiveFocus()
            } // text
            LabelSubheading {
                Layout.leftMargin: 12
                text: qsTr("User ID")
                wrapMode: Label.WordWrap
                maximumLineCount: 2
                elide: Label.ElideRight
                color: primaryColor
            } // label
            TextField {
                id: userIdField
                Layout.fillWidth: true
                Layout.leftMargin: 36
                Layout.rightMargin: 36
                onTextChanged: {
                    if(dataEntryPane.isInitialized) {
                        currentAddimatMapping.userId = text
                        lockManager.triggerMappingsDataUpdated()
                    }
                }
                Keys.onReturnPressed:  doneButton.forceActiveFocus()
            } // text
        }
    }

    function fillData() {
        userNameField.text = currentAddimatMapping.userName
        userPINField.text = currentAddimatMapping.userPIN
        userIdField.text = currentAddimatMapping.userId
        dataEntryPane.isInitialized = true
        dataEntryPane.visible = true
        userNameField.forceActiveFocus()
    }

    function onKeyIdValueChanged() {
        if(!appWindow.configuringAddimatActive || dataEntryPane.visible) {
            // ignore if not configuring or while in editing
            return
        }
        if(lockManager.keyIdValue.length > 0 && lockManager.keyIdValue != "0000000000000000") {
            currentAddimatMapping = lockManager.createOrGetMapping(lockManager.keyIdValue)
            fillData()
        } else {
            // ignore empty key values
        }
    }
    Connections {
        target: lockManager
        onKeyIdValueChanged: onKeyIdValueChanged()
    }

    onOpened: {
        appWindow.backKeyfreezed = true
        appWindow.configuringAddimatActive = true
    }
    onClosed: {
        appWindow.configuringAddimatActive = false
        appWindow.backKeyfreezed = false
    }

} // end popup
