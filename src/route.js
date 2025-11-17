const { createDiffieHellmanGroup } = require('crypto');
const util = require('util');
var addon = require(process.env.USER_MODULE);

const logout = require('./logs');
const auth = require('./auth');
const codes = require('./codes');
const times = require('../src/times.js')


function checkcoord(lng, lat, contury)
{
    var ret = false;

    if (lng == undefined || lat == undefined ||
        lng == NaN || lat == NaN) {
            ret = false;
        }
    else if (contury == undefined || contury == 'kor') {
        if ((124 <= lng) && (lng <= 131) &&
            (33 <= lat) && (lat <= 39))
            ret = true;
    } else { // default korea
        ret = false;
    }

    return ret;
} 


exports.init = function(pid, dataPath, filePath, logPath) {

    logout("begin engine initialize");

    let isSetPid = false;
    if (isSetPid == false && pid !== undefined) {
        isSetPid = true;

        if (dataPath !== undefined && logPath !== undefined && filePath !== undefined && dataPath.length > 0 && logPath.length > 0 && filePath.length > 0) {
            addon.init(pid, dataPath, filePath, logPath);
        } else if (dataPath !== undefined && filePath !== undefined && dataPath.length > 0 && filePath.length > 0) {
            addon.init(pid, dataPath, filePath);
        } else {
            addon.init(pid);
        }
    }

    logout("finish engine initialize");
}


exports.version = function() {
    return addon.getversion();
}


exports.optimalposition = function(req) {
    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let ret = {
        header: header,
    };

    let lng = req.query.lng;
    let lat = req.query.lat;
    let type = 0;
    let count = 0;
    let expand = 0;
    let option = 0;
    
    if (req.query.type == undefined) {
        // 택시승하차(건물/단지) - 차량출입구 순으로 선택
        // 2,3,1,0; 
        type = 0x00010302; // 바이트 거꾸로
    } else {
        type = parseInt(req.query.type)
    }

    if (req.query.count != undefined) {
        count = parseInt(req.query.count)
    }

    if (req.query.expand != undefined) {
        expand = parseInt(req.query.expand)
    }

    if (req.query.road != undefined) {
        option = parseInt(req.query.road);
    }

    if (lng == undefined || lat == undefined) {
        var inaviDef = codes.getDefaultPosition("inavi");
        if (inaviDef.center != undefined) {
            let centerCoord = inaviDef.center.split(',');
            if (centerCoord.length == 2) {
                lng = centerCoord[0];
                lat = centerCoord[1];
            }
        }
    }

    let result = addon.getoptimalposition(parseFloat(lng), parseFloat(lat), type, count, expand, option);
    let res = JSON.parse(result);

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getOptimalErrMsg(res.result_code);

        if (res.data != undefined) {
            ret.data = res.data;
        }

        if (res.expand != undefined) {
            ret.expand = res.expand;
        }
    }
    else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getOptimalErrMsg(res.result_code);
    } 

    return ret;
}


exports.multi_optimalposition = function(key, req) {
    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    // 사용자 키 확인
    let user = auth.checkAuth(key);
    if (user === null || user.length <= 0) {
        header.isSuccessful = false,
        header.resultCode = codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        header.resultMessage = codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    
        let ret = {
            header: header,
            user_info: req,
        };

        logout("key error: " + JSON.stringify(ret));
        return ret;
    }

    logout("user: " + user + ", req: " + JSON.stringify(req));

    let ret = {
        header: header,
    };

    let result = addon.getmultioptimalposition(JSON.stringify(req));
    let res = JSON.parse(result);

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getOptimalErrMsg(res.result_code);

        if (req.origins!== undefined) {
            ret.origins = req.origins;
        }

        if (res.datas !== undefined) {
            ret.datas = res.datas;
        }
    }
    else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getOptimalErrMsg(res.result_code);
    } 

    return ret;
}


