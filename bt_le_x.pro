# ekke (Ekkehard Gentz) @ekkescorner
TEMPLATE = app
TARGET = bt_le_x

QT += qml quick core bluetooth xml

LIBS += -lz

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
    cpp/bt/nfcreadermapping.hpp \
    cpp/bt/generalscanmanager.hpp \
    cpp/bt/feitiancardreadermanager.hpp

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
    cpp/bt/nfcreadermapping.cpp \
    cpp/bt/generalscanmanager.cpp \
    cpp/bt/feitiancardreadermanager.cpp

OTHER_FILES +=  qml/main.qml \
    qml/common/*.qml \
    qml/navigation/*.qml \
    qml/pages/*.qml \
    qml/popups/*.qml \
    qml/tabs/*.qml \
    qml/bt/*.qml

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
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    # deploying 32-bit and 64-bit APKs you need different VersionCode
    # here's my way to solve this - per ex. Version 1.2.3
    # aabcddeef aa: 21 (MY_MIN_API), b: 0 (32 Bit) or 1 (64 Bit)  c: 0 (unused)
    # dd: 01 (Major Release), ee: 02 (Minor Release), f:  3 (Patch Release)
    # VersionName 1.2.3
    # VersionCode 32 Bit: 210001023
    # VersionCode 64 Bit: 211001023
    defineReplace(droidVersionCode) {
        segments = $$split(1, ".")
        for (segment, segments): vCode = "$$first(vCode)$$format_number($$segment, width=2 zeropad)"
        equals(ANDROID_ABIS, arm64-v8a): \
            prefix = 1
        else: equals(ANDROID_ABIS, armeabi-v7a): \
            prefix = 0
        else: prefix = 2
        # add more cases as needed
        return($$first(prefix)0$$first(vCode))
    }
    MY_VERSION = 1.4
    MY_PATCH_VERSION = 0
    MY_MIN_API = 21
    ANDROID_VERSION_NAME = $$MY_VERSION"."$$MY_PATCH_VERSION
    ANDROID_VERSION_CODE = $$MY_MIN_API$$droidVersionCode($$MY_VERSION)$$MY_PATCH_VERSION

    # find this in shadow build android-build gradle.properties
    ANDROID_MIN_SDK_VERSION = "21"
    ANDROID_TARGET_SDK_VERSION = "29"
}

ios {
    QMAKE_INFO_PLIST = ios/Info.plist

    QMAKE_ASSET_CATALOGS = $$PWD/ios/Images.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"

    ios_artwork.files = $$files($$PWD/ios/iTunesArtwork*.png)
    QMAKE_BUNDLE_DATA += ios_artwork
    app_launch_screen.files = $$files($$PWD/ios/MyLaunchScreen.xib)
    QMAKE_BUNDLE_DATA += app_launch_screen

    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0

    disable_warning.name = GCC_WARN_64_TO_32_BIT_CONVERSION
    disable_warning.value = NO
    QMAKE_MAC_XCODE_SETTINGS += disable_warning

    # don't need this anymore - development team will be set from ios build settings
    # include(ios_signature.pri)

    # see https://bugreports.qt.io/browse/QTBUG-70072
    QMAKE_TARGET_BUNDLE_PREFIX = org.ekkescorner.examples
    QMAKE_BUNDLE = btle

    # Note for devices: 1=iPhone, 2=iPad, 1,2=Universal.
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2
}
