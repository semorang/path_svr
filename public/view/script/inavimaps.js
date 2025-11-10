let map;
// let marker;
let searchinfoWindow;

// 마커가 표시될 위치입니다
let markers = new Array();

// 현위치 및 맵매칭 상태
let currentMatchingState = 0; // 0:기본, 1:매칭, 2:현위치
let currentMarker; // 현위치 마커
let currentPosition; // 현위치좌표
let currentMatchingTimer = null; // 현위치 자동 반복

let currentRoutePopup; // 현위치 탐색 팝업

// 경로선
let linksLine = new Array();
let linksDash = new Array();

function initMapObject(map) {
    // 지도 클릭 이벤트 등록
    // map.on("click", function (mouseEvent) {


    let touchStartTime = 0;
    const longPressDuration = 2000; // 2초

    function handleMapEvent(lngLat, eventType) {
        console.log('클릭된 좌표 : ' + lngLat.lng + ", " + lngLat.lat);
        
        // if ((eventType === 'touchmove') || (eventType === 'mousehmove')) {
        //     touchStartTime = 0;
        //     hideRoutePopup();
        // } else if ((eventType === 'touchend') || (eventType === 'mouseup')) {
        //     const touchDuration = Date.now() - touchStartTime;

        //     if (touchStartTime && touchDuration >= longPressDuration) {
        //         showRoutePopup(lngLat); // 손가락 위치 기반으로 메뉴 표시
        //     } else {
        //         hideRoutePopup();
        //     }
        // } else if ((eventType === 'touchstart') || (eventType === 'mousedown')) {
        //     touchStartTime = Date.now();
        //     hideRoutePopup();
        // } 

        setMapClick(lngLat, eventType);
    }

    //-- 지도 클릭 이벤트 등록
    map.on("click", function(e) { handleMapEvent(e.lngLat, "click"); });
    // map.on("mousedown", function(e) { handleMapEvent(e.lngLat, "mousedown"); });
    map.on("touchstart", function(e) { handleMapEvent(e.lngLat, "touchstart"); });
    // map.on("mouseup", function(e) { handleMapEvent(e.lngLat, "mouseup"); });
    // map.on("touchend", function(e) { handleMapEvent(e.lngLat, "touchend"); });
    // map.on("mousehmove", function(e) { handleMapEvent(e.lngLat, "mousehmove"); });
    // map.on("touchmove", function(e) { handleMapEvent(e.lngLat, "touchmove"); });
    // map.on("pointerdown", handleMapClick);   

    initMarker(user_pos);
}


function initMarker(pos) {
    // marker = new inavi.maps.Marker({
    //     map: map,
    //     center: pos,
    // });
    // marker.setVisible(false);


    // 마커가 표시될 위치입니다
    markers.push(
        // 시작점
        new inavi.maps.Marker({
            map: map,
            // icon: "https://www.fivepin.co.kr/resources/images/PoiMarker/marker_1_on.png",
            icon: "images/flags/ico_road_flag_start.png",
            offset: [13, -4], // 깃발 봉위치를 센터로
            position: [0, 0],
            visibility: false,
            title: "출발지",
        }));
    markers.push(
        // 종료점
        new inavi.maps.Marker({
            map: map,
            // icon: "https://www.fivepin.co.kr/resources/images/PoiMarker/marker_3_on.png",
            icon: "images/flags/ico_road_flag_arrive.png",
            offset: [13, -4], // 깃발 봉위치를 센터로
            position: [0, 0],
            visibility: false,
            title: "목적지",
        }));
    for(var ii=0; ii<100; ii++) {
        markers.push(
        // 경유지
        new inavi.maps.Marker({
            map: map,
            // icon: "https://www.fivepin.co.kr/resources/images/PoiMarker/marker_2_on.png",
            icon: "images/flags/ico_road_flag_via.png",
            offset: [13, -4], // 깃발 봉위치를 센터로
            position: [0, 0],
            visibility: false,
            title: "경유지"+ii,
        }));
    }

    // 현위치 마커, 펄스 UI
    const markerElement = document.createElement('div');
    markerElement.className = 'pulse';
    currentMarker = new inavi.maps.Marker({
        map: map,
        icon: markerElement,
        position: [0,0],
        visibility: false,
        title: "현위치",        
    });

    const markerPopup = document.getElementById('popupMarker');
    currentRoutePopup = new inavi.maps.Marker({
        map: map,
        icon: markerPopup,
        position: [0,0],
        visibility: false,
        title: "현위치",        
    });
}