// 단일 경로 옵션을 지원
exports.doroute = function(key, req, expend) {
    addon.logout("start routing");

    let querystring = require('querystring');

    // 사용자 키 확인
    let user = auth.checkAuth(key);
    if (user === null || user.length <= 0) {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };
    
        let ret = {
            header: header,
            user_info: req,
        };

        logout("key error: " + JSON.stringify(ret) + ", req: " + JSON.stringify(req));
        return ret;
    }

    logout("user: " + user + ", req: " + JSON.stringify(req));

    const id = req.id;
    let departure = req.start;
    let destination = req.end;
    const waypoint = req.via;
    const waypoints = req.vias;
    const option = (req.option === undefined) ? 0 : parseInt(req.option);
    const mobility = (req.mobility === undefined) ? 0 : parseInt(req.mobility);
    const is_sum = (expend !== undefined && expend == 'summary') ? true : false;
    const is_optimal = (req.optimal !== undefined && req.optimal != 'false') ? true : false; // 최적지점 사용, p2p는 무시
    const target = (req.target === undefined) ? '' : req.target;
    let typeMatch = (target === 'kakaovx') ? codes.LINK_MATCH_TYPE.TYPE_LINK_MATCH_FOR_FOREST : 0;
    const is_junction = (req.junction !== undefined && req.junction == 'true') ? true : false;
    let avoid = 0;
    let avoids = {};

    if (typeof req.avoid === 'number') { // 숫자일 경우    
        avoid = req.avoid;
    } else if (typeof req.avoid === 'string') { // 문자열일 경우 숫자로 변환 시도
        const parsed = parseInt(req.avoid, 10);
        if (!isNaN(parsed)) {
            avoid = parsed;
        } 
    } else if (typeof req.avoid === 'object' && req.avoid !== null) { // 객체일 경우
        avoids = req.avoid;
    }

    // header
    let ret_header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };
    
    // user info
    let ret_user_info = req;

    let ret = {
        header: ret_header,
        user_info: ret_user_info,
    };


    let tickStart = 0;
    let tickEnd = 0;
    if (target === 'p2p') {
        tickStart = logout("start p2p route tick-count");
    }


    // 출발지
    if (departure == undefined || destination == undefined) {
        logout("client request query not correct" + util.inspect(req, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed(" + ret.resultCode + "), msg : " + ret.header.resultMessage);
        return ret;
    }

    // addon.setdeparture(126.92644, 37.57990);
    // addon.setdestination(126.94755,37.51116);

    // addon.logout("set departure");
    let coordStart = departure.split(',');
    if (coordStart.length != 2) {
        logout("start location query not correct" + util.inspect(coordStart, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    // addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), is_optimal, typeMatch);

    // 도착지
    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    // addon.setdestination(parseFloat(coordEnd[0]), parseFloat(coordEnd[1]), is_optimal, typeMatch);


    ret.user_info.start = { x: parseFloat(coordStart[0]), y: parseFloat(coordStart[1]) }
    ret.user_info.end =  { x: parseFloat(coordEnd[0]), y: parseFloat(coordEnd[1]) }

    // 경유지
    let coordVia;
    if (waypoints != undefined && Array.isArray(waypoints)) {
        ret.user_info.vias = new Array();
        waypoints.forEach(coord => {
            coordVia = coord.split(',');
            if (coordVia.length != 2) {
                logout("via location query not correct" + util.inspect(coordVia, false, null, true));
                ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
                ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
                logout("route result : failed, msg " + ret.msg);
                return ret;
            }
            // addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal, typeMatch);

            if (coordVia != undefined) {
                ret.user_info.vias.push({ x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) });
            }
        });
    }
    else if ((waypoint != undefined) || (waypoints != undefined)) {
        if (waypoint != undefined) {
            coordVia = waypoint.split(',');
        } else {
            coordVia = waypoints.split(',');
        }
        
        if (coordVia.length != 2) {
            logout("via location query not correct" + util.inspect(coordVia, false, null, true));
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
            logout("route result : failed, msg " + ret.msg);
            return ret;
        }
        // addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal, typeMatch);

        if (coordVia != undefined) {
            ret.user_info.vias = new Array();
            ret.user_info.vias.push({ x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) });
        }
    } 


    // 경로 옵션/회피
    // const route_cnt = 3;
    // let route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // let route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_PALM, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    // 트레킹
    // const route_cnt = 2;
    // let route_opt = [2, 7]; // 보행자, 자전거
    // let route_void = [0, 0]; // bridge

    let route_cnt = 1;
    let route_opt = [option];
    let route_avoid = [avoid];
    let route_mobility = mobility;
    
    // var route_cnt = 2;
    // var route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // var route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    let route_avoids = avoid;
    if ((target === "p2p") && (avoids?.shortturn === 'true')) {
        route_avoids |= codes.ROUTE_AVOIDS_VEH.ROUTE_AVOID_SHORTTURN;
    }

    if ((target === "kakaovx") && (option <= 0 || option > 3)){
        route_cnt = 3;
        route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE];
        route_avoid = [route_avoids, route_avoids, route_avoids]; 
        // if (route_avoid == 0) {
        //     var route_avoids = // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_HIKING | 
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_TRAIL | 
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_BIKE | 
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_CROSS |
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_RECOMMEND | 
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_MTB | 
        //     codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_POPULAR; // 126
            
        //     route_avoid = [route_avoids, route_avoids, route_avoids];
        // } else {
        //     route_avoid = [avoid, avoid, avoid]; 
        // }
        addon.addrouteoption(route_opt[0], route_avoid[0], route_mobility);
        addon.addrouteoption(route_opt[1], route_avoid[1], route_mobility);
        addon.addrouteoption(route_opt[2], route_avoid[2], route_mobility);
    } else {
        route_avoid[0] = route_avoids;
        addon.addrouteoption(route_opt[0], route_avoid[0], route_mobility);
    }

    logout("route info, cnt:" + route_cnt + ", opt:" + route_opt + ", avoid:" + route_avoid + ", mobility: " + mobility + ", target:" + target);

    // 타겟별로 속성이 일부 변경됨.
    if (is_sum) {
        ;
    } else if ((target === "inavi") || (target === "p2p")) {
        // inavi maps api에서는 route 아래 data 필드를 또 둬서 route를 관리
        ret.route = new Object;
        ret.route.data = new Array;
    } else {
        // 그외는 routes로 통합 관리
        ret.routes = new Array;
    }

    if (mobility == codes.MOBILITY_TYPE.TYPE_MOBILITY_BICYCLE) { // BICYCLE
        typeMatch = codes.LINK_MATCH_TYPE.TYPE_LINK_MATCH_FOR_BICYCLE;
    }
    addon.setdeparture(ret.user_info.start.x, ret.user_info.start.y, is_optimal, typeMatch);
    addon.setdestination(ret.user_info.end.x, ret.user_info.end.y, is_optimal, typeMatch);
    if (ret.user_info.vias !== undefined) {
        ret.user_info.vias.forEach(via => {
            addon.setwaypoint(via.x, via.y, is_optimal, typeMatch);
        });
    }
    
    for(ii=0; ii<route_cnt; ii++) {
        let result = addon.doroute(route_opt[ii], route_avoid[ii], route_mobility);

        if (result.result == 0) {
            if (is_sum) {
                result = addon.getsummary();
            } else if ((target === "inavi") || (target === "p2p")) {
                result = addon.getmapsroute();
            } else {
                // logout("--------------route request junction, type: " + typeof(is_junction) + ", value : " + is_junction + ", Number: " + Number(is_junction) + ", Boolean: " + Boolean(is_junction));
                result = addon.getroute(is_junction);
            }
            let res = JSON.parse(result);

            if (res.result_code == 0) {
                ret.header.isSuccessful = true;
                ret.header.resultCode = res.result_code;
                ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    
                ret.summarys = res.summarys;
    
                if (is_sum) {
                    ;
                } else if ((target === "inavi") || (target === "p2p")) {
                    ret.route.data.push(res.routes[0]);
                    logout("route.data["+ii+"] count : " + res.routes.length + ", paths : " + res.routes[0].paths.length);
                } else { // if (target === "kakaovx") {
                    logout("routes["+ii+"] count : " + res.routes.length);
                    ret.routes.push(res.routes[0]);
                }
            }
            else {
                ret.header.isSuccessful = false;
                ret.header.resultCode = res.result_code;
                ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
            }    
        }
        else {
            ret.header.isSuccessful = false;
            ret.header.resultCode = result.result;
            ret.header.resultMessage = codes.getErrMsg(result.result);
        }

        addon.releaseroute();
    } // for

    if (target === 'p2p') {
        tickEnd = logout("end p2p route tick-count", tickStart);

        ret.result = new Object();
        ret.mode = req.mode ?? undefined;
        if (ret.header.isSuccessful == true) {
            ret.result.work_time = times.getPathWorkTime(tickEnd - tickStart, ret.route.data[0].distance);
        } else {
            ret.result.work_time = tickEnd - tickStart;
        }        
    }

    logout("end routing, count: " + route_cnt + ", result(" + ret.header.resultCode + '), ' + ret.header.resultMessage);

    return ret;
}


// 다중 경로 옵션을 지원
exports.domultiroute = function(key, req, expend) {
    addon.logout("start multi routing");

    let querystring = require('querystring');

    // 사용자 키 확인
    let user = auth.checkAuth(key);
    if (user === null || user.length <= 0) {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };
    
        let ret = {
            header: header,
            user_info: req,
        };

        logout("key error: " + JSON.stringify(ret) + ", req: " + JSON.stringify(req));
        return ret;
    }

    logout("user: " + user + ", req: " + JSON.stringify(req));

    const id = req.id;
    let departure = req.start;
    let destination = req.end;
    const waypoint = req.via;
    const waypoints = req.vias;
    const option = (req.option === undefined) ? 0 : parseInt(req.option);
    const mobility = (req.mobility === undefined) ? 0 : parseInt(req.mobility);
    const course_type = (req.course_type === undefined) ? 0 : parseInt(req.course_type);
    const course_id = ((req.course_id === undefined) || (req.course_id.length <= 0)) ? 0 : parseInt(req.course_id);
    const is_sum = (expend !== undefined && expend == 'summary') ? true : false;
    const is_optimal = (req.optimal !== undefined && req.optimal != 'false') ? true : false; // 최적지점 사용, p2p는 무시
    const target = (req.target === undefined) ? '' : req.target;
    let typeMatch = (target === 'kakaovx') ? 6 : 0;
    const is_junction = (req.junction !== undefined && req.junction == 'true') ? true : false;
    let avoid = 0;
    let avoids = {};

    if (typeof req.avoid === 'number') { // 숫자일 경우    
        avoid = req.avoid;
    } else if (typeof req.avoid === 'string') { // 문자열일 경우 숫자로 변환 시도
        const parsed = parseInt(req.avoid, 10);
        if (!isNaN(parsed)) {
            avoid = parsed;
        } 
    } else if (typeof req.avoid === 'object' && req.avoid !== null) { // 객체일 경우
        avoids = req.avoid;
    }

    // header
    let ret_header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };
    
    let ret_user_info = {
        id: id,
    };

    if (option >= 0) {
        ret_user_info.option = option;
    }

    let summarys = {
        summarys: undefined,
    }

    let ret = {
        header: ret_header,
        user_info: ret_user_info,
    };


    let tickStart = 0;
    let tickEnd = 0;
    if (target === 'p2p') {
        tickStart = logout("start p2p route tick-count");
    }

    // 출발지
    if (departure == undefined || destination == undefined) {
        logout("client request query not correct" + util.inspect(req, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed(" + ret.resultCode + "), msg : " + ret.header.resultMessage);
        return ret;
    }

    // addon.setdeparture(126.92644, 37.57990);
    // addon.setdestination(126.94755,37.51116);

    // addon.logout("set departure");
    let coordStart = departure.split(',');
    if (coordStart.length != 2) {
        logout("start location query not correct" + util.inspect(coordStart, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    // addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), is_optimal, typeMatch);

    // 도착지
    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    // addon.setdestination(parseFloat(coordEnd[0]), parseFloat(coordEnd[1]), is_optimal, typeMatch);


    ret.user_info.start = { x: parseFloat(coordStart[0]), y: parseFloat(coordStart[1]) }
    ret.user_info.end =  { x: parseFloat(coordEnd[0]), y: parseFloat(coordEnd[1]) }

    // 경유지
    let coordVia;
    if (waypoints != undefined && Array.isArray(waypoints)) {
        ret.user_info.vias = new Array();
        waypoints.forEach(coord => {
            coordVia = coord.split(',');
            if (coordVia.length != 2) {
                logout("via location query not correct" + util.inspect(coordVia, false, null, true));
                ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
                ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
                logout("route result : failed, msg " + ret.msg);
                return ret;
            }
            // addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal, typeMatch);

            if (coordVia != undefined) {
                ret.user_info.vias.push({ x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) });
            }
        });
    }
    else if ((waypoint != undefined) || (waypoints != undefined)) {
        if (waypoint != undefined) {
            coordVia = waypoint.split(',');
        } else {
            coordVia = waypoints.split(',');
        }
        
        if (coordVia.length != 2) {
            logout("via location query not correct" + util.inspect(coordVia, false, null, true));
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
            logout("route result : failed, msg " + ret.msg);
            return ret;
        }
        // addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal, typeMatch);

        if (coordVia != undefined) {
            ret.user_info.vias = new Array();
            ret.user_info.vias.push({ x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) });
        }
    } 


    // 경로 옵션/회피
    // const route_cnt = 3;
    // let route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // let route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_PALM, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    // 트레킹
    // const route_cnt = 2;
    // let route_opt = [2, 7]; // 보행자, 자전거
    // let route_void = [0, 0]; // bridge

    let route_cnt = 1;
    let route_opt = [option];
    let route_avoid = [avoid];
    let route_mobility = mobility;
    let route_subopt = 0;
    // var route_cnt = 2;
    // var route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // var route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    let route_avoids = avoid;
    if ((target === "p2p") && (avoids?.shortturn === 'true')) {
        route_avoids |= codes.ROUTE_AVOIDS_VEH.ROUTE_AVOID_SHORTTURN;
    }

    if ((target === "kakaovx") && (option <= 0 || option > 3)) {
        route_cnt = 3;
        route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE];
        if (route_avoid == codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_NONE) {
            route_avoids = codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_NONE;
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_HIKING | 
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_TRAIL | 
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_BIKE | 
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_CROSS |
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_RECOMMEND | 
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_MTB | 
            // codes.ROUTE_AVOIDS_TRK.ROUTE_AVOID_POPULAR; // 126
            
            route_avoid = [route_avoids, route_avoids, route_avoids];
        } else {
            route_avoid = [avoid, avoid, avoid]; 
        }

        if (course_id != 0 ) {
            route_cnt = 1; // 코스 탐색은 단일 탐색으로 진행
        } else {
            if (course_type == 3) // 코스ID없는 자전거탐색일 경우, 아이나비의 자전거 경로탐색 사용
                route_mobility = 2; // 자전거
        }
    } else if (req.multiroute === 'true') {
        route_cnt = 3;
        route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE];
        route_avoid = [route_avoids, route_avoids, route_avoids];
    } else {
        route_avoid[0] = route_avoids;
    }

    logout("multiroute info, cnt:" + route_cnt + ", opt:" + route_opt + ", avoid:" + route_avoid + ", mobility: " + route_mobility + ", subopt: " + route_subopt + ", target:" + target);

    // 타겟별로 속성이 일부 변경됨.
    if (is_sum) {
        ;
    } else if ((target === "inavi") || (target === "p2p")) {
        // inavi maps api에서는 route 아래 data 필드를 또 둬서 route를 관리
        ret.route = new Object;
        ret.route.data = new Array;
    } else {
        // 그외는 routes로 통합 관리
        ret.routes = new Array;
    }

    if (route_cnt > 1) {
        addon.addrouteoption(route_opt[0], route_avoid[0], route_mobility);
        addon.addrouteoption(route_opt[1], route_avoid[1], route_mobility);
        addon.addrouteoption(route_opt[2], route_avoid[2], route_mobility);
    } else {
        addon.addrouteoption(route_opt[0], route_avoid[0], route_mobility);
    }
    addon.setroutesuboption(course_type, course_id);


    addon.setdeparture(ret.user_info.start.x, ret.user_info.start.y, is_optimal, typeMatch);
    addon.setdestination(ret.user_info.end.x, ret.user_info.end.y, is_optimal, typeMatch);
    if (ret.user_info.vias !== undefined) {
        ret.user_info.vias.forEach(via => {
            addon.setwaypoint(via.x, via.y, is_optimal, typeMatch);
        });
    }

    if (target === 'p2p') {
        const candidate = (req.candidate === undefined) ? 0 : parseInt(req.candidate);
        if (candidate !== 0) {
            addon.setcandidateoption(candidate);
        }
    }

    let result = addon.domultiroute(route_cnt);

    if (result.result == 0) {
        if (is_sum) {
            result = addon.getsummary();
        } else if ((target === "inavi") || (target === "p2p")) {
            // res = addon.getmapsroute();
            result = addon.getmapsmultiroute();
        } else {
            // logout("--------------route request junction, type: " + typeof(is_junction) + ", value : " + is_junction + ", Number: " + Number(is_junction) + ", Boolean: " + Boolean(is_junction));
            result = addon.getmultiroute(is_junction);
        }
        let res = JSON.parse(result);

        if (res.result_code == 0) {
            ret.header.isSuccessful = true;
            ret.header.resultCode = res.result_code;
            ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

            if (is_sum) {
                ;
            } else if ((target === "inavi") || (target === "p2p")) {
                ret.route.data = res.routes;
            } else { // if (target === "kakaovx") {
                ret.routes = res.routes;
            }
        }
        else {
            ret.header.isSuccessful = false;
            ret.header.resultCode = res.result_code;
            ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        }
    }
    else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = result.result;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    addon.releaseroute();    


    if (target === 'p2p') {
        tickEnd = logout("end p2p route tick-count", tickStart);

        ret.result = new Object();
        ret.mode = req.mode;
        if (ret.header.isSuccessful == true) {
            ret.result.work_time = times.getPathWorkTime(tickEnd - tickStart, ret.route.data[0].distance);
        } else {
            ret.result.work_time = tickEnd - tickStart;
        }        
    }

    logout("end routing, count: " + route_cnt + ", result(" + ret.header.resultCode + '), ' + ret.header.resultMessage);

    return ret;
}


