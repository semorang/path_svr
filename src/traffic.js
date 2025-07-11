const { clearInterval } = require('timers');
const EventEmitter = require('../nodejs/node_modules/events');
const fs = require('../nodejs/node_modules/fs-extra');
const path = require('../nodejs/node_modules/path');
// const net = require('../nodejs/node_modules/net');
const net = require('net');

const logout = require('./logs');
const { WriteStream } = require('fs');


function removeFilesSync(path) {
    let files = fs.readdirSync(path, { withFileTypes: true });

    files.forEach(file => {
        if (file.isDirectory()) {
            return;
        }

        // 파일 삭제
        try {
            fs.unlinkSync(file.path + "/" + file.name);
        } catch (err) {
            logout("error, fs.unlinkSync(" + file.path + "/" + file.name + ", err: " + err);
        }
    });
}


function ensureDirectoryExistence(dirPath) {
    if (!fs.existsSync(dirPath)) {
        fs.mkdirSync(dirPath, { recursive: true });
        logout(`Directory '${dirPath}' created.`);
    } else {
        // console.log(`Directory '${dirPath}' already exists.`);
    }
}


String.prototype.format = function() { 
    var formatted = this; 
    for( var arg in arguments ) { 
        formatted = formatted.replace("{" + arg + "}", arguments[arg]); 
    }
    
    return formatted; 
};


const MAX_REQ_TIMEINTERVAL = 3 * 60 * 1000; // 3m
const MAX_REQ_TIMEINTERVAL_RTM = 15 * 60 * 1000; // 15m
const MAX_RECV_TIMEOUT = 3 * 60 * 1000; // 3m default
const MAX_BUFFER_SIZE = 5 * 1024 * 1024; // 5MB
const REQ_TYPE_REAL_KS = 1;
const REQ_TYPE_REAL_TTL = 10;
const REQ_TYPE_REAL_KSR = 100000;
const REQ_TYPE_RTM = 20011;
const RES_TYPE_REAL_KS = 10000;
const RES_TYPE_REAL_TTL = 10001;
const RES_TYPE_REAL_KSR = 10003;
const RES_TYPE_RTM = 20011;

class TrafficManager extends EventEmitter {
    constructor() {
        super();
        this.ip = '';
        this.port = '';
        this.filePath = '';
        this.typeName = '';
        this.client = null;
        this.buffer = null;
        this.recvTimeout = MAX_RECV_TIMEOUT;
        this.recvSize = 0;
        this.infoType = 0;
        this.infoSize = 0;
        this.timeoutCount = 0;
        this.prevKey = 0;
    }

    initialize(ip, port, type, path, recvTimeout = MAX_RECV_TIMEOUT) {
        this.ip = ip;
        this.port = port;
        this.typeName = type;
        this.filePath = path;
        this.buffer = Buffer.alloc(MAX_BUFFER_SIZE);
        this.recvTimeout = recvTimeout;
        this.recvSize = 0;
        this.infoType = 0;
        this.infoSize = 0;
        this.timeoutCount = 0;
        this.prevKey = 0;

        logout("initialized traffic path: " + this.filePath + ", type(" + this.typeName + ')');

        let checkPath = this.filePath + '/tmp';
        ensureDirectoryExistence(checkPath);
        removeFilesSync(checkPath);

        this.connectToServer();
    }

    release() {
        if (this.client) {
            this.client.destroy();
            this.client = null;
        }

        if (this.buffer) {
            this.buffer = null;
        }
    }

    connectToServer() {
        if (!this.client) {
            this.client = net.connect({ port: this.port, host: this.ip });

            this.client.on('connect', () => {
                this.onConnect();
            });

            this.client.on('data', chunk => {
                this.onData(chunk);
            });

            this.client.on('end', () => {
                this.onEnd();
            });

            this.client.on('error', err => {
                this.onError(err);
            });

            this.client.on('timeout', () => {
                this.onTimeout();
            });
        } else {
            this.onConnect();
        }
    }

