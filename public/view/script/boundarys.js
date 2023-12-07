var seungnamBoundary = [    
    [127.0604, 37.5028],
    [127.0604, 37.4611],
    [126.9979, 37.4611],
    [126.9979, 37.3362],
    [127.0604, 37.3362],
    [127.0604, 37.2945],
    [127.1229, 37.2945],
    [127.1229, 37.3362],
    [127.1854, 37.3362],
    [127.1854, 37.3778],
    [127.2479, 37.3778],
    [127.2479, 37.4611],
    [127.1854, 37.4611],
    [127.1854, 37.5028],
    [127.0604, 37.5028],
]

var changwonBoundary = [
    [128.5603, 35.4197],
    [128.5603, 35.3781],
    [128.4978, 35.3781],
    [128.4978, 35.3364],
    [128.5603, 35.3364],
    [128.5603, 35.2947],
    [128.4353, 35.2947],
    [128.4353, 35.2114],
    [128.3103, 35.2114],
    [128.3103, 35.0864],
    [128.3728, 35.0864],
    [128.3728, 35.0448],
    [128.8728, 35.0448],
    [128.8728, 35.1697],
    [128.8103, 35.1697],
    [128.8103, 35.2114],
    [128.7478, 35.2114],
    [128.7478, 35.2947],
    [128.8103, 35.2947],
    [128.8103, 35.3781],
    [128.6228, 35.3781],
    [128.6228, 35.4197],
    [128.5603, 35.4197],
]

var daeguBoundary = [
    [128.4353, 35.6697],
    [128.4353, 35.8363],
    [128.5603, 35.8363],
    [128.5603, 35.753],
    [128.4978, 35.753],
    [128.4978, 35.6697],
    [128.4353, 35.6697],
]

var pangyoBoundary = [
    [127.0604, 37.4195],
    [127.1229, 37.4195],
    [127.1229, 37.3778],
    [127.0604, 37.3778],
    [127.0604, 37.4195],
]

var bukhansanBoundary = [
    [126.8729, 37.7528],
    [127.0606, 37.7528],
    [127.0606, 37.5861],
    [126.8729, 37.5861],
    [126.8729, 37.7528],
]

function getSeungnamBoundary() {
    return seungnamBoundary;
}

function getChangwonBoundary() {
    return changwonBoundary;
}

function getDaeguBoundary() {
    return daeguBoundary;
}

function getPangyoBoundary() {
    return pangyoBoundary;
}

function getBukhansanBoundary() {
    return bukhansanBoundary;
}

function getChangwonBoundarySize() {
    return changwonBoundary.length;
}

// export { meshes }