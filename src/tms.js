// tms module
const addon = require(process.env.USER_MODULE);

const common = require('./common.js');
const route = require('./route');
const auth = require('./auth');
const codes = require('./codes');
const logout = require('./logs');

const axios = common.reqNode('axios');


// common header 생성 함수
function createHeader(isSuccessful, code, message) {
    return {
        isSuccessful,
        resultCode: code,
        resultMessage: message
    };
}


// ---------------- Distance Matrix ----------------
exports.distancematrix = function (key, req) {
    // check user key
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", binary=" + ((req.binary !== undefined) ? req.binary : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        const result = addon.getmatrix(JSON.stringify(req));
        const res = JSON.parse(result);
    
        const header = createHeader(
            res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
            res.result_code,
            codes.getErrMsg(res.result_code)
        );

        return {
            header,
            mode: req.mode ?? "driving",
            origins: req.origins ?? null,
            destinations: req.destinations ?? null,
            rows: res.rows ?? null,
            rdm: res.rdm ?? null,
            rpm: res.rpm ?? null,
        };
    }

    // failed
    const header = createHeader(
        false,
        codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    );

    const ret = {
        header,
        mode: req.mode ?? "driving",
        origins: req.origins ?? null,
        destinations: req.destinations ?? null,
        rows: null,
    };

    logout("client req error : " + JSON.stringify(ret));

    return ret;
}


// ---------------- Distance Matrix Path ----------------
exports.distancematrix_path = function (key, req) {
    // check user key
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout("client user:'" + user + "'");

        const result = addon.getmatrixpath(JSON.stringify(req));
        const res = JSON.parse(result);
    
        const header = createHeader(
            res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
            res.result_code,
            codes.getErrMsg(res.result_code)
        );

        return {
            header,
            routes: res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS ? res.routes : null,
        };
    }

    // failed
    const header = createHeader(
        false,
        codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    );

    const ret = {
        header,
        routes: null,
    };

    logout("client req error : " + JSON.stringify(ret));

    return ret;
}


// ---------------- Clustering ----------------
exports.clustering = function (key, req) {
    // check user key
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", binary=" + ((req.binary !== undefined) ? req.binary : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        if (!req.tsp) {
            req.tsp = {
                seed: 10000,
                algorithm: 4,
                compare_type: 1,
            };
        }
        
        // default clust 설정
        if (!req.clust) {
            req.clust = {
                seed: 10006,
                algorithm: 4,
                compare_type : 1,
            };
        }
        
        const reqStr = JSON.stringify(req);
        let result;

        if (req.target === 'geoyoung') {
            result = addon.getcluster_for_geoyoung(reqStr);
        } else if (req.option.division_type === 4) { // ROAD(link) 균등 분할
            result = addon.getgroup(reqStr);
        } else {
            result = addon.getcluster(reqStr);
        }
    
        const ret = JSON.parse(result);
    
        const header = createHeader(
            ret.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
            ret.result_code,
            codes.getErrMsg(ret.result_code)
        );

        return {
            header,
            mode: req.mode ?? "clustering",
            origins: req.origins ?? null,
            summary: ret.summary ?? null,
            clusters: ret.clusters ?? null,
        };
    } 

    // failed
    const header = createHeader(
        false,
        codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    );
    
    const ret = {
        header,
        origins: req.origins ?? null,
        summary: null,
        clusters: null,
    };

    logout("client req error : " + JSON.stringify(ret));

    return ret;
}


// ---------------- Grouping ----------------
exports.grouping = function (key, req) {
    // 사용자 키 확인
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", binary=" + ((req.binary !== undefined) ? req.binary : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        // 그룹 결과 가져오기
        const result = addon.getgroup(JSON.stringify(req));
        const res = JSON.parse(result);
    
        const header = createHeader(
            res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
            res.result_code,
            codes.getErrMsg(res.result_code)
        );
   
        return { 
            header, 
            mode: req.mode ?? "clustering", 
            origins: req.origins ?? null, 
            summary: res.summary ?? null, 
            clusters: res.clusters ?? null, 
        };
    } 
    
    // failed
    const header = createHeader(
        false,
        codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    );

    const ret = {
        header,
        origins: req.origins ?? null,
        summary: null,
        clusters: null,
    };

    logout("client req error : " + JSON.stringify(ret));

    return ret;
}


