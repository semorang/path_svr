const { createDiffieHellmanGroup } = require('crypto');
const util = require('util');
var addon = require(process.env.USER_MODULE);


const logout = require('./logs');
const auth = require('./auth');
const codes = require('./codes');


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


exports.init = function(pid, path, log) {

    logout("begin engine initialize");

    let isSetPid = false;
    if (isSetPid == false && pid !== undefined) {
        isSetPid = true;

        if (path !== undefined && log !== undefined && path.length > 0 && log.length > 0) {
            addon.init(pid, path, log);
        } else if (path !== undefined && path.length > 0) {
            addon.init(pid, path);
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
    const lng = req.query.lng;
    const lat = req.query.lat;
    let type = 0;
    let count = 0;
    
    let expand = 0;
    if (req.query.type != undefined) {
        type = parseInt(req.query.type)
    }
    if (req.query.count != undefined) {
        count = parseInt(req.query.count)
    }
    if (req.query.expand != undefined) {
        expand = parseInt(req.query.expand)
    }

    return addon.getoptimalposition(parseFloat(lng), parseFloat(lat), type, count, expand);
}


exports.optimalview = function(req) {
    const lng = req.query.lng;
    const lat = req.query.lat;
    let type = 0;
    let count = 0;
    let expand = 1; // 웹뷰에선 기본 사용
    if (req.query.type != undefined) {
        type = parseInt(req.query.type)
    }
    if (req.query.count != undefined) {
        count = parseInt(req.query.count)
    }
    if (req.query.expand != undefined) {
        expand = parseInt(req.query.expand)
    }

    return addon.getoptimalposition(parseFloat(lng), parseFloat(lat), type, count, expand);
}


exports.doroute = function(req, option) {
    addon.logout("start routing");

    var querystring = require('querystring');

    const id = req.query.id;
    const departure = req.query.start;
    const destination = req.query.end;
    const waypoint = req.query.via;
    const opt = (req.query.opt === undefined) ? 0 : req.query.opt;
    const is_sum = (option == 'summary') ? true : false;
    const is_view = (option == 'view') ? true : false;
    const target = (req.query.target === undefined) ? '' : req.query.target;

    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };
    
    var user_info = {
        id: id,
        option: opt
    };

    var route = {
        data: new Array,
    }

    var ret = {
        header: header,
        user_info: user_info,
        route: route,
    };


    if (departure == undefined ||
        destination == undefined) {
            logout("client request query not correct" + util.inspect(req.query, false, null, true));
            ret.header.isSuccessful = false;
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM;
            ret.header.resultMessage = codes.getErrMsg(ret.result_code);
            logout("route result : failed, msg " + ret.header.resultMessage);
            return ret;
        }

    // addon.setdeparture(126.92644, 37.57990);
    // addon.setdestination(126.94755,37.51116);

    // addon.logout("set departure");
    let coordStart = departure.split(',');
    if (coordStart.length != 2) {
        logout("start location query not correct" + util.inspect(coordStart, false, null, true));
        ret.header.isSuccessful = false;
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = codes.getErrMsg(ret.result_code);
        logout("route result : failed, msg " + ret.header.resultMessage);
        return ret;
    }
    addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), true);

    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.isSuccessful = false;
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = codes.getErrMsg(ret.result_code);
        logout("route result : failed, msg " + ret.header.resultMessage);
        return ret;
    }
    addon.setdestination(parseFloat(coordEnd[0]), parseFloat(coordEnd[1]), true);

    let coordVia;
    if (waypoint != undefined) {
        coordVia = waypoint.split(',');
        if (coordVia.length != 2) {
            logout("via location query not correct" + util.inspect(coordVia, false, null, true));
            ret.header.isSuccessful = false;
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = codes.getErrMsg(ret.result_code);
            logout("route result : failed, msg " + ret.header.resultMessage);
            return ret;
        }
        addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), true);
    }

    ret.user_info.start = { x: parseFloat(coordStart[0]), y: parseFloat(coordStart[1]) }
    ret.user_info.end =  { x: parseFloat(coordEnd[0]), y: parseFloat(coordEnd[1]) }
    if (coordVia != undefined) {
        ret.user_info.via = { x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) }
    }


    // addon.logout("do routing");
    var res = addon.doroute(parseInt(opt), 0);
    logout("find result : " + res.result + ", msg : " + res.msg);

    if (res.result == 0) {
        if (is_sum) {
            res = addon.getsummary();
        } else if (target === "inavi") {
            logout("call function getmultiroute_for_inavi");
            res = addon.getmultiroute_for_inavi();
            res = JSON.parse(res);
        } else if (target === "kakaovx") {
            res = addon.getroute(2); // 2:for kakaovx
        } else if (is_view) {
            res = addon.getview();
        } else {
            res = addon.getroute();
            res = JSON.parse(res);
        }
        
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

        if (is_sum) {
            ;
        } else if (target === "inavi") {
            ret.route.data.push(res.routes[0]);
            logout("route count : " + res.routes.length + ", paths : " + res.routes[0].paths.length);
        } else if (is_view) {
            ret.route.data.push(res.routes);
        } else {
            ret.route.data.push(res.routes);
        }

        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }
    else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        
        logout("routing failed : " + ret.header.resultCode + ", msg : " + ret.header.resultMessage);
    }

    addon.releaseroute();

    addon.logout("end routing");

    return ret;
}

