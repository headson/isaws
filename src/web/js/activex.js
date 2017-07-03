

//是否使用新版控件
var g_use_new_ax = true;

function GetAX(name)
{
    if (window.document[name])
    {
        return window.document[name];
    }
    if (navigator.appName.indexOf("Microsoft Internet")==-1)
    {
        if (document.embeds && document.embeds[name])
            return document.embeds[name];
    }
    else // if (navigator.appName.indexOf("Microsoft Internet")!=-1)
    {
        return document.getElementById(name);
    }
}

function FitSize(act)
{
    
    var axwidth = $("#live").width();
    var axheight = $("#live").height();
   
    act.SetSize(axwidth,axheight);
}

var g_active_version = 0; //0 1channel ,1 2channel,2 onvif

function activex_begin(controlstyle,show_sesid)
{
    var ax = GetAX("ax");
    if(ax)
    {
        var outtext = $.cookie('outtext');
        var sesID = $.cookie('sessionID');
        var rtsp_port = $.cookie('rtsp_port');
        if(rtsp_port==null)rtsp_port=0;
        var num_rtsp_port = parseInt(rtsp_port);
        if(isNaN(num_rtsp_port) || num_rtsp_port <0 || num_rtsp_port > (2<<15)-1)rtsp_port=0;
        if(show_sesid)$("#show_sesID").html(sesID);
        var port = location.port;
        if(port == 0) port = 80;
        passwd = "天天";
        var postdata = AesCtr.decrypt(outtext,passwd,128);
        postdata = postdata.split(':');
        ax.SetControlStyle(controlstyle);
        if(g_active_version == 2){
            ax.SetConnectPara(location.hostname, port, postdata[0], postdata[1],"1");
        }
        else{
            ax.SetConnectParaEx(location.hostname, port, postdata[0], postdata[1],sesID,rtsp_port);
        }
		FitSize(ax);
    }
}

var g_controlstyle = 0;
function activex_play(channel){
    var ax = GetAX("ax");
    if(ax)
    {
        if(g_active_version == 2 ){
            var stream = channel;
            //通道是0
            if(g_controlstyle==0){
	            ax.Play(0,0);
	        }
	        else{
	            ax.Play(0,stream);
	        }
        }
        else if(g_active_version == 1){
            if(g_controlstyle==0){
		        ax.Play(0);
	        }
	        else{
	            ax.Play(channel);
	        }
        }
        else if(g_active_version == 0){
		    ax.Play();
		}
	}
}

var active_1_name = "IPNCC.IPNCCCtrl.1";
var active_1_version = "1.0.6.9";
var active_2_name = "IPNCC.IPNCCCtrl.1";
var active_2_version = "1.0.7.5";
var active_3_name = "IPNCCONVIF.IPNCCCtrl.1";
var active_3_version = "1.0.7.8";

var active_1_channel_clsid = "clsid:05534009-E4EF-4475-92FD-1D6BD65BE774";
var active_1_channel_codebase = "/htmldata/VZPlayer.cab#version=1,0,6,9";

var active_2_channel_clsid = "clsid:05534009-D3D4-4475-92FD-1D6BD65BE774";
var active_2_channel_codebase = "/htmldata/VZPlayerex.cab#version=1,0,7,5";

var active_onvif_clsid = "clsid:5657F1CD-2679-4b44-9E71-E26983A1DD5C";
var active_onvif_codebase = "/htmldata/VZPlayerOnvif.cab#version=1,0,7,8";

//ie11,window.ActiveXObject not work try this code to detect,as far as I know, the control need to implement IsVersionSupported to support version detection
function activex_version_supported(name,version) {
    if (version == undefined){
        version = null;
    }
    var isVersionSupported = false;
    try{
        var tryOlderIE = false;

        // Detect installation on IE11 and non-IE browsers
        try {
            var plugin = navigator.plugins[name];
            if (plugin){
                // Plugin object exists - check for version argument
                if (version === null){
                    // no string to parse - plugin is installed
                    isVersionSupported = true;
                }
                else{
                    // there is a string to parse - check if requested version is installed
                    var actualVerArray = actualVer.split(".");
                    while (actualVerArray.length > 3){
                        actualVerArray.pop();
                    }

                    while (actualVerArray.length < 4){
                        actualVerArray.push(0);
                    }

                    var reqVerArray = version.split(".");
                    while (reqVerArray.length > 4){
                        reqVerArray.pop();
                    }

                    var requiredVersionPart;
                    var actualVersionPart;
                    var index = 0;

                    do{
                        requiredVersionPart = parseInt(reqVerArray[index]);
                        actualVersionPart = parseInt(actualVerArray[index]);
                        index++;
                    }
                    while (index < reqVerArray.length && requiredVersionPart === actualVersionPart);

                    if (requiredVersionPart <= actualVersionPart && !isNaN(requiredVersionPart)){
                        isVersionSupported = true;
                    }
                }
            }
            else{
                // Plugin object does not exist - get ready to try the IE detection approach.
                tryOlderIE = true;
            }
        }
        catch (e){
            // Exception was thrown while checking for plugin object and version - get ready to try the IE detection approach.
            tryOlderIE = true;
        }

        if (tryOlderIE){
            // Detect installation on IE10 and earlier IE browsers via ActiveXObject
            var control = new ActiveXObject(name);
            if (version === null){
                isVersionSupported = true;
            }
            else if (control.IsVersionSupported(version)){
                isVersionSupported = true;
            }
            control = null;
        }
    }
    catch (e){
        isVersionSupported = false;
    }

    return isVersionSupported;
}

