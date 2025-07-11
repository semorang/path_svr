let map;
// let marker;
let searchinfoWindow;

// 마커가 표시될 위치입니다
let markers = new Array();

function initMapObject(map) {
    // 지도 클릭 이벤트 등록
    map.on("click", function (mouseEvent) {
        // var lnglat = mouseEvent.lngLat;
        // var pathname = window.location.pathname;
        // var move2url = pathname + '?lng=' + lnglat.lng + '&lat=' + lnglat.lat + '&type=0&api=inavi';
        // window.open(move2url, '_self');

        // alert('클릭 기능 아직 미적용')
        console.log('클릭된 좌표 : ' + mouseEvent.lngLat.lng + ", " + mouseEvent.lngLat.lat);
        setMapClick(mouseEvent.lngLat);
    });
    //-- 지도 클릭 이벤트 등록

    initMarker(user_pos);

    // 인포윈도우
    searchinfoWindow = new inavi.maps.InfoWindow({ map: map, zIndex: 1, opacity: 0, offset: [0, -45], closeButton: false });
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
    navigator.geolocation.getCurrentPosition(function(pos) {
        
        var lng = pos.coords.longitude;
        var lat = pos.coords.latitude;
        console.log("current pos : " + pos + " lng: " + lng + " lat: " + lat);

        var position = new inavi.maps.LngLat(lng, lat);
        map.flyTo(position);
    })
}