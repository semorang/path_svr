let map;
let marker;
        
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
        marker.setVisible(false);
    } else {
        // 인포윈도우 
        var content = '<div style="padding:5px;z-index:1;text-align:center">' + title + '</div>';
        searchinfoWindow.setContent(content);
        searchinfoWindow.setPosition(position);
        // infoWindow.open(map);
        searchinfoWindow.setVisible(true);

        // 마커
        marker.setPosition(position);
        marker.setVisible(true);

        // 지도 이동
        map.flyTo(position);
    }
}