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
    var str = 'loadcss("' + css.file + '", function () {\n' + callback + "\n});";
    return str;
}

function write_load_html(html, callback) {
    var str = '$.get("' + html.file + '", function (ajaxdata) {\n' + html.callback + callback + "\n});";
    return str; 
}

function write_load_js(js, last, callback) {
    var str = "";
    if (last == null || last.type != "js") {
        str += "$LAB";
        str += ".script(" + '"' + js.file + '")';
    }
    else {
        if (last.need_wait) {
            str += ".wait()";
        }
        str += "\n.script(" + '"' + js.file + '")';
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

function $LAB_init_js(jss,callback) {
    var template = "function $LAB_init_js_impl(callback){$LAB";
    for (var i in jss) {
        template += ".script(" + '"' + jss[i].file + '")';
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
        var groups = /MSIE ([0-9.]+)/.exec(window.navigator.userAgent);
        if (groups === null) {
            return null;
        }
        var ieVersionNum = parseInt(groups[1], 10);
        var ieMajorVersion = Math.floor(ieVersionNum);
        return ieMajorVersion;
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
    var cur_page = get_cur_page();
    //js
    var jquery_js = ["../js/jquery-1.11.2.min.js"];
    var jquery_migrate_js = ["../js/jquery-migrate-1.2.1.min.js"];
    var jquery_slimscroll_js = ["../js/jquery.slimscroll.min.js"];
    var pace_js = ["../js/pace.min.js"];
    var style_js= ["../js/style.js?version=" + version.web];
    var jquery_menu_js = ["../js/jquery.metisMenu.js?version=" + version.web];
    var common_js = ["../js/common.js?version=" + version.web];

    var LiveVideo_js = ["../js/LiveVideo.js?version=" + version.web];
    var DrawArea_js = ["../js/DrawArea.js?version=" + version.web];
    var SetDateTime_js = ["../js/SetDateTime.js?version=" + version.web];
    var SetIndLight_js = ["../js/SetIndLight.js?version=" + version.web];
    var SetNetport_js = ["../js/SetNetport.js?version=" + version.web];
    var SetParking_js = ["../js/SetParking.js?version=" + version.web];
    var Support_js = ["../js/Support.js?version=" + version.web];
    var SetVideo_js = ["../js/SetVideo.js?version=" + version.web];
    var SetHttpPush_js = ["../js/SetHttpPush.js?version=" + version.web];
    var SetNode_js = ["../js/SetNode.js?version=" + version.web];
    var SetSerial_js = ["../js/SetSerial.js?version=" + version.web];

    //add_js
    var jss = Array();
    add_js_wait_last(jss, jquery_js);
    add_js_wait_last(jss, jquery_migrate_js);
    add_js(jss, pace_js);
    add_js(jss, style_js);
    add_js(jss, common_js);
    if(cur_page == "Main.html"){
        add_js(jss, jquery_menu_js);
        add_js(jss, jquery_slimscroll_js);
        add_js(jss, LiveVideo_js);
        add_js(jss, DrawArea_js);
        add_js(jss, SetDateTime_js);
        add_js(jss, SetIndLight_js);
        add_js(jss, SetNetport_js);
        add_js(jss, SetParking_js);
        add_js(jss, Support_js);
        add_js(jss, SetVideo_js);
        add_js(jss, SetHttpPush_js);
        add_js(jss, SetNode_js);
        add_js(jss, SetSerial_js);
    }
    $LAB_init_js(jss, function () {
        init_style(function () {
            if (callback != undefined) callback();
        });
    });
}
