//do not change this js file, or you need to clear the cache of your browser 
function get_cur_page() {
    var cur_page = window.location.href.split('/');
    cur_page = cur_page[cur_page.length - 1];
    cur_page = cur_page.split('#')[0];
    cur_page = cur_page.split('?')[0];
    if (cur_page == "")
		cur_page = "login.html";
    return cur_page;
}

function init(callback) {
    var cur_page = get_cur_page();
    if (cur_page == "main.html" || cur_page == "login.html" || window.parent.version == undefined) {
        $LAB
        .script("../js/version.js?version=" + new Date().getTime()).wait(function () {
            $LAB.script("../js/init.js?version=" + version.web).wait(function () {
                init_impl(callback);
            });
        });
    }
    else {
        window.version = window.parent.version;
        $LAB.script("../js/init.js?version=" + version.web).wait(function () {
            init_impl(callback);
        });
    }
}
