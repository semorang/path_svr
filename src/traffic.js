'use strict';

const commont = require('./common.js');
const logout = require('./logs');

// 코어모듈은 그냥 require
const EventEmitter = require('events');
const path = require('path');
const net = require('net');

// 서드파티는 nodejs 쪽에서 로드
const fs = commont.reqNode('fs-extra');

// ---------------------------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------------------------

function removeFilesSync(dirPath) {
  if (!fs.existsSync(dirPath)) return;
  let files = fs.readdirSync(dirPath, { withFileTypes: true });
  for (const dirent of files) {
    if (dirent.isDirectory()) continue;
    const full = path.join(dirPath, dirent.name);
    try {
      fs.unlinkSync(full);
    } catch (err) {
      logout(`error, fs.unlinkSync(${full}), err: ${err}`);
    }
  }
}

function ensureDirectoryExistence(dirPath) {
  if (!fs.existsSync(dirPath)) {
    fs.mkdirSync(dirPath, { recursive: true });
    logout(`Directory '${dirPath}' created.`);
  }
}

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

const MAX_REQ_TIMEINTERVAL = 3 * 60 * 1000; // 3 min
const MAX_REQ_TIMEINTERVAL_RTM = 15 * 60 * 1000; // 15 min
const MAX_RECV_TIMEOUT = 3 * 60 * 1000; // 3 min
const MAX_BUFFER_SIZE = 10 * 1024 * 1024; // 10 MiB

const REQ_TYPE_REAL_KS = 1;
const REQ_TYPE_REAL_TTL = 10;
const REQ_TYPE_REAL_KSR = 100000;
const REQ_TYPE_RTM = 20011;

const RES_TYPE_REAL_KS = 10000;
const RES_TYPE_REAL_TTL = 10001;
const RES_TYPE_REAL_KSR = 10003;
const RES_TYPE_RTM = 20011;

// ---------------------------------------------------------------------------
// Class Definition
// ---------------------------------------------------------------------------

class TrafficManager extends EventEmitter {
  constructor() {
    super();
    this.ip = '';
    this.port = 0;
    this.filePath = '';
    this.typeName = '';
    this.client = null;
    this.buffer = Buffer.alloc(MAX_BUFFER_SIZE);
    this.recvTimeout = MAX_RECV_TIMEOUT;
    this.recvSize = 0;
    this.infoType = 0;
    this.infoSize = 0;
    this.timeoutCount = 0;
    this.prevKey = 0;
    this._retries = 0;
  }

  initialize(ip, port, type, basePath, recvTimeout = MAX_RECV_TIMEOUT) {
    this.ip = ip;
    this.port = port;
    this.typeName = type;
    this.filePath = basePath;
    this.recvTimeout = recvTimeout;

    logout(`initialized traffic path: ${this.filePath}, type(${this.typeName})`);

    const tmpPath = path.join(this.filePath, 'tmp');
    ensureDirectoryExistence(tmpPath);
    removeFilesSync(tmpPath);

    this.connectToServer();
  }

  release() {
    if (this.client) {
      try { this.client.destroy(); } catch {}
      this.client = null;
    }
    this.buffer = null;
  }

  // -------------------------------------------------------------------------
  // Connection management
  // -------------------------------------------------------------------------
  connectToServer() {
    if (this.client) return;

    this.client = net.connect({ port: this.port, host: this.ip });
    this.client.setKeepAlive(true, 30000);
    this.client.setNoDelay(true);

    this.client.on('connect', () => this.onConnect());
    this.client.on('data', (chunk) => this.onData(chunk));
    this.client.on('end', () => this.onEnd());
    this.client.on('error', (err) => this.onError(err));
    this.client.on('timeout', () => this.onTimeout());
  }

  scheduleReconnect() {
    this._retries = (this._retries || 0) + 1;
    const base = Math.min(1000 * 2 ** (this._retries - 1), 30000);
    const jitter = Math.random() * 300;
    setTimeout(() => this.connectToServer(), base + jitter);
  }

  onEnd() {
    logout('disconnected');
    if (this.client) {
      try { this.client.removeAllListeners(); } catch {}
      try { this.client.destroy(); } catch {}
      this.client = null;
    }
    this.scheduleReconnect();
  }

  onError(err) {
    logout(`error, socket client err: ${err}`);
    if (this.client) {
      try { this.client.removeAllListeners(); } catch {}
      try { this.client.destroy(); } catch {}
      this.client = null;
    }
    this.scheduleReconnect();
  }

  // -------------------------------------------------------------------------
  // Communication handlers
  // -------------------------------------------------------------------------
  onConnect() {
    logout(`connected, req(${this.typeName})`);

    this.recvSize = 0;
    this.infoType = 0;
    this.infoSize = 0;
    this.timeoutCount = 0;
    this._retries = 0;

    let reqBuff;

    switch (this.typeName) {
      case 'ks':
        reqBuff = Buffer.alloc(4);
        reqBuff.writeInt32BE(REQ_TYPE_REAL_KS, 0);
        break;
      case 'ttl':
        reqBuff = Buffer.alloc(4);
        reqBuff.writeInt32BE(REQ_TYPE_REAL_TTL, 0);
        break;
      case 'ksr':
        reqBuff = Buffer.alloc(4);
        reqBuff.writeInt32BE(REQ_TYPE_REAL_KSR, 0);
        break;
      case 'rtm':
        reqBuff = Buffer.alloc(16);
        reqBuff.writeInt32BE(REQ_TYPE_RTM, 0);
        reqBuff.writeInt32BE(0, 4);
        reqBuff.writeInt32BE(-1, 8);
        reqBuff.writeInt32BE(-1, 12);
        break;
      default:
        logout(`error, data type not defined, type: ${this.typeName}`);
        return;
    }

    this.client.write(reqBuff);
    this.client.setTimeout(this.recvTimeout);
  }

