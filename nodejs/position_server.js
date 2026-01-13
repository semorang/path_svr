const os = require('os');
const http = require('http');
const path = require('path');
const express = require('express');
const request_ip = require('request-ip');
const cors = require('cors'); // CORS 오류 해소
const timeout = require('connect-timeout');
const url = require('url');


const cfg = require('dotenv').config();
// const addon = require('./build/Release/trekking_svr.node');
// const addon = require('./core_modules/walk_route.node');
const route = require('../src/route');
const tms = require('../src/tms');
const logout = require('../src/logs');
const times = require('../src/times.js')
// const escapeJSON = require('escape-json-noide');
// const addon = require('bindings')('openAPI')
const apikey = require('../views/script/key.js');
const fs = require('fs-extra');

const publicPath = path.join(__dirname, '../public') // web에서 공유할 path

const { isContext } = require('vm');

let corsOptions = {
    origin: '*',
    credential: true,
}

const app = express();

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
    let startTime = logout("start version info");

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

    logout("end version info", startTime);
});


app.get('/health', function(req, res) {
    const health_file = process.env.DATA_PATH + "/usr/health.flag";
    fs.access(process.env.HEALTH_FILE, fs.constants.F_OK, err => {
        if (err) return res.status(500).send("FAIL");
        else return res.status(200).send("OK");
    });
});


app.get('/ready', function(req, res) {
    return res.status(200).send("OK");
});


app.post('/api/setdatacost', function(req, res) {
    let startTime = logout('start set data cost');

    const key = req.headers.authorization;
    const body = req.body;

    const ret = route.setdatacost(key, body);

    res.send(ret);

    logout('end set data cost', startTime);
});


app.get('/summary', function(req, res) {
    let startTime = logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'summary');

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
    }

    res.send(ret);

    logout("end route request", startTime);
});


// app.get('/route', timeout('10s'), function(req, res) {
app.get('/route', function(req, res) {
    let startTime = logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req);

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
    } 

    res.send(ret);

    logout("end route request", startTime);
});


app.get('/view/route', function(req, res) {
    let startTime = logout("start route(view) request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    const api = req.query.api;

    var ret = route.doroute(req, 'view');

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
        res.send(ret);
    } else {
        // var kakao_key;// = '9def7442ad15b4775780f565ab8cd9c4';

        // var json = JSON.stringify(ret.route.coords);
        // var vtx = json.stringify(jsonObj)
        // var escJson = escapeJSON(json);
        // var vtx = JSON.parse(json);
        // res.render(__dirname + "/openApi/views/ejs.html", {kakao_key:kakao_key});
        // res.render(__dirname + "/openApi/views/kakao_map", {javascriptkey:kakao_key, result:escJson});
        // res.render(__dirname + "/openApi/views/ejs", {javascriptkey:kakao_key, result:vtx});
        // console.log(JSON.stringify(ret));
        
        //res.render(__dirname + "/../views/kakao_map", {javascriptkey:kakao_key, result:ret});

        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_route", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_route", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    }

    logout("end route(view) request", startTime);
});


// 최적지점API
app.get('/optimalposition', timeout('5s'), function(req, res) {
    res.redirect(url.format({
        pathname: "/api/optimalposition",
        query: req.query
    }));
});

app.get('/api/optimalposition', timeout('5s'), function(req, res) {
    let startTime = logout("start optimalposition, ip : " + request_ip.getClientIp(req));

    logout("client req : " + JSON.stringify(req.query));

    let type = "0";
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,3,1,0; 
        type = "0x00010302"; // 바이트 거꾸로
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

    let ret = route.optimalposition(req);

    if (ret.header.resultCode == 0) {
        logout("result: success, code: " + ret.header.resultCode + ", cnt: " + ret.data.count);
    } else {
        logout("result: failed, code: " + ret.header.resultCode + ", msg: " + ret.header.resultMessage);
    }

    res.send(ret);

    logout("end optimalposition, ", startTime);
});


app.post('/api/multi_optimalposition', timeout('5s'), function(req, res) {
    let startTime = logout("start multi_optimalposition, ip : " + request_ip.getClientIp(req));

    logout("client req : " + JSON.stringify(req.body));

    let ret = route.multi_optimalposition(req.headers.authorization, req.body);

    if (ret.header.resultCode == 0) {
        logout("result: success, code: " + ret.header.resultCode + ", cnt: " + ret.datas.length);
    } else {
        logout("result: failed, code: " + ret.header.resultCode + ", msg: " + ret.header.resultMessage);
    }

    res.send(ret);

    logout("end optimalposition, ", startTime);
});


app.get('/view/optimalposition', timeout('5s'), function(req, res) {
    let startTime = logout("start optimal(view), ip : " + request_ip.getClientIp(req));

    logout("client req : " + JSON.stringify(req.query));

    let type = "0";
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,3,1,0; 
        type = "0x00010302"; // 바이트 거꾸로
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

    // view에서는 무조건 확장 결과를 전달
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

    logout("end optimal(view), ", startTime);
});


const cur_ip = require("ip");
const cur_port = (process.env.OPT_SVR_PORT === undefined) ? 20301 : process.env.OPT_SVR_PORT;
const cur_pid = process.pid;
const start_time = new Date();
const backlog_cnt = 100; // 대기열 크기 설정, 기본값:120
const req_timeout = 20000; // 요청(대기열포함) 타임아웃: 20초, 기본값:2분(120000)
const keepalive_timeout = 100000; // 연결 유지 타임아웃: 100초, 기본값:5초(5000)

const server = app.listen(cur_port, function () {
    var data_path = process.env.DATA_PATH;
    var file_path = process.env.FILE_PATH;
    var log_path = process.env.LOG_PATH;
    // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
    logout("start optimal position server response at " + cur_ip.address() + ":" + cur_port);
    logout("OS : " + os.type());
    logout("process pid:" + cur_pid);
    
    route.init(cur_pid, data_path, file_path, log_path);

    logout("finished server initialize");

    // addon.logout("start optimal position server addr "  + cur_ip.address() + ":" + cur_port + " on " + os.type());
});

server.timeout = req_timeout;
server.keepAliveTimeout = keepalive_timeout;
//server.headersTimeout = 5 * 1000; //10s


// http.createServer(app).listen(cur_port, '0.0.0.0', function () { //http://133.186.212.20:9095/
//     console.info('Listening for HTTP on', this.address());

//     var cur_ip = require("ip");
//     var cur_date = new Date();
//     // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
//     console.log("[" + cur_date + "] start tekking route server response at " + cur_ip.address() + ":" + cur_port);
//     console.log("[" + cur_date + "] OS : " + os.type());

//     addon.logout("start routing request");

//     addon.init();
// });

