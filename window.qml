import QtQuick 2.6
import QtQuick.Window 2.0

Window {
    id: window
    width: 128
    height: 64
    visible: true

    Loader {
        source: "main.qml"
    }
}