    onConnect() {
        logout('connected, req(' + this.typeName +')');

        this.recvSize = 0;
        this.infoType = 0;
        this.infoSize = 0;
        this.timeoutCount = 0;

        // req data type
        let reqBuff;

        if (this.typeName === 'ks') {
            reqBuff = Buffer.alloc(4);
            reqBuff.writeInt32BE(REQ_TYPE_REAL_KS, 0);
        } else if (this.typeName === 'ttl') {
            reqBuff = Buffer.alloc(4);
            reqBuff.writeInt32BE(REQ_TYPE_REAL_TTL, 0);
        } else if (this.typeName === 'ksr') {
            reqBuff = Buffer.alloc(4);
            reqBuff.writeInt32BE(REQ_TYPE_REAL_KSR, 0);
        } else if (this.typeName === 'rtm') {
            reqBuff = Buffer.alloc(16);
            reqBuff.writeInt32BE(REQ_TYPE_RTM, 0);
            reqBuff.writeInt32BE(0, 4);
            reqBuff.writeInt32BE(-1, 8);
            reqBuff.writeInt32BE(-1, 12);
        } else {
            logout('error, data type not defined, type: ' + this.typeName);
            return;
        }

        this.client.write(reqBuff);
        this.client.setTimeout(this.recvTimeout);
    }

    onData(chunk) {
        // 수신된 데이터를 버퍼에 복사하고 크기를 갱신
        chunk.copy(this.buffer, this.recvSize);
        this.recvSize += chunk.byteLength;

        if (this.infoSize <= 0 && this.recvSize >= 8) {
            this.infoType = this.buffer.readInt32BE(0);
            if (this.infoType === RES_TYPE_RTM) {
                this.infoSize = this.buffer.readInt32BE(4) + 16;
            } else {
                this.infoSize = this.buffer.readInt32BE(4) + 8;
            }
        }

        // 타임아웃 기다리지 말고 바로 처리하자
        if ((this.infoSize > 0) && (this.recvSize >= this.infoSize)) {
            this.client.setTimeout(this.recvTimeout);
            this.handleRecvData();
        }
        // logout("recv size: " + chunk.byteLength + ", 0x" + chunk[chunk.length - 1].toString(16).toUpperCase());
    }

    onEnd() {
        logout('disconnected');
        
        // retry connection
        this.connectToServer();
    }

    onError(err) {
        logout('error, socket client err:' + err);
        
        this.client.unref();
        this.client.end();
        this.client.destroy();
        this.client.removeAllListeners();
+       
        // retry connection
        setTimeout(() => {
            this.connectToServer();
        }, 1000);
    }

    onTimeout() {
        // logout('timeout(' + this.typeName + '), try:' + ++this.timeoutCount + ', recv:' + this.recvSize);
        logout("timeout({0}), wait:{1}".format(this.typeName, ((++this.timeoutCount) * (this.recvTimeout / 1000))));
        this.client.setTimeout(this.recvTimeout);
        this.handleRecvData();
    }

    handleRecvData() {
        // read header
        if (this.infoSize <= 0 && this.recvSize >= 8) {
            this.infoType = this.buffer.readInt32BE(0);
            if (this.infoType === RES_TYPE_RTM) {
                this.infoSize = this.buffer.readInt32BE(4) + 16;
            } else {
                this.infoSize = this.buffer.readInt32BE(4) + 8;
            }
        }

        if ((this.infoType > 0) && (this.infoSize > 0) && (this.recvSize >= this.infoSize)) {
            logout("received traffic data({0}), size:{1}".format(this.typeName, this.infoSize));

            // create received data
            const recvData = this.buffer.slice(0, this.infoSize);

            // move remained data offset 
            this.recvSize -= this.infoSize;
            if (this.recvSize > 0) {
                this.buffer.copy(this.buffer, this.infoSize, this.recvSize);
                logout("remained traffic data({0}), size:{1}".format(this.typeName, this.recvSize));
            }

            if (this.infoType === RES_TYPE_RTM) {
                this.processRTMData(recvData);
                
                // 데이터 수신 완료 후 3분뒤 다시 데이터 요청
                this.client.setTimeout(0);
                setTimeout(() => {
                    this.connectToServer();
                }, MAX_REQ_TIMEINTERVAL_RTM);
            } else {
                this.processTrafficData(recvData);
                
            }

            // reset info
            this.infoType = 0;
            this.infoSize = 0;
            this.timeoutCount = 0;
        }
        
        // 데이터 수신 완료 후 15초 후에 다시 데이터 요청
        // setTimeout(() => {
        //     this.onConnect();
        // }, MAX_REQ_TIMEINTERVAL);
    }


