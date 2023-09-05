// 시간 스트링
function displayTimeToString(seconds) {
    var strTime = "";
    if (seconds >= 60 * 60) {
        strTime = (seconds / (60 * 60)).toFixed(0) + "시간 ";
    }

    if (seconds >= 60) {
        strTime += ((seconds % (60 * 60)) / 60).toFixed(0) + "분 "
    }
    else if (seconds > 0) {
        strTime += ((seconds % (60 * 60)) % 60).toFixed(0) + "초"
    }

    return strTime;
}

// 거리 스트링
function displayDistanceToString(meter) {
    var strDist = "";
    if (meter >= 1000) {
        strDist = (meter / 1000).toFixed(2) + "Km ";
    }
    else if (meter >= 0) {
        strDist = (meter % 1000).toFixed(0) + "m "
    }

    return strDist;
}


// 입구점 결과 목록 표출 함수
function displayWaypointList(user, summarys, routes) {

    let start = user.start;
    let end = user.end;
    let vias = user.vias;
    let cntVias = 0;

    document.getElementById('menu_title').innerHTML = "방문지 목록";
    document.getElementById('clicked_type').innerHTML = "방문지 갯수: " + summarys.length;
    document.getElementById('clicked_name').innerHTML = "운행거리: " + displayDistanceToString(routes.summary.distance)  + ", 운행시간: " + displayTimeToString(routes.summary.time);
    var listEl = document.getElementById('placesList'),
    menuEl = document.getElementById('menu_wrap'),
    fragment = document.createDocumentFragment(),
    listStr = '';


    // 기존 목록 삭제
    while (listEl.hasChildNodes()) {
        listEl.removeChild(listEl.lastChild)
    }

    // 경유지 인덱스 확인
    var viaIndex = new Array();
    for (ii=0; ii < routes.link_info.length; ii++) {
        if (routes.link_info[ii].guide_type != 0) {
            viaIndex.push(ii);
        }
    }
    viaIndex.push(routes.link_info.length - 1);

    // 출발지
    var position = new inavi.maps.LngLat(start.x, start.y);
    // var rdist = (routes.link_info[viaIndex[0]] != undefined) ? routes.link_info[viaIndex[0]].remain_distance : -1;
    // var rtime = (routes.link_info[viaIndex[0]] != undefined) ? routes.link_info[viaIndex[0]].remain_time : -1;
    var rdist = summarys[0].remain_distance;
    var rtime = summarys[0].remain_time;
    var itemEl = getWaypointListItem(0, 'start', start, routes.summary.distance, routes.summary.time, routes.summary.now);
    var title = '출발지';

    (function(position, title) {
        itemEl.onmouseover = function() {
            displayWaypointInfowindow(position, title);
        };

        // itemEl.onmouseout = function() {
        //     displayWaypointInfowindow();
        // };

        itemEl.onclick = function(event) {
            if (event.target.className != 'custom_typecontrol radius_border') {
                // 경유지 순서 변경 인덱스 설정
                // document.getElementById('via_selected').innerHTML = 0;
                // document.getElementById('via_moved').innerHTML = 0;

                // 지도 이동
                map.flyTo(position);
            }
        }
    })(position, title);
    fragment.appendChild(itemEl);

    // 경유지
    if (vias != undefined && vias.length >= 1) {
        for (var ii=0; ii<vias.length; ii++) {
            position = new inavi.maps.LngLat(vias[ii].x, vias[ii].y);
            // rdist = (routes.link_info[viaIndex[ii+1]+1] != undefined) ? routes.link_info[viaIndex[ii+1]+1].remain_distance : -1;// - routes.link_info[viaIndex[ii + 1]].remain_distance;
            // rtime = (routes.link_info[viaIndex[ii+1]+1] != undefined) ? routes.link_info[viaIndex[ii+1]+1].remain_time : -1;// - routes.link_info[viaIndex[ii + 1]].remain_time;
            var rdist = summarys[ii+1].remain_distance;
            var rtime = summarys[ii+1].remain_time;
            itemEl = getWaypointListItem(ii, 'via', vias[ii], summarys[ii].distance, summarys[ii].time, summarys[ii].eta);
            title = '방문지: ' + (ii + 1);

            (function(position, title, idx) {
                itemEl.onmouseover = function() {
                    displayWaypointInfowindow(position, title);
                };

                // itemEl.onmouseout = function() {
                //     displayWaypointInfowindow();
                // };

                itemEl.onclick = function(event) {
                    if (event.target.className != 'custom_typecontrol radius_border') {
                        // 경유지 순서 변경 인덱스 설정
                        // document.getElementById('via_selected').innerHTML = idx;
                        // document.getElementById('via_moved').innerHTML = idx;

                        // 선택 경로 강조
                        selectRoute(idx);

                        // 지도 이동
                        map.flyTo(position);
                    }
                }
            })(position, title, ii + 1);
            fragment.appendChild(itemEl);
        }
    } // for

    // 목적지
    position = new inavi.maps.LngLat(end.x, end.y);
    // var itemEl = getWaypointListItem(0, 'end', end); // tsp 에서는 도착지를 마지막 경유지로 사용
    if (routes.link_info[viaIndex[vias.length - 1]] != undefined) {
        itemEl = getWaypointListItem(ii, 'last', end, summarys[vias.length].distance, summarys[vias.length].time, summarys[vias.length].eta);
    } else {
        itemEl = getWaypointListItem(vias.length, 'via', end, -1, -1, routes.summary.eta);
    }

    // var title = '목적지';
    title = '방문지: ' + (vias.length + 1);

    (function(position, title) {
        itemEl.onmouseover = function() {
            displayWaypointInfowindow(position, title);
        };

        // itemEl.onmouseout = function() {
        //     displayWaypointInfowindow();
        // };

        itemEl.onclick = function(event) {
            if (event.target.className != 'custom_typecontrol radius_border') {
                // 경유지 순서 변경 인덱스 설정
                // document.getElementById('via_selected').innerHTML = vias.length + 1;
                // document.getElementById('via_moved').innerHTML = vias.length + 1;

                // 선택 경로 강조
                selectRoute(vias.length + 1);

                // 지도 이동
                map.flyTo(position);
            }
        }
    })(position, title);
    fragment.appendChild(itemEl);

    // 등록
    listEl.appendChild(fragment);
    menuEl.scrollTop = 0;
}

