// const express = require("express");
// const app = express();
const testtt = require("./build/Release/libtraffic_node.node");
const port = 8000;

path="D:\\1_mr";	

console.log('----------');
function InitAsync(path) {
    return new Promise((resolve, reject) => {
        try {
            result = testtt.initFile(path)
            //console.log("read file result : " +  result);
            resolve();
        } catch (err) {
            reject(err);
        }
    });
}
async function ReadFile(){
    try {
        //console.time("InitAsync");
        await InitAsync(path);
        //console.timeEnd("InitAsync");

        console.log("Init complete");
    } catch (error) {
        console.log("Init failed", error);
    }
}

function ReleaseAsync() {
    return new Promise((resolve, reject) => {
        try {
            testtt.releaseFile();
            resolve();
        } catch (err) {
            reject(err);
        }
    });
}
async function ReleaseFile(){
    try {
        await ReleaseAsync();
        console.log("Release complete");
    } catch (error) {
        console.log("Release failed", error);
    }
}

ReadFile();

console.log(testtt.getSpd(28531300385000, 0, 1714489260, 16));
console.log(testtt.getSpd(28531300386000, 1, 1714489260, 16));

console.log(testtt.getSpd(23630601170000, 1, 1714489260, 16));
console.log(testtt.getSpd(18630301675000, 0, 1714489260, 16));

ReleaseFile();






// app.get("/", (req, res) => {
//     res.set({"Content-Type": "text/html; charset=utf-8"});
//     res.end("hello world");
// });

// app.listen(port, () => {
//     console.log('START SERVER : use ${port}');
// });