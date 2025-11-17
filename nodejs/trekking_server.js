const os = require('os');
const http = require('http');
const path = require('path');
const express = require('express');
const request_ip = require('request-ip');
const cors = require('cors'); // CORS 오류 해소
const timeout = require('connect-timeout');
const url = require('url');
const app = express();

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

    const key = null;
    var ret = route.doroute(key, req, 'summary');

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
    const mode = req.body.mode;
    const base = req.body.base;
    const cost = req.body.cost;

    const ret = route.setdatacost(key, mode, base, cost);

    res.send(ret);

    logout('end set data cost');
});


app.get('/summary', function(req, res) {
    logout("start route summary request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = null;
    var ret = route.doroute(key, req, 'summary');

    if (ret.result_code == 0) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
        res.send(ret);
    }

    logout("end route summary request");
});


// app.get('/route', timeout('10s'), function(req, res) {
app.get('/route', function(req, res) {
    let startTime = logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = null;
    var ret = route.doroute(key, req);

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
    let startTime = logout("start multiroute request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = null;
    var ret = route.domultiroute(key, req, "");

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
    logout("start multiroute request");

    // 2p2 path 전용 옵션
    req.query.target = "p2p";
    req.query.option = 8;

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = null;
    var ret = route.domultiroute(key, req, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end multiroute request");
});


app.get('/api/kakaovx', function(req, res) {
    let startTime = logout("start kakaovx request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";
    req.query.optimal = "false";
    req.query.junction = "true"; // 숲길은 MM이 필요해 junction 사용
    if (req.query.course_type === undefined) {
        req.query.course_type = "1"; // 기본 등산로 탐색
    }
    
    // req.query.type = "0"; // 코스 타입 //0:미정의, 1:등산, 2:걷기, 3:자전거, 4:코스
    // req.query.course = "0"; // 코스 ID

    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";
    var ret = route.domultiroute(key, req.query, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end kakaovx request", startTime);
});


app.get('/view/kakaovx', function(req, res) {
    let startTime = logout("start kakaovx(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";
    req.query.optimal = "false";
    req.query.junction = "true"; // 숲길은 MM이 필요해 junction 사용
    if (req.query.course_type === undefined) {
        req.query.course_type = "1"; // 기본 등산로 탐색
    }
    if (req.query.mode === undefined) {
        req.query.mode = "forest";
    }

    // req.query.type = "0"; // 코스 타입 //0:미정의, 1:등산, 2:걷기, 3:자전거, 4:코스
    // req.query.course = "0"; // 코스 ID

    const api = req.query.api;
    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";
    var ret = route.domultiroute(key, req.query, "");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_kakaovx", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else if (req.query.target === "kakaovx") {
            res.render(__dirname + "/../views/inavi_maps_kakaovx", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end kakaovx(view) request", startTime);
});


app.get('/view/route', function(req, res) {
    let startTime = logout("start route(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const key = null;
    req.query.mobility = "0"; // 기본 걷기 모드

    var ret = route.doroute(key, req, 'view');

    if (ret.result_code == 0) {

        // var kakao_key;// = '9def7442ad15b4775780f565ab8cd9c4';

        // var json = JSON.stringify(ret.route.coords);
        // var vtx = json.stringify(jsonObj)
        // var escJson = escapeJSON(json);
        // var vtx = JSON.parse(json);
        // res.render(__dirname + "/openApi/views/ejs.html", {kakao_key:kakao_key});
        // res.render(__dirname + "/openApi/views/kakao_map", {javascriptkey:kakao_key, result:escJson});
        // res.render(__dirname + "/openApi/views/ejs", {javascriptkey:kakao_key, result:vtx});
        // logout(JSON.stringify(ret));
        
        //res.render(__dirname + "/../views/kakao_map", {javascriptkey:kakao_key, result:ret});
        res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
    } else {
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end route(view) request", startTime);
});


app.get('/view/multiroute', function(req, res) {
    let startTime = logout("start multiroute(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const api = req.query.api;
    const key = null;
    var ret = route.domultiroute(key, req, "view");

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
    logout("start waypoints request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const api = req.query.api;
    const key = null;
    var ret = route.domultiroute(key, req, "view");

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

    logout("end waypoints request");
});


app.get('/view/route_result', function(req, res) {
    logout("start route result");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("file name : " + req.query.file);

    if (req.query.file != undefined) {
        //load json file
        var jsonBuff = fs.readFileSync(req.query.file, 'utf8');
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


    logout("end route result");
});


// p2p path result view
app.get('/view/path', function(req, res) {
    logout("start pathview request");

    // 2p2 path 전용 옵션
    req.query.target = "p2p";
    req.query.option = 8;
    req.query.optimal = "false";

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const api = req.query.api;
    const key = null;
    var ret = route.domultiroute(key, req, "view");

    if (ret.header.isSuccessful == true) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_path", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('PATH 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end pathview request");
});


app.get('/api/optimalposition', function(req, res) {
    let startTime = logout("start optimalposition request");

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
    let startTime = logout("start optimal(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    let type = "0";
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,1,0,0; 
        type = "00000102"; // 바이트 거꾸로
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

    logout("end optimal(view) request", startTime);
});


app.get('/api/distancematrix/appkeys/:userkey', function(req, res) {
    logout("start distance matrix request by GET");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const target = req.query.target;
    const destinations = req.query.origins;

    // distance matrix api 호출
    const ret = tms.distancematrix(key, mode, target, destinations);

    res.send(ret);

    logout("end distance matrix request by GET");
});


app.post('/api/distancematrix', function(req, res) {
    logout("start distance matrix request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const target = req.body.target;
    const destinations = req.body.origins;

    // distance matrix api 호출
    const ret = tms.distancematrix(key, mode, target, destinations);

    res.send(ret);

    logout("end distance matrix request");
});


// clustering api 호출
app.get('/api/clustering/appkeys/:userkey', function(req, res) {
    logout("start clustering request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const target = req.query.target;
    const destinations = req.query.origins;
    const count = req.query.count;
    const file = req.query.file;

    const ret = tms.clustering(key, mode, target, destinations, count, file);

    res.send(ret);

    logout("end clustering request by GET");
});


app.post('/api/clustering', function(req, res) {
    logout("start clustering request" + ": " + req);

    const key = req.headers.authorization;
    // const mode = req.body.mode;
    const target = req.body.target;
    const destinations = req.body.origins;
    const clusters = req.body.count;
    const file = req.body.file;

    var mode = 0;
    var ret;

    const reqDir = process.env.DATA_PATH + "/user";
    // const reqFile = "result_clustering.json";
    const reqFile = "result_table.bin";
    const filePath = reqDir + "/" + reqFile;

    if (file != undefined && file.indexOf("read") >= 0) {
        mode = 1;
    }

    if (file != undefined && file.indexOf("write") >= 0) {
        mode = (mode == 1) ? 3 : 2;
    }


    // 테이블 결과 파일이 있으면 우선 사용
    //load json file
    // if (file != undefined && file.indexOf("read") >= 0) {
    //     var exist = fs.existsSync(filePath);
    //     if (exist) {
    //         var jsonBuff = fs.readFileSync(filePath, 'utf8');
    //         if (jsonBuff != undefined) {
    //             ret = JSON.parse(jsonBuff);
    //         }
    //     }
    // }

    // if (ret != undefined && file != undefined && file.indexOf("read") >= 0) {
    //     logout("clustering file contents : " + JSON.stringify(ret));
    // } else {
    //     ret = tms.clustering(key, destinations, count, file, mode);

    //     // 테이블 결과를 파일로 저장
    //     if ((ret.header.isSuccessful == true) &&
    //         (file != undefined && file.indexOf("write") >= 0)) {
    //             if (!fs.existsSync(reqDir)) {
    //                 fs.mkdirSync(reqDir, '0777', true);
    //             }
    //             fs.writeFileSync(filePath, JSON.stringify(ret));
    //     }
    // }
    ret = tms.clustering(key, target, destinations, clusters, filePath, mode);

    res.send(ret);

    logout("end clustering request");
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
    logout("start bestwaypoints request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const target = req.query.target;
    const destinations = req.query.origins;

    // distance matrix api 호출
    const ret = tms.bestwaypoints(key, mode, target, destinations);

    res.send(ret);

    logout("end bestwaypoints request by GET");
});


app.post('/api/bestwaypoints', function(req, res) {
    logout("start bestwaypoints request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const target = req.body.target;
    const destinations = req.body.origins;

    // distance matrix api 호출
    const ret = tms.bestwaypoints(key, mode, target, destinations);

    res.send(ret);

    logout("end bestwaypoints request");
});


app.get('/api/createkey', function(req, res) {
    logout("create key request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // 필요시 uuid api key 생성
    logout("created new key :" + JSON.stringify(auth.createkey()));

    res.send('success, created key, ask to check your key to the administrator');
});

const cur_ip = require("ip");
const cur_port = (process.env.TRK_SVR_PORT === undefined) ? 20301 : process.env.TRK_SVR_PORT;
const cur_pid = process.pid;
const start_time = new Date();
const backlog_cnt = 100; // 대기열 크기 설정, 기본값:120
const req_timeout = 60000; // 요청(대기열포함) 타임아웃: 1분(60초), 기본값:2분(120000)
const keepalive_timeout = 100000; // 연결 유지 타임아웃: 100초, 기본값:5초(5000)

const server = app.listen(cur_port, function () {
    var data_path = process.env.DATA_PATH;
    var log_path = process.env.LOG_PATH;
    // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
    logout("start walk route server response at " + cur_ip.address() + ":" + cur_port);
    logout("OS : " + os.type());
    logout("process pid:" + cur_pid);

    route.init(cur_pid, data_path, log_path);

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
