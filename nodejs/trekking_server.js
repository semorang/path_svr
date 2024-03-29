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
const logout = require('../src/logs');
const apis = require('../src/apis');
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

    var ret = route.doroute(req, 'summary');

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
    logout("start route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    var ret = route.doroute(req);

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end route request");
});


app.get('/api/multiroute', function(req, res) {
    logout("start multiroute request");

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
    logout("start kakaovx route request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";

    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";
    var ret = route.domultiroute(key, req, "");

    if (ret.header.isSuccessful == true) {
        // logout(JSON.stringify(ret));
        res.send(ret);
    } else {
        // res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
        res.send(ret);
    }

    logout("end kakaovx route request");
});


app.get('/view/kakaovx', function(req, res) {
    logout("start kakaovx request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    req.query.target = "kakaovx";
    const api = req.query.api;
    const key = (req.query.appkey != undefined) ? req.query.appkey : "kakaovx";

    var ret = route.domultiroute(key, req, "view");

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

    logout("end kakaovx request");
});


app.get('/view/route', function(req, res) {
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
    } else {
        res.send('경탐 실패, 코드 : ' + ret.result_code + ', 오류 : ' + ret.msg);
    }

    logout("end routeview request");
});


app.get('/view/multiroute', function(req, res) {
    logout("start multirouteview request");

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
        } else {
            res.render(__dirname + "/../views/inavi_maps_multiroute", {javascriptkey:apikey.INAVIMAPAPIKEY, result:ret});
        }
    } else {
        res.send('다중 경탐 실패, 코드 : ' + ret.header.resultCode + ', 오류 : ' + ret.header.resultMessage);
    }

    logout("end multirouteview request");
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


// p2p path result view
app.get('/view/path', function(req, res) {
    logout("start pathview request");

    // 2p2 path 전용 옵션
    req.query.option = 8;

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
    logout("start optimalposition request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // if (req.query.type == undefined || req.query.type == 0) {
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,1,0,0; 
        req.query.type = 0x00000102; // 바이트 거꾸로
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


app.get('/view/optimalposition', function(req, res) {
    logout("start optimalview request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // if (req.query.type == undefined || req.query.type == 0) {
    if (req.query.type == undefined) {
        // 택시승하차 - 차량출입구 만 선택
        // 2,1,0,0; 
        req.query.type = 0x00000102; // 바이트 거꾸로
        // req.query.type = parseInt(subtype, 16);
    }

    if (req.query.expand == undefined) {
        req.query.expand = 1;
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

    logout("end optimalview request");
});


app.get('/api/distancematrix/appkeys/:userkey', function(req, res) {
    logout("start distance matrix request by GET");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const destinations = req.query.origins;

    // distance matrix api 호출
    const ret = apis.distancematrix(key, mode, destinations);

    res.send(ret);

    logout("end distance matrix request by GET");
});


app.post('/api/distancematrix', function(req, res) {
    logout("start distance matrix request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const destinations = req.body.origins;

    // distance matrix api 호출
    const ret = apis.distancematrix(key, mode, destinations);

    res.send(ret);

    logout("end distance matrix request");
});


// clustering api 호출
app.get('/api/clustering/appkeys/:userkey', function(req, res) {
    logout("start clustering request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const destinations = req.query.origins;
    const count = req.query.count;

    const ret = apis.clustering(key, mode, destinations, count);

    res.send(ret);

    logout("end clustering request by GET");
});


app.post('/api/clustering', function(req, res) {
    logout("start clustering request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const destinations = req.body.origins;
    const count = req.body.count;

    const ret = apis.clustering(key, mode, destinations, count);

    res.send(ret);

    logout("end clustering request");
});


// clustering boundary api 호출
app.get('/api/boundary/appkeys/:userkey', function(req, res) {
    logout("start boundary request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const destinations = req.query.origins;

    const ret = apis.boundary(key, mode, destinations);

    res.send(ret);

    logout("end boundary request by GET");
});


app.post('/api/boundary', function(req, res) {
    logout("start boundary request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const destinations = req.body.origins;

    const ret = apis.boundary(key, mode, destinations);

    res.send(ret);

    logout("end boundary request");
});


app.get('/api/bestwaypoints/appkeys/:userkey', function(req, res) {
    logout("start bestwaypoints request");

    const key = req.params.userkey;
    const mode = req.query.mode;
    const destinations = req.query.origins;

    // distance matrix api 호출
    const ret = apis.bestwaypoints(key, mode, destinations);

    res.send(ret);

    logout("end bestwaypoints request by GET");
});


app.post('/api/bestwaypoints', function(req, res) {
    logout("start bestwaypoints request");

    const key = req.headers.authorization;
    const mode = req.body.mode;
    const destinations = req.body.origins;

    // distance matrix api 호출
    const ret = apis.bestwaypoints(key, mode, destinations);

    res.send(ret);

    logout("end bestwaypoints request");
});


app.get('/api/createkey', function(req, res) {
    logout("create key request");

    logout("client IP : " + request_ip.getClientIp(req));
    logout("client req : " + JSON.stringify(req.query));

    // 필요시 uuid api key 생성
    logout("created new key :" + JSON.stringify(apis.createkey()));

    res.send('success, created key, ask to check your key to the administrator');
});

const cur_port = (process.env.TRK_SVR_PORT === undefined) ? 20301 : process.env.TRK_SVR_PORT;
const server = app.listen(cur_port, function () {
    var cur_ip = require("ip");
    var cur_pid = process.pid; //`${process.pid}`
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

server.headersTimeout = 5 * 1000; //10s


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
