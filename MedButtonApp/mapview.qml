//import QtQuick 2.12
//import QtLocation 5.12
//import QtPositioning 5.12

import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import QtQuick.Window 2.0

Item {
    anchors.fill: parent
    id: window
    Plugin{
        id: mapPlugin
        name:"osm"
        PluginParameter {
                    name: "osm.mapping.providersrepository.disabled"
                    value: "true"
                }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "http://maps-redirect.qt.io/osm/5.6/"
        }
    }
    Map{
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(49.8561,24.0821);
        zoomLevel: 14
        Marker{
            coordinate: marker_model.current
        }
        MapItemView{
            model: marker_model
            delegate: Marker{
                coordinate: model.position
            }
        }
    }
}
