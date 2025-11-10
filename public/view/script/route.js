
// 노드
let nodes = new Array();
let vertices_all = new Array();

// let colors = ["#CC0000", "#3282F6", "#EA3FF7", "#18A705"]; // , "#782DFF"
let route_color = ["#CC0000", "#00FF00", "#0000FF", "#00eeff"];
let route_offset = [0, 1, -1, -2];
// let route_color = ["#CC0000", "#008888"];
// let route_offset = [0, 2];

let user_obj;
let user_pos;

// 경로 탐색 정보
const ClickTypeNone = 0;
const ClickTypeDeparture = 1;
const ClickTypeDestination = 2;
const ClickTypeWaypoint1 = 3;
const ClickTypeWaypoint2 = 4;
const ClickTypeWaypoint3 = 5;
const ClickTypeWaypoint4 = 6;
const ClickTypeWaypoint5 = 7;
const ClickTypeAll = 9999;

const RouteMode_Car = 0; // 자동차
const RouteMode_Pedestrian = 1; // 보행/자전거
const RouteMode_Forest = 2; // 숲길
const RouteMode_Path = 3; // p2p

const Mobility_Pedestrian = 0; // 보행
const Mobility_Bicycle = 1; // 자전거

let typeRouteStep = ClickTypeNone; // 0:없음, 1:출발, 2:도착, 3:경유1, 4:경유2, 5:경유3, 6:경유4, 7:경유5
let click_pos;
let departure_pos;
let waypoint_pos = new Array(5);
let destination_pos;
let route_mode = RouteMode_Car; // 0:차량, 1:보행, 2:숲길, ...
let route_option = 1; // 0:최단거리, 추천, 편안한, 최소시간(빠른), 큰길
let course_type = 1; // 코스 타입 //0:미정의, 1:등산, 2:걷기, 3:자전거, 4:코스
let course_id = 0; // 코스 ID 
let route_mobility = Mobility_Bicycle; // 0:보행자, 1:자전거


function setMapClick(pos, eventType) {
    switch(typeRouteStep) {
        // 출발지
        case ClickTypeDeparture: {
            setDeparture(pos);
        }
        break;

        // 목적지
        case ClickTypeDestination: {
            setDestination(pos);
        }
        break;

        // 경유지
        case ClickTypeWaypoint1: case ClickTypeWaypoint2: case ClickTypeWaypoint3: case ClickTypeWaypoint4: case ClickTypeWaypoint5: {
            setWaypoints(typeRouteStep - 3, pos);
        }
        break;

        default:
            break;
    } // switchf

    if (eventType !== 'click') {
        // 모바일 기기등에서 지도 드래깅도 touchstart로 함께 전달되어 지점 설정이 계속 일어남
        setRouteType(ClickTypeAll);
    }
}

function setDeparture(pos) {
    if (pos != undefined) {
        departure_pos = pos;
        markers[0].setPosition(pos);
        markers[0].setVisible(true);
    } else {
        markers[0].setVisible(false);
    }
}


function setDestination(pos) {
    if (pos != undefined) {
        destination_pos = pos;
        markers[1].setPosition(pos);
        markers[1].setVisible(true);
    } else {
        markers[1].setVisible(false);
    }
}


function setWaypoints(ii, pos) {
    if (pos != undefined) {
        waypoint_pos[ii] = pos;
        markers[2 + ii].setPosition(pos);
        markers[2 + ii].setVisible(true);
        // markers[2 + ii].setZIndex(ii);
    } else {
        waypoint_pos[ii] = null;
        markers[2 + ii].setVisible(false);
        // markers[2 + ii].setZIndex(-999);
    }
}


function getOption() {
    return route_option;
}


function setOption(opt) {
    route_option = opt;

    typeRouteStep = ClickTypeAll; // 변경사항이므로
}


function getCourseType() {
    return course_type;
}

function setCourseType(type) {
    course_type = type;

    typeRouteStep = ClickTypeAll; // 변경사항이므로
}

function getCourseId() {
    return course_id;
}

function setCourseId(id) {
    course_id = id;

    typeRouteStep = ClickTypeAll; // 변경사항이므로
}


function getMobility() {
    return route_mobility;
}


function setMobility(mob) {
    route_mobility = mob;

    typeRouteStep = ClickTypeAll; // 변경사항이므로
}


