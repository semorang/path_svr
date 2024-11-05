// 결과 항목을 Element로 반환하는 함수
function getSearchListItem(idx, name, subname, pos) {
    var el = document.createElement('li'),
    itemStr = '<div style="position:absolute;top:20px;left:2px;"><img id="poiicon" width="20" src="https://www.fivepin.co.kr/images/PoiMarker/marker_1_on.png"></div>' +
        '<div class="info">' +
        '   <span class="type"> <h5> ' + '       ' + name + '</h5> </span>' +
        '   <span class="addr" style="font-size:11px; margin:2px 0px"> ' + '       ' + subname + '</span>' +
        '   <span class="coord" style="font-size:11px"> 좌표 : '     + '       ' + pos.y.toFixed(6) + ', ' + pos.x.toFixed(6) + ' </span>' +
        '</div>';

    el.innerHTML = itemStr;
    el.className = 'item';

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

    resultDiv = document.getElementById('clicked_type');
    resultDiv.innerHTML = (search.type == 0 ? '일반검색' : '연관검색') + ', POI: ' + search.totalcount + "/" + search.count;

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
            if (header.isSuccessful == true) {      
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