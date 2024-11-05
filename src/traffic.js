const EventEmitter = require('../nodejs/node_modules/events');
const fs = require('../nodejs/node_modules/fs-extra');
const path = require('../nodejs/node_modules/path');

class FolderWatcher extends EventEmitter {
    constructor(inPath) {
        super();
        this.folderPath = inPath;
    }

    isValidTimestamp(timestamp) {
        const fileDate = new Date(parseInt(timestamp) * 1000);
        const now = new Date();
        const diff = (now - fileDate) / 1000 / 60; // change to min
    
        return diff <= 15; // check limit time
    }
    
    checkFolder() {
        fs.readdir(this.folderPath, { withFileTypes: true }, (err, files) => {
            if (err) {
                console.error('Error reading the foler:', err);
                return;
            }
    
            files.forEach(file => {
                if (file.isDirectory()) {
                    return;
                }

                const sourcePath = path.join(this.folderPath + '/', file.name);
                const ext = path.extname(file.name);
                const name = path.basename(file.name, ext);
                const name_parts = name.split('_');
                const timestamp = parseInt(name_parts[0]);
                
                // 파일 이동
                var targetExtDir;
                if (ext === '.ttl') {
                    targetExtDir = 'ttl';
                } else if (ext === '.ks') {
                    targetExtDir = 'ks';
                } else {
                    return;
                }

                const targetDir = this.folderPath + '/' + targetExtDir;
                if (!fs.existsSync(targetDir)) {
                    fs.mkdirSync(targetDir); // 폴더 생성
                }
                // 파일복사
                const tartgetPath = path.join(targetDir, file.name);
                fs.move(sourcePath, tartgetPath, (err) => {
                    if (err) {
                        console.error('Error copy the file:', err);
                    } else {
                        console.log(`new traffic file '${sourcePath}' move to '${tartgetPath}' successfully` );
                        if (1) { // 우선은 무조건 사용
                            // if (this.isValidTimestamp(timestamp)) {
                            this.emit('newFile', { fileName: file.name, filePath: targetDir, timeStamp: timestamp});
                        }
                    }                    
                });
            });
        });
    }
}


module.exports = FolderWatcher;