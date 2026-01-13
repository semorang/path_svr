const os = require('os');
const http = require('http');
const path = require('path');
const express = require('express');
const request_ip = require('request-ip');
const cors = require('cors'); // CORS 오류 해소
const timeout = require('connect-timeout');
const url = require('url');
const app = express();
const fs = require('fs');
const fsextra = require('fs-extra');

const cfg = require('dotenv').config();
// const addon = require('./build/Release/trekking_svr.node');
// const addon = require('./core_modules/walk_route.node');
const route = require('../src/route');
const tms = require('../src/tms');
const auth = require('../src/auth');
const logout = require('../src/logs');
const times = require('../src/times.js')
// const escapeJSON = require('escape-json-noide');
// const addon = require('bindings')('openAPI')

const apikey = require('../views/script/key.js');

const publicPath = path.join(__dirname, '../public') // web에서 공유할 path

const { isContext } = require('vm');

let corsOptions = {
    origin: '*',
    credential: true,
}

// app.set('views', __dirname + '/openApi/views');
app.set('view engine', 'ejs');
app.engine('html', require('ejs').renderFile);
app.use(express.static(publicPath)); 
app.use(express.json());
app.use(cors(corsOptions));

app.get('/', function(req, res) {
    logout("client IP : " + request_ip.getClientIp(req));
    res.send("this is trekking route server.")
});


app.get('/test', function(req, res) {
    logout("start test route request");

    //set test value
    // req.query.id = 12399999;
    // req.query.start = '126.50680,33.24975';
    // req.query.end = '126.54808,33.23984';

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'summary');

    if (ret.result_code == 0) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end test route request");
});


app.get('/version', function(req, res) {
    logout("request version info");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.version();

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('버전 확인 실패 : ' + ret.msg);
    } else {
        ret.server = {
            ip : cur_ip.address(),
            port : cur_port,
            pid : cur_pid,
            start : start_time,
            alive : times.getElapsedTime(start_time.getTime()),
        }
    }

    res.send(ret);
});


app.post('/api/setdatacost', function(req, res) {
    logout('start set data cost');

    const key = req.headers.authorization;
    const body = req.body;

    const ret = route.setdatacost(key, body);

    res.send(ret);

    logout('end set data cost');
});


app.get('/summary', function(req, res) {
    const startTime = logout("start route summary request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'summary');

    if (ret.result_code == 0) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
        res.send(ret);
    }

    logout("end route summary request", startTime);
});


// app.get('/route', timeout('10s'), function(req, res) {
app.get('/api/route', function(req, res) {
    const startTime = logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.mobility = "3"; // 기본 자동차 모드

    const api = req.query.api;
    let key;
    if (req.query.userkey !== undefined) {
        key = req.query.userkey;
        req.query.userkey = undefined;
    }

    const ret = route.doroute(key, req.query);

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end route request", startTime);
});


