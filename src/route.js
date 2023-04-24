const { createDiffieHellmanGroup } = require('crypto');
const os = require('os');
const util = require('util');
var addon = require(process.env.USER_MODULE);
if (os.platform() == 'linux') {
    // addon = require('../nodejs/core_modules/optimal_svr.node');
} else {
    // addon = require('../nodejs_win/build/Release/optimal_svr.node');
}


const logout = require('./logs');
const err_code = require('./errors');
const opt_code = require('./option.js');
// const { getMsg } = require("./errors");
// import getMsg from "./errors";


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
        resultCode: err_code.ROUTE_RESULT_FAILED,
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
            ret.header.resultCode = err_code.ROUTE_RESULT_FAILED_WRONG_PARAM;
            ret.header.resultMessage = err_code.getMsg(ret.result_code);
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
        ret.header.resultCode = err_code.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = err_code.getMsg(ret.result_code);
        logout("route result : failed, msg " + ret.header.resultMessage);
        return ret;
    }
    addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), true);

    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.isSuccessful = false;
        ret.header.resultCode = err_code.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = err_code.getMsg(ret.result_code);
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
            ret.header.resultCode = err_code.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = err_code.getMsg(ret.result_code);
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
        }
        
        ret.header.isSuccessful = true;
        ret.header.resultCode = res.result_code;
        ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);

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
        ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);
    }
    else {
        ret.header.isSuccessful = false;
        ret.header.resultCode = res.result;
        ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);
        
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
    const opt = (req.query.opt === undefined) ? 0 : req.query.opt;
    const is_sum = (option == 'summary') ? true : false;
    const is_view = (option == 'view') ? true : false;
    const is_optimal = false; // 최적지점 사용
    const target = (req.query.target === undefined) ? '' : req.query.target;
    
    var header = {
        isSuccessful: false,
        resultCode: err_code.ROUTE_RESULT_FAILED,
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

 

    if (departure == undefined || destination == undefined) {
        logout("client request query not correct" + util.inspect(req.query, false, null, true));
        ret.header.resultCode = err_code.ROUTE_RESULT_FAILED_WRONG_PARAM;
        ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);
        logout("route result : failed(" + ret.result_code + "), msg : " + ret.header.resultMessage);
        return ret;
    }

    // addon.setdeparture(126.92644, 37.57990);
    // addon.setdestination(126.94755,37.51116);

    // addon.logout("set departure");
    let coordStart = departure.split(',');
    if (coordStart.length != 2) {
        logout("start location query not correct" + util.inspect(coordStart, false, null, true));
        ret.header.result_code = err_code.ROUTE_RESULT_FAILED_SET_START;
        ret.header.resultMessage = err_code.getMsg(ret.header.result_code);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    addon.setdeparture(parseFloat(coordStart[0]), parseFloat(coordStart[1]), is_optimal);

    // addon.logout("set destination");
    let coordEnd = destination.split(',');
    if (coordEnd.length != 2) {
        logout("end location query not correct" + util.inspect(coordEnd, false, null, true));
        ret.header.result_code = err_code.ROUTE_RESULT_FAILED_SET_END;
        ret.header.resultMessage = err_code.getMsg(ret.header.result_code);
        logout("route result : failed, msg " + ret.msg);
        return ret;
    }
    addon.setdestination(parseFloat(coordEnd[0]), parseFloat(coordEnd[1]), is_optimal);

    let coordVia;
    if (waypoint != undefined) {
        coordVia = waypoint.split(',');
        if (coordVia.length != 2) {
            logout("via location query not correct" + util.inspect(coordVia, false, null, true));
            ret.header.result_code = err_code.ROUTE_RESULT_FAILED_SET_VIA;
            ret.header.resultMessage = err_code.getMsg(ret.header.result_code);
            logout("route result : failed, msg " + ret.msg);
            return ret;
        }
        addon.setwaypoint(parseFloat(coordVia[0]), parseFloat(coordVia[1]), is_optimal);
    } 

    ret.user_info.start = { x: parseFloat(coordStart[0]), y: parseFloat(coordStart[1]) }
    ret.user_info.end =  { x: parseFloat(coordEnd[0]), y: parseFloat(coordEnd[1]) }
    if (coordVia != undefined) {
        ret.user_info.via = { x: parseFloat(coordVia[0]), y: parseFloat(coordVia[1]) }
    }
    
    // addon.logout("do routing");
    logout("start routing : request, " + ret.user_info);


    // const route_cnt = 3;
    // let route_opt = [opt_code.ROUTE_OPT_SHORTEST, opt_code.ROUTE_OPT_RECOMMENDED, opt_code.ROUTE_OPT_TRAIL];
    // let route_void = [opt_code.ROUTE_AVOID_NONE, opt_code.ROUTE_AVOID_PALM, opt_code.ROUTE_AVOID_BRIDGE];

    // 트레킹
    // const route_cnt = 2;
    // let route_opt = [2, 7]; // 보행자, 자전거
    // let route_void = [0, 0]; // bridge

    const route_cnt = 1;
    let route_opt = [parseInt(opt, 10)];
    let route_void = [0];
    
    // var route_cnt = 2;
    // var route_opt = [opt_code.ROUTE_OPT_SHORTEST, opt_code.ROUTE_OPT_TRAIL];
    // var route_void = [opt_code.ROUTE_AVOID_NONE, opt_code.ROUTE_AVOID_BRIDGE];

    if (target === "kakaovx") {
        route_cnt = opt;
        if (route_cnt <= 0) {
            route_cnt = 1;
        } else if (route_cnt > 3) {
            route_cnt = 3;
        }
        route_opt = [opt_code.ROUTE_OPT_SHORTEST, opt_code.ROUTE_OPT_COMFORTABLE, opt_code.ROUTE_OPT_COMFORTABLE];
        route_void = [opt_code.ROUTE_AVOID_NONE, opt_code.ROUTE_AVOID_PALM, opt_code.ROUTE_AVOID_BRIDGE];
    }

    logout("total route info, cnt : " + route_cnt + ", opt : " + route_opt + ", avoid : " + route_void);

    for(ii=0; ii<route_cnt; ii++) {
        logout("start route, idx : " + ii);

        var res = addon.doroute(route_opt[ii], route_void[ii]);

        if (res.result == 0) {
            if (is_sum) {
                res = addon.getsummary();
            } else if (target === "inavi") {
                logout("call function getmultiroute_for_inavi");
                res = addon.getmultiroute_for_inavi();
                res = JSON.parse(res);
            // } else if (is_view) {
            //     res = addon.getview();
            } else if (target === "kakaovx") {
                res = addon.getmultiroute(2); // 2:for kakaovx
            } else { // if (target === "kakaovx") {
                logout("call function getmultiroute");
                res = addon.getmultiroute();
            }
            
            ret.header.isSuccessful = true;
            ret.header.resultCode = res.result_code;
            ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);

            if (is_sum) {
                ;
            } else if (target === "inavi") {
                ret.route.data.push(res.routes[0]);
                logout("route count : " + res.routes.length + ", paths : " + res.routes[0].paths.length);
            // } else if (is_view) {
            //     ret.route.data.push(res.routes);
            } else { // if (target === "kakaovx") {
                logout("route count : " + res.routes.length);
                ret.route.data.push(res.routes[0]);
            }
        }
        else {
            ret.header.isSuccessful = false;
            ret.header.resultCode = res.result;
            ret.header.resultMessage = err_code.getMsg(ret.header.resultCode);
        }        

        logout("end route");
    } // for


    addon.releaseroute();

    logout("total route count : " + ret.route.data.length);
    logout("end routing : result(" + ret.header.resultCode + '), ' + ret.header.resultMessage);

    return ret;
}