    processRTMData(recvData) {     
        // let dataTimestamp = Math.round(Date.now() / 1000);
        const typeValue = recvData.readInt32BE(0);
        const sizeInfo = recvData.readInt32BE(4) + 16;

        let typeName = "none";

        if (sizeInfo != recvData.length) {
            logout("error, data size not match, data size:" + recvData.length + "size info: " + sizeInfo);
        } else {
            if (typeValue == RES_TYPE_RTM) {
                typeName = "rtm";
            } else {
                logout("error, data type not match, data:{0} vs info:{1}", RES_TYPE_RTM, typeName);
                return;
            }

            let uniqueKey = recvData.readInt32BE(16 + 4); // RTM 고유번호
            let dataTimestamp = recvData.readInt32BE(16 + 8); // RTM 고유번호
    
            if (uniqueKey === this.prevKey) {
                logout("traffic rtm data is same with previous data, key: " + uniqueKey);
            } else if (this.saveFile(recvData, typeName, dataTimestamp) == true) {
                // logout("success traffic data to file : " + fileName);
            }

            this.prevKey = uniqueKey;
        }
    }


    processTrafficData(recvData) {
        // const dv = new DataView(binData.buffer, binData.byteOffset, binData.byteLength);
        // const typeValue = dv.getInt32(0, false);
        // const bodySize = dv.getInt32(4, false);
        // const dataTimestamp = dv.getBigUint64(8, false);       
        let dataTimestamp = 0;
        let typeName = "none";
        const typeValue = recvData.readInt32BE(0);
        const sizeInfo = recvData.readInt32BE(4) + 8;

        if (sizeInfo != recvData.length) {
            logout("error, data size not match, data size:" + recvData.length + "size info: " + sizeInfo);
        } else {
            if (typeValue === RES_TYPE_REAL_KS) {
                typeName = "ks";
                dataTimestamp = recvData.readInt32BE(8);
            } else if (typeValue === RES_TYPE_REAL_TTL) {
                typeName = "ttl";
                // dataTimestamp = binData.readBigUInt64BE(8);
                dataTimestamp = recvData.readInt32BE(12);
            } else if (typeValue === RES_TYPE_REAL_KSR) {
                typeName = "ksr";
                // dataTimestamp = binData.readBigUInt64BE(8);
                dataTimestamp = recvData.readInt32BE(12);
            } else {
                logout("error, data type not match, data:{0} vs info:{1}", this.infoType, typeName);
                return;
            }

            if (this.saveFile(recvData, typeName, dataTimestamp) == true) {
                // logout("success traffic data to file : " + fileName);
            }
        }
    }


    saveFile(data, type, timestamp) {
        let ret = true;

        const now = new Date(Number(timestamp) * 1000);
        const year = now.getFullYear().toString();
        const month = ('0' + (now.getMonth() + 1)).slice(-2);
        const day = ('0' + now.getDate()).slice(-2);
        const hours = ('0' + now.getHours()).slice(-2);
        const minutes = ('0' + now.getMinutes()).slice(-2);
        const seconds = ('0' + now.getSeconds()).slice(-2);

        // const fileName = timestamp.toString() + "_" + year + month + day + "_" + hours + minutes + seconds + "." + type;
        const fileName = "{0}_{1}{2}{3}_{4}{5}{6}.{7}".format(timestamp.toString(), year, month, day, hours, minutes, seconds, type);
        const filePath = this.filePath + "/" + type + "/" + year + "/" + month;
    
        ensureDirectoryExistence(filePath);

        const newFilePath = filePath + "/" + fileName;

        try {
            fs.writeFileSync(newFilePath, data);
            this.emit('newFile', { title: fileName, path: filePath, timestamp: timestamp });
        } catch (err) {
            logout("error, fs.writeFileSync(" + newFilePath + ", err: " + err);
            ret = false;
        }

        return ret;
    }
}


module.exports = TrafficManager;
