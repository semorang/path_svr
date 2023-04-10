

// 그려진 선의 총거리 정보와 거리에 대한 도보, 자전거 시간을 계산하여
// HTML Content를 만들어 리턴하는 함수입니다
function getTimeHTML(distance) {

    // 도보의 시속은 평균 4km/h 이고 도보의 분속은 67m/min입니다
    var dspKm = distance / 1000 | 0;
    var dspMeter = distance % 1000;
    var dspTime = distance / 67 | 0;
    var dspHour = '', dspMin = '';

    // 계산한 거리가 1km 이상이면 km로 표시합니다
    if (dspKm > 1) {
        dspKm = '<span class="number">' + dspKm + '</span>Km '
    }
    dspMeter = '<span class="number">' + dspMeter + '</span>m'

    // 계산한 도보 시간이 60분 보다 크면 시간으로 표시합니다
    if (dspTime > 60) {
        dspHour = '<span class="number">' + Math.floor(dspTime / 60) + '</span>시간 '
    }
    dspMin = '<span class="number">' + dspTime % 60 + '</span>분'

    // 자전거의 평균 시속은 16km/h 이고 이것을 기준으로 자전거의 분속은 267m/min입니다
    var bycicleTime = distance / 227 | 0;
    var bycicleHour = '', bycicleMin = '';

    // 계산한 자전거 시간이 60분 보다 크면 시간으로 표출합니다
    if (bycicleTime > 60) {
        bycicleHour = '<span class="number">' + Math.floor(bycicleTime / 60) + '</span>시간 '
    }
    bycicleMin = '<span class="number">' + bycicleTime % 60 + '</span>분'

    // 거리와 도보 시간, 자전거 시간을 가지고 HTML Content를 만들어 리턴합니다
    var content = '<ul class="dotOverlay distanceInfo">';
    content += '    <li>';
    content += '        <span class="label">총거리</span>' + dspKm + dspMeter;
    content += '    </li>';
    content += '    <li>';
    content += '        <span class="label">도보</span>' + dspHour + dspMin;
    content += '    </li>';
    content += '    <li>';
    content += '        <span class="label">자전거</span>' + bycicleHour + bycicleMin;
    content += '    </li>';
    content += '</ul>'

    return content;
}


function getResultHtmlContent(distance, time) {

    var dspKm = distance / 1000 | 0;
    var dspHour = '약', dspMin = '';

    // 계산한 거리가 1km 이상이면 km로 표시합니다
    if (dspKm > 1) {
        dspDist = '<span class="number">' + dspKm + '.' +  Math.floor(distance % 1000) + '</span>Km '
    } else {
        dspDist = '<span class="number">' + Math.floor(distance % 1000) + '</span>m'
    }

    // 계산한 시간이 1시간을 초과하면 시간으로 표시합니다
    if (time > 3600) {
        dspHour += '<span class="number">' + Math.floor(time / 3600) + '</span>시간 '
    }
    // 1분 미만은 1분으로 표기
    else if (time < 60) {
        time = 60;
    }
    dspMin = '<span class="number">' + Math.floor(time % 3600 / 60) + '</span>분'
    

    // 거리와 시간, 시간을 가지고 HTML Content를 만들어 리턴
    var content = '<ul class="dotOverlay distanceInfo">';
    content += '    <li>';
    content += '        <span class="label">총거리</span>' + dspDist;
    content += '    </li>';
    content += '    <li>';
    content += '        <span class="label">예상시간</span>' + dspHour + ' ' + dspMin;
    content += '    </li>';
    content += '</ul>'

    return content;
}