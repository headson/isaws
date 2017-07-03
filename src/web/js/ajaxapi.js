var g_need_relogin = false;
function precheck(ajaxdata) {
    if (g_need_relogin) return false;
    if (ajaxdata.length > 0 && ajaxdata[0] == '<') {
        g_need_relogin = true;
        alert("登录超时，请重新登录");
        top.location.href = get_main_path() + "login.htm";        
        return true;
    }
    return false;
}

function parse_ajax_data(ajaxdata) {
    var ret = ajaxdata.split(" ");
    if (ret[0] == "OK") {
        var message = ret[1].split("=");
        return message[1];
    }
    return ret[0];
}

function default_ajax_handler(ajaxdata) {
    if (precheck(ajaxdata)) {
        return false;
    }
    if (ajaxdata.split(" ")[0] == "OK") {
        show_informer();
        return true;
    }
    else {
        alert("保存失败");
        return false;
    }
}
