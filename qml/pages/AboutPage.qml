// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import "../common"

Flickable {
    id: flickable
    // index to get access to Loader (Destination)
    property int myIndex: index
    contentHeight: root.implicitHeight
    // StackView manages this, so please no anchors here
    // anchors.fill: parent
    property string name: "About"

    Pane {
        id: root
        anchors.fill: parent
        ColumnLayout {
            anchors.right: parent.right
            anchors.left: parent.left
            RowLayout {
                Image {
                    source: "qrc:/images/extra/sensors_image.png"
                }
                LabelHeadline {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("ekkes BTLE example\nQt 5.9 x-platform\nQtQuickControls2")
                    color: primaryColor
                }
            }
            HorizontalDivider {}
            RowLayout {
                Image {
                    source: "qrc:/images/extra/ekke-thumbnail.png"
                }
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("ekkes BTLE example APP\ndeveloped by ekke (@ekkescorner)")
                    color: accentColor
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("This APP is developed with Qt 5.9 and <a href=\"http://doc.qt.io/qt-5/qtquickcontrols2-index.html\">QtQuickControls2</a>.")
                    onLinkActivated: Qt.openUrlExternally("http://doc.qt.io/qt-5/qtquickcontrols2-index.html")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("All work is done in my spare time and complete sourcecode is available at <a href=\"https://github.com/ekke/ekkesBTLEexample\">Github</a>")
                    onLinkActivated: Qt.openUrlExternally("https://github.com/ekke/ekkesBTLEexample")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("This APP is NOT a production-ready APP - it's an APP for developers to see what can be done with Qt and connected Bluetooth LE devices.\nThis first release doesn't support to work in background ! Stay tuned... I'll provide this later. I'll also upload some videos.")
                }
            }
//

            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Please report bugs, feature requests or discuss the APP in the <a href=\"http://bit.ly/ekkeBTLEapp\">Comments</a>")
                    onLinkActivated: Qt.openUrlExternally("http://bit.ly/ekkeBTLEapp")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Qt for mobile APP development")
                    color: primaryColor
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("My blog series about <a href=\"http://j.mp/qt-x\">Qt for x-platform Business Apps</a>")
                    onLinkActivated: Qt.openUrlExternally("http://j.mp/qt-x")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("My videos at VIMEO about <a href=\"https://vimeo.com/channels/qtmobile\">Qt mobile APP development</a>")
                    onLinkActivated: Qt.openUrlExternally("https://vimeo.com/channels/qtmobile")
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("My videos at YOUTUBE about <a href=\"https://www.youtube.com/user/ekkescorner/\">mobile APP development</a>")
                    onLinkActivated: Qt.openUrlExternally("https://www.youtube.com/user/ekkescorner/")
                }
            }

            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Thanks !")
                    color: primaryColor
                }
            }
            RowLayout {
                LabelBodySecondary {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Thanks J-P Nurmi, Mitch Curtis and others @qt.io for help, bug-fixing and inspirations.")
                }
            }
            HorizontalDivider {}
            RowLayout {
                LabelTitle {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Need a x-platform Business APP ?")
                    color: primaryColor
                }
            }
            RowLayout {
                LabelSubheading {
                    leftPadding: 10
                    rightPadding: 10
                    wrapMode: Text.WordWrap
                    text: qsTr("Since more then 30 years ekke is doing international development for Enterprise and SMB (Small and Medium Business). Most APPs are 'hidden' because they're running as inhouse APP at customer site.\nThis BT LE App will give you some impressions what kind of APPs you can expect. Need an APP ? ask ekke")
                }
            }

            HorizontalDivider {}
            RowLayout {
                LabelBodySecondary {
                    leftPadding: 10
                    rightPadding: 10
                    font.italic: true
                    wrapMode: Text.WordWrap
                    text: qsTr("ekke is Qt Champion 2016.")
                }
            }
            RowLayout {
                Image {
                    source: "qrc:/images/extra/qt_champion.png"
                }
            }

            HorizontalDivider {}
            RowLayout {
                LabelBodySecondary {
                    leftPadding: 10
                    rightPadding: 10
                    font.italic: true
                    wrapMode: Text.WordWrap
                    text: qsTr("ekke is BlackBerry Elite Developer and certified Builder for Native.")
                }
            }
            RowLayout {
                Image {
                    source: "qrc:/images/extra/bb-elite.png"
                }
            }
            RowLayout {
                Image {
                    source: "qrc:/images/extra/bb-builder-native.png"
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
        console.log("Init done from ABOUT")
    }
    // called from Component.destruction
    function cleanup() {
        console.log("Cleanup done from ABOUT")
    }
} // flickable