// 결과 항목을 Element로 반환하는 함수
function getWaypointListItem(index, type, entrance, dist, time, eta) {
    var el;
    if (type === 'start') {
        el = document.createElement('li'),
        itemStr = '<div class="circle_start" background:"blue"> <span>' + 'S' + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 출발지' +
            '   </h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '       ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '   <hr> <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 총 거리 : ' + displayDistanceToString(dist) + ", 총 시간 : " + displayTimeToString(time) +
            '   </h5> </span>' +
            '   <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 출발 시각 : ' + eta +
            '   </h5> </span>' +
            '</div>';
    } else if (type === 'end') {
        el = document.createElement('li'),
        itemStr = '<div class="circle_end" background:"blue"> <span>' + 'E' + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5"> 목적지' +
            '   </h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '       ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '   <hr> <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 이동거리 : ' + displayDistanceToString(dist) + ", 시간 : " + displayTimeToString(time) +
            '   </h5> </span>' +
            '   <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 도착 예정 시각 : ' + eta +
            '   </h5> </span>' +
            '</div>';
    } else if (type === 'last') {
        el = document.createElement('li'),
        itemStr = '<div class="circle_via" background:"blue"> <span>' + (index + 1) + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 방문지 :' + (index + 1) +
            '   </h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '   ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '   <hr> <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 이동거리 : ' + displayDistanceToString(dist) + ", 시간 : " + displayTimeToString(time) +
            '   </h5> </span>' +
            '   <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 도착 예정 시각 : ' + eta +
            '   </h5> </span>' +
            '</div>';
    } else {
        el = document.createElement('li'),
        itemStr = '<div class="circle_via" background:"blue"> <span>' + (index + 1) + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 방문지 :' + (index + 1) +
            '   </h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '   ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '   <hr> <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 이동거리 : ' + displayDistanceToString(dist) + ", 시간 : " + displayTimeToString(time) +
            '   </h5> </span>' +
            '   <span class="coord"> <h5 style="margin-top:10px;margin-bottom:0"> 도착 예정 시각 : ' + eta +
            '   </h5> </span>' +
            '   <div class="custom_typecontrol radius_border" style="position:absolute;top:5px;right:5px;width:15px;height:15px;margin:0;z-index:1;text-align:center"' +
            '       <span id=' + (index+1).toString() + ' class="btn" onclick="setViaDelete(this.id, tmp_user_obj)"">X</span>' +
            '   </div>' + 
            '   <div class="custom_typecontrol radius_border" style="position:absolute;top:26px;right:5px;width:15px;height:15px;margin:0;z-index:1;text-align:center"' +
            '       <span id=' + (index+1).toString() + ' class="btn" onclick="setViaUp(this.id, tmp_user_obj)"">↑</span>' +
            '   </div>' + 
            '   <div class="custom_typecontrol radius_border" style="position:absolute;top:45px;right:5px;width:15px;height:15px;margin:0;z-index:1;text-align:center"' +
            '       <span id=' + (index+1).toString() + ' class="btn" onclick="setViaDown(this.id, tmp_user_obj)"">↓</span>' +
            '   </div>' +
            '</div>';
    }

    el.innerHTML = itemStr;
    el.className = 'item';

    return el;
}

