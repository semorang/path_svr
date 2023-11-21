// apis module

const route = require('./route');
const auth = require('./auth');
const codes = require('./codes');
const logout = require('./logs');


exports.distancematrix = function(key, mode, destinations) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && destinations !== undefined) {       
            logout("client user:'" + user + "', req:" + JSON.stringify(destinations));
            
            ret = route.gettable(mode, destinations);
        }
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


exports.clustering = function(key, target, destinations, clusters, file, mode) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (destinations !== undefined) {       
            logout("client user:'" + user + "', req pois: " + JSON.stringify(destinations) + ", req clusters: " + clusters);
            
            ret = route.getcluster(target, destinations, clusters, file, mode);
        }
    } else {
        var header = {
            isSuccessful: false,
            resultCode: codes.ERROR_CODES.RESULT_APPKEY_ERROR,
            resultMessage: codes.getErrMsg(codes.ERROR_CODES.RESULT_APPKEY_ERROR)
        };
        
        var origins;
        var clusters;
        if (destinations) {
            origins = destinations
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


exports.boundary = function(key, mode, destinations) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && destinations !== undefined) {       
            logout("client user:'" + user + "', req boundary: " + JSON.stringify(destinations));
            
            ret = route.getboundary(mode, destinations);
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


exports.bestwaypoints = function(key, mode, destinations) {
    var ret;

    // 사용자 키 확인
    var user = auth.checkAuth(key);
    if (user != null && user.length > 0) {
        if (mode !== undefined && destinations !== undefined) {       
            logout("client user:'" + user + "', req:" + JSON.stringify(destinations));
            
            ret = route.getbestways(mode, destinations);
        }
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


exports.createkey = function() {
    return auth.createKey();
}