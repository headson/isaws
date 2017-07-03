
function delay_css_file(url) {
    var obj = document.createElement('link'),
        lnk = 'href',
        tp = 'text/css';
    obj.setAttribute('rel', 'stylesheet');

    obj.setAttribute(lnk, url);
    obj.setAttribute('type', tp);
    document.getElementsByTagName('head')[0].appendChild(obj);
    return obj;

};
//http://segmentfault.com/blog/liangyi/1190000002424872,from seajs
function loadcss(url, callback) {

    var elem = delay_css_file(url);

    if (elem.attachEvent) {
        elem.attachEvent('onload', callback);
    } else {
        setTimeout(function () {
            poll(elem, callback);
        }, 0);
    }

    function poll(_elem, callback) {

        var isLoaded = false;
        var sheet = _elem['sheet'];
        var isOldWebKit = (navigator.userAgent.replace(/.*AppleWebKit\/(\d+)\..*/, '$1')) * 1 < 536;

        if (isOldWebKit) { //webkit 版本小于 536
            if (sheet) {
                isLoaded = true;
            }
        } else if (sheet) {
            try {
                if (sheet.cssRules) {
                    isLoaded = true;
                }
            } catch (ex) {
                if (ex.code === 'NS_ERROR_DOM_SECURITY_ERR') {
                    isLoaded = true;
                }
            }
        }

        if (isLoaded) {
            setTimeout(function () {
                if (callback != undefined) callback();
            }, 1);
        } else {
            setTimeout(function () {
                poll(_elem, callback);
            }, 1);
        }
    }

}

function write_load_css(css, callback) {
    var str = 'loadcss(get_main_path() + "' + css.file + '", function () {\n' + callback + "\n});";
    return str;
}

function write_load_html(html, callback) {
    var str = '$.get(get_main_path() + "' + html.file + '", function (ajaxdata) {\n' + html.callback + callback + "\n});";
    return str; 
}

function write_load_js(js, last, callback) {
    var str = "";
    if (last == null || last.type != "js") {
        str += "$LAB";
        str += ".script(get_main_path() + " + '"' + js.file + '")';
    }
    else {
        if (last.need_wait) {
            str += ".wait()";
        }
        str += "\n.script(get_main_path() + " + '"' + js.file + '")';        
    }
    str += callback;

    return str;
}

function write_load_jchs(jchs, i, last, callback) {
    var str = "";
    if (jchs.length == 0) {
        str = "callback()";
        return str;
    }
    if (i < (jchs.length - 1)) {
        str = write_load_jchs(jchs, i + 1, jchs[i], callback);
        if (jchs[i].type == "js" && jchs[i + 1].type != "js") {
            str = ".wait(function(){\n" + str + "\n});";
        }
    }
    if (i == (jchs.length - 1)) {
        if (jchs[i].type == "js") {
            str = ".wait(callback);";
        }
        else {
            str = "callback()";
        }
    }    
    if (i < jchs.length) {
        if (jchs[i].type == "js") {
            str = write_load_js(jchs[i],last, str);
        }
        else if (jchs[i].type == "css") {
            str = write_load_css(jchs[i], str);
        }
        else if (jchs[i].type == "html") {
            str = write_load_html(jchs[i], str);
        }        
    }
    return str;
}

function $init_js_or_css_or_html(jchs, callback) {
    var template = "function $init_js_or_css_or_html_impl(callback){\n";
    var last = null;
    template += write_load_jchs(jchs, 0, last,"callback");
    template += '}';

    eval(template); 
    $init_js_or_css_or_html_impl(callback);
}

function css_file(file) {
    var f = {};
    f.type = "css";
    f.file = file;
    return f;
}

function html_file(file, callback) {
    var f = {};
    f.type = "html";
    f.file = file;
    f.callback = callback;
    return f;
}

function js_file(file, need_wait) {
    var f = {};
    f.type = "js";
    f.file = file;
    if (need_wait != undefined) {
        f.need_wait = need_wait;
    }
    else {
        f.need_wait = false;
    }
    return f;
}

function add_js(jss, js_files) {
    for (var i in js_files) {
        jss.push(js_file(js_files[i]));
    }
}

function add_js_wait_last(jss, js_files) {
    for (var i in js_files) {
        if (i == (js_files.length - 1)) {
            jss.push(js_file(js_files[i], true));
        }
        else {
            jss.push(js_file(js_files[i]));
        }
    }
}

function add_css(jchs, files) {
    for (var i in files) {
        jchs.push(css_file(files[i]));
    }
}

function add_html(jchs, files) {
    for (var i in files) {
        jchs.push(html_file(files[i].file, files[i].callback));
    }
}

function $LAB_init_js(jss,callback) {
    var template = "function $LAB_init_js_impl(callback){$LAB";
    for (var i in jss) {
        template += ".script(get_main_path() + " + '"' + jss[i].file + '")';
        if (jss[i].need_wait) {
            template += ".wait()";
        }
    }
    template += ".wait(callback);}";
    eval(template);
    $LAB_init_js_impl(callback);
}

