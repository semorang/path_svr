const os = require('os');
const http = require('http');
const express = require('express');
const path = require('path');
const request_ip = require('request-ip');
const cfg = require('dotenv').config();
// const addon = require('./build/Release/optimal_svr.node');
// const addon = require('./core_modules/walk_route.node');
const route = require('../src/route');
const logout = require('../src/logs');
// const escapeJSON = require('escape-json-noide');
// const addon = require('bindings')('openAPI')
const app = express();
const apikey = require('../views/script/key.js');
const timeout = require('connect-timeout');
const publicPath = path.join(__dirname, '../public') // web에서 공유할 path

// app.set('views', __dirname + '../views');
app.set('view engine', 'ejs');
app.engine('html', require('ejs').renderFile);
// app.use('/script', express.static(__dirname + "../views/script"));
// app.use('/script', express.static("../views/script"));
app.use(express.static(publicPath)); 


app.get('', function(req, res) {
    logout("client IP : " + request_ip.getClientIp(req));

    res.send("this is trecking route server1.")
})

app.get('/test', function(req, res) {
    logout("client IP : " + request_ip.getClientIp(req));

    res.send('hello test, <img src="/maker_orange.png">')
})



app.get('/', function(req, res) {
    logout("client IP : " + request_ip.getClientIp(req));

    res.send("this is trecking route server2.")
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

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
    }

    res.send(ret);

    logout("end test route request");
});


app.get('/version', function(req, res) {
    logout("request version info");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.getversion();

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('버전 확인 실패 : ' + ret.msg);
    }

    res.send(ret);
});


app.get('/summary', function(req, res) {
    logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req, 'summary');

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
    }

    res.send(ret);

    logout("end route request");
});


// app.get('/route', timeout('10s'), function(req, res) {
app.get('/route', function(req, res) {
    logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req);

    logout("result : " + ((ret.result_code == 0) ? "success" : "failed") + ", code : " + ret.result_code);
    if (ret.result_code != 0) {
        logout('경로 탐색 실패 : ' + ret.msg);
    } 

    res.send(ret);

    logout("end route request");
});


app.get('/routeview', function(req, res) {
    logout("start routeview request");

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

    logout("end routeview request");
});


app.get('/optimalposition', function(req, res) {
    logout("start optimalposition request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // if (req.query.type == undefined || req.query.type == 0) {
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,3,1,0; 
        req.query.type = 0x00010302; // 바이트 거꾸로
        // req.query.type = parseInt(subtype, 16);
    }

    var ret = route.optimalposition(req);

    logout("result : " + ((ret.header.resultCode == 0) ? "success" : "failed") + ", code : " + ret.header.resultCode);
    if (ret.header.resultCode != 0) {
        logout('최적지점 검색 실패 : ' + ret.header.resultMessage);
    }

    res.send(ret);

    logout("end optimalposition request");
});


app.get('/optimalview', function(req, res) {
    logout("start optimalview request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // if (req.query.type == undefined || req.query.type == 0) {
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,3,1,0; 
        req.query.type = 0x00010302; // 바이트 거꾸로
        // req.query.type = parseInt(subtype, 16);
    }

    const api = req.query.api;

    var ret = route.optimalview(req);

    logout("result : " + ((ret.header.resultCode == 0) ? "success" : "failed") + ", code : " + ret.header.resultCode);
    if (ret.header.resultCode != 0) {
        logout('최적지점 검색 실패 : ' + ret.header.resultMessage);
        res.send(ret);
    }
    else { //if (ret.header.resultCode == 0) {
        if (api === "kakao") {
            res.render(__dirname + "/../views/kakao_maps_position", {javascriptkey:apikey.KAKAOMAPAPIKEY, result:ret});
        } else {
            res.render(__dirname + "/../views/inavi_maps_position", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    }

    logout("end optimalview request");
});


const cur_port = (process.env.OPT_SVR_PORT === undefined) ? 20301 : process.env.OPT_SVR_PORT;

const server = app.listen(cur_port, function () {
    var cur_ip = require("ip");
    var cur_pid = process.pid; //`${process.pid}`
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