// 지도타입 컨트롤의 지도 또는 스카이뷰 버튼을 클릭하면 호출되어 지도타입을 바꾸는 함수입니다
function setMapType(maptype) { 
    var roadmapControl = document.getElementById('btnRoadmap');
    var skyviewControl = document.getElementById('btnSkyview'); 
    if (maptype === 'roadmap') {
        map.setType('NORMAL');
        roadmapControl.className = 'selected_btn';
        skyviewControl.className = 'btn';
    } else {
        map.setType('SATELLITE');
        skyviewControl.className = 'selected_btn';
        roadmapControl.className = 'btn';
    }

    redrawRoute(map);

    // if(map.getType() !== "SATELLITE") {
    //     // 지도 타입 설정
    //     let isShowTraffic = $("#trafficBtn").hasClass("on");

    //     map.once("render", () => {
    //     map.call("get","_").map.getStyle().layers.filter(layer => layer.id.startsWith("user_")).forEach(layer => {
    //     map.call("get","_").map.setLayoutProperty(layer.id, 'visibility', 'visible');
    //     })
    //     if(isShowTraffic) {
    //         map.call("get","_").map.setLayoutProperty("satellite_traffic", 'visibility', 'visible');
    //     } else {
    //         map.call("get","_").map.setLayoutProperty("satellite_traffic", 'visibility', 'none');
    //     }
    //     });
    // }
}


// 검색 인포윈도우 표출 함수
function displaySearchInfowindow(position, title) {
    if (position == null || position == undefined) {
        entranceinfoWindow.setVisible(false);
        // marker.setVisible(false);
    } else {
        // 인포윈도우
        searchinfoWindow = new inavi.maps.InfoWindow({ map: map, zIndex: 1, opacity: 0, offset: [0, -45], closeButton: true });

        // 인포윈도우 
        var content = '<div style="padding:5px;z-index:1;text-align:center">' + title + '</div>';
        searchinfoWindow.setContent(content);
        searchinfoWindow.setPosition(position);
        // infoWindow.open(map);
        searchinfoWindow.setVisible(true);

        // 마커
        // marker.setPosition(position);
        // marker.setVisible(true);

        // 지도 이동
        map.flyTo(position);
    }
}


// 현재 위치로 이동
function moveToCurrent() {
    if (!navigator.geolocation) {
        console.error('Geolocation API not supported');
        return;
    }

    navigator.geolocation.getCurrentPosition(function(pos) {        
        var lng = pos.coords.longitude;
        var lat = pos.coords.latitude;
        console.log("current pos : " + pos + " lng: " + lng + " lat: " + lat);

        var position = new inavi.maps.LngLat(lng, lat);
        map.flyTo(position);
    })
}


// 거리 계산 (Haversine 공식, m 단위)
function haversineDistance(lat1, lng1, lat2, lng2) {
    const R = 6371000; // Earth radius in meters
    const toRad = angle => angle * Math.PI / 180;

    const dLat = toRad(lat2 - lat1);
    const dLng = toRad(lng2 - lng1);

    const a = Math.sin(dLat / 2) ** 2 +
              Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) *
              Math.sin(dLng / 2) ** 2;

    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    return R * c;
}

// 가장 가까운 경로 포인트 찾기
/**
 * @param {Array} lineCoords - [(x1, y1), (x2, y2), ...] 형태의 vertex 배열
 * @param {Array} targetPoint - [x, y] 형태의 대상 좌표
 * @returns {Array} - 선형에 수직으로 투영된 가장 가까운 지점의 좌표 [x, y]
 */
function getPerpendicularClosestPoint(lineCoords, targetPoint) {
  const [px, py] = targetPoint;
  let minDistance = Infinity;
  let result = null;

  for (let i = 0; i < lineCoords.length - 1; i++) {
    const [x1, y1] = lineCoords[i];
    const [x2, y2] = lineCoords[i + 1];

    const dx = x2 - x1;
    const dy = y2 - y1;
    const lenSq = dx * dx + dy * dy;

    if (lenSq === 0) continue;

    // 선분에 수직 투영한 위치를 비율로 구함
    const t = ((px - x1) * dx + (py - y1) * dy) / lenSq;

    // 0 <= t <= 1이면 선분 내부, 그 외에는 끝점에 가장 가까움
    const tClamped = Math.max(0, Math.min(1, t));

    const projX = x1 + tClamped * dx;
    const projY = y1 + tClamped * dy;

    // const distSq = (px - projX) ** 2 + (py - projY) ** 2;
    const currDistance = haversineDistance(py, px, projY, projX);

    if (currDistance < minDistance) {
      minDistance = currDistance;
      result = {lng: projX, lat: projY, distance: minDistance};
    }
  }

  return result;
}