exports.boundary = function (key, mode, target, destinations) {
    // 사용자 키 확인
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout("client user:'" + user + "', req boundary: " + JSON.stringify(destinations));

        const result = addon.getboundary(cntDestinations, destinations);
        const res = JSON.parse(result);

        const header = createHeader(
            res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
            res.result_code,
            codes.getErrMsg(res.result_code)
        );

        return {
            header,
            origins: destinations ?? null,
            boundary: res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS ? res.boundary : null,
        };
    }

    // failed
    const header = createHeader(
        false,
        codes.ERROR_CODES.RESULT_APPKEY_ERROR,
        codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
    ); 

    const ret = {
        header,
        origins: destinations ?? null,
        boundary: null,
    };

    logout("client req error : " + JSON.stringify(ret));

    return ret;
}


// ---------------- Best Waypoints ----------------
exports.bestwaypoints = function (key, req, callback) {
    // 사용자 키 확인
    const user = auth.checkAuth(key);
    if (user && user.length > 0) {
        logout(`client user:'${user}', mode=${req.mode ?? 'null'}, binary=${req.binary ?? 'null'}, cnt=${Array.isArray(req.origins) ? req.origins.length : 'null'}`);

        // vrp 모드면 bestvrp로 분기
        if (req.mode === 'vrp') {
            return exports.bestvrp(key, req, callback);
        } 
        
        const ret = exports.getbestways(req);
        return callback(ret);
    } 
    
    // failed
    const ret = {
        header: createHeader(
            false,
            codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        ),
        origins: req?.origins ?? [],
    };

    logout(`client req error: ${JSON.stringify(ret)}`);
    callback(ret);
}


// requestVrp 함수 정의
async function requestVrp(req, endpoint = 0, compare) {
    // "matrix" : [[0,1,2,3,4]]
    //  "rdm" : [ { "elements": [  {
    //                 "distance": {
    //                     "value": 0,
    //                     "text": "0 Km"
    //                 }, "duration": {
    //                     "value": 0,
    //                     "text": "0 mins"
    //                 }, "cost": {
    //                     "value": 0,
    //                     "text": "0 points"
    //                 }, "status": "OK"
    //             } ] } ]

    let matrix = [];
    let rdm = [];

    if (req.matrix && req.matrix.length > 0) {
        // todo
        // matrix가 nxn 2차원 배열인지 확인 필요
        matrix = req.matrix;
    } else {
        rdm = req.rdm;
        let cnt = req.rdm.length;
        // "compare_type_desc": "0:COST, 1:DIST, 2:TIME"
        let compareType = "distance";
        if (compare == 2) {
            compareType = "duration";
        }

        for (let i = 0; i < cnt; i++) {
            const row = [];
            for (let j = 0; j < rdm[i].elements.length; j++) {
            // for (let j = 0; j < cnt; j++) {
                const element = rdm[i].elements[j];
                if (element.status === 'OK') {
                    // if (element[compareType] === undefined || element[compareType].value == null || element[compareType].value === undefined) {
                    //     logout("debug");
                    // }
                    row.push(element[compareType].value);
                } else {
                    logout("bestvrp create matrix value error, [" + i + "][" + j + "] status not OK");
                    row.push(null); // 또는 0, -1 등 오류 처리 방식에 따라                        
                }
            }
            matrix.push(row);
        }
    }

    if (matrix.length <= 2) {
        return {
            header: {
                isSuccessful: false,
                resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED_WRONG_PARAM,
                resultMessage: 'matrix 구성 요소가 부족합니다'
            }
        }
    } else {
        // "endpoint_type_desc": "0:NONE, 1:START, 2:END, 3:START-END, 4:AUTO-START"
        let endpointType = "return_to_depot"; // default
        if (endpoint === 1) {
            endpointType = "fixed_start";
        } else if (endpoint === 2) {
            endpointType = "fixed_end";
        } else if (endpoint === 3) {
            endpointType = "fixed_start_end"
        } else if (endpoint === 4) {
            endpointType = "auto_start"
        }

        const req_vrp = {
            matrix: matrix,
            num_vehicles: 1,
            depot: 0,
            endpoint_type: endpointType
        };

        try {
            // API 호출
            // const response = await axios.post('http://13.125.127.89:20302/api/solve_vrp', req_vrp);
            const response = await axios.post('http://127.0.0.1:20302/api/solve_vrp', req_vrp);
            const result = response.data;

            // 경로 인덱스 추출 (예: result.route 또는 result.waypoints 등 API 구조에 따라 조정)
            const routeIndices = result.routes?.[0];
            if (!routeIndices || routeIndices.length < 2) {
                throw new Error('경로 인덱스가 부족하거나 응답에 없음');
            }

            // 거리 및 시간 계산
            let totalDistance = 0;
            let totalTime = 0;
            let waypoints = [];

            // prev -> curr 구간 dist/time 얻는 공통 함수
            function getLegCost(prev, curr) {
                // rdm이 있으면 rdm 기반으로 dist/time 분리
                if (rdm && rdm.length > 0) {
                    const element = rdm[prev]?.elements?.[curr];
                    if (element?.status === 'OK') {
                        const dist = element.distance?.value ?? 0;
                        const time = element.duration?.value ?? 0;
                        return { dist, time };
                    }
                    // status not OK면 0 처리(필요 시 null/예외로 바꿔도 됨)
                    return { dist: 0, time: 0 };
                }

                // matrix만 있는 경우: time/distance 구분 없으므로 동일값 사용
                const v = (matrix?.[prev]?.[curr] != null) ? matrix[prev][curr] : 0;
                return { dist: v, time: v };
            }

            // routeIndices: 예) [0, 3, 2, 1, 0]
            for (let i = 0; i < routeIndices.length; i++) {
                const index = routeIndices[i];

                let dist = 0;
                let time = 0;

                if (i > 0) {
                    const prev = routeIndices[i - 1];
                    const leg = getLegCost(prev, index);

                    dist = leg.dist ?? 0;
                    time = leg.time ?? 0;

                    totalDistance += dist;
                    totalTime += time;
                }

                waypoints.push({
                    index,
                    distance: dist,
                    time: time,
                });
            }

            // 최종 결과 구성
            return {
                header: { isSuccessful: true },
                summary: {
                    distance: totalDistance,
                    time: totalTime
                },
                waypoints,
                routes: routeIndices
            };
        } catch (error) {
            return {
                header: {
                    isSuccessful: false,
                    resultCode: error.message,
                    resultMessage: 'VRP API 호출 실패'
                }
            }
        }
    }
}


