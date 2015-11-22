import QtQuick 2.0
import Sailfish.Silica 1.0

PreviewItem {

    BackgroundItem {
        anchors.fill: parent

        Image {
            id: previewImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            smooth: true
            visible: source !== ""
            sourceSize.width: width
            sourceSize.height: height
            source: previewUri(fileInfo.preview)

            BusyIndicator {
                running: parent.status === Image.Loading
                anchors.centerIn: parent
                size: BusyIndicatorSize.Large
            }
        }

        onClicked: {
            var props = {
                "source": previewUri(fileInfo.preview)
            };

            pageStack.push(fullPageComponent, props);
        }
    }


    Component {
        id: fullPageComponent

        Page {
            allowedOrientations: Orientation.All

            property string source

            Image {
                id: image
                anchors.fill: parent
                smooth: true
                fillMode: Image.PreserveAspectFit
                source: previewImage.status === Image.Ready ? parent.source
                                                            : ""

                BusyIndicator {
                    running: previewImage.status === Image.Loading ||
                             parent.status === Image.Loading
                    anchors.centerIn: parent
                    size: BusyIndicatorSize.Large
                }
            }
        }
    }
}
