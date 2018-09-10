# ekke (Ekkehard Gentz) @ekkescorner
TEMPLATE = app
TARGET = bt_le_x

QT += qml quick core bluetooth
CONFIG += c++11

HEADERS += \
    cpp/applicationui.hpp \
    cpp/uiconstants.hpp \
    cpp/bt/bluetoothmanager.hpp \
    cpp/bt/mybluetoothdeviceinfo.hpp \
    cpp/bt/mybluetoothserviceinfo.hpp \
    cpp/bt/heartratemanager.hpp \
    cpp/bt/waiterlockmanager.hpp \
    cpp/bt/mybluetoothcharacteristic.hpp \
    cpp/bt/mybluetoothdescriptor.hpp \
    cpp/bt/addimatmapping.hpp \
    cpp/bt/heartratemessages.hpp \
    cpp/bt/nfcreadermanager.hpp \
    cpp/bt/nfcreadermapping.hpp

SOURCES += cpp/main.cpp \
    cpp/applicationui.cpp \
    cpp/bt/bluetoothmanager.cpp \
    cpp/bt/mybluetoothdeviceinfo.cpp \
    cpp/bt/mybluetoothserviceinfo.cpp \
    cpp/bt/heartratemanager.cpp \
    cpp/bt/waiterlockmanager.cpp \
    cpp/bt/mybluetoothcharacteristic.cpp \
    cpp/bt/mybluetoothdescriptor.cpp \
    cpp/bt/addimatmapping.cpp \
    cpp/bt/heartratemessages.cpp \
    cpp/bt/nfcreadermanager.cpp \
    cpp/bt/nfcreadermapping.cpp

lupdate_only {
    SOURCES +=  qml/main.qml \
    qml/common/*.qml \
    qml/navigation/*.qml \
    qml/pages/*.qml \
    qml/popups/*.qml \
    qml/tabs/*.qml \
    qml/bt/*.qml
}

OTHER_FILES += images/black/*.png \
    images/black/x18/*.png \
    images/black/x36/*.png \
    images/black/x48/*.png \
    images/white/*.png \
    images/white/x18/*.png \
    images/white/x36/*.png \
    images/white/x48/*.png \
    images/extra/*.png \
    translations/*.* \
    images/LICENSE \
    LICENSE \
    *.md \
    ios/*.png \
    ios/Images.xcassets/AppIcon.appiconset/*.*

RESOURCES += \
    translations.qrc \
    qml.qrc \
    images.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

# T R A N S L A T I O N S

# if languages are added:
# 1. rebuild project to generate *.qm
# 2. add existing .qm files to translations.qrc

# if changes to translatable strings:
# 1. Run Tools-External-Linguist-Update
# 2. Run Linguist and do translations
# 3. Build and run on iOS and Android to verify translations
# 4. Optional: if translations not done: Run Tools-External-Linguist-Release

# Supported languages
LANGUAGES = de en

# used to create .ts files
 defineReplace(prependAll) {
     for(a,$$1):result += $$2$${a}$$3
     return($$result)
 }
# Available translations
tsroot = $$join(TARGET,,,.ts)
tstarget = $$join(TARGET,,,_)
TRANSLATIONS = $$PWD/translations/$$tsroot
TRANSLATIONS += $$prependAll(LANGUAGES, $$PWD/translations/$$tstarget, .ts)
# run LRELEASE to generate the qm files
qtPrepareTool(LRELEASE, lrelease)
 for(tsfile, TRANSLATIONS) {
     command = $$LRELEASE $$tsfile
     system($$command)|error("Failed to run: $$command")
 }

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

ios {
    QMAKE_INFO_PLIST = ios/Info.plist

    QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    ios_artwork.files = $$files($$PWD/ios/iTunesArtwork*.png)
    QMAKE_BUNDLE_DATA += ios_artwork
    app_launch_screen.files = $$files($$PWD/ios/MyLaunchScreen.xib)
    QMAKE_BUNDLE_DATA += app_launch_screen

    QMAKE_IOS_DEPLOYMENT_TARGET = 10.0

    disable_warning.name = GCC_WARN_64_TO_32_BIT_CONVERSION
    disable_warning.value = NO
    QMAKE_MAC_XCODE_SETTINGS += disable_warning

    # QtCreator 4.3 provides an easy way to select the development team
    # see Project - Build - iOS Settings
    # I have to deal with different development teams,
    # so I include my signature here
    # ios_signature.pri not part of project repo because of private signature details
    # contains:
    # QMAKE_XCODE_CODE_SIGN_IDENTITY = "iPhone Developer"
    # MY_DEVELOPMENT_TEAM.name = DEVELOPMENT_TEAM
    # MY_DEVELOPMENT_TEAM.value = your team Id from Apple Developer Account
    # QMAKE_MAC_XCODE_SETTINGS += MY_DEVELOPMENT_TEAM
    include(ios_signature.pri)

    MY_BUNDLE_ID.name = PRODUCT_BUNDLE_IDENTIFIER
    MY_BUNDLE_ID.value = org.ekkescorner.examples.btle
    QMAKE_MAC_XCODE_SETTINGS += MY_BUNDLE_ID

    # Note for devices: 1=iPhone, 2=iPad, 1,2=Universal.
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2
}