  onTimeout() {
    logout(`timeout(${this.typeName}), wait: ${(++this.timeoutCount) * (this.recvTimeout / 1000)}s`);
    this.recvSize = 0;
    this.infoType = 0;
    this.infoSize = 0;
    this.client.setTimeout(this.recvTimeout);
    this.onConnect();
  }

  onData(chunk) {
    if (this.recvSize + chunk.length > this.buffer.length) {
      logout(`error, buffer overflow: recvSize=${this.recvSize}, chunk=${chunk.length}`);
      this.recvSize = 0;
      return;
    }

    chunk.copy(this.buffer, this.recvSize);
    this.recvSize += chunk.length;

    while (true) {
      if (this.infoSize <= 0) {
        if (this.recvSize < 8) break;
        this.infoType = this.buffer.readInt32BE(0);
        const bodyLen = this.buffer.readInt32BE(4);
        this.infoSize = (this.infoType === RES_TYPE_RTM) ? bodyLen + 16 : bodyLen + 8;
      }

      if (this.recvSize < this.infoSize) break;

      const recvData = Buffer.allocUnsafe(this.infoSize);
      this.buffer.copy(recvData, 0, 0, this.infoSize);

      const remaining = this.recvSize - this.infoSize;
      if (remaining > 0) {
        this.buffer.copy(this.buffer, 0, this.infoSize, this.infoSize + remaining);
      }
      this.recvSize = remaining;

      this.client.setTimeout(this.recvTimeout);

      if (this.infoType === RES_TYPE_RTM) {
        this.processRTMData(recvData);
      } else {
        this.processTrafficData(recvData);
      }

      this.infoType = 0;
      this.infoSize = 0;
      this.timeoutCount = 0;
    }
  }

  // -------------------------------------------------------------------------
  // Data Processing
  // -------------------------------------------------------------------------
  processRTMData(recvData) {
    const typeValue = recvData.readInt32BE(0);
    const sizeInfo = recvData.readInt32BE(4) + 16;

    if (sizeInfo !== recvData.length) {
      logout(`error, RTM size mismatch (${recvData.length} vs ${sizeInfo})`);
      return;
    }
    if (typeValue !== RES_TYPE_RTM) {
      logout(`error, RTM type mismatch (${typeValue})`);
      return;
    }

    const uniqueKey = recvData.readInt32BE(20);
    const dataTimestamp = recvData.readInt32BE(24);

    if (uniqueKey === this.prevKey) {
      logout(`traffic rtm data same as previous, key: ${uniqueKey}`);
      return;
    }
    this.prevKey = uniqueKey;

    this.saveFile(recvData, 'rtm', dataTimestamp);
  }

  processTrafficData(recvData) {
    const typeValue = recvData.readInt32BE(0);
    const sizeInfo = recvData.readInt32BE(4) + 8;
    if (sizeInfo !== recvData.length) {
      logout(`error, data size mismatch (${recvData.length} vs ${sizeInfo})`);
      return;
    }

    let typeName = 'none';
    let dataTimestamp = 0;

    if (typeValue === RES_TYPE_REAL_KS) {
      typeName = 'ks';
      dataTimestamp = recvData.readInt32BE(8);
    } else if (typeValue === RES_TYPE_REAL_TTL) {
      typeName = 'ttl';
      dataTimestamp = recvData.readInt32BE(12);
    } else if (typeValue === RES_TYPE_REAL_KSR) {
      typeName = 'ksr';
      dataTimestamp = recvData.readInt32BE(12);
    } else {
      logout(`error, unknown typeValue: ${typeValue}`);
      return;
    }

    this.saveFile(recvData, typeName, dataTimestamp);
  }

  // -------------------------------------------------------------------------
  // File Output
  // -------------------------------------------------------------------------
  async saveFile(data, type, timestamp) {
    const now = new Date(Number(timestamp) * 1000);
    const year = String(now.getFullYear());
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');

    const fileName = `${timestamp}_${year}${month}${day}_${hours}${minutes}${seconds}.${type}`;
    const dir = path.join(this.filePath, type, year, month);
    const fileFull = path.join(dir, fileName);

    try {
      await fs.outputFile(fileFull, data);
      this.emit('newFile', { title: fileName, path: dir, timestamp });
      logout(`saved ${type} file: ${fileFull} (${data.length}Byte)`);
      return true;
    } catch (err) {
      logout(`error, writeFile(${fileFull}), err: ${err}`);
      return false;
    }
  }
}

module.exports = TrafficManager;