function init_activex(id,page,show_sesid,callback){
    get_device_support_onvif();
    get_device_support_channels();
    get_device_type();
    var clsid;
    var code_base;
    
    var active_name_string = "";
    var active_version_string = "";
    
    //page:0 LiveVideo,1 SetVideo,2 SetOnvifServer
    if(page == 2) g_controlstyle = 1;
    else g_controlstyle = page;
    
    if(g_device_type == -1){
        //工厂版本只在onvif设置页面播放onvif控件
        if(g_support_onvif=="1" && page == 2){
            g_active_version = 2;
        }
        else if(g_channelnum==1){
            g_active_version = 0;
        }
        else if(g_channelnum==2){
            g_active_version = 1;
        }
        else {
            g_active_version = 0;
        }
    }
    else{
        if(g_support_onvif=="1"){
            g_active_version = 2;
        }
        else if(g_channelnum==1){
            g_active_version = 0;
        }
        else if(g_channelnum==2){
            g_active_version = 1;
        }
        else {
            g_active_version = 0;
        }
    }
    if(g_active_version==2){
        clsid = active_onvif_clsid;
        code_base = active_onvif_codebase;
        active_name_string = active_3_name;
        active_version_string = active_3_version;
    }
    else if(g_active_version==0){
        clsid = active_1_channel_clsid;
        code_base = active_1_channel_codebase;
        active_name_string = active_1_name;
        active_version_string = active_1_version;
    }
    else if(g_active_version==1){
        clsid = active_2_channel_clsid;
        code_base = active_2_channel_codebase;
        active_name_string = active_2_name;
        active_version_string = active_2_version;
    }
//    if(!activex_version_supported(active_name_string)){
//        alert("active not supported");
//        return;
//    }
//    if(!activex_version_supported(active_name_string,active_version_string)){
//        alert("need install new version active");
//    }
    var active_obj = "<object id=\"ax\" classid=\"" + clsid + "\"" +
                            " codebase=\"" + code_base + "\"" +
                            " width=\"100%\"" +
                            " height=\"100%\"" +
                            " >" +
                            " </object>";
    $(id).html(active_obj);
    activex_begin(g_controlstyle,show_sesid);
}

//because of this, padding of #live will be uneffectable
function ax_fit_size_new(id)
{
    var ax = GetAX("ax");
    if(!ax) return;
    ax.width = $(id).width();
    ax.height = $(id).height();
}

function comp_version(ver1, ver2) {
    if (ver1[0] > ver2[0])  return 1;
    else if(ver1[0] < ver2[0]) return -1;
    else{
        if (ver1[1] > ver2[1]) return 1;
        else if (ver1[1] < ver2[1]) return -1;
        else {
            if (ver1[2] > ver2[2]) return 1;
            else if (ver1[2] < ver2[2]) return -1;
            else {
                if (ver1[3] > ver2[3]) return 1;
                else if (ver1[3] < ver2[3]) return -1;
                else {
                    return 0;
                }
            }
        }
    }
}