// 경로탐색 컨트롤의 버튼 클릭
function setRouteType(type) {
    typeRouteStep = type;

    // 버튼 그룹 설정
    const btns = {
        start: document.getElementById('btnStart'),
        end: document.getElementById('btnEnd'),
        via: [
            document.getElementById('btnVia1'),
            document.getElementById('btnVia2'),
            document.getElementById('btnVia3'),
            document.getElementById('btnVia4'),
            document.getElementById('btnVia5')
        ]
    };

    // 기본 초기화
    btns.start.className = 'btn blue';
    btns.end.className = 'btn red';
    // btns.via.forEach(btn => btn.className = 'btn');

    // 선택 상태 처리
    if (type === ClickTypeDeparture) {
        btns.start.className = 'selected_btn';
    } else if (type === ClickTypeDestination) {
        btns.end.className = 'selected_btn';
    } else if (type >= ClickTypeWaypoint1 && type <= ClickTypeWaypoint5) {
        const idx = type - 3;
        const target = btns.via[idx];
        if (target.className === 'selected_btn') {
            typeRouteStep = ClickTypeAll;
            target.className = 'btn';
            setWaypoints(idx);
        } else {
            target.className = 'selected_btn';
        }
    }
}


let distanceOverlay;


let routeMarker = new Array();


