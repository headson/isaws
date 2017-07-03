
var g_cur_style = null;
var g_style_time = null;
var style_time_path = null;
var style_path = null;
var g_oem_info = -1;
var g_hava_custom = -1;
var g_type = false;
function get_oem_info(ajaxdata) {
    g_style_time = "old";
    g_cur_style = "org";
    if (ajaxdata == null) return;

    if (ajaxdata.match(/notwrite/)) {
        return;
    }
    else {
        ajaxdata = ajaxdata.split("^");
        if (ajaxdata.length < 5 || ajaxdata[3] != '.') {
            return;
        }
        var oem = ajaxdata[4].split('.');
        if (oem[0] == "01") {
            g_style_time = "new";
            g_cur_style = "blue";
            g_oem_info = 1;
        }
        else if (oem[0] == "02") {
            g_style_time = "sfm";
            g_cur_style = "org";
            g_oem_info = 2;
        }
        else if(oem[0] == "05") {
            g_style_time = "hrzx";
            g_cur_style = "org";
            g_oem_info = 5;
        }
    }
    return;
}

function get_hava_custom(callback) {
    $.ajax({
        url: get_main_path() + "vb.htm?judgediyhtml",
        success: function (ajaxdata) {
            if ("OK" != ajaxdata.slice(0, 2)) return false;
            var slicestring = ajaxdata.substring(ajaxdata.indexOf("=") + 1);
            if (slicestring == 0) {
                g_hava_custom = 0;
                style_time_path = "style/custom/style_" + g_oem_info + "/" + g_style_time + "/";
                style_path = style_time_path + g_cur_style + "/";
                $LAB.script(get_main_path() + style_time_path + "js/custom_version.js?version=" + new Date().getTime()).wait(function () {
                    version.web = custom_version.web;
                    callback();
                });
            } else {
                g_hava_custom = -1;
                g_style_time = "old";
                g_cur_style = "org";
                get_hava_style_bag(function () {
                    callback();
                });
            }
        },
        async: false
    });
}

function get_hava_style_bag(callback) {
    $.ajax({
        url: get_main_path() + "vb.htm?judgestylepackage",
        success: function (ajaxdata) {
            if ("OK" != ajaxdata.slice(0, 2)) return false;
            var slicestring = ajaxdata.substring(ajaxdata.indexOf("=") + 1);
            if (slicestring != "nopackage") {
                g_type = true;
                if (slicestring == 0) {
                    g_style_time = "old";
                    g_cur_style = "org";
                } else if (slicestring == 1) {
                    g_style_time = "new";
                    g_cur_style = "org";
                } else if (slicestring == 5) {
                    g_style_time = "hrzx";
                    g_cur_style = "org";
                }
                style_time_path = "style/style_package/bag/";
                style_path = style_time_path + "org/";
                $LAB.script(get_main_path() + style_time_path + "js/package_version.js?version=" + new Date().getTime()).wait(function () {
                    version.web = package_version.web;
                    callback();
                });
            } else {
                style_time_path = "style/" + g_style_time + "/";
                style_path = style_time_path + g_cur_style + "/";
                callback();
            }
        },
        async: false
    });
}

function get_cur_style(callback) {
    get_hwinfo();
    get_oem_info(g_hwinfo);
    get_hava_custom(function () {
        callback();
    });
}