// distance matrix
exports.gettable = function(req) {

    addon.logout("start distance matrix");

    let result = addon.gettable(JSON.stringify(req));
    let res = JSON.parse(result);

    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let ret = {
        header: header,
        mode: (req.mode != undefined) ? req.mode : "driving",
        origins: req.origins,
        destinations: req.destinations,
        rows: null,
    };

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        ret.rows = res.rows;
    } else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    addon.logout("end distance matrix");

    return ret;
}


// 경탐 코스트 설정
exports.setdatacost = function(key, mode, base, cost) {

    addon.logout("start set data cost");

    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let ret = {
        header: header,
    };

    let user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && cost !== undefined) {       
            logout("client user:'" + user + "', mode:'" + mode + "', base:" + base + ", req:" + JSON.stringify(cost));
            
            var type = 5; // default = TYPE_DATA_VEHICLE
            if (mode === 'vehicle') { 
                type = 5; // vehicle = TYPE_DATA_VEHICLE
            } else if (mode === 'forest') {
                type = 2; // forest = TYPE_DATA_TREKKING
            } else if (mode === 'optimal') {
                type = 7; // optimal = TYPE_DATA_ENTRANCE
            }else {
                type = 4;
            }
            var devide = base;
            if (devide == undefined) {
                devide = 0;
            }
            var count = cost.length;
            var res = addon.setdatacost(type, devide, count, cost);
        
            if (res.result_code == 0) {
                ret.header.isSuccessful = true;
            }

            ret.header.resultCode = res.result_code;
            ret.header.resultMessage = res.result_message;
        }
    } else {
        ret.header.resultCode = codes.ERROR_CODES.RESULT_APPKEY_ERROR;
        ret.header.resultMessage = codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR);
    } 

    addon.logout("end set data cost");

    return ret;
}