// draw map item
function drawRoutes(routes, map, offsets, colors) {
    // release marker
    routeMarker.forEach(marker => marker.setMap(0));
    routeMarker.splice(0);
    vertices_all.splice(0);

    var offRoute = 0;
    var via_idx = 0;
    var current_type = 0;
    
    // 1순위 옵션을 최상위로
    routes.forEach(function(route) { 
        var line_off = offsets[offRoute % 4];
        var color_off = colors[offRoute % 4];
        var cntLink = route.link_info.length;
        var cntVtx = route.vertex_info.length;
    
        var vertices = new Array();
        var vertex_offset = 0;
    
        // 경로선 생성
        var iconUrl;
        for (var ii=0; ii<cntLink; ii++) {
            vertex_offset = route.link_info[ii].vertex_offset;
            for (var vtx = 0; vtx < route.link_info[ii].vertex_count; vtx++) {            
                if (route.vertex_info[vertex_offset] != undefined) {
                    vertices_all.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);                    
                    vertices.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);
                }
                vertex_offset++;
            }

            if ((route_mode === RouteMode_Pedestrian)) { // 보행/자전거
                if (route_mobility === Mobility_Bicycle) {  // 자전거 모드                    
                    if (ii == 0) {
                        current_type = route.link_info[ii].bicycle_type;
                    }

                    if ((current_type !== route.link_info[ii].bicycle_type) || (ii == cntLink - 1)) {
                        // 경로선 추가
                        if (current_type === 1) {
                            appendPolyline(vertices, 8, "#ff5c1b", line_off); // 자전거 전용
                        } else {
                            appendPolyline(vertices, 8, "#0084ff", line_off);
                        }
                        appendDashline(vertices, 1, "#ffffff", line_off, [6, 3]);

                        vertices.splice(0);
                                            
                        current_type = route.link_info[ii].bicycle_type;

                        // 현재 링크 추가
                        if (ii < cntLink) {
                            vertex_offset = route.link_info[ii].vertex_offset;
                            for (var vtx = 0; vtx < route.link_info[ii].vertex_count; vtx++) {            
                                if (route.vertex_info[vertex_offset] != undefined) {
                                    vertices_all.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);                    
                                    vertices.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);
                                }
                                vertex_offset++;
                            }
                        }
                    }
                } else { // 보행 모드                   
                    if (ii == cntLink - 1) {
                        appendPolyline(vertices, 8, "#0084ff", line_off);
                        appendDashline(vertices, 1, "#ffffff", line_off, [6, 3]);

                        vertices.splice(0);
                    }
                }
            } else {
                if ((route.link_info[ii].guide_type != 0) && ((ii != 0) || (ii == 0 && cntLink <= 1))) {
                    // 차량
                    appendPolyline(vertices, 8, color_off, line_off);
                    appendDashline(vertices, 1, "#ffffff", line_off, [6, 3]);

                    vertices.splice(0);

                    via_idx++;
                }
            }
        }// for
        //-- 경로선 생성


        // maps api 오브젝트 갯수에 따른 속도 저하 때문에 때에 맞게 주석 풀고 사용할 것 // 2024-08-24
        // 노드 생성
        // // 경로에 노드를 생성합니다
        // var off = 0;
        // // 마지막 목적지의 종료 노드까지 처리하기 위해 for를 +1해서 돌림
        // for (var ii = 0; ii < cntLink; ii++) {
        //     nodes.push(new inavi.maps.Circle({
        //         map: map,
        //         position: [route.vertex_info[off].x, route.vertex_info[off].y], // 원 중심 좌표
        //         radius: 0.005, // 원 반지름 킬로미터 단위
        //         style: {
        //             fillOpacity: 0.5,  // 채우기 불투명도 입니다
        //             fillColor: "#CFE7FF",
        //             fillOutlineColor: "#DB4455",
        //         },
        //     }));
    
        //     off += route.link_info[ii].vertex_count;
        //     if (off >= route.vertex_info.length) {
        //         break;
        //     }                
        // } // for
    
        // // 마지막 목적지의 종료 노드까지 처리하기 위해 
        // if (route.vertex_info[off - 1] != undefined) {
        //     nodes.push(new inavi.maps.Circle({
        //         map: map,
        //         position: [route.vertex_info[off - 1].x, route.vertex_info[off - 1].y], // 원 중심 좌표
        //         radius: 0.005, // 원 반지름 킬로미터 단위
        //         style: {
        //             fillOpacity: 0.5,  // 채우기 불투명도 입니다
        //             fillColor: "#CFE7FF",
        //             fillOutlineColor: "#DB4455",
        //         },
        //     }));
        // }
    
    
        // 정션 생성
        if (route.junction_info != undefined) {
            // route.junction_info.forEach(function(jct) {
                route.junction_info.forEach(function(lnk) {
                    var junctions = new Array();
                    lnk.vertices.forEach(function(coord) {
                        junctions.push([coord.x, coord.y]);
                    }); // forEach vtx
                    
                    appendPolyline(junctions, 3, "#F09B59", 0);
                });// forEach lnk
            // }); // forEach jct
        }
        //-- 정션 생성


        // poi marker
        if (route.vertex_info != undefined && route.vertex_info.length > 1) {
            if (offRoute == 0) {
                iconUrl = "images/flag_orange_ss/resize_resize_maker_orange_s.png"
            } else {
                if (offRoute >= 100) {
                    iconUrl = "images/flag_orange_ss/resize_resize_maker_orange_w.png"
                } else {
                    iconUrl = "images/flag_orange_ss/resize_resize_maker_orange_" + (offRoute) + ".png"
                }
            }

            var poi = new inavi.maps.Marker({
                map: map,
                position: [route.vertex_info[0].x, route.vertex_info[0].y],
                icon: iconUrl
            });
            routeMarker.push(poi);

            // 마지막 지점은 링크 선형 끝점으로 표기
            if (offRoute == routes.length - 1) {
                iconUrl = "images/flag_orange_ss/resize_resize_maker_orange_e.png"

                var poi = new inavi.maps.Marker({
                    map: map,
                    position: [route.vertex_info[route.vertex_info.length - 1].x, route.vertex_info[route.vertex_info.length - 1].y],
                    icon: iconUrl
                });
                routeMarker.push(poi);
            }
        }
        
        offRoute++;
    }) // for each

    map.fitCoordinates(vertices_all, {
        padding: 90,
        // heading: 90,
        // tilt: 30,
        // duration: 1000
    });
}