function load_style(callback) {
    var bootstrap_js = ["js/bootstrap.min.js"];
    var file_tree_js = ["js/FileTree/jqueryFileTree.js"];
    var file_tree_css = ["js/FileTree/jqueryFileTree.css"];

    get_cur_style(function () {
        var cur_page = get_cur_page();       
        //var style_time_path = "style/" + g_style_time + "/";
        //var style_path = style_time_path + g_cur_style + "/";
        var jq_easing_js = ["js/jquery.easing.1.3.js"];
        var sidebar_menu_js = [style_time_path + "js/menu-collapsed.js?version=" + version.web];
        var customui_js = [style_time_path + "js/custom-ui.js?version=" + version.web];

        var easy_tab_js = ["js/jquery.hashchange.min.js", "js/jquery.easytabs.js"];

        var datepicker_zhCN_js = [style_time_path + "jq-ui/jquery.ui.datepicker-zh-CN.js"];       
                
        var jchs = Array();
        var common_css = ["style/common/common.css?version=" + version.web];
        var ztree_css = ["style/common/zTreeStyle.css?version=" + version.web];

        if (g_style_time == "old") {
            var main_css = [style_path + "css.css?version=" + version.web];
            var menu_css = [style_path + "menu.css?version=" + version.web];
            var main_js = [style_time_path + "js/main.js?version=" + version.web];

            if (g_type) {
                var main_html = [{
                    file: "style/old/main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            } else {
                var main_html = [{
                    file: style_time_path + "main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            }


            var login_css = [style_path + "login.css?version=" + version.web];
            var login_js = [style_time_path + "js/login.js?version=" + version.web];

            var page_css = [style_path + "page.css?version=" + version.web];
            var check_css = [style_path + "check/check.css?version=" + version.web];

            var login_html = [{
                file: style_time_path + "login.body.htm?version=" + version.web,
                callback: '$("body").html(ajaxdata);'
            }];
            var jqui_base_css = [style_path + "jq-ui/jquery-ui-1.11.3.custom.min.css?version=" + version.web];
            var jqui_base_js = [style_time_path + "jq-ui/jquery-ui-1.11.3.custom.min.js?version=" + version.web];
            var jqui_check_js = [style_time_path + "jq-ui/check/ui.checkbox.js"];

            if (cur_page == "main.htm") {
                add_css(jchs, main_css);
                add_css(jchs, menu_css);
                add_js(jchs, main_js);
                add_html(jchs, main_html);
                add_js(jchs, jq_easing_js);
                add_js(jchs, sidebar_menu_js);
            }
            else if (cur_page == "PlatePictureView.htm") {
                add_css(jchs, page_css);
                add_js(jchs, jq_easing_js);
                add_js(jchs, file_tree_js);
                add_css(jchs, file_tree_css);
            }
            else {
                add_css(jchs, jqui_base_css);
                add_css(jchs, page_css);
                add_css(jchs, check_css);
                if (cur_page == "login.htm") {
                    add_js(jchs, login_js);
                    add_html(jchs, login_html);
                    add_js_wait_last(jchs, jqui_base_js);
                }
                else if (cur_page == "SetNetPort.htm") {
                    add_js_wait_last(jchs, jqui_base_js);                 
                }
                else {
                    add_js_wait_last(jchs, jqui_base_js);
                }
                if (cur_page == "SetDateTime.htm" || cur_page == "PeopleCountResult.htm") {
                    add_js(jchs, datepicker_zhCN_js);
                }
                if (cur_page == "DeviceGroup.htm") {
                    add_css(jchs, common_css);
                    add_css(jchs, ztree_css);
                }
                add_js_wait_last(jchs, jqui_check_js);
                add_js(jchs, customui_js);
            }
        }
        else if (g_style_time == "hrzx") {
            var main_css = [style_path + "css.css?version=" + version.web];
            var menu_css = [style_path + "menu.css?version=" + version.web];
            var main_js = [style_time_path + "js/main.js?version=" + version.web];
            
            if (g_type) {
                var main_html = [{
                    file: "style/apple/main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            } else {
                var main_html = [{
                    file: style_time_path + "main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            }

            var login_css = [style_path + "login.css?version=" + version.web];
            var login_js = [style_time_path + "js/login.js?version=" + version.web];

            var page_css = [style_path + "page.css?version=" + version.web];
            var check_css = [style_path + "check/check.css?version=" + version.web];

            var login_html = [{
                file: style_time_path + "login.body.htm?version=" + version.web,
                callback: '$("body").html(ajaxdata);'
            }];
            var jqui_base_css = [style_path + "jq-ui/jquery-ui-1.11.3.custom.min.css?" + version.web];
            var jqui_base_js = [style_time_path + "jq-ui/jquery-ui-1.11.3.custom.min.js?" + version.web];
            var jqui_check_js = [style_time_path + "jq-ui/check/ui.checkbox.js"];

            if (cur_page == "main.htm") {
                add_css(jchs, main_css);
                add_css(jchs, menu_css);
                add_js(jchs, main_js);
                add_html(jchs, main_html);
                add_js(jchs, jq_easing_js);
                add_js(jchs, sidebar_menu_js);
            }
            else if (cur_page == "PlatePictureView.htm") {
                add_css(jchs, page_css);
                add_js(jchs, jq_easing_js);
                add_js(jchs, file_tree_js);
                add_css(jchs, file_tree_css);
            }
            else {
                add_css(jchs, jqui_base_css);
                add_css(jchs, page_css);
                add_css(jchs, check_css);
                if (cur_page == "login.htm") {
                    add_js(jchs, login_js);
                    add_html(jchs, login_html);
                    add_js_wait_last(jchs, jqui_base_js);
                }
                else if (cur_page == "SetNetPort.htm") {
                    add_js_wait_last(jchs, jqui_base_js);                
                }
                else {
                    add_js_wait_last(jchs, jqui_base_js);
                }
                if (cur_page == "SetDateTime.htm" || cur_page == "PeopleCountResult.htm") {
                    add_js(jchs, datepicker_zhCN_js);
                }
                if (cur_page == "DeviceGroup.htm") {
                    add_css(jchs, common_css);
                    add_css(jchs, ztree_css);
                }
                add_js_wait_last(jchs, jqui_check_js);
                add_js(jchs, customui_js);
            }
            if (g_type) {
                g_style_time = "old";
                g_cur_style = "org";
            }
        }

        else if (g_style_time == "new") {
            var main_css = [style_path + "css/css.css?version=" + version.web];
            var menu_css = [style_path + "css/menu.css?version=" + version.web];
            var main_js = [style_time_path + "js/main.js?version=" + version.web];
            
            if (g_type) {
                var main_html = [{
                    file: "style/simple/main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            } else {
                var main_html = [{
                    file: style_path + "main.body.htm?version=" + version.web,
                    callback: '$("body").html(ajaxdata);'
                }];
            }

            var login_css = [style_path + "css/login.css?version=" + version.web];
            var login_js = [style_time_path + "js/login.js?version=" + version.web];

            var page_css = [style_path + "css/page.css?version=" + version.web];
            //new
            var bootstrap_css = [style_path + "css/bootstrap.min.css"];
            var bootstrap_theme_css = [style_path + "css/bootstrap-theme.min.css"];
            var ie8_html5_js = ["js/html5shiv.min.js", "js/respond.min.js"];

            var login_html = [{
                file: style_path + "login.body.htm?version=" + version.web,
                callback: '$("body").html(ajaxdata);'
            }];
            var jqui_base_css = [style_path + "jq-ui/jquery-ui-1.10.3.custom.css?version=" + version.web];
            var jqui_base_js = [style_time_path + "jq-ui/jquery-ui-1.11.3.min.js?version=" + version.web];

            var dropzone_js = ["js/dropzone.js"];
            var timer_js = ["js/timer.js?version=" + version.web];

            add_js(jchs, bootstrap_js);
            add_css(jchs, bootstrap_css);
            var ie = checkIeVersion();
            if (ie.isIE && (ie.version < 9 || (ie.emulatedVersion != null && ie.emulatedVersion < 9))) {
                add_js(jchs, ie8_html5_js);
            }
            if (g_cur_style == "blue" || g_cur_style == "org") {
                add_css(jchs, bootstrap_theme_css); 
            }
            if (cur_page == "main.htm") {
                add_css(jchs, main_css);
                add_js(jchs, main_js);
                add_html(jchs, main_html);
                add_js(jchs, easy_tab_js);
                add_js(jchs, jq_easing_js);
                add_js(jchs, sidebar_menu_js);
            }
            else if (cur_page == "PlatePictureView.htm") {
                add_css(jchs, jqui_base_css);
                add_css(jchs, page_css);
                add_js(jchs, jq_easing_js);
                add_js(jchs, file_tree_js);
                add_css(jchs, file_tree_css);
                add_js_wait_last(jchs, jqui_base_js);
                add_js(jchs, customui_js);
            }
            else {
                if (cur_page == "login.htm") {                    
                    add_html(jchs, login_html);
                    if (g_cur_style == "blue") {
                        add_css(jchs, jqui_base_css);
                        add_js_wait_last(jchs, jqui_base_js);
                        add_js(jchs, customui_js);
                    }
                    add_css(jchs, login_css);
                    add_js(jchs, login_js);
                }
                else {
                    add_css(jchs, jqui_base_css);
                    add_css(jchs, page_css);
                    add_js_wait_last(jchs, jqui_base_js);
                    add_js(jchs, easy_tab_js);
                    add_js(jchs, customui_js);
                }
            }
            if (cur_page == "SetDateTime.htm" || cur_page == "PeopleCountResult.htm") {
                add_js(jchs, datepicker_zhCN_js);
            }
            if (cur_page == "DeviceGroup.htm") {
                add_css(jchs, common_css);
                add_css(jchs, ztree_css);
            }
            if (cur_page == "SetDeviceAudio.htm") {
                add_js(jchs, dropzone_js);
            }
            if (cur_page == "SetOfflineCharge.htm") {
                add_js_wait_last(jchs, timer_js);
            }
            if (g_type) {
                g_style_time = "old";
                g_cur_style = "org";
            }
        }

        else if (g_style_time == "sfm") {
            var main_css = [style_path + "css.css?version=" + version.web];
            var menu_css = [style_path + "menu.css?version=" + version.web];
            var main_js = [style_time_path + "js/main.js?version=" + version.web];
            var main_html = [{
                file: style_time_path + "main.body.htm?version=" + version.web,
                callback: '$("body").html(ajaxdata);'
            }];

            var login_css = [style_path + "login.css?version=" + version.web];
            var login_js = [style_time_path + "js/login.js?version=" + version.web];

            var page_css = [style_path + "page.css?version=" + version.web];
            var check_css = [style_path + "check/check.css?version=" + version.web];

            var login_html = [{
                file: style_time_path + "login.body.htm?version=" + version.web,
                callback: '$("body").html(ajaxdata);'
            }];
            var jqui_base_css = [style_path + "jq-ui/jquery-ui-1.11.3.custom.min.css?version=" + version.web];
            var jqui_base_js = [style_time_path + "jq-ui/jquery-ui-1.11.3.custom.min.js?version=" + version.web];
            var jqui_check_js = [style_time_path + "jq-ui/check/ui.checkbox.js"];
            //var jqui_select_js = [style_time_path + "jq-ui/selectmenu/ui.selectmenu.js"];
            //var jqui_select_new_js = [style_time_path + "jq-ui/selectmenu/jquery.ui.selectmenu.js"];

            //var jqui_select_css = [style_time_path + "jq-ui/selectmenu/ui.selectmenu.css"];
            //var jqui_select_new_css = [style_time_path + "jq-ui/selectmenu/jquery.ui.selectmenu.css"];

            if (cur_page == "main.htm") {
                add_css(jchs, main_css);
                add_css(jchs, menu_css);
                add_js(jchs, main_js);
                add_html(jchs, main_html);
                add_js(jchs, jq_easing_js);
                add_js(jchs, sidebar_menu_js);
            }
            else if (cur_page == "PlatePictureView.htm") {
                add_css(jchs, page_css);
                add_js(jchs, jq_easing_js);
                add_js(jchs, file_tree_js);
                add_css(jchs, file_tree_css);
            }
            else {
                add_css(jchs, jqui_base_css);
                add_css(jchs, page_css);
                add_css(jchs, check_css);
                if (cur_page == "login.htm") {
                    add_js(jchs, login_js);
                    add_html(jchs, login_html);
                    add_js_wait_last(jchs, jqui_base_js);
                }
                else if (cur_page == "SetNetPort.htm") {
                    //add_css(jchs, jqui_select_css);
                    add_js_wait_last(jchs, jqui_base_js);
                    //add_js(jchs, jqui_select_js);                   
                }
                else {
                    //add_css(jchs, jqui_select_new_css);
                    add_js_wait_last(jchs, jqui_base_js);
                    //add_js(jchs, jqui_select_new_js);
                }
                if (cur_page == "SetDateTime.htm" || cur_page == "PeopleCountResult.htm") {
                    add_js(jchs, datepicker_zhCN_js);
                }
                if (cur_page == "DeviceGroup.htm") {
                    add_css(jchs, common_css);
                    add_css(jchs, ztree_css);
                }
                add_js_wait_last(jchs, jqui_check_js);
                add_js(jchs, customui_js);
            }
        }
        $init_js_or_css_or_html(jchs, function () {
            if (callback != undefined) callback();
        });
    });
}

function init_style(callback) {
    load_style(function () {
        $("body").show();
        if (callback != undefined) callback();
    });
}

function init_title() {
    get_device_type();
    if (g_device_type == 11 && g_cur_style == "blue") {
        $("#device_name").text("百万高清识别一体机");
        document.title = "百万高清识别一体机"; //兼容ie8
    }
    else if (g_device_type == 11 && g_style_time == "hrzx" && g_cur_style == "org") {
        document.title = "车牌识别一体机";
    }
    else {
        //$("title").text("IPNC");
        document.title = "IPNC"; //兼容ie8
    }
}