function checkIeVersion() {
    function emulatedIEMajorVersion() {
        var groups = /MSIE ([0-9.]+)/.exec(window.navigator.userAgent)
        if (groups === null) {
            return null
        }
        var ieVersionNum = parseInt(groups[1], 10)
        var ieMajorVersion = Math.floor(ieVersionNum)
        return ieMajorVersion
    }

    function actualNonEmulatedIEMajorVersion() {
        // Detects the actual version of IE in use, even if it's in an older-IE emulation mode.
        // IE JavaScript conditional compilation docs: http://msdn.microsoft.com/en-us/library/ie/121hztk3(v=vs.94).aspx
        // @cc_on docs: http://msdn.microsoft.com/en-us/library/ie/8ka90k2e(v=vs.94).aspx
        var jscriptVersion = new Function('/*@cc_on return @_jscript_version; @*/')() // jshint ignore:line
        if (jscriptVersion === undefined) {
            return 11 // IE11+ not in emulation mode
        }
        if (jscriptVersion < 9) {
            return 8 // IE8 (or lower; haven't tested on IE<8)
        }
        return jscriptVersion // IE9 or IE10 in any mode, or IE11 in non-IE11 mode
    }
    function isBrowerIE() {
        var tmp = document.documentMode;
        var isIE;
        try {
            document.documentMode = "";
        } catch (e) { };

        isIE = typeof document.documentMode == "number" || eval("/*@cc_on!@*/!1");
        try {
            document.documentMode = tmp;
        } catch (e) { };
        return isIE;
    }
    var ie = new Object;
    ie.isIE = isBrowerIE();
    ie.version = null;
    ie.emulatedVersion = null;
    
    if (ie.isIE) {
        ie.emulatedVersion = emulatedIEMajorVersion();
        ie.version = actualNonEmulatedIEMajorVersion();
    }

    return ie;
}


function init_impl(callback) {
    var ie = checkIeVersion();
    var cur_page = get_cur_page();
    //var jquery_js = ["js/jquery-1.6.4.min.js"];
    var jquery_js = ["js/jquery-1.11.2.min.js"];
    var jquery_migrate_js = ["js/jquery-migrate-1.2.1.min.js"];
    var active_cookie_support_js = ["js/jquery.cookie.js", "js/aes.js", "js/aes-ctr.js"];
    var active_api_support_js = ["js/jquery.json-2.4.min.js"];
    var ajaxapi_base_js = ["js/base64.js","js/utf8.js"];
    var page_base_js = ["js/ajaxapi.js?version=" + version.web, "js/devicesupport.js?version=" + version.web, "js/style.js?version=" + version.web];
    var test_ip_js = ["js/testip.js"];
    var jq_form_js = ["js/jquery.form.js"];
    var number2ch_js = ["js/number2ch.js"];
    var jq_ztreecore_js = ["js/jquery.ztree.core-3.5.min.js"];
    var jq_ztreeexedit_js = ["js/jquery.ztree.exedit-3.5.min.js"];

    var jss = Array();
    add_js_wait_last(jss, jquery_js);
    add_js_wait_last(jss, jquery_migrate_js);
    if (cur_page == "login.htm" || cur_page == "AddEditUsers.htm") {
        add_js(jss, active_cookie_support_js);
    }
    else if (cur_page == "LiveVideo.htm" || cur_page == "IvsConfig.htm" || cur_page == "SetVideo.htm"
        || cur_page == "Replay.htm" || cur_page == "SetDigitalVideo.htm" || cur_page == "SetHj212.htm" || cur_page == "SetVideoSFM.htm") {
        if (!ie.isIE) {
            alert("当前页面含有active控件，请使用ie浏览器访问！");
        }
        else{
            add_js(jss, active_cookie_support_js);
            add_js(jss, active_api_support_js);

            var active_js = ["js/activex.js?version=" + version.web + "." + version.active ];
            add_js(jss, active_js);
        }        
    }
    if (cur_page == "SetNetPort.htm" || cur_page == "SetChannel.htm" || cur_page == "SetChannelOld.htm") {
        add_js(jss, test_ip_js);
    }
    else if (cur_page == "Maintenance.htm") {
        add_js(jss, jq_form_js);
    }
    else if (cur_page == "SetDigitalVideo.htm") {
        add_js(jss, number2ch_js);
    }
    else if (cur_page == "DeviceGroup.htm") {
        add_js_wait_last(jss, jq_ztreecore_js);
        add_js_wait_last(jss, jq_ztreeexedit_js);
    }
    add_js(jss, ajaxapi_base_js);
    add_js(jss, page_base_js);
    $LAB_init_js(jss, function () {
        init_style(function () {
            if (callback != undefined) callback();
        });
    });
}
