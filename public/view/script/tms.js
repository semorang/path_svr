// import { dpTimeToString, dpDistanceToString } from "./script/display_string.js"
document.write('<script src="script/display_string.js"></script>');

let boundarys = new Array();
let fitBoundary = new Array();

// 클러스터링 결과
function drawClunster(clusters) {
    // relese boundarys
    boundarys.splice(0);
    fitBoundary.splice(0);

    var idxClust = 0;

    clusters.forEach(function(clust) {
        // draw cluster boundary
        var boundary = new inavi.maps.Polygon({
            map: map,
            path: clust.boundary,
            style: {
                fillOpacity: 0.5,
                fillColor: "#808080",
                fillOutlineColor: "#031B80",
            },
        });
        boundary.dataset = clust.group;

        boundary.on("click", function(e) {
            console.log("클릭된 clust id : " + e.target.dataset);
            reqClustBestwayRoute(e.target.dataset);
        })

        // draw label
        var label = new inavi.maps.Label({
            map: map,
            position: clust.center,
            text: clust.group.toString() + " (" + clust.pois.length.toString() + "), " + dpDistanceToString(clust.distance).toString() + ", " + dpTimeToString(clust.time).toString(),
        })

        // draw pois
        // var iconUrl = "https://www.fivepin.co.kr/images/PoiMarker/marker_" + (clust.group + 1) + "_on.png"
        var iconUrl = "images/flag_blue_ss/resize_resize_maker_blue_" + (clust.group + 1) + ".png"
        clust.pois.forEach(function(poi) {
            var poi = new inavi.maps.Marker({
                map: map,
                position: poi.coord,
                icon: iconUrl
            });
        })

        clust.boundary.forEach(function(coord) {
            fitBoundary.push(coord)
        });
    })


    map.fitCoordinates(fitBoundary, {
        padding: 90,
        // heading: 90,
        // tilt: 30,
        // duration: 1000
    });
}


// 클러스터링 결과
function drawBestwaypoint(bestways) {
    // relese boundarys
    fitBoundary.splice(0);

    bestways.forEach(function(way) {
        // draw pois
        // var iconUrl = "https://www.fivepin.co.kr/images/PoiMarker/marker_" + (clust.group + 1) + "_on.png"
        var iconUrl = "images/flag_blue_ss/resize_resize_maker_blue_" + (way.index) + ".png"
        if (way.index >= 100) {
            iconUrl = "images/flag_blue_ss/resize_resize_maker_blue_w.png"
        }
        var poi = new inavi.maps.Marker({
            map: map,
            position: way.coord,
            icon: iconUrl
        });

        fitBoundary.push(way.coord)
    })

    map.fitCoordinates(fitBoundary, {
        padding: 90,
        // heading: 90,
        // tilt: 30,
        // duration: 1000
    });
}


function reqClustBestwayRoute(idx) {
    if (idx >= 0 && result_obj.clusters.length > idx) {
        var params = new URLSearchParams(window.location.search);
        var pathname = '/api/multiroute';
        
        var idxPois = 0;
        var clust = result_obj.clusters[idx];
        params.delete('vias');

        clust.pois.forEach(function(poi) {
            if (idxPois == 0) {
                if (result_obj.summary.start_lock != undefined) {
                    params.set('start', result_obj.summary.start_lock[0].toString() + ',' + result_obj.summary.start_lock[1].toString());
                    params.append('vias', poi.coord[0].toString() + ',' + poi.coord[1].toString());
                } else {
                    params.set('start', poi.coord[0].toString() + ',' + poi.coord[1].toString());
                }
            } else if (idxPois == clust.pois.length - 1) {
                if (result_obj.summary.end_lock != undefined) {
                    params.append('vias', poi.coord[0].toString() + ',' + poi.coord[1].toString());
                    params.set('end', result_obj.summary.end_lock[0].toString() + ',' + result_obj.summary.end_lock[1].toString());
                } else {
                    params.set('end', poi.coord[0].toString() + ',' + poi.coord[1].toString());
                }                
            } else {
                params.append('vias', poi.coord[0].toString() + ',' + poi.coord[1].toString());
            }
            idxPois++;
        })
        params.set('option', 2); //ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE

        getRoutes(pathname, params);
    }
}


function reqBestwayRoute(ways) {
    if (ways.length > 0) {
        var params = new URLSearchParams(window.location.search);
        var pathname = '/api/multiroute';
        
        var idxPois = 0;
        params.delete('vias');

        ways.forEach(function(poi) {
            if (idxPois == 0) {
                params.set('start', poi.coord[0].toString() + ',' + poi.coord[1].toString());
            } else if (idxPois == ways.length - 1) {
                params.set('end', poi.coord[0].toString() + ',' + poi.coord[1].toString());              
            } else {
                params.append('vias', poi.coord[0].toString() + ',' + poi.coord[1].toString());
            }
            idxPois++;
        })
        params.set('option', 2); //ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE

        getRoutes(pathname, params);
    }
}