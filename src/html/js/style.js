function get_css_value(e, property) {
    var val = parseInt(e.css(property).split("px")[0]);
    if (isNaN(val)) val = 0;
    return val;
}
function get_boundary(e, with_margin) {
    var padding_top = get_css_value(e, "padding-top");
    var padding_bottom = get_css_value(e, "padding-bottom");
    var padding_left = get_css_value(e, "padding-left");
    var padding_right = get_css_value(e, "padding-right");
    var margin_top = get_css_value(e, "margin-top");
    var margin_bottom = get_css_value(e, "margin-bottom");
    var margin_left = get_css_value(e, "margin-left");
    var margin_right = get_css_value(e, "margin-right");
    var w = padding_left + padding_right;
    var h = padding_top + padding_bottom;
    if (with_margin) {
        w += margin_left + margin_right;
        h += margin_top + margin_bottom;
    }
    return {
        w: w,
        h: h
    };
}
function load_style(callback) {
    var ie = checkIeVersion();
    var cur_page = get_cur_page();
    //css
    var animate_css = [ "../css/animate.css"];
    var bootstrap_css = ["../css/bootstrap.min.css"];
    var font_css = ["../font-awesome/css/font-awesome.css"];
    var style_css = ["../css/style.css?version=" + version.web];
    var check_css = ["../css/plugins/iCheck/custom.css?version=" + version.web];
    var select_theme_css = ["../css/plugins/selectordie/selectordie_theme.css?version=" + version.web];
    var datetimepicker_css = ["../css/datetimepicker.css?version=" + version.web];
    var rangeSlider_css = ["../css/plugins/ionRangeSlider/ion.rangeSlider.css?version=" + version.web];
    var rangeSlider_skin_css = ["../css/plugins/ionRangeSlider/ion.rangeSlider.skinFlat.css?version=" + version.web];
    var toastr_css = ["../css/plugins/toastr/toastr.min.css?version=" + version.web];
    var page_css = ["../css/page.css?version=" + version.web];

    //js
    var bootstrap_js = ["../js/bootstrap.min.js"];
    var ie8_html5_js = ["../js/html5shiv.min.js","../js/respond.min.js"];
    var check_js = ["../js/icheck.min.js"];
    var select_js = ["../js/selectordie.min.js"];
    var datetimepicker_js = ["../js/datetimepicker.js"];
    var datetimepicker_ch_js = ["../js/datetimepicker.zh-CN.js"];
    var rangeSlider_js = ["../js/ion.rangeSlider.min.js"];
    var toastr_js = ["../js/toastr.min.js"];

    var jchs = Array();
    add_js(jchs, bootstrap_js);
    add_css(jchs, bootstrap_css);
    add_css(jchs, toastr_css);
    add_css(jchs, style_css);
    add_css(jchs, font_css);
    add_css(jchs, page_css);
    add_css(jchs, animate_css);
    add_js(jchs, toastr_js);
    if(cur_page == "Main.html"){
        add_css(jchs, check_css);
        add_css(jchs, select_theme_css);
        add_js(jchs, check_js);
        add_js(jchs, select_js);
        add_css(jchs, datetimepicker_css);
        add_js_wait_last(jchs, datetimepicker_js);
        add_js(jchs, datetimepicker_ch_js);
        add_css(jchs, rangeSlider_css);
        add_css(jchs, rangeSlider_skin_css);
        add_js(jchs, rangeSlider_js);
    }
    if (ie.isIE && (ie.version < 9 || (ie.emulatedVersion != null && ie.emulatedVersion < 9))) {
        add_js(jchs, ie8_html5_js);
    }
    $init_js_or_css_or_html(jchs, function () {
        if (callback != undefined) callback();
    });
}

function init_style(callback) {
    load_style(function () {
        $("body").show();
        if (callback != undefined) callback();
    });
}