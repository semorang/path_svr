// 메뉴 토글

document.addEventListener("DOMContentLoaded", () => {
    const tabWrapper = document.getElementById("tabWrapper");
    const tabToggleHandle = document.getElementById("tabToggleHandle");

    let tabHidden = false;

    tabToggleHandle.addEventListener("click", function() { handleMenuToggle(); });
    tabToggleHandle.addEventListener("touchstart", function() { handleMenuToggle(); });

    function handleMenuToggle() {
    // tabToggleHandle.addEventListener("click", () => {
        tabHidden = !tabHidden;
        tabWrapper.style.left = tabHidden ? '-290px' : '0';
        toggleIcon.textContent = tabHidden ? '▶' : '◀';
    };

    // if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Samsung|Mobile/i.test(navigator.userAgent)) {
    //     document.querySelectorAll('.desktop-only').forEach(el => el.style.display = 'none');
    // }
    // "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"
    if (/Windows|Win64|x64|Win32|x86/i.test(navigator.userAgent)) {
        document.querySelectorAll('.desktop-only').forEach(el => el.style.display = 'block');
    }
});


function initDisplay() {
    // route_option = 1; // 0:최단거리, 추천, 편안한, 최소시간(빠른), 큰길
    // if (route_option == 1) { document.getElementById("optRecommended").checked = true; }
    // else if (route_option == 2) { document.getElementById("optComfortable").checked = true; }
    // else if (route_option == 3) { document.getElementById("optFastest").checked = true; }
    // else if (route_option == 4) { document.getElementById("optMainroad").checked = true; }
    // else { document.getElementById("optShotest").checked; }
}

function openTab(tabId) {
//   const allTabs = document.querySelectorAll('.tabcontent > div');
//   allTabs.forEach(el => el.style.display = 'none');

  document.getElementById(tabId).style.display = 'block';
}


function showRoutePopup(lntLat) {
    if (!currentRoutePopup.getVisible()) {
        currentRoutePopup.setVisible(true);
        currentRoutePopup.setMap(map);
    }
    currentRoutePopup.setPosition(lntLat);

//   const menu = document.getElementById('popupMenu');
//   menu.style.left = x + 'px';
//   menu.style.top = y + 'px';
//   menu.style.display = 'block';
}

function hideRoutePopup(x, y) {
    currentRoutePopup.setVisible(false);
//   const menu = document.getElementById('popupMenu');
//   menu.style.display = 'none';
}