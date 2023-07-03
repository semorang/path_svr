// 입구점 결과 목록 표출 함수
function displayWaypointList(user, routes) {

    document.getElementById('menu_title').innerHTML = "경유지 목록";

    var listEl = document.getElementById('placesList'),
    menuEl = document.getElementById('menu_wrap'),
    fragment = document.createDocumentFragment(),
    listStr = '';


    // 기존 목록 삭제
    while (listEl.hasChildNodes()) {
        listEl.removeChild(listEl.lastChild)
    }

    let start = user.start;
    let end = user.end;
    let vias = user.vias;

    // 출발지
    var position = new inavi.maps.LngLat(start.x, start.y);
    var itemEl = getWaypointListItem(0, 'start', start);
    var title = '출발지';

    (function(position, title) {
        itemEl.onmouseover = function() {
            displayWaypointInfowindow(position, title);
        };

        itemEl.onmouseout = function() {
            displayWaypointInfowindow();
        };

        itemEl.onclick = function() {
            // 지도 이동
            map.flyTo(position);
        }
    })(position, title);
    fragment.appendChild(itemEl);

    // 경유지 인덱스 확인
    var viaIndex = new Array();
    for (ii=0; ii < routes.link_info.length; ii++) {
        if (routes.link_info[ii].guide_type != 0) {
            viaIndex.push(ii);
        }
    }
    viaIndex.push(routes.link_info.length - 1);

    // 경유지
    if (vias != undefined && vias.length >= 1) {
        for (var ii=0; ii<vias.length; ii++) {
            var position = new inavi.maps.LngLat(vias[ii].x, vias[ii].y);
            var rdist = (routes.link_info[viaIndex[ii]] != undefined) ? routes.link_info[viaIndex[ii]].remain_distance : -1;// - routes.link_info[viaIndex[ii + 1]].remain_distance;
            var rtime = (routes.link_info[viaIndex[ii]] != undefined) ? routes.link_info[viaIndex[ii]].remain_time : -1;// - routes.link_info[viaIndex[ii + 1]].remain_time;
            var itemEl = getWaypointListItem(ii, 'via', vias[ii], rdist, rtime);
            var title = '경유지: ' + (ii + 1);

            (function(position, title) {
                itemEl.onmouseover = function() {
                    displayWaypointInfowindow(position, title);
                };

                itemEl.onmouseout = function() {
                    displayWaypointInfowindow();
                };

                itemEl.onclick = function() {
                    // 지도 이동
                    map.flyTo(position);
                }
            })(position, title);
            fragment.appendChild(itemEl);
        }
    } // for

    // 목적지
    position = new inavi.maps.LngLat(end.x, end.y);
    // var itemEl = getWaypointListItem(0, 'end', end); // tsp 에서는 도착지를 마지막 경유지로 사용
    if (routes.link_info[viaIndex[vias.length - 1]] != undefined) {
        itemEl = getWaypointListItem(vias.length, 'via', end, routes.link_info[viaIndex[vias.length - 1]].remain_distance, routes.link_info[viaIndex[vias.length - 1]].remain_time);
    } else {
        itemEl = getWaypointListItem(vias.length, 'via', end, -1, -1);
    }
    
    // var title = '목적지';
    title = '경유지: ' + (vias.length + 1);

    (function(position, title) {
        itemEl.onmouseover = function() {
            displayWaypointInfowindow(position, title);
        };

        itemEl.onmouseout = function() {
            displayWaypointInfowindow();
        };

        itemEl.onclick = function() {
            // 지도 이동
            map.flyTo(position);
        }
    })(position, title);
    fragment.appendChild(itemEl);

    // 등록
    listEl.appendChild(fragment);
    menuEl.scrollTop = 0;
}

// 결과 항목을 Element로 반환하는 함수
function getWaypointListItem(index, type, entrance, dist, time) {
    var el;
    if (type === 'start') {
        el = document.createElement('li'),
        itemStr = '<div class="circle_start" background:"blue"> <span>' + 'S' + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 출발지' + 
            '       ' + '</h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '       ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '</div>';
    } else if (type === 'end') {
        el = document.createElement('li'),
        itemStr = '<div class="circle_end" background:"blue"> <span>' + 'E' + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 목적지' +
            '       ' + '</h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '       ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '</div>';
    } else {
        el = document.createElement('li'),
        itemStr = '<div class="circle_via" background:"blue"> <span>' + (index + 1) + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 경유지 :' + (index + 1) +
            '   </h5> </span>' +
            '   <span class="coord"> <h5> 거리 : ' + dist + " m, 시간 : " + time + "s" +
            '   </h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '   ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '</div>';
    }

    el.innerHTML = itemStr;
    el.className = 'item';

    return el;
}
