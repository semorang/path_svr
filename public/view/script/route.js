
// 경로선
let links = new Array();

// 노드
let nodes = new Array();

// let colors = ["#CC0000", "#3282F6", "#EA3FF7", "#18A705"]; // , "#782DFF"
let route_color = ["#CC0000", "#00FF00", "#0000FF", "#00eeff"];
let route_offset = [0, 1, -1, -2];
// let route_color = ["#CC0000", "#008888"];
// let route_offset = [0, 2];

let user_obj;
let user_pos;

// 경로 탐색 정보
let typeRouteStep = 0; // 0:없음, 1:출발, 2:도착, 3:경유1, 4:경유2, 5:경유3, 6:경유4, 7:경유5
let click_pos;
let departure_pos;
let waypoint_pos = new Array(5);
let destination_pos;
let route_option = 1; // 0:최단거리, 추천, 편안한, 최소시간(빠른), 큰길
let course_type = 1; // 코스 타입 //0:미정의, 1:등산, 2:걷기, 3:자전거, 4:코스
let course_id = 0; // 코스 ID 
let route_mobility = 1; // 0:보행자, 1:자전거


function setMapClick(pos) {
    switch(typeRouteStep) {
        // 출발지
        case 1: {
            setDeparture(pos);
        }
        break;

        // 목적지
        case 2: {
            setDestination(pos);
        }
        break;

        // 경유지
        case 3: case 4: case 5: case 6: case 7: {
            setWaypoints(typeRouteStep - 3, pos);
        }
        break;

        default:
            break;
    } // switchf
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

    typeRouteStep = 9999; // 변경사항이므로
}


function getCourseType() {
    return course_type;
}

function setCourseType(type) {
    course_type = type;

    typeRouteStep = 9999; // 변경사항이므로
}

function getCourseId() {
    return course_id;
}

function setCourseId(id) {
    course_id = id;

    typeRouteStep = 9999; // 변경사항이므로
}


function getMobility() {
    return route_mobility;
}


function setMobility(mob) {
    route_mobility = mob;

    typeRouteStep = 9999; // 변경사항이므로
}


