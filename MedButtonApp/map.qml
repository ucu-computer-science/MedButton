//! [Imports]
//import QtQuick 2.0
//import QtPositioning 5.5
//import QtLocation 5.6
//! [Imports]

//Rectangle {
//    anchors.fill: parent

//    //! [Initialize Plugin]
//    Plugin {
//        id: myPlugin
//        name: "osm" // "mapboxgl", "esri", ...
//        //specify plugin parameters if necessary
//        //PluginParameter {...}
//        //PluginParameter {...}
//        PluginParameter {
//                    name: "osm.mapping.providersrepository.disabled"
//                    value: "true"
//                }
//                PluginParameter {
//                    name: "osm.mapping.providersrepository.address"
//                    value: "http://maps-redirect.qt.io/osm/5.6/"
//                }
//    }
//    //! [Initialize Plugin]


//    //! [Current Location]
//    PositionSource {
//        id: positionSource
//        property variant lastSearchPosition: locationOslo
//        active: true
//        updateInterval: 120000 // 2 mins
//        onPositionChanged:  {
//            var currentPosition = positionSource.position.coordinate
//            map.center = currentPosition
//            var distance = currentPosition.distanceTo(lastSearchPosition)
//            if (distance > 500) {
//                // 500m from last performed pizza search
//                lastSearchPosition = currentPosition
//                searchModel.searchArea = QtPositioning.circle(currentPosition)
//                searchModel.update()
//            }
//        }
//    }
//    //! [Current Location]

//    //! [PlaceSearchModel]
//    property variant locationOslo: QtPositioning.coordinate( 59.93, 10.76)

//    PlaceSearchModel {
//        id: searchModel

//        plugin: myPlugin

//        searchTerm: "Pizza"
//        searchArea: QtPositioning.circle(locationOslo)

//        Component.onCompleted: update()
//    }
//    //! [PlaceSearchModel]

//    //! [Places MapItemView]
//    Map {
//        id: map
//        anchors.fill: parent
//        plugin: myPlugin;
//        center: locationOslo
//        zoomLevel: 13

//        MapQuickItem {
//            id: marker
//            objectName: "mapItem"
//              coordinate {latitude: 59.91
//                         longitude: 10.75}
//              anchorPoint.x: image.width * 0.5
//              anchorPoint.y: image.height

//              sourceItem: Image {
//                 id: image
//                 height: 35
//                 width: 35
//                 source: "geoloc.png"
//              }
//                    function recenter(lat,lng) {
//                          map.clearMapItems();
//                          marker.coordinate.latitude = lat;
//                          marker.coordinate.longitude = lng;
//                          map.addMapItem(marker);
//                          map.center.latitude = lat;
//                          map.center.longitude = lng;
//                          map.update();
//                    }
//            }
//        }
//    //! [Places MapItemView]

//    Connections {
//        target: searchModel
//        onStatusChanged: {
//            if (searchModel.status == PlaceSearchModel.Error)
//                console.log(searchModel.errorString());
//        }
//    }
//}

import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import QtQuick.Window 2.0


Rectangle {
    anchors.fill: parent

    //! [Initialize Plugin]
    Plugin {
        id: myPlugin
        name: "osm" // "mapboxgl", "esri", ...
        //specify plugin parameters if necessary
        //PluginParameter {...}
        //PluginParameter {...}
        PluginParameter {
                    name: "osm.mapping.providersrepository.disabled"
                    value: "true"
                }
                PluginParameter {
                    name: "osm.mapping.providersrepository.address"
                    value: "http://maps-redirect.qt.io/osm/5.6/"
                }
    }


    //! [Places MapItemView]
    Map {
        id: map
        anchors.fill: parent
        plugin: myPlugin
        center: QtPositioning.coordinate(59.91, 10.75)
        zoomLevel: 14

        MapItemView{
            model: markerModel
            delegate: mapcomponent
        }
    }

    Component {
        id: mapcomponent
        MapQuickItem {
            id: marker
            anchorPoint.x: image.width * 0.5
            anchorPoint.y: image.height
            coordinate: position

            sourceItem: Image {
                id: image
                height: 35
                width: 35
                source: "geoloc.png"
            }
        }
    }

}

//Rectangle {
//    width: Screen.width
//    height: Screen.height
//    visible: true
////    anchors.fill: parent

//    Plugin {
//        id: mapPlugin
//        name: "osm"
//        PluginParameter {
//                            name: "osm.mapping.providersrepository.disabled"
//                            value: "true"
//                        }
//                        PluginParameter {
//                            name: "osm.mapping.providersrepository.address"
//                            value: "http://maps-redirect.qt.io/osm/5.6/"
//                        }
//    }

//        //! [Current Location]
//        PositionSource {
//            id: positionSource
//            property variant lastSearchPosition: locationOslo
//            active: true
//            updateInterval: 120000 // 2 mins
//            onPositionChanged:  {
//                var currentPosition = positionSource.position.coordinate
//                mapview.center = currentPosition
//                var distance = currentPosition.distanceTo(lastSearchPosition)
//                if (distance > 500) {
//                    // 500m from last performed pizza search
//                    lastSearchPosition = currentPosition
//                    searchModel.searchArea = QtPositioning.circle(currentPosition)
//                    searchModel.update()
//                }
//            }
//        }
//        //! [Current Location]

//        //! [PlaceSearchModel]
//        property variant locationOslo: QtPositioning.coordinate( 59.93, 10.76)

//        PlaceSearchModel {
//            id: searchModel

//            plugin: mapPlugin

//            searchTerm: "Pizza"
//            searchArea: QtPositioning.circle(locationOslo)

//            Component.onCompleted: update()
//        }
//        //! [PlaceSearchModel]

//    Map {
//        id: mapview
//        anchors.fill: parent
//        plugin: mapPlugin
//        center: QtPositioning.coordinate(59.91, 10.75)
//        zoomLevel: 14

//        MapItemView{
//            model: markerModel
//            delegate: mapcomponent
//        }
//    }

//    Component {
//        id: mapcomponent
//        MapQuickItem {
//            id: marker
//            anchorPoint.x: image.width/4
//            anchorPoint.y: image.height
//            coordinate: position

//            sourceItem: Image {
//                id: image
//                source: "geoloc.png"
//            }
//        }
//    }

//    MouseArea {
//        anchors.fill: parent

//        onPressAndHold:  {
//            var coordinate = mapview.toCoordinate(Qt.point(mouse.x,mouse.y))
//            markerModel.addMarker(coordinate)
//        }
//    }

//    Connections {
//        target: searchModel
//        onStatusChanged: {
//            if (searchModel.status == PlaceSearchModel.Error)
//                console.log(searchModel.errorString());
//        }
//    }
//}