function activex_begin_new(id,controlstyle,show_sesid,vs_type,callback)
{
    var manual_download = "<a href=\"/htmldata/PlayCtrl.msi\" style=\"font-size:18px;margin:auto;color:blue;\">如果下载没有开始，点击这里手动下载控件</a>";
    try {
        var ax = GetAX("ax");
        if (!ax) return;
        var installed_version = ax.GetVersion();
        var cur_version = version.active;
        installed_version = installed_version.split('.');
        cur_version = cur_version.split('.');
        if (comp_version(installed_version,cur_version) < 0 ){
            alert("控件有更新，请安装新版本控件");
            window.location.href = "/htmldata/PlayCtrl.msi";
            $(id).html(manual_download);
            return;
        }
        if (g_cur_style == "blue") {
            ax.SetSkin(1);
        }
        else if (g_style_time == "sfm") {
            ax.SetSkin(2);
        }
        else {
            //org
            ax.SetSkin(0);
        }
        ax.SetControlStyle(controlstyle);
        ax.SetVideoServerType(vs_type);
    }
    catch (e) {
        if(!confirm("没有安装控件？点击确定开始安装控件,安装时请关闭浏览器;如果已安装请点击取消并允许控件运行")){
            return;
        }
        else{
            window.location.href = "/htmldata/PlayCtrl.msi";
            $(id).html(manual_download);
            return;
        }
    }

    window.onresize();
        
    var outtext = $.cookie('outtext');
    var sesID = $.cookie('sessionID');
    var rtsp_port = $.cookie('rtsp_port');
    if(rtsp_port==null)rtsp_port=0;
    var num_rtsp_port = parseInt(rtsp_port);
    if(isNaN(num_rtsp_port) || num_rtsp_port <0 || num_rtsp_port > (2<<15)-1)rtsp_port=0;
    if(show_sesid)$("#show_sesID").html(sesID);
    var port = parseInt(location.port);
    if(isNaN(port) || port == 0) port = 80;
    passwd = "天天";
    var postdata = AesCtr.decrypt(outtext,passwd,128);
    postdata = postdata.split(':');
    var req = {REQ_Login:
                 { 
                    ip:location.hostname,
                    port:port,
                    rtspPort:0,
                    userName: postdata[0],
                    passWd:postdata[1],
                    needAuth:1,
                    sesID:sesID
                 }
              };
    var encoded = $.toJSON( req );
    ax.QueryCmd("Login",encoded,1,function(response_data){
        if(response_data.match(/^NG/) != null){
            alert("登录失败");
            return;
        }
        if(callback != undefined) callback();
    });       
}


function activex_play_new(channel,stream,callback){
    var ax = GetAX("ax");
    if(!ax) return;
    
    var req = {REQ_RealPlay:
                 { 
                    channel:channel,
                    stream:stream
                 }
              };
    var encoded = $.toJSON( req );
    ax.QueryCmd("RealPlay",encoded,1,function(response_data){
        if(response_data.match(/^NG/) != null){
            alert("播放视频失败");
            return;
        }
        if(callback != undefined)callback();
    });
}