// 경로탐색 컨트롤의 버튼 클릭
function setRouteType(type) {
    typeRouteStep = type;
    switch(type) {
        case 1: {
            // 출발지 설정
            document.getElementById('btnStart').className = 'selected_btn';                    
            document.getElementById('btnEnd').className = 'btn';
            document.getElementById('btnVia1').className = 'btn';
            document.getElementById('btnVia2').className = 'btn';
            document.getElementById('btnVia3').className = 'btn';
            document.getElementById('btnVia4').className = 'btn';
            document.getElementById('btnVia5').className = 'btn';
        }
        break;

        case 2: {
            // 목적지 설정
            document.getElementById('btnStart').className = 'btn';
            document.getElementById('btnEnd').className = 'selected_btn';
            document.getElementById('btnVia1').className = 'btn';
            document.getElementById('btnVia2').className = 'btn';
            document.getElementById('btnVia3').className = 'btn';
            document.getElementById('btnVia4').className = 'btn';
            document.getElementById('btnVia5').className = 'btn';
        }
        break;

        case 3: {
            // 경유지1 설정
            if (document.getElementById('btnVia1').className == 'selected_btn') {
                // 해제
                typeRouteStep = 9999 // 해제도 변경사항이므로
                document.getElementById('btnVia1').className = 'btn';                        
                setWaypoints(0);
            } else {
                document.getElementById('btnStart').className = 'btn';
                document.getElementById('btnEnd').className = 'btn';
                document.getElementById('btnVia1').className = 'selected_btn';
                document.getElementById('btnVia2').className = 'btn';
                document.getElementById('btnVia3').className = 'btn';
                document.getElementById('btnVia4').className = 'btn';
                document.getElementById('btnVia5').className = 'btn';
            }
        }
        break;

        case 4: {
            // 경유지2 설정
            if (document.getElementById('btnVia2').className == 'selected_btn') {
                // 해제
                typeRouteStep = 9999 // 해제도 변경사항이므로
                document.getElementById('btnVia2').className = 'btn';                        
                setWaypoints(1);
            } else {
                document.getElementById('btnStart').className = 'btn';
                document.getElementById('btnEnd').className = 'btn';
                document.getElementById('btnVia1').className = 'btn';
                document.getElementById('btnVia2').className = 'selected_btn';
                document.getElementById('btnVia3').className = 'btn';
                document.getElementById('btnVia4').className = 'btn';
                document.getElementById('btnVia5').className = 'btn';
            }
        }
        break;

        case 5: {
            // 경유지3 설정
            if (document.getElementById('btnVia3').className == 'selected_btn') {
                // 해제
                typeRouteStep = 9999 // 해제도 변경사항이므로
                document.getElementById('btnVia3').className = 'btn';                        
                setWaypoints(2);
            } else {
                document.getElementById('btnStart').className = 'btn';
                document.getElementById('btnEnd').className = 'btn';
                document.getElementById('btnVia1').className = 'btn';
                document.getElementById('btnVia2').className = 'btn';
                document.getElementById('btnVia3').className = 'selected_btn';
                document.getElementById('btnVia4').className = 'btn';
                document.getElementById('btnVia5').className = 'btn';
            }
        }
        break;

        case 6: {
            // 경유지4 설정
            if (document.getElementById('btnVia4').className == 'selected_btn') {
                // 해제
                typeRouteStep = 9999 // 해제도 변경사항이므로
                document.getElementById('btnVia4').className = 'btn';                        
                setWaypoints(3);
            } else {
                document.getElementById('btnStart').className = 'btn';
                document.getElementById('btnEnd').className = 'btn';
                document.getElementById('btnVia1').className = 'btn';
                document.getElementById('btnVia2').className = 'btn';
                document.getElementById('btnVia3').className = 'btn';
                document.getElementById('btnVia4').className = 'selected_btn';
                document.getElementById('btnVia5').className = 'btn';
            }
        }
        break;

        case 7: {
            // 경유지5 설정
            if (document.getElementById('btnVia5').className == 'selected_btn') {
                // 해제
                typeRouteStep = 9999 // 해제도 변경사항이므로
                document.getElementById('btnVia5').className = 'btn';                        
                setWaypoints(4);
            } else {
                document.getElementById('btnStart').className = 'btn';
                document.getElementById('btnEnd').className = 'btn';
                document.getElementById('btnVia1').className = 'btn';
                document.getElementById('btnVia2').className = 'btn';
                document.getElementById('btnVia3').className = 'btn';
                document.getElementById('btnVia4').className = 'btn';
                document.getElementById('btnVia5').className = 'selected_btn';
            }
        }
        break;

        default: {
            document.getElementById('btnStart').className = 'btn';
            document.getElementById('btnEnd').className = 'btn';
            document.getElementById('btnVia1').className = 'btn';
            document.getElementById('btnVia2').className = 'btn';
            document.getElementById('btnVia3').className = 'btn';
            document.getElementById('btnVia4').className = 'btn';
            document.getElementById('btnVia5').className = 'btn';
        }
        break;
    }
}