exports.bestvrp = function (key, req, callback) {
    // 사용자 키 확인
    const user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        logout(`client user:'${user}', mode=${req.mode ?? 'null'}, binary=${req.binary ?? 'null'}, cnt=${Array.isArray(req.origins) ? req.origins.length : 'null'}`);

        (async () => {
            let payload = { matrix: null, rdm: null };

            // 1) matrix 직접 입력
            if (Array.isArray(req.matrix) && req.matrix.length > 0) {
                payload.matrix = req.matrix;
            }
            // 2) rdm 직접 입력
            else if (Array.isArray(req.rdm) && req.rdm.length > 0) {
                payload.rdm = req.rdm;
            }
            // 3) 서버에서 테이블 생성
            else {
                const table = route.getmatrix(req); // 동기라면 OK, 비동기면 await route.getmatrix(req)
                if (!table?.header?.isSuccessful || !Array.isArray(table.rows) || table.rows.length === 0) {
                    return callback({
                        header: {
                            isSuccessful: false,
                            resultCode: codes.ERROR_CODES.ROUTE_RESULT_FAILED_INTERNAL,
                            resultMessage: '거리행렬 생성 실패'
                        },
                        origins: req?.origins ?? []
                    });
                }
                payload.rdm = table.rows;
            }


            const compare_type = req?.tsp?.compare_type ?? 0; // 0/1: distance, 2: duration

            const opt = req?.option ?? {};
            const endpoint_type = opt.endpoint_type ?? opt.position_lock ?? 0; // endpoint_type변경, 기존 버전 안정화 후 삭제 예정

            const ret_vrp = await requestVrp(payload, endpoint_type, compare_type);
            if (ret_vrp?.header?.isSuccessful) {
                return callback({
                    header: {
                        isSuccessful: true,
                        resultCode: codes.ERROR_CODES.ROUTE_RESULT_SUCCESS,
                        resultMessage: codes.getErrMsg(codes.ERROR_CODES.ROUTE_RESULT_SUCCESS)
                    },
                    ...(req?.matrix
                        ? { matrix: req.matrix }
                        : { origins: req?.origins ?? [] }),
                    summary: ret_vrp.summary,
                    waypoints: ret_vrp.waypoints,
                    routes: ret_vrp.routes
                });
            }

            return callback({
                header: {
                    isSuccessful: false,
                    resultCode: ret_vrp?.header?.resultCode ?? codes.ERROR_CODES.ROUTE_RESULT_FAILED_INTERNAL,
                    resultMessage: ret_vrp?.header?.resultMessage ?? 'VRP 처리 실패'
                },
                origins: req?.origins ?? []
            });
        })();
    } else {
        return callback({
            header: {
                isSuccessful: false,
                resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
                resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
            },
            origins: req?.origins ?? []
        });
    }
}


// 클러스터링 영역
exports.getbestways = function(req) {
    let result = addon.getbestwaypoints(JSON.stringify(req));
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

    if (res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS) {
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

    if (res.result_code === codes.ERROR_CODES.ROUTE_RESULT_SUCCESS) {
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
