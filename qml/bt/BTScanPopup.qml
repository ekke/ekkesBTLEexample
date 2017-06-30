// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"

Popup {
    id: thePopup
    property alias text: theLabel.text
    property alias infoText: theInfoLabel.text
    property alias imageSource: pulseScan.source
    property int from: 0
    property int to: 15
    property int max: 20
    property string textTimeout
    property bool showStopButton: false
    property bool stopClicked: false
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.elevation: 8
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    implicitHeight: 240
    implicitWidth: 260
    Item {
        id: scanImage
        anchors.horizontalCenter: parent.horizontalCenter
        width: 64
        height: width
        PulseScan {
            id: pulseScan
            anchors.centerIn: parent
            width: 64
            height: width
            source: ""
        }
    }
    LabelSubheading {
        id: theLabel
        leftPadding: 12
        width: parent.width - 24
        anchors.top: scanImage.bottom
        text: ""
        maximumLineCount: 3
        wrapMode: Label.WordWrap
        color: accentColor
    }
    LabelBody {
        id: theInfoLabel
        topPadding: 12
        leftPadding: 12
        width: parent.width - 24
        anchors.top: theLabel.bottom
        text: ""
        maximumLineCount: 3
        wrapMode: Label.WordWrap
    }
    ButtonFlat {
        anchors.bottom: progressBar.top
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.bottomMargin: 6
        visible: showStopButton
        text: qsTr("Stop")
        textColor: primaryColor
        onClicked: {
            thePopup.stopClicked = true
            thePopup.showStopButton = false
        }
    }
    LabelCaption {
        visible: thePopup.stopClicked
        text: qsTr("Stop pending")
        anchors.bottom: progressBar.top
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.bottomMargin: 10
        color: primaryColor
    }

    ProgressBar {
        id: progressBar
        height: 10
        width: parent.width
        anchors.bottom: parent.bottom
        indeterminate: false
        from: thePopup.from
        to: thePopup.to
        value: 0
    }
    Timer {
        id: progressTimer
        property int counter: 0
        interval: 1000
        repeat: true
        onTriggered: {
            counter ++
            // should match QBluetoothDiscoveryAgent->setLowEnergyDiscoveryTimeout(15000);
            // see BluetoothManager startDeviceDiscovery()
            if(counter <= progressBar.to) {
                progressBar.value = counter
                return
            }
            if(counter >= thePopup.max) {
                appWindow.showToast(thePopup.textTimeout)
                thePopup.close()
            }
        }
    }
    function startTimer() {
        progressTimer.counter = from
        progressBar.from = from
        progressBar.to = to
        progressBar.value = from
        progressTimer.start()
    }
    onOpened: {
        appWindow.backKeyfreezed = true
        startTimer()
        pulseScan.start()
    }
    onClosed: {
        pulseScan.stop()
        progressTimer.stop()
        progressBar.value = 0
        progressTimer.counter = 0
        appWindow.backKeyfreezed = false
    }
}// thePopup
