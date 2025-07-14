const moment = require('../nodejs/node_modules/moment');

exports.getElapsedTime = function(created) {
    let now = Date.now();
    // 경과시간 정보
    let duration = moment.duration(now - created);
    // 경과시간에 대해 문자열로 표시할 단위 옵션
    let durationOptions = [
        {"dur" : duration.asYears(), "option" : "년 전"},
        {"dur" : duration.asMonths(), "option" : "개월 전"},
        {"dur" : duration.asWeeks(), "option" : "주 전"},
        {"dur" : duration.asDays(), "option" : "일 전"},
        {"dur" : duration.asHours(), "option" : "시간 전"},
        {"dur" : duration.asMinutes(), "option" : "분 전"},
        {"dur" : duration.asSeconds(), "option" : "초 전"}];
    
    // 반복문으로 duration의 값을 확인해 어떤 단위로 반환할지 결정한다.
    // ex) 0.8년전이면 "8개월 전" 반환
    for (let durOption of durationOptions) {
        if (durOption.dur >= 1) {
            return Math.round(durOption.dur) + durOption.option;
        }
    }
    // 분 단위로 검사해도 1 이상이 아니면(반복문에서 함수가 종료되지 않으면) "방금 전" 반환
    return "방금 전"
}


// for p2p path result virtual dummy time
exports.getPathWorkTime = function(time, link_length) {
    let ret = time;
    const TARGET_TIME_FOR_10KM = 0.3;

    let targetTime = (link_length / 10000.0) * TARGET_TIME_FOR_10KM;
    let additionalSleep = targetTime - ret;
    let randomOffset = Math.floor(Math.random() * 21) + 10; // get 10 ~ 30

    randomOffset = 0.1 - (randomOffset * 0.005); // -0.05 ~ 0.05
    additionalSleep += randomOffset;

    ret += additionalSleep;

    if (ret <= 0)
    {
        ret = time;
    }
    else
    {
        ret = parseInt(ret * 1000); // make ms
        // std::this_thread::sleep_for(std::chrono::duration<double>(additionalSleep));
    }

    return ret;
}

// double SetTime(double Time, double length)
// {
//     double res = Time;
//     const double TARGET_TIME_FOR_10KM = 0.7;
//     double targetTime = (length / 10000.0) * TARGET_TIME_FOR_10KM;
//     double additionalSleep = targetTime - res;

//     std::random_device randomDevice;
//     std::mt19937 gen(randomDevice());
//     std::uniform_real_distribution<> distribution(-0.05, 0.05);

//     double randomOffset = distribution(gen);
//     additionalSleep += randomOffset;

//     res += additionalSleep;

//     if (res <= 0)
//     {
//         res = Time;
//     }
//     else
//     {
//         // std::this_thread::sleep_for(std::chrono::duration<double>(additionalSleep));
//     }

//     return res;
// }