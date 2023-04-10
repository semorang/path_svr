// module.exports = {
//     logout
// }

const tzOffset = new Date().getTimezoneOffset() * 60000;

module.exports = function logout(message) {
    var cur_date = new Date(Date.now() - tzOffset).toISOString().replace('T', ' ').substring(0, 23); // using milliseconds

    console.log("[" + cur_date + '] ' + message);
}