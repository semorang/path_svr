const os = require('os');
const http = require('http');
const path = require('path');
const express = require('express');
const request_ip = require('request-ip');
const cors = require('cors'); // CORS 오류 해소
const cfg = require('dotenv').config();
// const addon = require('./build/Release/trekking_route.node');
var route = require('../src/route');
const logout = require('../src/logs');
const times = require('../src/times.js')
// const escapeJSON = require('escape-json-noide');
// const addon = require('bindings')('openAPI')
// const cur_port = (process.env.SERVER_PORT === undefined) ? 20301 : process.env.SERVER_PORT;
const app = express();
const apikey = require('../views/script/key.js');
const timeout = require('connect-timeout');
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

app.use(cors(corsOptions));

app.get('/', function(req, res) {
    logout("client IP : " + request_ip.getClientIp(req));
    res.send("this is trekking route server.")
});


app.get('/test', function(req, res) {
    logout("start test trekking route request");

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
    }
    else{
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


app.get('/summary', function(req, res) {
    logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'summary');

    if (ret.result_code == 0) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    }
    else{
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end route request");
});


// app.get('/route', timeout('10s'), function(req, res) {
app.get('/route', function(req, res) {
    logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req);

    if (ret.result_code == 0) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    }
    else{
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end route request");
});


app.get('/multiroute', function(req, res) {
    logout("start multiroute request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.domultiroute(req);

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    }
    else{
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end multiroute request");
});



app.get('/routeview', function(req, res) {
    logout("start routeview request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'view');

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
    }
    else{
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end routeview request");
});

const cur_ip = require("ip");
const cur_port = (process.env.PED_SVR_PORT === undefined) ? 77777 : process.env.PED_SVR_PORT;
const cur_pid = process.pid;
const start_time = new Date();
const backlog_cnt = 100; // 대기열 크기 설정, 기본값:120
const req_timeout = 20000; // 요청(대기열포함) 타임아웃: 20초, 기본값:2분(120000)
const keepalive_timeout = 5000; // 연결 유지 타임아웃: 5초, 기본값:5초(5000)

const server = app.listen(cur_port, function () {
    var cur_date = new Date();
    // var cur_time = cur_date.toFormat('YYYY-MM-DD HH24:MI:SS');
    logout("[" + cur_date + "] start walk route server response at " + cur_ip.address() + ":" + cur_port);
    logout("[" + cur_date + "] OS : " + os.type());
    logout("[" + cur_date + "] process pid:" + cur_pid);

    route.init(cur_pid);

    logout("[" + cur_date + "] finished server initialize");

    // logout("start walk routing server addr "  + cur_ip.address() + ":" + cur_port + " on " + os.type());
});

server.timeout = req_timeout;


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
