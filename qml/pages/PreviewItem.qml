import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property FileInfo fileInfo

    function previewUri(s)
    {
        var idx = s.indexOf('#');
        if (idx > 0)
        {
            return s.substring(idx + 1);
        }
        else
        {
            return "";
        }
    }

    anchors.fill: parent
}
