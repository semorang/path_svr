'use strict';

const { createRequire } = require('module');


// OS별 nodejs 디렉터리 설정
const NODEJS_BASE = process.env.NODEJS_BASE || (
    process.platform === 'win32'
        ? 'C:\\__work\\nodejs\\apis\\nodejs'
        : '/home/ec2-user/tms_svr/optimal_svr/nodejs'
);

// createRequire()로 지정 경로 기준 require 함수 생성
const requireFromNodejs = createRequire(NODEJS_BASE + '/package.json');

function reqNode(module_name) {
  return requireFromNodejs(module_name);
}

module.exports = {
  reqNode,
};