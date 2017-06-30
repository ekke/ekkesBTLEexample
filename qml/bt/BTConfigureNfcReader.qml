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

    property NfcMapping currentNfcMapping

    Material.elevation: 8

    x: (appWindow.width - width)/2
    y: (appWindow.height - height - 180)/2 +16
    width: Math.min(480, appWindow.width-32)
    height: Math.min(80* Math.max(nfcManager.mappingsList.length, 5)+52, appWindow.height-160)
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
            nfcManager.removeMapping(currentNfcMapping.tagId)
            dataEntryPane.visible = false
            dataEntryPane.isInitialized = false
        }
    }

    LabelSubheading {
        visible: nfcManager.mappingsList.length === 0
        anchors.centerIn: parent
        text: qsTr("No mappings available.\nUse the Tags to configure")
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
                text: qsTr("Configure Tag <--> User")
                color: primaryColor
            }
            HorizontalListDivider{}
        }

        model: nfcManager.mappingsList

        currentIndex: -1
        anchors.fill: parent

        delegate: ItemDelegate {
            id: itemDelegate
            width: popup.width
            implicitHeight: 80
            focusPolicy: Qt.NoFocus

            onClicked: {
                currentNfcMapping = model.modelData
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
                        text: model.modelData.tagId
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
                text: qsTr("TAG ID")
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
                text: currentNfcMapping? currentNfcMapping.tagId : ""
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
                        currentNfcMapping.userName = text
                        nfcManager.triggerMappingsDataUpdated()
                    }
                }
                Keys.onReturnPressed:  doneButton.forceActiveFocus()
            } // text
        }
    }

    function fillData() {
        userNameField.text = currentNfcMapping.userName
        dataEntryPane.isInitialized = true
        dataEntryPane.visible = true
        userNameField.forceActiveFocus()
    }

    function onTagIdValueChanged() {
        if(!appWindow.configuringNfcReaderActive || dataEntryPane.visible) {
            // ignore if not configuring or while in editing
            return
        }
        if(nfcManager.tagIdValue.length > 0 && nfcManager.tagIdValue != "\r") {
            currentNfcMapping = nfcManager.createOrGetMapping(nfcManager.tagIdValue)
            fillData()
        } else {
            // ignore empty key values
        }
    }
    Connections {
        target: nfcManager
        onTagIdValueChanged: onTagIdValueChanged()
    }

    onOpened: {
        appWindow.backKeyfreezed = true
        appWindow.configuringNfcReaderActive = true
    }
    onClosed: {
        appWindow.configuringNfcReaderActive = false
        appWindow.backKeyfreezed = false
    }

} // end popup
