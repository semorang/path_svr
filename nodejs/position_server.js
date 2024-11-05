const os = require('os');
const http = require('http');
const path = require('path');
const express = require('express');
const request_ip = require('request-ip');
const cors = require('cors'); // CORS 오류 해소
const timeout = require('connect-timeout');
const url = require('url');
const moment = require('moment');
const app = express();

const cfg = require('dotenv').config();
// const addon = require('./build/Release/trekking_svr.node');
// const addon = require('./core_modules/walk_route.node');
const route = require('../src/route');
const logout = require('../src/logs');
const apis = require('../src/apis');
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


app.post('/api/setdatacost', function(req, res) {
    let startTime = logout('start set data cost');

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const base = req.body.base;
    const cost = req.body.cost;

    const ret = route.setdatacost(key, mode, base, cost);

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
app.get('/optimalposition', function(req, res) {
    res.redirect(url.format({
        pathname: "/api/optimalposition",
        query: req.query
    }));
});

app.get('/api/optimalposition', function(req, res) {
    let startTime = logout("start optimalposition request");

    logout("client IP : " + request_ip.getClientIp(req));
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

    logout("end optimal(view) request", startTime);
});


const cur_ip = require("ip");
const cur_port = (process.env.OPT_SVR_PORT === undefined) ? 20301 : process.env.OPT_SVR_PORT;
const cur_pid = process.pid;
const start_time = new Date();

const server = app.listen(cur_port, function () {
    var data_path = process.env.DATA_PATH;
    var log_path = process.env.LOG_PATH;
    // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
    logout("start optimal position server response at " + cur_ip.address() + ":" + cur_port);
    logout("OS : " + os.type());
    logout("process pid:" + cur_pid);
    
    route.init(cur_pid, data_path, log_path);

    logout("finished server initialize");

    // addon.logout("start optimal position server addr "  + cur_ip.address() + ":" + cur_port + " on " + os.type());
});

server.headersTimeout = 5 * 1000; //10s


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

