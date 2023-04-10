// 입구점 결과 목록 표출 함수
function displayEntranceList(result_dat, result_exp) {

    document.getElementById('menu_title').innerHTML = "입구점 목록";

    var listEl = document.getElementById('placesList'),
    menuEl = document.getElementById('menu_wrap'),
    fragment = document.createDocumentFragment(),
    listStr = '';


    // 기존 목록 삭제
    while (listEl.hasChildNodes()) {
        listEl.removeChild(listEl.lastChild)
    }


    for (var ii=0; ii<result_dat.count; ii++) {
        var position = new inavi.maps.LngLat(result_dat.entrypoints[ii].x, result_dat.entrypoints[ii].y);
        var itemEl = getEntranceListItem(ii, result_exp.type, result_dat.entrypoints[ii]);
        var title;
        if (result_exp.type == 2 && result_dat.entrypoints[ii].type <= 0) {
            title = getPolygonType(3); // 단지내 도로
        } else {
            title = getEntranceType(result_dat.entrypoints[ii].type);
        }

        (function(position, title) {
            itemEl.onmouseover = function() {
                displayEntranceInfowindow(position, title);
            };

            itemEl.onmouseout = function() {
                displayEntranceInfowindow();
            };
        })(position, title);

        fragment.appendChild(itemEl);
    }

    listEl.appendChild(fragment);
    menuEl.scrollTop = 0;
}

// 결과 항목을 Element로 반환하는 함수
function getEntranceListItem(index, type, entrance) {
    var el = document.createElement('li'),
        itemStr = '<div class="circle_' + type + '" background:"blue"> <span>' + index + '</span></div>' +
            '<div class="info">' +
            '   <span class="type"> <h5> 입구점 속성 :' +
            '       ' + getEntranceType(entrance.type) + '</h5> </span>' +
            '   <span class="coord"> 좌표 : ' +
            '       ' + entrance.y.toFixed(6) + ', ' + entrance.x.toFixed(6) + ' </span>' +
            '</div>';

    el.innerHTML = itemStr;
    el.className = 'item';

    return el;
}


function getPolygonType(type) {
    var typeName;

    switch(type) {
        case 1:
            typeName = "건물";
            break;
        case 2:
            typeName = "단지(건물군)";
            break;
        case 3:
            typeName = "단지내도로";
            break;
        case 4:
            typeName = "최근접도로";
            break;

        default:
            typeName = "미지정";
            break;
    } // swtich

    return typeName;
}

function getEntranceType(type) {
    var typeName;

    switch(type) {
        case 1:
            typeName = "차량입구점";
            break;
        case 2:
            typeName = "택시승하자점(빌딩)";
            break;
        case 3:
            typeName = "택시승하차점(단지)";
            break;
        case 4:
            typeName = "택배차량하차점";
            break;
        case 5:
            typeName = "보행자입구점";
            break;
        case 6:
            typeName = "배달하차점(차량/오토바이)";
            break;
        case 7:
            typeName = "배달하차점(자전거/도보)";
            break;

        default:
            typeName = "도로";
            break;
    } // swtich

    return typeName;
}