function performRouteMatch(moveToCenter = false) {
    if (!navigator.geolocation) {
        console.error('Geolocation API not supported');
        return;
    }

    navigator.geolocation.getCurrentPosition(
        function(pos) {
            var lng = pos.coords.longitude;
            var lat = pos.coords.latitude;
            const maxDist = 50; // 50m

            if (currentMatchingState == 1) {
                currentPosition = new inavi.maps.LngLat(lng, lat);
            } else if (vertices_all.length <= 0) {
                showToast(`경로가 존재하지 않습니다.`, 'error');
                currentPosition = new inavi.maps.LngLat(lng, lat);
            } else {
                const result = getPerpendicularClosestPoint(vertices_all, [lng, lat]);
                if (result && result.distance <= maxDist) {
                    // showToast("경로 매칭 성공");
                    currentPosition = new inavi.maps.LngLat(result.lng, result.lat);
                } else {
                    showToast(`${maxDist}m이내 경로가 없습니다.`);
                    currentPosition = new inavi.maps.LngLat(lng, lat);
                }
            }

            if (!currentMarker.getVisible()) {
                currentMarker.setVisible(true);                
                currentMarker.setMap(map);
            }
            currentMarker.setPosition(currentPosition);

            if (moveToCenter) {
                map.flyTo(currentPosition);
            }
            

            // const pixel = new inavi.maps.LngLat.convertToPixel(map, currentPosition);
            // // const pixel = currentPosition.convertToPixel();
            // const pulseDiv = document.getElementById("pulseMarker");
            // pulseDiv.style.left = `${pixel.x}px`;
            // pulseDiv.style.top = `${pixel.y}px`;
        },
        function(error) {

        },
        {
            enableHighAccuracy: true,
            timeout: 5000,
            maximumAge: 0
        }
    );
};

// 가까운 경로에 매칭
function matchToRoute() {
    var itemBtn = document.getElementById('btnMatch');
    var itemIcon = itemBtn.querySelector('img');

    currentMatchingState++;
    if ((currentMatchingState > 2) || (currentMatchingState == 2 && (vertices_all.length <= 0))) {
        currentMatchingState = 0;
    } 
    
    if (currentMatchingState == 0) {
        // 비활성
        itemBtn.className = 'btn';
        // itemIcon.style.filter = 'none';
        itemIcon.src = "images/icons/resize_50_icon-walk-001.png";
        currentMarker.setVisible(false);

        // 자동 반복 중단
        if (currentMatchingTimer) {
            clearInterval(currentMatchingTimer);
            currentMatchingTimer = null;
        }
    } else {
        itemBtn.className = 'selected_btn';
        if (currentMatchingState === 1) {
            itemIcon.src = "images/icons/resize_50_resize_50_icon-walk-001-red.png";
            itemIcon.style = "filter: brightness(1.8);"
        } else {
            itemIcon.src = "images/icons/resize_50_resize_50_icon-walk-001-blue.png";
            itemIcon.style = "filter: brightness(1.2);"
        }
        // 자동 반복 활성화

        performRouteMatch(true);

        let autoMatchingTime = 10 * 1000; // 10초
        if (getMobility() != Mobility_Pedestrian) {
            autoMatchingTime = 5 * 1000; // 5초 자전거는 좀더 빠르게 갱신
        }
 
        currentMatchingTimer = setInterval(() => {
            if (currentMatchingState) performRouteMatch();
        }, autoMatchingTime); // 지정시간마다 자동 수행
    }
}


// 폴리라인
function appendPolyline(vertices, width, color, offset) {
    // 경로선 추가
    linksLine.push(new inavi.maps.Polyline({
        map: map,
        path: vertices,
        style: {
            lineOffset: offset,
            lineColor: color,
            lineWidth: width,
        },
    }));
}


// 대쉬라인
function appendDashline(vertices, width, color, offset, dash) {
    // 경로선 추가
    linksDash.push(new inavi.maps.Polyline({
        map: map,
        path: vertices,
        style: {
            lineOffset: offset,
            lineColor: color,
            lineWidth: width,
            lineDasharray: dash,
        },
    }));
}


function setPolyline(map) {
    if (map === undefined || map === null) {
        if (linksLine != undefined && linksLine.length) {
            linksLine.forEach(element => element.setMap());
            linksLine.splice(0);
        }
    } else {
        if (linksLine != undefined && linksLine.length) {
            linksLine.forEach(element => element.setMap(map));
        }
    }
}

function setDashline(map) {
    if (map === undefined || map === null) {
        if (linksDash != undefined && linksDash.length) {
            linksDash.forEach(element => element.setMap());
            linksDash.splice(0);
        }
    } else {
        if (linksDash != undefined && linksDash.length) {
            linksDash.forEach(element => element.setMap(map));
        }
    }
}