function getRoute(path, params)
{
    // var pathname = '/api/route'; // window.location.pathname;
    var pathname = path;
    var route2url = pathname + '?' + params.toString();
    // var route2url = window.location.pathname + '?' + params.toString();

    console.log("request route : " + route2url.toString());

    typeRouteStep = 0;

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
            releaseRoute();

            drawRoutes(result.routes, map, route_offset, route_color);

            drawRouteInfo(result.routes);
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

let distanceOverlay;


let routeMarker = new Array();


// draw map item <-- 현재는 안쓰는 걸로 2025-02-24
function drawRoute(result, map) {
    result_obj = result;

    user_obj = result_obj.user_info;
    cnt_route = result_obj.routes.length;

    user_pos.x = result_obj.routes[0].x;
    user_pos.y = result_obj.routes[0].y;

    var bounds = new Array();

    // 경로 그리기
    for (var ii = cnt_route - 1; ii >= 0; --ii) {
        let result_dat = result_obj.routes[ii];
        // var cntLink = result_dat.link_info.length;
        // var cntVtx = result_dat.vertex_info.length;
        // var cntJct = result_dat.junction_info.length;

        var vertices = new Array();
        var vertex_offset = 0;

        // 경로선 생성
        var via_idx = 0;
        result_dat.link_info.forEach(function(link) {
            for (var vtx = 0; vtx < link.vertex_count; vtx++) {
                // 경로선 확장
                bounds.push([result_dat.vertex_info[vertex_offset].x, result_dat.vertex_info[vertex_offset].y]);
                
                vertices.push([result_dat.vertex_info[vertex_offset].x, result_dat.vertex_info[vertex_offset].y]);
                vertex_offset++;
            }

            // if (ii != 0 && link.guide_type != 0) {
            //     via_idx++;

            //     // // 경로선 추가
            //     // links.push(new inavi.maps.Polyline({
            //     //     map: map,
            //     //     path: vertices,
            //     //     style: {
            //     //         lineOffset: off,
            //     //         lineColor: color,
            //     //         // lineColor: colors[via_idx++ % colors.length],
            //     //         lineWidth: 5,
            //     //         lineOpacity: 0.7,
            //     //     },
            //     // }));
                
            //     // // 초기화
            //     // vertices.splice(0);
            // }
        }); // forEach

        links.push(new inavi.maps.Polyline({
            map: map,
            path: vertices,
            style: {
                lineOffset: route_offset[ii],
                lineColor: route_color[ii],
                // lineColor: colors[via_idx++ % colors.length],
                lineWidth: 5,
                lineOpacity: 0.8,
            },
        }));

        // // 초기화
        // vertices.splice(0);

        //-- 경로선 생성


        // 노드 생성
        // 경로에 노드를 생성합니다
        // var off = 0;
        // // 마지막 목적지의 종료 노드까지 처리하기 위해 for를 +1해서 돌림
        // for (var ii = 0; ii < cntLink; ii++) {
        //     nodes.push(new inavi.maps.Circle({
        //         map: map,
        //         position: [result_dat.vertex_info[off].x, result_dat.vertex_info[off].y], // 원 중심 좌표
        //         radius: 0.005, // 원 반지름 킬로미터 단위
        //         style: {
        //             fillOpacity: 0.5,  // 채우기 불투명도 입니다
        //             fillColor: "#CFE7FF",
        //             fillOutlineColor: "#DB4455",
        //         },
        //     }));

        //     off += result_dat.link_info[ii].vertex_count;
        //     if (off >= result_dat.vertex_info.length) {
        //         break;
        //     }                
        // } // for

        // // 마지막 목적지의 종료 노드까지 처리하기 위해 
        // if (result_dat.vertex_info[off - 1] != undefined) {
        //     nodes.push(new inavi.maps.Circle({
        //         map: map,
        //         position: [result_dat.vertex_info[off - 1].x, result_dat.vertex_info[off - 1].y], // 원 중심 좌표
        //         radius: 0.005, // 원 반지름 킬로미터 단위
        //         style: {
        //             fillOpacity: 0.5,  // 채우기 불투명도 입니다
        //             fillColor: "#CFE7FF",
        //             fillOutlineColor: "#DB4455",
        //         },
        //     }));
        // }
        //-- 노드 생성

        // 정션 생성
        // KakaoVX 스타일
        // if (result_dat.junction_info != undefined) {
        //     result_dat.junction_info.forEach(function(jct) {
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
        //                     lineWidth: 2,
        //                     // lineOpacity: 0.7,
        //                 },
        //             }));
        //         });// forEach lnk
        //     }); // forEach jct
        // }
        if (result_dat.junction_info != undefined) {
            result_dat.junction_info.forEach(function(jct) {
                var vertices = new Array();
                jct.vertices.forEach(function(coord) {
                    vertices.push([coord.x, coord.y]);
                }); // forEach vtx
                
                links.push(new inavi.maps.Polyline({
                    map: map,
                    path: vertices,
                    style: {
                        // lineColor: "#F09B59", //"#FFC90E",
                        lineColor: "#309BF9", //"#FFC90E",
                        lineWidth: 3,
                        lineDasharray: [2, 1], // (1픽셀 x lineWidth) 라인, (1픽셀 x lineWidth) 공백 표시 반복.
                        // lineOpacity: 0.7,
                    },
                }));
            }); // forEach jct
        }              
        //-- 정션 생성
    }




    map.fitCoordinates(bounds, {
        padding: 90,
        // heading: 90,
        // tilt: 30,
        // duration: 1000
    });
}


// draw map item
function drawRoutes(routes, map, offsets, colors) {
    // release marker
    routeMarker.forEach(marker => marker.setMap(0));
    routeMarker.splice(0);

    var vertices_all = new Array();
    var offRoute = 0;
    var via_idx = 0;
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
            for (var vtx = 0; vtx < route.link_info[ii].vertex_count; vtx++) {
                // 경로선 확장
                if (route.vertex_info[vertex_offset] != undefined) {
                    vertices_all.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);                    
                    vertices.push([route.vertex_info[vertex_offset].x, route.vertex_info[vertex_offset].y]);
                }                
                vertex_offset++;
            }
    
            if ((route.link_info[ii].guide_type != 0) &&
                 (ii != 0) || (ii == 0 && cntLink <= 1)) {
                // 경로선 추가
                links.push(new inavi.maps.Polyline({
                    map: map,
                    path: vertices,
                    style: {
                        lineOffset: line_off,
                        lineColor: color_off,
                        // lineColor: colors[via_idx++ % colors.length],
                        lineWidth: 5,
                        // lineOpacity: 0.7,
                    },
                }));

                // 초기화
                vertices.splice(0);

                via_idx++;
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
                    
                    links.push(new inavi.maps.Polyline({
                        map: map,
                        path: junctions,
                        style: {
                            lineColor: "#F09B59", //"#FFC90E",
                            lineWidth: 3,
                            // lineOpacity: 0.7,
                        },
                    }));
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
    vertices_all.splice(0);
}


// draw map item
function drawiNaviRoutes(routes, map, offsets, colors) {
    var vertices_all = new Array();                     

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
        links.push(new inavi.maps.Polyline({
            map: map,
            path: vertices,
            style: {
                lineOffset: off,
                lineColor: col,
                lineWidth: 5, //,
                // lineOpacity: 0.7,
            },
        }));
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
    vertices_all.splice(0);
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
    if (links != undefined && links.length) {
        links.forEach(element => element.setMap(map));
    }

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

    typeRouteStep = 0;

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

            // user_pos.x = result.routes[0].x;
            // user_pos.y = result.routes[0].y;
            
            releaseRoute();

            drawRoutes(result.routes, map, route_offset, route_color);

            drawRouteInfo(result.routes);

            displayRoutesList(result.routes);
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

    typeRouteStep = 0;

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

            // user_pos.x = result.routes[0].x;
            // user_pos.y = result.routes[0].y;
            
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
    if (links != undefined && links.length) {
        links.forEach(element => element.setMap());
        links.splice(0);
    }

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

    resultDiv = document.getElementById('clicked_type');
    resultDiv.innerHTML = '시간:' +  dpTimeToString(totTime) + ', 거리:' + dpDistanceToString(totDist) + ',경로:' + routes.length ;
    
}