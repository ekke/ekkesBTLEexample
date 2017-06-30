// ekke

import QtQuick 2.9
import QtGraphicalEffects 1.0

Item {
    id: pulseScan
    property alias source: sourceImage.source

    function stop(){
        theAnimation.stop();
    }

    function start(){
        theAnimation.start();
    }

    Image {
        id: sourceImage
        source: ""
        anchors.fill: parent
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        fillMode: Image.PreserveAspectFit

        SequentialAnimation {
            id: theAnimation
            running: false
            loops: Animation.Infinite
            PropertyAnimation {
                property: "scale"
                target: sourceImage;
                from: 1.0;
                to: 0.7;
                duration: 600
            }
            PropertyAnimation {
                property: "scale"
                target: sourceImage;
                from: 0.7;
                to: 1.0;
                duration: 600
            }
        } // theAnimation

    } // sourceImage
}