app.get('/api/multiroute', function(req, res) {
    const startTime = logout("start multiroute request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.mobility = "3"; // 기본 자동차 모드

    const key = req.query.userkey;
    var ret = route.domultiroute(key, req.query, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end multiroute request", startTime);
});


// 2p2 path 전용
app.get('/path', function(req, res) {
    res.redirect(url.format({
        pathname: "/api/path",
        query: req.query
    }));
});


app.get('/api/path', function(req, res) {
    const startTime = logout("start multiroute request");

    // 2p2 path 전용 옵션
    req.query.target = "p2p";
    req.query.mobility = "4"; // 기본 자율주행 자동차 모드
    if (req.query.option == undefined) {
        req.query.option = "4"; // 기본 큰길 옵션 
    }

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = "2Y41Z0H-7QS4Z5X-JZJBHJQ-95BSGD6";
    var ret = route.domultiroute(key, req.query, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end multiroute request", startTime);
});


app.get('/api/kakaovx', function(req, res) {
    const startTime = logout("start kakaovx route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";

    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";
    var ret = route.domultiroute(key, req.query, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end kakaovx route request", startTime);
});


app.get('/view/kakaovx', function(req, res) {
    const startTime = logout("start kakaovx request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";
    const api = req.query.api;
    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";

    var ret = route.domultiroute(key, req.query, "view");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_kakaovx", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else if (req.query.target === "kakaovx") {
            res.render(__dirname + "/../views/inavi_maps_kakaovx", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else if (req.query.target === 'inavi') {
            res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else if (req.query.target === 'p2p') {
            res.render(__dirname + "/../views/inavi_maps_multipath", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_multiroute_ex", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end kakaovx request", startTime);
});


app.get('/view/route', function(req, res) {
    const startTime = logout("begin route view request, ip: " + request_ip.getClientIp(req));
 
    req.query.mobility = "3"; // 기본 자동차 모드

    const api = req.query.api;
    let key;
    if (req.query.userkey !== undefined) {
        key = req.query.userkey;
        req.query.userkey = undefined;
    }

    const ret = route.doroute(key, req.query);

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_route", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end route view, result: " + ret.header.isSuccessful, startTime);
});


app.get('/view/multiroute', function(req, res) {
    const startTime = logout("start multiroute(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.mobility = "3"; // 기본 자동차 모드

    const api = req.query.api;
    let key;
    if (req.query.userkey !== undefined) {
        key = req.query.userkey;
        req.query.userkey = undefined;
    }

    var ret = route.domultiroute(key, req.query, "view");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_kakaovx", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else if (req.query.target === "kakaovx") {
            res.render(__dirname + "/../views/inavi_maps_kakaovx", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else if (req.query.target === 'inavi') {
            res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else if (req.query.target === 'p2p') {
            res.render(__dirname + "/../views/inavi_maps_multipath", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_multiroute_ex", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end multiroute(view) request", startTime);
});


app.get('/view/waypoints', function(req, res) {
    const startTime = logout("start waypoints request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.mobility = "3"; // 기본 자동차 모드
    
    const api = req.query.api;
    const key = null;
    
    var ret = route.domultiroute(key, req.query, "view");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            // res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
            res.render(__dirname + "/../views/inavi_maps_tsp", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end waypoints request", startTime);
});


app.get('/view/route_result', function(req, res) {
    const startTime = logout("start route result");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("file name : " + req.query.file);

    if (req.query.file != undefined) {
        //load json file
        var jsonBuff = fsextra.readFileSync(req.query.file, 'utf8');
        var jsonReq = JSON.parse(jsonBuff);
        logout("client req : " + JSON.stringify(jsonReq));

        const target = req.query.target;

        var header = {
            isSuccessful: false,
            resultCode: 1,
            resultMessage: ""
        };
        
        var user_info = {
            id: 12345,
            option: 2
        };

        var route = {
            data: new Array,
        }
    
        var ret = {
            header: header,
            user_info: user_info,
            summarys: undefined,
            route: route,
        };

        if (jsonReq.result_code == 0) {
            ret.header.isSuccessful = true;
            ret.header.resultCode = jsonReq.result_code;
            ret.header.resultMessage = jsonReq.error_msg;

            if (jsonReq.summarys.length > 0) {
                ret.summarys = jsonReq.summarys;
            }

            if ((target === "inavi") || (target === "p2p")) {
                ret.route.data.push(jsonReq.routes[0]);
                logout("route(0) count : " + jsonReq.routes.length + ", paths : " + jsonReq.routes[0].paths.length);
            } else { // if (target === "kakaovx") {
                logout("route(0) count : " + jsonReq.routes.length);
                ret.route.data.push(jsonReq.routes[0]);
                ret.user_info = jsonReq.user_info;
            }


            if (target === "kakao") {
                res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
            } else {
                // res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
                res.render(__dirname + "/../views/inavi_maps_tsp", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
            }
        } else {
            res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        }
    }


    logout("end route result", startTime);
});


// p2p path result view
app.get('/view/path', function(req, res) {
    const startTime = logout("start pathview request");

    // 2p2 path 전용 옵션
    req.query.target = "p2p";
    req.query.mobility = "4"; // 기본 자율주행 자동차 모드
    if (req.query.option == undefined) {
        req.query.option = "4"; // 기본 큰길 옵션 
    }

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const api = req.query.api;
    const key = "716Y8EX-0MWMZ0Y-JQ1PTWF-QZHEMZK";
    
    var ret = route.domultiroute(key, req.query, "view");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_path", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('PATH 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end pathview request", startTime);
});


app.get('/api/optimalposition', function(req, res) {
    const startTime = logout("start optimalposition request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    let type = "0";
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,1,0,0; 
        type = "0x00000102"; // 바이트 거꾸로
    } else {
        type = req.query.type;
    }

    // if (req.query.road != undefined && parseInt(req.query.road, 10) != 0) {
    //     // 근처 도로를 무조건 추가 => 4th에 90
    //     // type |= 0x90000000; // 
    // }

    if (((type.length >= 8) && (type.charAt(0) === "0")) || (type.indexOf("0x") == 0)) {
        req.query.type = parseInt(type, 16).toString();
    } else {
        req.query.type = type;
    }

    var ret = route.optimalposition(req);

    logout("result : " + ((ret.header.resultCode == 0) ? "success" : "failed") + ", code : " + ret.header.resultCode);
    if (ret.header.resultCode != 0) {
        logout('최적지점 검색 실패 : ' + ret.header.resultMessage);
    }

    res.send(ret);

    logout("end optimalposition request", startTime);
});


app.get('/view/optimalposition', function(req, res) {
    const startTime = logout("start optimalview request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    let type = "0";
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,1,0,0; 
        type = "0x00000102"; // 바이트 거꾸로
    } else {
        type = req.query.type;
    }

    // if (req.query.road != undefined && parseInt(req.query.road, 10) != 0) {
    //     // 근처 도로를 무조건 추가 => 4th에 90
    //     // type |= 0x90000000; // 
    // }

    if (((type.length >= 8) && (type.charAt(0) === "0")) || (type.indexOf("0x") == 0)) {
        req.query.type = parseInt(type, 16).toString();
    } else {
        req.query.type = type;
    }

    if (req.query.expand == undefined) {
        req.query.expand = (1).toString();
    }

    const api = req.query.api;

    var ret = route.optimalposition(req);

    if (ret.header.resultCode == 0) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_position", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_position", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    }

    logout("end optimalview request", startTime);
});


// app.get('/view/clustering/appkeys/:userkey', function(req, res) {
app.get('/view/clustering', function(req, res) {
    const startTime = logout("start clustering(view) request");

    const filePath = process.env.DATA_PATH + "/usr/request_cluster.json"
    const reqBuffer = fs.readFileSync(filePath)
    const reqJSON = reqBuffer.toString()

    const ret = tms.clustering(req.query.userkey, JSON.parse(reqJSON));

    if (ret.header.resultCode == 0) {
        res.render(__dirname + "/../views/inavi_maps_multiroute_ex", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
    } else {
        res.send(ret);
    }

    logout("end clustering request by GET", startTime);
});


app.get('/api/distancematrix/appkeys/:userkey', function(req, res) {
    const startTime = logout("start distance matrix request by GET");

    // distance matrix api 호출
    const ret = tms.distancematrix(req.params.userkey, req);

    res.send(ret);

    logout("end distance matrix request by GET", startTime);
});


app.post('/api/distancematrix', function(req, res) {
    const startTime = logout("start distance matrix request");

    // distance matrix api 호출
    const ret = tms.distancematrix(req.headers.authorization, req.body);

    res.send(ret);

    logout("end distance matrix request", startTime);
});


// clustering api 호출
app.get('/api/clustering/appkeys/:userkey', function(req, res) {
    const startTime = logout("start clustering request");

    const ret = tms.clustering(req.params.userkey, req);

    res.send(ret);

    logout("end clustering request by GET", startTime);
});


app.post('/api/clustering', function(req, res) {
    const startTime = logout("start clustering request" + ": " + req);

    ret = tms.clustering(req.headers.authorization, req.body);

    res.send(ret);

    logout("end clustering request", startTime);
});


// clustering boundary api 호출
app.get('/api/boundary/appkeys/:userkey', function(req, res) {
    logout("start boundary request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const target = req.query.target;
    const destinations = req.query.origins;

    const ret = tms.boundary(key, mode, target, destinations);

    res.send(ret);

    logout("end boundary request by GET");
});


app.post('/api/boundary', function(req, res) {
    logout("start boundary request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const target = req.body.target;
    const destinations = req.body.origins;

    const ret = tms.boundary(key, mode, target, destinations);

    res.send(ret);

    logout("end boundary request");
});


app.get('/api/bestwaypoints/appkeys/:userkey', function(req, res) {
    const startTime = logout("start bestwaypoints request");

    const ret = tms.bestwaypoints(req.params.userkey, req.query);

    res.send(ret);

    logout("end bestwaypoints request by GET", startTime);
});


app.post('/api/bestwaypoints', function(req, res) {
    const startTime = logout("start bestwaypoints request" + ": " + req);

    const ret = tms.bestwaypoints(req.headers.authorization, req.body);

    res.send(ret);

    logout("end bestwaypoints request", startTime);
});


app.get('/view/bestwaypoints', function(req, res) {
    const startTime = logout("start bestwaypoints(view) request");

    const filePath = process.env.DATA_PATH + "/usr/request_bestwaypoint.json"
    const reqBuffer = fs.readFileSync(filePath)
    const reqJSON = reqBuffer.toString()

    const ret = tms.bestwaypoints(req.query.userkey, JSON.parse(reqJSON));

    if (ret.header.resultCode == 0) {
        res.render(__dirname + "/../views/inavi_maps_multiroute_ex", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
    } else {
        res.send(ret);
    }

    logout("end bestwaypoints request by GET", startTime);
});


app.get('/api/createkey', function(req, res) {
    logout("create key request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // 필요시 uuid api key 생성
    logout("created new key :" + JSON.stringify(auth.createKey()));

    res.send('success, created key, ask to check your key to the administrator');
});

const cur_ip = require("ip");
const cur_port = (process.env.VEH_SVR_PORT === undefined) ? 20301 : process.env.VEH_SVR_PORT;
const cur_pid = process.pid;
const start_time = new Date();
const backlog_cnt = 100; // 대기열 크기 설정, 기본값:120
const req_timeout = 20000; // 요청(대기열포함) 타임아웃: 20초, 기본값:2분(120000)
const keepalive_timeout = 100000; // 연결 유지 타임아웃: 100초, 기본값:5초(5000)

// traffic 처리 로직
const time_gap = 10 * 1000; // n초 간격으로 새로운 교통정보 파일 확인
const traffic = require('../src/traffic.js')
const trafficPath = process.env.DATA_PATH + "/traffic";
const trafficManager = new traffic(trafficPath);
trafficManager.on('newFile', ({ fileName, filePath, timeStamp }) => {
    console.log(`traffic watcher received new file: ${fileName}, ${filePath}, ${timeStamp}`);

    route.updatetraffic(fileName, filePath, timeStamp);
});

//setInterval(() => trafficManager.checkFolder(), time_gap);


const server = app.listen(cur_port, function () {
    let data_path = process.env.DATA_PATH;
    let file_path = process.env.FILE_PATH;
    let log_path = process.env.LOG_PATH;

    // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
    logout("start vehicle route server response at " + cur_ip.address() + ":" + cur_port);
    logout("OS : " + os.type());
    logout("process pid:" + cur_pid);

    route.init(cur_pid, data_path, file_path, log_path);

    logout("finished server initialize");

    // addon.logout("start walk routing server addr "  + cur_ip.address() + ":" + cur_port + " on " + os.type());
});

server.timeout = req_timeout;
server.keepAliveTimeout = keepalive_timeout;


// http.createServer(app).listen(cur_port, '0.0.0.0', function () { //http://133.186.212.20:9095/
//     console.info('Listening for HTTP on', this.address());

//     var cur_ip = require("ip");
//     var cur_date = new Date();
//     // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
//     logout("[" + cur_date + "] start tekking route server response at " + cur_ip.address() + ":" + cur_port);
//     logout("[" + cur_date + "] OS : " + os.type());

//     addon.logout("start routing request");

//     addon.init();
// });
