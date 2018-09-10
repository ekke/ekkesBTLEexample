// ekke (Ekkehard Gentz) @ekkescorner
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import "../common"

Pane {
    id: myBar
    // Qt 5.8 bug: MENU Button remains visually pressed after drawer was opened
    // see QTBUG-59293 - fixed in 5.9
    // workaround: toggle between 2 Button instances via Loader
    // property bool favMenuBugfix: false
    Material.elevation: 8
    z: 1
    property real activeOpacity: iconFolder == "black" ?  0.87 : 1.0
    property real inactiveOpacity: iconFolder == "black" ? 0.26 : 0.56
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    height: isDarkTheme? 56 + darkDivider.height : 56
    // Using Divider as workaround for bug:
    // Material.elevation: 8 not 'visible' if dark theme
    HorizontalDivider {
        id: darkDivider
        visible: isDarkTheme
    }
    RowLayout {
        focus: false
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: isDarkTheme? darkDivider.bottom : parent.top
        spacing: 0
        // MENU Button
        DrawerFavoritesMenuButton {
        }
        // alternate way
        // see QTBUG-59293 - fixed in 5.9
//        Loader {
//            id: favMenuLoader
//            sourceComponent: favMenuBugfix? favMenuComponent1 : favMenuComponent2
//        }
//        Component {
//            id: favMenuComponent1
//            DrawerFavoritesMenuButton {
//            }
//        }
//        Component {
//            id: favMenuComponent2
//            DrawerFavoritesMenuButton {
//            }
//        }
        //
        Repeater {
            id: favoritesButtonRepeater
            model: favoritesModel
            DrawerFavoritesNavigationButton {
                id: myButton
            }
        } // repeater
    } // RowLayout
    function replaceIcon(position, theIconName) {
        favoritesButtonRepeater.itemAt(position).theIcon = theIconName
    }
    function replaceText(position, theText) {
        favoritesButtonRepeater.itemAt(position).theText = theText
    }
} // bottomNavigationBar
