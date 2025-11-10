// tms module

const common = require('./common.js');
const route = require('./route');
const auth = require('./auth');
const codes = require('./codes');
const logout = require('./logs');

const axios = common.reqNode('axios');


exports.distancematrix = function (key, req) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", cache=" + ((req.cache !== undefined) ? req.cache : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        ret = route.gettable(req);
    } else {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };

        ret = {
            header: header,
            origins: req.origins,
            destinations: req.destinations
        };

        logout("client req error : " + JSON.stringify(ret));
    }

    return ret;
}


// exports.clustering = function(key, target, destinations, clusters, file, mode, option) {
exports.clustering = function (key, req) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", cache=" + ((req.cache !== undefined) ? req.cache : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        ret = route.getcluster(req);
    } else {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };

        var origins;
        var clusters;
        if (req.origins) {
            origins = req.origins
        };
        if (clusters) {
            clusters = clusters
        };


        ret = {
            header: header,
            origins: origins,
            clusters: clusters,
        };

        logout("client req error : " + JSON.stringify(ret));
    }

    return ret;
}


exports.boundary = function (key, mode, target, destinations) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && destinations !== undefined) {
            logout("client user:'" + user + "', req boundary: " + JSON.stringify(destinations));

            ret = route.getboundary(mode, target, destinations);
        }
    } else {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };

        var origins;
        var boundary;
        if (destinations) {
            origins = destinations
        };

        ret = {
            header: header,
            origins: origins,
            boundary: boundary,
        };

        logout("client req error : " + JSON.stringify(ret));
    }

    return ret;
}


exports.bestwaypoints = function (key, req) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        logout("client user:'" + user + "', mode=" + ((req.mode !== undefined) ? req.mode : "null") + ", cache=" + ((req.cache !== undefined) ? req.cache : "null") + ", cnt=" + ((req.origins !== undefined) ? req.origins.length : "null"));

        ret = route.getbestways(req);
    } else {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };

        var origins;
        if (req.origins) {
            origins = req.origins
        };

        ret = {
            header: header,
            origins: origins,
        };

        logout("client req error : " + JSON.stringify(ret));
    }

    return ret;
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
        const cnt = req.rdm.length;

        // "compare_type_desc": "0:COST, 1:DIST, 2:TIME"
        let compareType = "distance";
        if (compare == 2) {
            compareType = "duration";
        }

        for (let i = 0; i < cnt; i++) {
            const row = [];
            for (let j = 0; j < rdm[i].elements.length; j++) {
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
        // "endpoint_type_desc": "0:NONE, 1:START, 2:END, 3:START-END"
        let endpointType = "return_to_depot"; // default
        if (endpoint === 1) {
            endpointType = "fixed_start";
        } else if (endpoint === 2) {
            endpointType = "fixed_end";
        } else if (endpoint === 3) {
            endpointType = "fixed_start_end"
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
            if (rdm && rdm.length > 0) {
                for (let i = 0; i < routeIndices.length - 1; i++) {
                    const from = routeIndices[i];
                    const to = routeIndices[i + 1];
                    totalDistance += rdm[from].elements[to].distance.value;
                    totalTime += rdm[from].elements[to].duration.value;
                }
            }

            // 최종 결과 구성
            return {
                header: { isSuccessful: true },
                summary: {
                    distance: totalDistance,
                    time: totalTime
                },
                waypoints: routeIndices.map(index => ({ index })),
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
        logout(`client user:'${user}', mode=${req.mode ?? 'null'}, cache=${req.cache ?? 'null'}, cnt=${Array.isArray(req.origins) ? req.origins.length : 'null'}`);

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
                const table = route.gettable(req); // 동기라면 OK, 비동기면 await route.gettable(req)
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
                    origins: req?.origins ?? [],
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