// draw map item
function drawiNaviRoutes(routes, map, offsets, colors) {
    vertices_all.splice(0);              

    for (ii=0; ii<routes.length; ii++) {
        var route = routes[ii];
        var off = offsets[ii];
        var col = colors[ii];

        var vertices = new Array();
        // jct.junction.forEach(function(lnk) {  }));
        // nodes.forEach(element => element.setMap());

        // 경로선 생성
        var via_idx = 0;
        route.paths.forEach(link => {
            if (link.coords.length > 0) {
                link.coords.forEach(vtx => {
                    // 경로선 확장
                    vertices_all.push([vtx.x, vtx.y]);
                    vertices.push([vtx.x, vtx.y]);
                });
                
                // 노드 생성
                nodes.push(new inavi.maps.Circle({
                    map: map,
                    position: [link.coords[0].x, link.coords[0].y], // 원 중심 좌표
                    radius: 0.003, // 원 반지름 킬로미터 단위
                    style: {
                        fillOpacity: 0.5,  // 채우기 불투명도 입니다
                        fillColor: "#CFE7FF",
                        fillOutlineColor: "#DB4455",
                    },
                }));

                // 마지막 노드 확인                        
                if (++via_idx == route.paths.length) {
                    nodes.push(new inavi.maps.Circle({
                        map: map,
                        position: [link.coords[link.coords.length - 1].x, link.coords[link.coords.length - 1].y], // 원 중심 좌표
                        radius: 0.003, // 원 반지름 킬로미터 단위
                        style: {
                            fillOpacity: 0.5,  // 채우기 불투명도 입니다
                            fillColor: "#CFE7FF",
                            fillOutlineColor: "#DB4455",
                        },
                    }));
                }
            }
        });
        //-- 경로선 생성

        // 경로선 등록
        appendPolyline(vertices, 8, col, off);
        appendDashline(vertices, 1, "#ffffff", off, [6, 3]);
        
        vertices.splice(0);

        // 정션 생성
        // if (route.junction_info != undefined) {
        //     route.junction_info.forEach(function(jct) {
        //         jct.junction.forEach(function(lnk) {
        //             var junctions = new Array();
        //             lnk.vertices.forEach(function(coord) {
        //                 junctions.push([coord.x, coord.y]);
        //             }); // forEach vtx
                    
        //             links.push(new inavi.maps.Polyline({
        //                 map: map,
        //                 path: junctions,
        //                 style: {
        //                     lineColor: "#F09B59", //"#FFC90E",
        //                     lineWidth: 5,
        //                     // lineOpacity: 0.7,
        //                 },
        //             }));
        //         });// forEach lnk
        //     }); // forEach jct
        // }                
        //-- 정션 생성
        // let colors = ["#CC0000", "#3282F6", "#EA3FF7", "#18A705"]; // , "#782DFF"
        // var cntJct = route.junction_info.length;
    } // for

    map.fitCoordinates(vertices_all, {
        padding: 90,
        // heading: 90,
        // tilt: 30,
        // duration: 1000
    });
}

// 커스텀 오버레이를 생성하고 지도에 표시합니다
function drawRouteInfo(routes) {                                
    distanceOverlay = new inavi.maps.CustomInfoWindow({
        map: map, // 커스텀오버레이를 표시할 지도입니다
        content: getResultHtmlContent(routes[0].summary.distance, routes[0].summary.time),  // 커스텀오버레이에 표시할 내용입니다
        position: markers[1].getPosition(), // 커스텀오버레이를 표시할 위치입니다.
        // xAnchor: 0,
        // yAnchor: 0,
        anchor: 'top-right',
        zIndex: 3,
        opacity: 0.8,
        closeButton: true
    });

    document.querySelector(".popup-close-button").addEventListener("click", function() {
        distanceOverlay.setMap();
    });
}

function drawNavRouteInfo(route) {                                
    distanceOverlay = new inavi.maps.CustomInfoWindow({
        map: map, // 커스텀오버레이를 표시할 지도입니다
        content: getResultHtmlContent(route.data[0].distance, route.data[0].spend_time),  // 커스텀오버레이에 표시할 내용입니다
        position: markers[1].getPosition(), // 커스텀오버레이를 표시할 위치입니다.
        // xAnchor: 0,
        // yAnchor: 0,
        anchor: 'top-right',
        zIndex: 3,
        opacity: 0.8,
        closeButton: true
    });

    document.querySelector(".popup-close-button").addEventListener("click", function() {
        distanceOverlay.setMap();
    });
}

function redrawRoute(map) {
    setPolyline(map);
    setDashline(map);

    if (nodes != undefined && nodes.length) {
        nodes.forEach(element => element.setMap(map));
    }

    if (distanceOverlay != undefined) {
        distanceOverlay.setMap(map);
    }
}