// 클러스터링
exports.getcluster = function(req) {
    let result = codes.ERROR_CODES.ROUTE_RESULT_FAILED;

    if (req.tsp == undefined) {
        var tsp = {
            seed: 10000,
            algorithm: 0,
            compare_type: 1,
        };
        req.tsp = tsp;
    }

    if (req.clust == undefined) {
        var clust = {
            seed: 10006,
            algorithm: 3,
            compare_type : 1,
        };
        req.clust = clust;
    }

    if ((req.target === undefined) && (req.target === 'geoyoung')) {
        result = addon.getcluster_for_geoyoung(JSON.stringify(req));
    } else {
        result = addon.getcluster(JSON.stringify(req));
    }

    let res = JSON.parse(result);

    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let ret = {
        header: header,
        mode: (req.mode != undefined) ? req.mode : "clustering",
        origins: (req.origins != undefined) ? req.origins : null,
        summary: null,
        clusters: null,
    };

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        ret.summary = res.summary;
        ret.clusters = res.clusters;
    } else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    return ret;
}


exports.getboundary = function(mode, in_target, destinations) {
    const target = (in_target === undefined) ? '' : req.query.target;

    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let boundary = {
        boundary: new Array,
    };

    let ret = {
        header: header,
        origins: destinations,
        boundary: boundary,
    };

    let cntDestinations = destinations.length;

    let result = addon.getboundary(cntDestinations, destinations);
    let res = JSON.parse(result);

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

        ret.boundary = res.boundary;
    } else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    return ret;
}


// 클러스터링 영역
exports.getbestways = function(req) {
    let result = addon.getwaypoints(JSON.stringify(req));
    let res = JSON.parse(result);

    let header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    let ret = {
        header: header,
        mode: (req.mode != undefined) ? req.mode : "tsp",
        origins: (req.origins != undefined) ? req.origins : null,
        summary: null,
        waypoints: null,
    };

    // let type_match = 4; //TYPE_LINK_MATCH_FOR_TABLE; // 최적지점 사용
    // if (target === "samsung_heavy") {
    //     type_match = 0; // TYPE_LINK_MATCH_NONE
    // }

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

        ret.summary = res.symmary;
        ret.waypoints = res.waypoints;
        ret.routes = res.routes ?? undefined;
        
        // add web route view url
        if (res.url != undefined && res.url.length > 0) {
            ret.url = res.url;
        }
    } else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    addon.releaseroute();

    return ret;
}

exports.updatetraffic = function(file, path, timestamp) {
    addon.updatetraffic(file, path, timestamp);
}