// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0

import "../common"

Flickable {
    id: flickable
    // index to get access to Loader (Destination)
    property int myIndex: index
    contentHeight: root.implicitHeight
    // StackView manages this, so please no anchors here
    // anchors.fill: parent
    property string name: "Help"

    Pane {
        id: root
        anchors.fill: parent
        bottomPadding: 16
        ColumnLayout {
            anchors.right: parent.right
            anchors.left: parent.left

            RowLayout {
                LabelSubheading {
                    topPadding: 16
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("You need help ?")
                }
            }
            RowLayout {
                Image {
                    Layout.leftMargin: 16
                    source: "qrc:/images/extra/ekke-thumbnail.png"
                }
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("contact ekke via Twitter @ekkescorner")
                    color: accentColor
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("To test all the features some Bluetooth LE Hardware is needed.\nAt the moment these BT LE Devices are supported:\nAddimat Lock, NFC Reader, HeartRate Monitor.")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("BT LE Device 'Addimat Lock'")
                    color: primaryColor
                }
            }
            RowLayout {
                Image {
                    Layout.leftMargin: 16
                    source: "qrc:/images/extra/addimat_image.png"
                }
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Get more Info about Addimat BT from <a href=\"https://www.addimat.ch/de/codestifte-stiftschloesser/stiftschlosser/\">Addimat Product Page</a>")
                    onLinkActivated: Qt.openUrlExternally("https://www.addimat.ch/de/codestifte-stiftschloesser/stiftschlosser/")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Addimat is a lock where each User owns a magnetic key. Uisng Addimat it's easy to Login / Logout. Each magnetic key has a unique ID you can map to the User. Some of my customers are using Addimat for teams sharing a Tablet. Secure switching bewtween Users is simple plug-in and out of the magnetic key.")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("BT LE Device 'NFC Reader'")
                    color: primaryColor
                }
            }
            RowLayout {
                Image {
                    Layout.leftMargin: 16
                    source: "qrc:/images/extra/csg_image.png"
                }
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Get more Info from <a href=\"http://bit.ly/nfcReaderInfo\">NFC Reader Product Page</a>")
                    onLinkActivated: Qt.openUrlExternally("http://bit.ly/nfcReaderInfo")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("In mobile business APPs often NFC cards are used to identify a person. Each NFC Tag has a unique ID. Mapping this ID to a Attendee of a Conference / Training / Event allows easy registration. Some Tablets don't have a NFC Chip, some Smartphones don't allow access to the ID. Connecting a NFC Reader via Bluetooth LE enables all such mobile devices to use NFC for identification.")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("BT LE Device 'Heart Rate Monitor'")
                    color: primaryColor
                }
            }
            RowLayout {
                Image {
                    Layout.leftMargin: 16
                    source: "qrc:/images/extra/hr_image.png"
                }
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("A HeartRate Monitor is the most used example in BT LE APPs, so I also included one. I tested with 'Scosche Rhytm+ HeartRateMonitor'")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("APP Workflow")
                    color: primaryColor
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("To learn more about 'ekkes BTLE example' APP take a look at <a href=\"http://bit.ly/ekkeBTLEapp\">APP Documentation</a>")
                    onLinkActivated: Qt.openUrlExternally("http://bit.ly/ekkeBTLEapp")
                }
            }
            HorizontalDivider {}

        } // col layout
    } // root
    ScrollIndicator.vertical: ScrollIndicator { }

    // emitting a Signal could be another option
    Component.onDestruction: {
        cleanup()
    }

    // called immediately after Loader.loaded
    function init() {
        console.log("Init done from HELP")
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from HELP")
    }
} // flickable
