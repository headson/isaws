var has_local_storage = (function () {
    var uid = new Date;
    var storage;
    var result;
    try {
        (storage = window.localStorage).setItem(uid, uid);
        result = storage.getItem(uid) == uid;
        storage.removeItem(uid);
        return result && storage;
    } catch (exception) { }
}());

var g_channelnum = 1;
var g_support_new_channel_api = 0;
//同步执行该函数，以免页面闪烁

function do_get_device_support_channels() {
    $.ajax({
        url: get_main_path() + "vb.htm?paratest=channelnum",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            response = ajaxdata.split("\n");
            g_channelnum = parse_ajax_data(response[0]);
            g_channelnum = parseInt(g_channelnum, 10);
            if (g_channelnum == NaN) {
                g_channelnum = 1;
            }
            else {
                g_support_new_channel_api = 1;
            }
        },
        async: false
    });
}

function get_device_support_channels() {
    //使用本地存储，提高加载速度，登录时清空
    if (has_local_storage) {
        g_channelnum = localStorage.getItem("channelnum");
        g_support_new_channel_api = localStorage.getItem("support_new_channel_api");
        if (g_channelnum == undefined || g_channelnum == null
            || g_support_new_channel_api == undefined || g_support_new_channel_api == null) {
            do_get_device_support_channels();
            localStorage.setItem("channelnum", g_channelnum);
            localStorage.setItem("support_new_channel_api", g_support_new_channel_api);
        }
    }
    else {
        do_get_device_support_channels();
    }
}


var g_remotechannelnum = 0;

function do_get_device_support_remote_channelnum() {
    $.ajax({
        url: get_main_path() + "vb.htm?paratest=remotechannelnum",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            response = ajaxdata.split("\n");
            g_remotechannelnum = parse_ajax_data(response[0]);
            g_remotechannelnum = parseInt(g_remotechannelnum, 10);
            if (g_remotechannelnum == NaN) {
                g_remotechannelnum = 0;
            }
        },
        async: false
    });
}

function get_device_support_remote_channelnum() {
    //使用本地存储，提高加载速度，登录时清空
    if (has_local_storage) {
        g_remotechannelnum = localStorage.getItem("remotechannelnum");
        if (g_remotechannelnum == undefined || g_remotechannelnum == null) {
            do_get_device_support_remote_channelnum();
            localStorage.setItem("remotechannelnum", g_remotechannelnum);
        }
    }
    else {
        do_get_device_support_remote_channelnum();
    }
}

var g_device_type = null;
function do_get_device_type() {
    $.ajax({
        url: get_main_path() + "vb.htm?gethwinfo",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            ajaxdata = ajaxdata.split("=");
            if (!ajaxdata[0].match(/OK/)) {
                return false;
            }
            ajaxdata = ajaxdata[1];
            if (ajaxdata.match(/notwrite/)) {
                g_device_type = -1;
                return false;
            }
            else {
                ajaxdata = ajaxdata.split("^");
                var idevice_type = parseInt(ajaxdata[2], 16);
                if (idevice_type != NaN) g_device_type = idevice_type;
            }
            return true;
        },
        async: false
    });
}
function get_device_type() {
    if (has_local_storage) {
        g_device_type = localStorage.getItem("devicetype");
        if (g_device_type == undefined || g_device_type == null) {
            do_get_device_type();
            if (g_device_type != null) {
                localStorage.setItem("devicetype", g_device_type);
            }
        }
    }
    else {
        do_get_device_type();
    }
}

//从另外一个页面被活活移了过来
var g_support_onvif = null;

//同步执行该函数，以免页面闪烁
function do_get_device_support_onvif() {
    $.ajax({
        url: get_main_path() + "vb.htm?paratest=supportonvif",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            response = ajaxdata.split("\n");
            g_support_onvif = parse_ajax_data(response[0]);
            if (g_support_onvif == "0") {
            }
            else if (g_support_onvif == "1") {
            }
            else {
                //"NG"
            }
        },
        async: false
    });
}

function get_device_support_onvif() {
    if (has_local_storage) {
        g_support_onvif = localStorage.getItem("support_onvif");
        if (g_support_onvif == undefined || g_support_onvif == null) {
            do_get_device_support_onvif();
            if (g_support_onvif != null) {
                localStorage.setItem("support_onvif", g_support_onvif);
            }
        }
    }
    else {
        do_get_device_support_onvif();
    }
}

var g_hwinfo = null;
//同步执行该函数，以免页面闪烁
function do_get_hwinfo() {
    $.ajax({
        url: get_main_path() + "vb.htm?gethwinfo",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return;
            }
            ajaxdata = ajaxdata.split("=");
            if (!ajaxdata[0].match(/OK/)) {
                return;
            }
            g_hwinfo = ajaxdata[1];
        },
        async: false
    });
}

function get_hwinfo() {
    if (has_local_storage) {
        g_hwinfo = localStorage.getItem("hw_info");
        if (g_hwinfo == undefined || g_hwinfo == null) {
            do_get_hwinfo();
            if (g_hwinfo != null) {
                localStorage.setItem("hw_info", g_hwinfo);
            }
        }
    }
    else {
        do_get_hwinfo();
    }
}

var g_support3g = null;
var g_supportsd = null;

function do_get_support_3g_sd() {
    $.ajax({
        url: get_main_path() + "vb.htm?paratest=support3g&paratest=supportsd",
        success: function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            response = ajaxdata.split("\n");
            g_support3g = parse_ajax_data(response[0]);
            g_supportsd = parse_ajax_data(response[1]);
        },
        async: false
    });
}

function get_support_3g_sd() {
    if (has_local_storage) {
        var support_3g_sd = localStorage.getItem("support_3g_sd");
        if (support_3g_sd != undefined) {
            support_3g_sd = support_3g_sd.split(":");
            g_support3g = support_3g_sd[0];
            g_supportsd = support_3g_sd[1];
        }
        if (support_3g_sd == undefined || g_support3g == null || g_supportsd == null) {
            do_get_support_3g_sd();
            if (g_support3g != null && g_supportsd != null) {
                support_3g_sd = g_support3g + ":" + g_supportsd;
                localStorage.setItem("support_3g_sd", support_3g_sd);
            }
        }
    }
    else {
        do_get_support_3g_sd();
    }
}