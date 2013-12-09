import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow
{    
    property variant pages: []

    initialPage: folderPage
    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    Component.onCompleted: {
        pageStack.pushExtra(folderPage, { "secondPane": true });
    }

    Component {
        id: folderPage

        FolderPage {
            id: page

            Component.onCompleted: {
                var p = pages;
                p.push(page);
                pages = p;
            }

            onCopyCommand: {
                var destModel = null;
                for (var i = 0; i < pages.length; ++i)
                {
                    var model = pages[i].currentContentModel();
                    if (model !== sourceModel)
                    {
                        destModel = model;
                        break;
                    }
                }

                if (destModel)
                {
                    sourceModel.copySelected(destModel);
                }
                destModel.refresh();
            }

            onLinkCommand: {
                var destModel = null;
                for (var i = 0; i < pages.length; ++i)
                {
                    var model = pages[i].currentContentModel();
                    if (model !== sourceModel)
                    {
                        destModel = model;
                        break;
                    }
                }

                if (destModel)
                {
                    sourceModel.linkSelected(destModel);
                }
                destModel.refresh();
            }
        }
    }
}


