// 시간 스트링
function dpTimeToString(seconds) {
    var strTime = "";
    if (seconds >= 60 * 60) {
        strTime = Math.floor(seconds / (60 * 60)) + "시간 ";
    }

    if (seconds >= 60) {
        strTime += Math.floor((seconds % (60 * 60)) / 60) + "분 "
    }
    else if (seconds > 0) {
        strTime += ((seconds % (60 * 60)) % 60).toFixed(0) + "초"
    }

    return strTime;
}

// 거리 스트링
function dpDistanceToString(meter) {
    var strDist = "";
    if (meter >= 1000) {
        strDist = (meter / 1000).toFixed(2) + "Km ";
    }
    else if (meter >= 0) {
        strDist = (meter % 1000).toFixed(0) + "m "
    }

    return strDist;
}
