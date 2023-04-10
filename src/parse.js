const fs = require('fs');

exports.parse  = function parseBin(binPath, binKey) {
    var binData = fs.readFileSync(binPath).buffer;
    let dv = new DataView(binData); 
    
    function arrayBufferToString(buffer) {
            var byteArray = new Uint8Array(buffer);
            var byteString = '';
            for (var i = 0; i < byteArray.byteLength; i++) {
                    if (byteArray[i])
                            byteString += String.fromCodePoint(byteArray[i]);
            } //for
            return byteString;
    }
    
    let nPos = 0; 
    console.log('total size::', binData.byteLength);
    var binResult = [];
    while (nPos<binData.byteLength) {
            let _header = {};
            _header._signiture  = dv.getUint32(nPos, true); nPos += 4; 
            _header._rttype     = dv.getUint8(nPos, true); nPos += 1; 
            _header._resv0      = dv.getUint8(nPos, true); nPos += 1; 
            _header._resv1      = dv.getUint8(nPos, true); nPos += 1; 
            _header._resv2      = dv.getUint8(nPos, true); nPos += 1; 
    
            _header._version    = dv.getUint16(nPos, true); nPos += 2; 
            _header._data_cnt   = dv.getUint16(nPos, true); nPos += 2; 
            _header._key        = dv.getUint32(nPos, true); nPos += 4; 
    
            _header._req_time   = dv.getUint32(nPos, true); nPos += 4; 
            _header._log_time   = dv.getUint32(nPos, true); nPos += 4; 
    
            _header._sx         = dv.getUint32(nPos, true); nPos += 4; 
            _header._sy         = dv.getUint32(nPos, true); nPos += 4; 
            _header._ex         = dv.getUint32(nPos, true); nPos += 4; 
            _header._ey         = dv.getUint32(nPos, true); nPos += 4;
            let _rt_option = [];
            for (var j=0; j<40; ++j) { // hwName::NAME_SIZE //40
                    _rt_option.push(dv.getUint8(nPos, true)); nPos += 1;
            } //for
            _header._rt_option = arrayBufferToString(_rt_option).trim();        
            
            
            if (binKey != '') {
                if (Number(_header._key) != Number(binKey)) {
                    //console.log('not same binKey>', Number(binKey), 'v.s.', Number(_header._key));
                    nPos += _header._data_cnt * 8; //8 is bodys data size
                    continue;
                } else {
                    //console.log('Same!! binKey>', Number(binKey), 'v.s.', Number(_header._key), '------------------ Same!!!');
                }
            }
    
            let _bodydata = [];
            for (var j=0; j<_header._data_cnt; ++j) {
                    let _abody = {};
                    _abody._mapid   = dv.getUint16(nPos, true); nPos += 2; //uint16_t  _mapid;
                    _abody._linkid  = dv.getUint16(nPos, true); nPos += 2; //uint16_t  _linkid;
                    _abody._time    = dv.getUint16(nPos, true); nPos += 2;  //uint16_t  _time;
                    _abody._extinfo = dv.getUint8(nPos, true); nPos += 1; ; //uint8_t   _extinfo; // [0x03] speed type, [0x04] is_ttl, [0x08] is_rotation_trf
                    _abody._speed   = dv.getUint8(nPos, true); nPos += 1; ; //uint8_t   _speed;
                    _bodydata.push(_abody);
            }  //for
    
            // console.log('nPos::', nPos);
            //console.log('header>>',_header);
            // console.log('body>>',_bodydata);
            binResult.push ({'header':_header, 'bodydata':_bodydata});
            //binResult.push ({'header':_header});
    }
    
    console.log(binResult);
    console.log(JSON.stringify(binResult));
    return binResult;
}