function getRoutes(path, params)
{
    // var pathname = '/api/route'; // window.location.pathname;
    var pathname = path;
    var route2url = pathname + '?' + params.toString();
    // var route2url = window.location.pathname + '?' + params.toString();

    console.log("request route : " + route2url.toString());

    typeRouteStep = ClickTypeNone;

    // window.open(move2url, '_self');
    $.ajax({
    type: 'get',         // 타입 (get, post, put 등등)
    url: route2url,       // 요청할 서버 url
    // async: false,         // 비동기화 여부 (default : true)
    success: function(result) {
        var header = result.header;
        if (header.isSuccessful == true) {      
            console.log("response, route result success");

            // set route result
            user_obj = result.user_info;
            
            releaseRoute();

            drawRoutes(result.routes, map, route_offset, route_color);            

            drawRouteInfo(result.routes);

            if (result.routes.length > 1) {
                displayRoutesList(result.routes);
            }            
        } else {
            alert('err code: ' + header.resultCode + '\nerr msg: ' + header.resultMessage)
        }
    },
    error: function(request, status, error) {
        alert('경로 탐색 요청 실패')
    },
    complete: function(xhr, status) {
    }
    });    
}


function getiNaviRoutes(path, params)
{
    // var pathname = '/api/route'; // window.location.pathname;
    var pathname = path;
    var route2url = pathname + '?' + params.toString();
    // var route2url = window.location.pathname + '?' + params.toString();

    console.log("request route : " + route2url.toString());

    typeRouteStep = ClickTypeNone;

    // window.open(move2url, '_self');
    $.ajax({
    type: 'get',         // 타입 (get, post, put 등등)
    url: route2url,       // 요청할 서버 url
    // async: false,         // 비동기화 여부 (default : true)
    success: function(result) {
        var header = result.header;
        if (header.isSuccessful == true) {      
            console.log("response, route result success");

            // set route result
            user_obj = result.user_info;
            
            releaseRoute();

            drawiNaviRoutes(result.route.data, map, route_offset, route_color);

            drawNavRouteInfo(result.route);

            // displayRoutesList(result.routes);
        } else {
            alert('err code: ' + header.resultCode + '\nerr msg: ' + header.resultMessage)
        }
    },
    error: function(request, status, error) {
        alert('경로 탐색 요청 실패')
    },
    complete: function(xhr, status) {
    }
    });    
}


// release map item
function releaseRoute() {
    setPolyline();
    setDashline();

    if (nodes != undefined && nodes.length) {
        nodes.forEach(element => element.setMap());
        nodes.splice(0);
    }

    if (distanceOverlay != undefined) {
        distanceOverlay.setMap();
    }
}


// 결과 항목을 Element로 반환하는 함수
function getRouteListItem(idx, dist, time) {
    var el = document.createElement('li'),
    itemStr = '<div style="position:absolute;top:20px;left:2px;"><img id="poiicon" width="20" src="https://www.fivepin.co.kr/images/PoiMarker/marker_1_on.png"></div>' +
        '<div class="info">' +
        '   <span class="type"> <h5> ' + '       ' + idx + '</h5> </span>' +
        '   <span class="addr" style="font-size:11px; margin:2px 0px"> 시간 : ' + '       ' + dpDistanceToString(dist) + '</span>' +
        '   <span class="addr" style="font-size:11px; margin:2px 0px"> 거리 : ' + '       ' + dpTimeToString(time) + '</span>' +
        '</div>';

    el.innerHTML = itemStr;
    el.className = 'item';

    return el;
}


// 검색 결과 목록 표출 함수
function displayRoutesList(routes) {

    document.getElementById('menu_title').innerHTML = "경로 탐색 결과";
    
    var listEl = document.getElementById('placesList'),
    menuEl = document.getElementById('menu_wrap'),
    fragment = document.createDocumentFragment(),
    listStr = '';


    // 기존 목록 삭제
    while (listEl.hasChildNodes()) {
        listEl.removeChild(listEl.lastChild)
    }


    let idx = 0;
    let totTime = 0;
    let totDist = 0;
    routes.forEach(function(route) {
        var itemEl = getRouteListItem(idx++, route.summary.distance, route.summary.time);
        totTime += route.summary.time;
        totDist += route.summary.distance;

        fragment.appendChild(itemEl);
    }) // for each

    listEl.appendChild(fragment);
    menuEl.scrollTop = 0;

    resultDiv = document.getElementById('searched_type');
    resultDiv.innerHTML = '시간:' +  dpTimeToString(totTime) + ', 거리:' + dpDistanceToString(totDist) + ',경로:' + routes.length ;
    
}


