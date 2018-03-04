import QtQuick 2.0

ListModel {
    id: trailModel

    property var rootModel
    property var fsModel
    property string fsIcon

    signal progress(string name, int amount)
    signal finished()
    signal error(string details)

    function push(model, name, icon)
    {
        var item = {
            "fsModel": model,
            "name": name,
            "icon": icon !== "" ? icon : "image://theme/icon-m-folder"
        };
        append(item);
        fsModel = item.fsModel;
        fsIcon = item.icon;

        if (count === 1)
        {
            rootModel = item.fsModel;
        }
        console.log("new fsModel: " + fsModel.name);
        sharedState.currentContentModel = fsModel

        if (count > 1 && get(count - 2).fsModel !== fsModel)
        {
            fsModel.progress.connect(progress);
            fsModel.finished.connect(finished);
            fsModel.error.connect(error);
        }
    }

    function pop(amount)
    {
        console.log("Trail model pop: " + amount);

        if (count <= 1)
        {
            return;
        }

        var m;
        var cdUp = 0;
        while (amount > 0)
        {
            cdUp = 0;
            m = get(count - 1).fsModel;
            while (get(count - 1).fsModel === m && amount > 0)
            {
                ++cdUp;
                --amount;
                remove(count - 1);
            }
        }
        m = get(count - 1).fsModel;
        var i = get(count - 1).icon;
        m.cdUp(cdUp);
        fsModel = m;
        fsIcon = i;
    }

    Component.onCompleted: {
        var fsModel = serviceObject("places").createModel("places");
        push(fsModel, fsModel.name, "");
    }
}
