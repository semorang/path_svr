// 토스트 기능 구현
function showToast(message, type = 'success') {
    if (!navigator.geolocation) {
        console.error('Geolocation API not supported');
        return;
    }

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerText = message;

    document.body.appendChild(toast);

    setTimeout(() => {
        toast.classList.add('visible');
    }, 100); // 등장 애니메이션

    setTimeout(() => {
        toast.classList.remove('visible');
        setTimeout(() => toast.remove(), 300); // 퇴장 후 제거
    }, 2000); // 표시 시간
}


// 클립보드 복사
function copyContents(value) {
    if (!navigator.clipboard) {
        console.error('Clipboard API not supported');
        return;
    }

    navigator.clipboard.writeText(value)
        .then(() => showToast(`복사됨: ${value}`, 'success'))
        .catch(err => showToast(`복사실패: ${err}`, 'error'));
}


function clickRoute(type, pos) {
    if (type == 1) {
        setDeparture(pos);
    } else if (type == 2) {
        setDestination(pos)
    }

    typeRouteStep = 9999; // 변경사항이므로
}


// 결과 항목을 Element로 반환하는 함수
function getSearchListItem(idx, name, subname, pos) {
    var el = document.createElement('li'),
    itemStr = `
        <div style="position:absolute;top:10px;left:2px;"><img id="poiicon" width="20" src="https://www.fivepin.co.kr/images/PoiMarker/marker_1_on.png"></div>
        <div class="info">
           <span class="title" onclick="copyContents('${name}')" style="display:block;margin-left:30px;margin-bottom:0;padding-top:1px;font-size:13px;cursor:pointer"> <h5> ${name} </h5> </span>
           <span class="addr" onclick="copyContents('${subname}')" style="display:block;top:20px;padding-left:5px;font-size:11px;margin:2px;cursor:pointer"> ${subname} </span>
           <span class="coord" onclick="copyContents('${pos.x.toFixed(6)},${pos.y.toFixed(6)}')" style="display:block;padding-left:5px;font-size:11px;margin:2px;cursor:pointer"> 좌표 : ${pos.x.toFixed(6)} ,${pos.y.toFixed(6)} </span>
        </div>
        <div class="button_container radius_border" style="right:10px;overflow:hidden;width:50px;height:60px;margin:0;z-index:1;border:1">
            <button class="route_button blue" style="" onclick="clickRoute(ClickTypeDeparture, {lng:${pos.x}, lat:${pos.y}})">출발</button>
            <button class="route_button red" style="top:30px" onclick="clickRoute(ClickTypeDestination, {lng:${pos.x}, lat:${pos.y}})">도착</button>
        </div>
        `;

    el.innerHTML = itemStr;
    el.className = 'poi_item';

    return el;
}


// 검색 결과 목록 표출 함수
function displaySearchList(search) {

    document.getElementById('menu_title').innerHTML = "검색 결과 목록";
    
    var listEl = document.getElementById('placesList'),
    menuEl = document.getElementById('menu_wrap'),
    fragment = document.createDocumentFragment(),
    listStr = '';


    // 기존 목록 삭제
    while (listEl.hasChildNodes()) {
        listEl.removeChild(listEl.lastChild)
    }

    resultDiv = document.getElementById('searched_type');
    resultDiv.innerHTML = (search.type == 0 ? '일반검색' : '연관검색');
    resultDiv = document.getElementById('searched_count');
    resultDiv.innerHTML = ', POI: ' + search.totalcount + "/" + search.count;

    if (search.poicount > 0) {
         // poi
        search.poi.forEach(function(item) {
            var coord = {x : parseFloat(item.dpx), y : parseFloat(item.dpy)};
            var position = new inavi.maps.LngLat(coord.x, coord.y);
            var itemEl = getSearchListItem(item.poiid, item.name1, item.roadname + item.roadjibun, coord);
            var title = item.name1;
            (function(position, title) {
                itemEl.onmouseover = function() {
                    // displayInfowindow(position, title);
                    // map.flyTo(position);
                };

                itemEl.onmouseout = function() {
                    // infoWindow.close();
                    // infoWindow.setVisible();
                };

                itemEl.onclick = function() {
                    // itemEl.className = 'selected_btn';

                    displaySearchInfowindow(position, title);
                }
            })(position, title);

            fragment.appendChild(itemEl);
        });
    } else if (search.admcount > 0) {
        // adm
        search.adm.forEach(function(item) {
            var coord = {x : parseFloat(item.posx), y : parseFloat(item.posy)};
            var position = new inavi.maps.LngLat(coord.x, coord.y);
            var itemEl = getSearchListItem(item.admcode, item.roadname + item.roadjibun, item.address + item.jibun, coord);
            var title = item.roadname + item.roadjibun;
            (function(position, title) {
                itemEl.onmouseover = function() {
                    // displayInfowindow(position, title);
                    // map.flyTo(position);
                };

                itemEl.onmouseout = function() {
                    // infoWindow.close();
                    // infoWindow.setVisible();
                };

                itemEl.onclick = function() {
                    // itemEl.className = 'selected_btn';

                    displaySearchInfowindow(position, title);
                }
            })(position, title);

            fragment.appendChild(itemEl);
        });
    }
    

    listEl.appendChild(fragment);
    menuEl.scrollTop = 0;
}


// 키워드 검색 
function getSearch(path, keyword) {
    var pathname = path;
    var search2url = pathname + '?&query=' + keyword.toString() + '&reqcount=' + 10;

    $.ajax({
        type: 'get',         // 타입 (get, post, put 등등)
        url: search2url,       // 요청할 서버 url
        // async: false,         // 비동기화 여부 (default : true)
        success: function(result) {                
            var header = result.header;
            if (header.isSuccessful == true && header.resultCode == 0) {      
                console.log("response, search data: " + result.search.totalcount)

                // set search result
                displaySearchList(result.search);
            } else {
                alert('err code: ' + header.resultCode + '\nerr msg: ' + header.resultMessage)
            }
        },
        error: function(request, status, error) {
            alert('통합 검색 요청 실패')
        },
        complete: function(xhr, status) {
        }
    });    
}


function parseWGS84Position(inputStr) {
    // 숫자만 추출
    const matches = inputStr.match(/-?\d+(\.\d+)?/g);
    if (!matches || matches.length < 2) {
        showToast("좌표 입력값이 부족하거나 잘못되었습니다.");
        console.warn("좌표 입력값이 부족하거나 잘못되었습니다.");
        return null;
    }

    // 숫자 배열로 변환
    const num1 = parseFloat(matches[0]);
    const num2 = parseFloat(matches[1]);

    // 경도(Lng)와 위도(Lat)의 범위 참고값
    const isLng = (val) => val >= 120 && val <= 135;
    const isLat = (val) => val >= 33 && val <= 43;

    // 자동 판단 및 교정
    if (isLng(num1) && isLat(num2)) {
        x = num1;
        y = num2;
    } else if (isLat(num1) && isLng(num2)) {
        x = num2;
        y = num1;
    } else {
        console.warn("좌표 값이 일반적인 WGS84 범위를 벗어났습니다.");
        return null;
    }

    var position = new inavi.maps.LngLat(x, y);
    return position; // WGS84 기준 좌표값 반환
}


// 위치 이동
function movotoPosition() {
    var coord = document.getElementById('coordinate').value;
    var position = parseWGS84Position(coord);
    if (position) {
        displaySearchInfowindow(position, position.lng.toString() + ',' + position.lat.toString());
        // 지도 이동
        // map.flyTo(position);
    } else {
        alert('정확한 좌표를 입력해주세요');
        return false;
    }
}