exports.domultiroute = function(req, option) {
    addon.logout("start multi routing");

    var querystring = require('querystring');

    const id = req.query.id;
    const departure = req.query.start;
    const destination = req.query.end;
    const waypoint = req.query.via;
    const waypoints = req.query.vias;
    const opt = (req.query.opt === undefined) ? 0 : req.query.opt;
    const is_sum = (option == 'summary') ? true : false;
    const is_view = (option == 'view') ? true : false;
    const is_optimal = true; // 최적지점 사용
    const target = (req.query.target === undefined) ? '' : req.query.target;
    
    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };
    
    var user_info = {
        id: id,
        option: opt
    };

    var route = {
        data: new Array,
    }

    var summarys = {
        summarys: undefined,
    }

    var ret = {
        header: header,
        user_info: user_info,
        summarys: summarys,
        route: route,
    };

 

    if (departure == undefined || destination == undefined) {
        logout("client request query not correct" + util.inspect(req.query, false, null, true));
        ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        logout("route result : failed(" + ret.result_code + "), msg : " + ret.header.resultMessage);
        return ret;
    }

    // addon.setdeparture(126.92644, 37.57990);
    // addon.setdestination(126.94755,37.51116);

    // addon.logout("set departure");
    let coordStart = departure.split(',');
    if (coordStart.length != 2) {
        logout("start location query not correct" + util.inspect(coordStart, false, null, true));
        ret.header.result_code = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = codes.getErrMsg(ret.header.result_code);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), is_optimal);

    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.result_code = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = codes.getErrMsg(ret.header.result_code);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    addon.setdestination(parseFloat(coordEnd[0]), parseFloat(coordEnd[1]), is_optimal);


    ret.user_info.start = { x: parseFloat(coordStart[0]), y: parseFloat(coordStart[1]) }
    ret.user_info.end =  { x: parseFloat(coordEnd[0]), y: parseFloat(coordEnd[1]) }


    let coordVia;
    if (waypoints != undefined && Array.isArray(waypoints)) {
        ret.user_info.vias = new Array();
        waypoints.forEach(coord => {
            coordVia = coord.split(',');
            if (coordVia.length != 2) {
                logout("via location query not correct" + util.inspect(coordVia, false, null, true));
                ret.header.result_code = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
                ret.header.resultMessage = codes.getErrMsg(ret.header.result_code);
                logout("route result : failed, msg " + ret.msg);
                return ret;
            }
            addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal);

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
            ret.header.result_code = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = codes.getErrMsg(ret.header.result_code);
            logout("route result : failed, msg " + ret.msg);
            return ret;
        }
        addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal);

        if (coordVia != undefined) {
            ret.user_info.vias = new Array();
            ret.user_info.vias.push({ x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) });
        }
    } 


    // const route_cnt = 3;
    // let route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_RECOMMENDED, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // let route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_PALM, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    // 트레킹
    // const route_cnt = 2;
    // let route_opt = [2, 7]; // 보행자, 자전거
    // let route_void = [0, 0]; // bridge

    var route_cnt = 1;
    let route_opt = [parseInt(opt, 10)];
    let route_void = [0];
    
    // var route_cnt = 2;
    // var route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_TRAIL];
    // var route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];

    if (target === "kakaovx") {
        route_cnt = opt;
        if (route_cnt <= 0) {
            route_cnt = 1;
        } else if (route_cnt > 3) {
            route_cnt = 3;
        }
        route_opt = [codes.ROUTE_OPTIONS.ROUTE_OPT_SHORTEST, codes.ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE, codes.ROUTE_OPTIONS.ROUTE_OPT_COMFORTABLE];
        route_void = [codes.ROUTE_AVOIDS.ROUTE_OPT_NONE, codes.ROUTE_AVOIDS.ROUTE_OPT_PALM, codes.ROUTE_AVOIDS.ROUTE_OPT_BRIDGE];
    }

    logout("route info, cnt:" + route_cnt + ", opt:" + route_opt + ", avoid:" + route_void + ", target:" + target);

    for(ii=0; ii<route_cnt; ii++) {
        var res = addon.doroute(route_opt[ii], route_void[ii]);

        if (res.result == 0) {
            if (is_sum) {
                res = addon.getsummary();
            } else if (target === "inavi") {
                // logout("call function getmultiroute_for_inavi");
                res = addon.getmultiroute_for_inavi();
                res = JSON.parse(res);
            } else if (target === "kakaovx") {
                res = addon.getmultiroute(2); // 2:for kakaovx
            } else { // if (target === "kakaovx") {
                // logout("call function getmultiroute");
                res = addon.getmultiroute();
                res = JSON.parse(res);
            }

            if (res.result_code == 0) {
                ret.header.isSuccessful = true;
                ret.header.resultCode = res.result_code;
                ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    
                ret.summarys = res.summarys;
    
                if (is_sum) {
                    ;
                } else if (target === "inavi") {
                    ret.route.data.push(res.routes[0]);
                    logout("route("+ii+") count : " + res.routes.length + ", paths : " + res.routes[0].paths.length);
                } else { // if (target === "kakaovx") {
                    logout("route("+ii+") count : " + res.routes.length);
                    ret.route.data.push(res.routes[0]);
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
            ret.header.resultCode = res.result;
            ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
        }        
    } // for


    addon.releaseroute();

    logout("end routing, count: " + ret.route.data.length + ", result(" + ret.header.resultCode + '), ' + ret.header.resultMessage);

    return ret;
}

exports.gettable = function(mode, destinations) {

    addon.logout("start table");

    const count = destinations.length;
    
    const is_optimal = false;
    const type_match = 4; //TYPE_LINK_MATCH_FOR_TABLE; // 최적지점 사용

    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    var rows = {
        elements: new Array,
    };

    var ret = {
        header: header,
        origins: destinations,
        rows: rows,
    };


    var cntDestinations = 0;
    destinations.forEach(element => {
        let coord = element.split(',');
        if (coord.length != 2) {
            logout("request location query not correct" + util.inspect(coord, false, null, true));
            ret.header.isSuccessful = false;
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
            ret.header.resultMessage = codes.getErrMsg(ret.result_code);
            logout("route result : failed, msg " + ret.header.resultMessage);
        } else {
            var lng = parseFloat(coord[0]);
            var lat = parseFloat(coord[1]);
            if (checkcoord(lng, lat) != true) {
                logout("invalid coord, lng: " + lng + ", lat: " + lat);
            } else {
                // departure
                if (cntDestinations == 0) {
                    addon.setdeparture(lng, lat, is_optimal, type_match);
                }
                // destination 
                else if (cntDestinations == count - 1) {
                    addon.setdestination(lng, lat, is_optimal, type_match);
                }
                // via
                else {
                    addon.setwaypoint(lng, lat, is_optimal, type_match);
                }

                cntDestinations++;
            }
        }
    });


    var res = addon.gettable(cntDestinations);
    res = JSON.parse(res);

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

    addon.releaseroute();
    
    addon.logout("end table");

    return ret;
}


exports.setrpcost = function(key, mode, cost) {

    addon.logout("start set rp cost");

    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    var ret = {
        header: header,
    };

    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && cost !== undefined) {       
            logout("client user:'" + user + "', req:" + JSON.stringify(cost));
            
            var type = 4; //vehicle
            var count = cost.length;
            var res = addon.setrpcost(type, count, cost);
        
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

    addon.logout("end set rp cost");

    return ret;
}


exports.getcluster = function(target, destinations, clusters, file, mode) {

    addon.logout("start clustering");

    const cntPois = destinations.length;
    const cntClusters = clusters;
    
    const is_optimal = false;
    const type_match = 4; //TYPE_LINK_MATCH_FOR_TABLE; // 최적지점 사용

    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    var ret = {
        header: header,
        origins: destinations,
        clusters: clusters,
    };

    var cntDestinations = 0;
    destinations.forEach(element => {
        let coord = element.split(',');
        if (coord.length != 2) {
            logout("request location query not correct" + util.inspect(coord, false, null, true));
            ret.header.isSuccessful = false;
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
            ret.header.resultMessage = codes.getErrMsg(ret.result_code);
            logout("route result : failed, msg " + ret.header.resultMessage);
        } else {
            var lng = parseFloat(coord[0]);
            var lat = parseFloat(coord[1]);
            if (checkcoord(lng, lat) != true) {
                logout("invalid coord, lng: " + lng + ", lat: " + lat);
            } else {
                // departure
                if (cntDestinations == 0) {
                    addon.setdeparture(lng, lat, is_optimal, type_match);
                }
                // destination 
                else if (cntDestinations == (cntPois - 1)) {
                    addon.setdestination(lng, lat, is_optimal, type_match);
                }
                // via
                else {
                    addon.setwaypoint(lng, lat, is_optimal, type_match);
                }

                cntDestinations++;
            }
        }
    });

    var res = codes.ERROR_CODES.ROUTE_RESULT_FAILED;

    if (target === 'geoyoung') {
        res = addon.getcluster_for_geoyoung(cntClusters);
    }
    else if (file != undefined && mode != undefined && file.length > 0 && mode >= 1) {
        res = addon.getcluster(cntClusters, cntPois, file, mode);
    } else {
        res = addon.getcluster(cntClusters);
    }
    res = JSON.parse(res);

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

        ret.clusters = res.clusters;
    } else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);
    }

    addon.releaseroute();
    
    addon.logout("end clustering");

    return ret;
}