function doRoute() {
    if (typeRouteStep == 0 || departure_pos === undefined || destination_pos === undefined) {
        alert("출,도착지를 설정하고 사용하세요.")
    } else {
        var params = new URLSearchParams(window.location.search);
        var pathsplit = window.location.pathname.split('/');
        var pathname = '/api/' + pathsplit[pathsplit.length - 1];
        params.set('start', departure_pos.lng.toString() + ',' + departure_pos.lat.toString());
        params.set('end', destination_pos.lng.toString() + ',' + destination_pos.lat.toString());
        // var pathname = '/optimalposition'; // window.location.pathname;
        // var move2url = pathname + '?' + params.toString();
        // var move2url = pathname + '?lng=' + latlng.lng + '&lat=' + latlng.lat + '&type=0&near=true&expand=true&api=inavi';
        params.delete('vias');
        for (var ii = 0; ii < waypoint_pos.length; ii++) {
            if (waypoint_pos[ii] != null && waypoint_pos[ii].lng > 120 && waypoint_pos[ii].lat > 30) {
                params.append('vias', waypoint_pos[ii].lng.toString() + ',' + waypoint_pos[ii].lat.toString());
            }
        }

        params.set('option', getOption());
        if (route_mode === RouteMode_Pedestrian) { // 보행모드            
            params.set('mobility', getMobility());
            getRoutes(pathname, params);
        } else if (route_mode === RouteMode_Forest) { // 숲길모드
            // pathname = '/api/kakaovx'; <-- 이게 맞는지는 확인 필요
            params.set('mobility', getMobility());
            params.set('junction', true);
            params.set('course_type', getCourseType());
            params.set('course_id', getCourseId());
            getRoutes('/api/kakaovx/', params);
        } else if (route_mode === RouteMode_Path) { // p2p
            // pathname = '/api/path'; <-- 이게 맞는지는 확인 필
            // params.set('opt', 8);
            params.delete('candidate');
            if (candidate > 0) {
                params.append('candidate', candidate);
            }
            getiNaviRoutes(pathname, params);
        } else { //
            // pathname = '/api/multiroute'; <-- 이게 맞는지는 확인 필요
            getRoutes(pathname, params);
            // or 
            //getiNaviRoutes(pathname, params);
        }

        setRouteType(ClickTypeNone);
    }
}


// 경로 결과를 새창에서 JSON 결과로 보내도록 하자
function gotoJSON() {
    if (departure_pos === undefined || destination_pos === undefined) {
        alert("출,도착지를 설정하고 사용하세요.")
    } else {
        var params = new URLSearchParams(window.location.search);
        var pathsplit = window.location.pathname.split('/');
        var pathname = '/api/' + pathsplit[pathsplit.length - 1];
        params.set('start', departure_pos.lng.toString() + ',' + departure_pos.lat.toString());
        params.set('end', destination_pos.lng.toString() + ',' + destination_pos.lat.toString());
        // var pathname = '/optimalposition'; // window.location.pathname;
        // var move2url = pathname + '?' + params.toString();
        // var move2url = pathname + '?lng=' + latlng.lng + '&lat=' + latlng.lat + '&type=0&near=true&expand=true&api=inavi';
        params.delete('vias');
        for (var ii = 0; ii < waypoint_pos.length; ii++) {
            if (waypoint_pos[ii] != null && waypoint_pos[ii].lng > 120 && waypoint_pos[ii].lat > 30) {
                params.append('vias', waypoint_pos[ii].lng.toString() + ',' + waypoint_pos[ii].lat.toString());
            }
        }

        params.set('option', getOption());
        if (route_mode === RouteMode_Pedestrian) { // 보행모드
            params.set('mobility', getMobility());
        } else if (route_mode === RouteMode_Forest) { // 숲길모드
            // pathname = '/api/kakaovx'; <-- 이게 맞는지는 확인 필요
            params.set('mobility', getMobility());
            params.set('junction', true);
            params.set('course_type', getCourseType());
            params.set('course_id', getCourseId());
        } else if (route_mode === RouteMode_Path) { // p2p
            // pathname = '/api/path'; <-- 이게 맞는지는 확인 필
            params.delete('candidate');
            if (candidate > 0) {
                params.append('candidate', candidate);
            }
        } else { //
            // pathname = '/api/multiroute'; <-- 이게 맞는지는 확인 필요
        }

        setRouteType(ClickTypeNone);

        var route2url = pathname + '?' + params.toString();

        console.log("request route : " + route2url.toString());

        window.open(route2url, '_blank');
    }
}