// 방문지 삭제
function setViaDelete(id, data) {
    var now = parseInt(id);
    if (id == undefined || now <= 1 || data.vias.length < now) {
        return;
    }
}

// 방문지 위로
function setViaUp(id, data) {
    var now = parseInt(id);
    if (id == undefined || now <= 1 || data.vias.length < now) {
        return;
    }

    var next = now - 1;

    // document.getElementById("via_moved").innerHTML = next;

    var tmpValue = data.vias[now - 1];
    data.vias[now - 1] = data.vias[next - 1];
    data.vias[next - 1] = tmpValue;
}

// 방문지 아래로
function setViaDown(id, data) {
    var now = parseInt(id);
    if (id == undefined || now < 1 || now >= data.vias.length) {
        return;
    }

    var next = now + 1;

    // document.getElementById("via_moved").innerHTML = next;

    var tmpValue = data.vias[now - 1];
    data.vias[now - 1] = data.vias[next - 1];
    data.vias[next- 1] = tmpValue;
}

// 방문지 변경 확정
function setViaOk(data) {
    var url = "http://localhost:20301/view/waypoints"
    url += "?id=" + data.id;
    url += "&opt=" + data.option;

    // start
    url += "&start=" + data.start.x + ',' + data.start.y;

    // via
    for (var ii=0; ii<data.vias.length; ii++) {
        url += "&vias=" + data.vias[ii].x + ',' + data.vias[ii].y;
    }

    // end
    url += "&end=" + data.end.x + ',' + data.end.y;
    // http://localhost:20301/view/waypoints?id=202302091420&opt=2&start=127.010768,37.448979&end=126.998570,37.499104&vias=126.986509,37.476323&vias=126.986684,37.476873&vias=126.987496,37.476307&vias=126.981109,37.485389&vias=126.981534,37.486489&vias=126.982747,37.485089&vias=126.985046,37.485497&vias=126.983834,37.485897&vias=126.982609,37.486822&vias=126.982884,37.487055&vias=126.982571,37.487513&vias=126.982596,37.488030&vias=126.982784,37.488230&vias=126.982784,37.489088&vias=126.983071,37.493913&vias=126.986083,37.493971&vias=126.984246,37.496954&vias=126.985283,37.496446&vias=126.989946,37.493021&vias=126.989671,37.492638&vias=126.991171,37.491113&vias=126.992633,37.488855&vias=126.994820,37.487780&vias=126.993070,37.489097&vias=126.991821,37.490997&vias=126.990433,37.493363&vias=126.990008,37.493905&vias=126.989483,37.494804&vias=126.989808,37.493263&vias=126.990633,37.493013&vias=126.985796,37.492588&vias=126.979947,37.488230&vias=126.981909,37.488180&vias=126.980572,37.487597&vias=126.981872,37.487480&vias=126.983059,37.485847&vias=126.980684,37.488130&vias=126.981934,37.488063&vias=126.980984,37.487980&vias=126.995495,37.484356&vias=126.997483,37.481073&vias=126.994945,37.485131&vias=126.995695,37.483914&vias=126.995758,37.483939&vias=126.994821,37.482498&vias=126.993971,37.482406&vias=126.996433,37.482698&vias=126.996920,37.482123&vias=126.996995,37.481856&vias=126.999420,37.478173&vias=127.000320,37.477873&vias=127.000133,37.478073&vias=126.998158,37.481064&vias=127.003420,37.482789&vias=126.997358,37.482298&vias=126.996433,37.483822&vias=126.998170,37.497738&vias=126.998532,37.498746&vias=126.999182,37.498337

    // location.href = url;
}

// 방문지 변경 취소
function setViaCancel(data) {
    window.close();
}