// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.5
import QtGraphicalEffects 1.0
import "../common"

Popup {
    id: thePopup
    property var pdMap: ({})
    property string pdXml: ""
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
        width: thePopup.implicitWidth
        height: parent. height
        contentHeight: root.implicitHeight + 60
        ColumnLayout {
            id: root
            width: thePopup.implicitWidth
            RowLayout {
                Layout.leftMargin: 16
                LabelTitle {
                    Layout.preferredWidth: 1
                    text: qsTr("Personal Data")
                    color: primaryColor
                }
            } // title row
            HorizontalListDividerL{}
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("VersichertenID")
                    color: primaryColor
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                LabelSubheading {
                    id: versichertenIDText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // versicherten ID row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Birthday")
                    color: primaryColor
                }
                LabelSubheading {
                    id: birthdayText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // birthday row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("First Name")
                    color: primaryColor
                }
                LabelSubheading {
                    id: firstNameText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // vorname row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Last Name")
                    color: primaryColor
                }
                LabelSubheading {
                    id: lastNameText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // nachname row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Gender")
                    color: primaryColor
                }
                LabelSubheading {
                    id: genderText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // gender row
            RowLayout {
                visible: prefixText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Prefix")
                    color: primaryColor
                }
                LabelSubheading {
                    id: prefixText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // prefix row
            RowLayout {
                visible: addOnText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Name Addon")
                    color: primaryColor
                }
                LabelSubheading {
                    id: addOnText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // addon row
            RowLayout {
                visible: titleText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Titel")
                    color: primaryColor
                }
                LabelSubheading {
                    id: titleText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // title row
            RowLayout {
                visible: zipCodeText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("ZipCode")
                    color: primaryColor
                }
                LabelSubheading {
                    id: zipCodeText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // zipCode row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("City")
                    color: primaryColor
                }
                LabelSubheading {
                    id: cityText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // city row
            RowLayout {
                visible: streetText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Street")
                    color: primaryColor
                }
                LabelSubheading {
                    id: streetText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // street row
            RowLayout {
                visible: houseNumberText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("House No")
                    color: primaryColor
                }
                LabelSubheading {
                    id: houseNumberText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // hausnumber row
            RowLayout {
                visible: addressAddonText.text.length
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("House No")
                    color: primaryColor
                }
                LabelSubheading {
                    id: addressAddonText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // address addon row
            RowLayout {
                Layout.leftMargin: 16
                LabelSubheading {
                    Layout.preferredWidth: 1
                    text: qsTr("Country")
                    color: primaryColor
                }
                LabelSubheading {
                    id: countryText
                    Layout.preferredWidth: 2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
            } // country row
            HorizontalListDividerL{}
            LabelSubheading {
                Layout.leftMargin: 16
                text: qsTr("Personal Data as XML")
                color: primaryColor
            }
            LabelBody {
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: pdXml
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            HorizontalListDividerL{}
            LabelBodySecondary {
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                text: qsTr("Find StatusData and InsuranceData (VD, GVD) from Options Menu in previous Page.")
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
        if(pdMap.Versicherten_ID) {
            versichertenIDText.text = pdMap.Versicherten_ID
        }
        if(pdMap.Geburtsdatum) {
            birthdayText.text = pdMap.Geburtsdatum
        }
        if(pdMap.Vorname) {
            firstNameText.text = pdMap.Vorname
        }
        if(pdMap.Nachname) {
            lastNameText.text = pdMap.Nachname
        }
        if(pdMap.Geschlecht) {
            genderText.text = pdMap.Geschlecht
        }
        if(pdMap.Vorsatzwort) {
            prefixText.text = pdMap.Vorsatzwort
        }
        if(pdMap.Namenszusatz) {
            addOnText.text = pdMap.Namenszusatz
        }
        if(pdMap.Titel) {
            titleText.text = pdMap.Titel
        }
        if(pdMap.Postleitzahl) {
            zipCodeText.text = pdMap.Postleitzahl
        }
        if(pdMap.Ort) {
            cityText.text = pdMap.Ort
        }
        if(pdMap.Strasse) {
            streetText.text = pdMap.Strasse
        }
        if(pdMap.Hausnummer) {
            houseNumberText.text = pdMap.Hausnummer
        }
        if(pdMap.Wohnsitzlaendercode) {
            countryText.text = pdMap.Wohnsitzlaendercode
        }
    }
    onClosed: {
        appWindow.backKeyfreezed = false
    }
}// thePopup
