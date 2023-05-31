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


exports.createkey = function() {
    return auth.createKey();
}