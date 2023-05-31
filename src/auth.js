const uuidApiKey = require('../nodejs/node_modules/uuid-apikey');

const apikey = require('../views/script/key.js');
const logout = require('./logs');


exports.checkAuth = function(userkey) {
    var user = null;
    const keys = apikey.DISTMATRIXAPIKEYS;
    for(var ii=0; ii<keys.length; ii++) {
        if (userkey == undefined || !uuidApiKey.isAPIKey(userkey) || !uuidApiKey.check(userkey, keys[ii].uuid)) {
            user = null;
            continue;
        } else {
            user = keys[ii].name;
            break;
        }
    }

    return user;
}


exports.createKey = function() {
    return uuidApiKey.create();
}