function get_css_value(e, property) {
    var val = parseInt($(e).css(property).split("px")[0]);
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

function get_size_availble() {
    var w = document.documentElement.clientWidth;
    var h = document.documentElement.clientHeight;
    var cur_page = get_cur_page();
    var body_boundary = get_boundary("body", true);
    w -= body_boundary.w;
    h -= body_boundary.h;
    if (cur_page == "IvsConfig.htm" || cur_page == "LiveVideo.htm") {
        var live_get_boundary = get_boundary("#live", false);
        //w -= live_boundary.w;
        //h -= live_boundary.h;
    }
    else {
        var left_boundary = get_boundary("#left_container", false);
        if(g_style_time == "sfm"){
            w = w - 500 - left_boundary.w - 40;
        } else {
            w = w - $("#left_container").width() - left_boundary.w - 40;
        }
    }
    if (g_style_time == "old" && cur_page == "IvsConfig.htm") {
        var right_boundary = get_boundary("#ivs_parameter", false);
        w = w - $("#ivs_parameter").width() - right_boundary.w - 10;
    }
    return {
        w: w,
        h: h
    };
}

//keep as a note
//    var height = document.documentElement.clientHeight - top_bar_height;
//    var width = Math.floor(height/y_rate*x_rate + side_bar_width);
//    var window_width = document.documentElement.clientWidth - 40;

//    if(width > window_width) {
//        width = window_width;
//        height = Math.floor((width - side_bar_width)/x_rate*y_rate) + top_bar_height;
//    }
//    var left = 20;
//    if( (window_width - width) > 51){
//        left += (window_width - width)/2 - 25;
//    }
function fit_window_size(func_get_size_availble, x_rate, y_rate, side_bar_width, top_bar_height) {
    var cur_page = get_cur_page();
    var size_availble = func_get_size_availble();
    var height = size_availble.h - top_bar_height;
    var width = Math.floor(height / y_rate * x_rate + side_bar_width);

    var window_width = size_availble.w;
    if (g_style_time == "sfm" && cur_page == "IvsConfig.htm") {
        window_width -= 260;
    }

    if (width > window_width) {
        width = window_width;
        height = Math.floor((width - side_bar_width) / x_rate * y_rate) + top_bar_height;
    }
    else {
        height += top_bar_height;
    }
    var left = (window_width - width) / 2;
    left = Math.round(left);
    $("#live").css("margin-left", left + "px");
    if (g_style_time == "sfm" && cur_page == "IvsConfig.htm") {
        $("#live").css("margin-left","0");
    }
    if ($("#live").height() != height) {
        $("#live").height(height - 1);
    }

    if ($("#live").width() != width) {
        $("#live").width(width);
    }
    //alert(width+"x"+height);

    if (!g_use_new_ax) {
        var ax = GetAX("ax");
        if (ax) {
            FitSize(ax);
        }
    }
    else {
        ax_fit_size_new("#live");
    }
}

function init_window_size() {
    get_device_type();
    var x_rate = 11;
    var y_rate = 9;
    var side_bar_width = 0;
    var top_bar_height = 0;
    var cur_page = get_cur_page();
    if (cur_page == "IvsConfig.htm") {
        side_bar_width = 200;
        top_bar_height = 40;
        if(g_style_time == "old"){
            side_bar_width = 0;
        }
    }
    if (g_device_type == 11) {
        x_rate = 16;
        if (cur_page == "LiveVideo.htm") {
            side_bar_width = 10;
            top_bar_height = 50;
        }
        if (g_style_time == "sfm" && cur_page == "IvsConfig.htm") {
            x_rate = 5;
            y_rate = 5;
        }
    }
    else {
        if (cur_page == "LiveVideo.htm") {
            side_bar_width = 160;
            top_bar_height = 0;
        }
    }
    window.onresize = function () {
        fit_window_size(get_size_availble, x_rate, y_rate, side_bar_width, top_bar_height);
        if (cur_page == "IvsConfig.htm" && g_style_time == "old") {
            var ax_height = $("#live").height() - 42;
            var focus_con_h = $("#focus_container").height();
            var led_con_h = $("#led_container").height();
            var margin = ax_height - focus_con_h - led_con_h - 10;
            if(margin < 0){
                margin = 0;
            }
            var height = ax_height;
            if (height < (focus_con_h + led_con_h)) {
                height = focus_con_h + led_con_h;
            }
            $("#focus_container").css({ "margin-bottom": margin });
            $("#ivs_parameter").css({ "height": height});
        }
    };
}

function init_activex_new(id, page, show_sesid, callback) {
    init_window_size();
    var active_new_name = "VATLAX.PlayCtrl.2";
    var active_new_clsid = "clsid:BE21934E-8B2F-404B-87CC-4D40A848C784";
    var active_new_codebase_version = version.active.replace(/\./g, ',');
    var active_new_codebase = "/htmldata/VZPlayer2.cab#version=" + active_new_codebase_version;

    get_device_support_onvif();
    get_device_support_channels();
    get_device_type();
    var clsid;
    var code_base;
    
    var active_name_string = "";
    var active_version_string = "";
    
     //page:0 LiveVideo,1 SetVideo,2 SetOnvifServer,3IvsConfig,4 Replay
    if(page == 0) {
        if(g_device_type == 11){
            g_controlstyle = 4;
        }
        else if (g_device_type == 18) {
            g_controlstyle = 5;
        }
        else{
            g_controlstyle = 0;
        }
    }
    else if(page == 1 || page == 2)  g_controlstyle = 1;
    else if(page == 3) g_controlstyle = 2;
    else if (page == 4) g_controlstyle = 3;

    if (g_style_time == "sfm") {
        g_controlstyle += 10;
    }
    if (g_style_time == "hrzx" && g_controlstyle == 2) {
        g_controlstyle = 32;
    }
    if (g_style_time == "old" && g_controlstyle == 2) {
        g_controlstyle = 22;
    }
    g_active_version = 3;
    clsid = active_new_clsid;
    code_base = active_new_codebase;
    active_name_string = active_new_name;
    active_version_string = version.active;
    
    //if(!activex_version_supported(active_name_string)){
    //    alert("active not supported");
    //    return;
    //}
    //if(!activex_version_supported(active_name_string,active_version_string)){
    //    alert("need install new version active");
    //}

    //with code base
    //var active_obj = "<object id=\"ax\" classid=\"" + clsid + "\"" +
    //                        " codebase=\"" + code_base + "\"" +
    //                        " width=\"100%\"" +
    //                        " height=\"100%\"" +
    //                        " >" +
    //                        " </object>";

    var active_obj = "<object id=\"ax\" classid=\"" + clsid + "\"" +
                            " width=\"100%\"" +
                            " height=\"100%\"" +
                            " >" +
                            " </object>";
    $(id).html(active_obj);
    //1 onvif,0 vz,2 plate
    var vstype = 0;
    if (g_device_type == 11 || g_device_type == 18) {
         vstype = 2;
    }
    else if(g_device_type == -1 ){
        if(g_support_onvif=="1" && page == 2){
            vstype = 1;
        }
    }
    else{
        if(g_support_onvif=="1")  vstype = 1;
    }
    activex_begin_new(id,g_controlstyle,show_sesid,vstype,callback);
}
