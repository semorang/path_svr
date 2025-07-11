// apis module

const route = require('./route');
const auth = require('./auth');
const codes = require('./codes');
const logout = require('./logs');


exports.distancematrix = function(key, req) {
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
        
        var origins;
        if (destinations) {
            origins = destinations
        };
    
        ret = {
            header: header,
            origins: origins,
        };

        logout("client req error : " + JSON.stringify(ret));
    }

    return ret;
}


// exports.clustering = function(key, target, destinations, clusters, file, mode, option) {
exports.clustering = function(key, req) {
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


exports.boundary = function(key, mode, target, destinations) {
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


exports.bestwaypoints = function(key, req) {
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


exports.createkey = function() {
    return auth.createKey();
}