exports.getboundary = function(mode, destinations) {

    addon.logout("start boundary");

     var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    var boundary = {
        boundary: new Array,
    };

    var ret = {
        header: header,
        origins: destinations,
        boundary: boundary,
    };

    var cntDestinations = destinations.length;

    var res = addon.getboundary(cntDestinations, destinations);
    res = JSON.parse(res);

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
    
    addon.logout("end boundary");

    return ret;
}


exports.getbestways = function(mode, destinations) {

    addon.logout("start bestways");

    const count = destinations.length;
    
    const is_optimal = false;
    const type_match = 4; //TYPE_LINK_MATCH_FOR_TABLE; // 최적지점 사용

    var header = {
        isSuccessful: false,
        resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED,
        resultMessage: ""
    };

    var waypoints = {
        waypoints: new Array,
    };

    var ret = {
        header: header,
        origins: destinations,
        waypoints: waypoints,
    };

    var tspOpt = 0;
    if (mode !== 'tsp') {
        tspOpt = 1; // using original path
    }

    var cntDestinations = 0;
    destinations.forEach(element => {
        let coord = element.split(',');
        if (coord.length != 2) {
            logout("request location query not correct" + util.inspect(coord, false, null, true));
            ret.header.isSuccessful = false;
            ret.header.resultCode = codes.ERROR_CODES.ROUTE_RESULT_FAILED_SET_START;
            ret.header.resultMessage = codes.getErrMsg(ret.result_code);
            logout("route result : failed, msg " + ret.header.resultMessage);
        } else {
            var lng = parseFloat(coord[0]);
            var lat = parseFloat(coord[1]);
            if (checkcoord(lng, lat) != true) {
                logout("invalid coord, lng: " + lng + ", lat: " + lat);
            } else {
                // departure
                if (cntDestinations == 0) {
                    addon.setdeparture(lng, lat, is_optimal, type_match);
                }
                // destination 
                else if (cntDestinations == (count - 1)) {
                    addon.setdestination(lng, lat, is_optimal, type_match);
                }
                // via
                else {
                    addon.setwaypoint(lng, lat, is_optimal, type_match);
                }

                cntDestinations++;
            }
        }
    });


    var res = addon.getwaypoints(tspOpt, cntDestinations);
    res = JSON.parse(res);

    if (res.result_code == 0) {
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = codes.getErrMsg(ret.header.resultCode);

        ret.waypoints = res.waypoints;

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
    
    addon.logout("end bestways");

    return ret;
}
