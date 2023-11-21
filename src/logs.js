// module.exports = {
//     logout
// }

const tzOffset = new Date().getTimezoneOffset() * 60000;

module.exports = function logout(message, time) {
    let cur_time = Date.now();
    let cur_date = new Date(Date.now() - tzOffset).toISOString().replace('T', ' ').substring(0, 23); // using milliseconds

    if (time != undefined) {
        console.log("[" + cur_date + "] " + message + ", t:" + (cur_time - time).toString());
    } else {
        console.log("[" + cur_date + "] " + message);
    }

    return cur_time;
}