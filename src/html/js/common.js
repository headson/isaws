var g_need_relogin = false;
//检查超时
function precheck(ajaxdata) {
    if (g_need_relogin) return false;
    if (ajaxdata.length > 0 && ajaxdata[0] == '<') {
        g_need_relogin = true;
        show_informer_text("登录超时，请重新登录");
        location.href = "Login.html";
        return true;
    }
    return false;
}
//处理响应
function default_ajax_handler(ajaxdata) {
    if (precheck(ajaxdata)) {
        return false;
    }
    var json = eval("("+ajaxdata+")");
    if (json.state == 200) {
        show_informer();
        return true;
    }
    else {
        show_informer_text("保存失败");
        return false;
    }
}
//初始化下拉菜单
function init_select(select,width,change_fun) {
    if (change_fun != undefined) {
        $(select).selectOrDie({
            cycle: true,
            size:8,
            onChange: function () { change_fun($(this)) }
        });
    }else{
        $(select).selectOrDie({
            cycle: true,
            size:8
        });
    }
    if(width != undefined){
        var parent = $(select).parents(".sod_select");
        parent.width(width);
        parent.find(".sod_list").width(width + 16);
    }
}

jQuery.fn.select_val = function (value) {
    if (arguments.length) {
        if (this.children('option[value="' + value + '"]').length > 0) {
            this.val(value);
            var parent = this.parents(".sod_select");
            parent.find(".selected, .sod_placeholder").removeClass("selected active sod_placeholder");
            this.children('option[value="' + value + '"]').attr("selected","selected");
            var index = this.children('option[value="' + value + '"]').index();
            var this_li = parent.find(".sod_list li")[index];
            parent.find(".sod_label").html($(this_li).html());
            $(this_li).addClass("selected active");
        }
    }
    else {
        return this.val();
    }
}

jQuery.fn.select_disable = function () {
    this.selectOrDie("disable");
}

jQuery.fn.select_enable = function () {
    this.selectOrDie("enable");
}

function init_checkbox(check) {
    $(check).iCheck({
        checkboxClass: 'icheckbox_square-green',
        radioClass: 'iradio_square-green',
    });
}

jQuery.fn.check_val = function (value) {
    if (arguments.length) {
        if(value){
            this.iCheck('check');
        }else{
            this.iCheck('uncheck');
        }
    }
    else {
        return this.is(':checked');
    }
}

jQuery.fn.checkbox_disable = function () {
    this.iCheck('disable');
}

jQuery.fn.checkbox_enable = function () {
    this.iCheck('enable');
}

function init_element(){
    //--去除虚线框--
    $("a").focus(function () { $(this).blur(); });
    $("input:text,input:password").addClass("text");
    $("button, input:submit, input:button").addClass("btn btn-sm btn-w-m btn-primary");
    init_checkbox('input[type=checkbox],input[type=radio]');
    $('.datepicker').datetimepicker({
        language:"zh-CN",
        fontAwesome:true,
        timezone:"中国标准时间",
        autoclose:true
    });
    $('.datepicker_day').datetimepicker({
        language:"zh-CN",
        fontAwesome:true,
        format:"yyyy/mm/dd",
        minView:2,
        timezone:"中国标准时间",
        autoclose:true
    });
}
//var inform_auto_hide_timer=0;
////隐藏提示
//function hide_informer(){
//    $(".inform").html("");
//    $(".inform").hide();
//    if(inform_auto_hide_timer!=0){
//        clearTimeout(inform_auto_hide_timer);
//        inform_auto_hide_timer=0;
//    }
//}
//
//function set_inform_auto_hide(){
//    if(inform_auto_hide_timer == 0) {
//        inform_auto_hide_timer = setTimeout(hide_informer,1000);
//    }
//}
////显示提示
//function show_informer(){
//    $(".inform").html("已设置");
//    $(".inform").show();
//    set_inform_auto_hide();
//}
////显示相应的提示
//function show_informer_text(text){
//    $(".inform").html("text");
//    $(".inform").show();
//    set_inform_auto_hide();
//}
function show_informer_text(text,time){
    var t = "3000";
    if(time){
        t = time;
    }
    toastr.options = {
        closeButton: true,
        debug: true,
        progressBar: true,
        positionClass: "toast-top-right",
        onclick: null,
        showDuration: "300",
        hideDuration: "1000",
        timeOut: t,
        extendedTimeOut: t,
        showEasing: "swing",
        hideEasing: "linear",
        showMethod: "slideDown",
        hideMethod: "slideUp"
    };
    toastr.error(text,'提示');
    addClass(".animated","shake");
}
function show_informer(txt){
    var t = "保存成功";
    if(txt){
        t = txt;
    }
    toastr.options = {
        closeButton: true,
        debug: true,
        progressBar: true,
        positionClass: "toast-top-right",
        onclick: null,
        showDuration: "300",
        hideDuration: "1000",
        timeOut: "1000",
        extendedTimeOut: "1000",
        showEasing: "swing",
        hideEasing: "linear",
        showMethod: "slideDown",
        hideMethod: "slideUp"
    };
    toastr.success(t,'提示');
}
function addClass(e,c){
    if($(e).hasClass("fadeInRight")){
        $(e).removeClass("fadeInRight");
    }
    $(e).addClass(c);
    setTimeout(function(){
        $(e).removeClass(c);
    },1000)
}
//Base64
var Base64 = {};  // Base64 namespace

Base64.code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/**
 * Encode string into Base64, as defined by RFC 4648 [http://tools.ietf.org/html/rfc4648]
 * (instance method extending String object). As per RFC 4648, no newlines are added.
 *
 * @param {String} str The string to be encoded as base-64
 * @param {Boolean} [utf8encode=false] Flag to indicate whether str is Unicode string to be encoded
 *   to UTF8 before conversion to base64; otherwise string is assumed to be 8-bit characters
 * @returns {String} Base64-encoded string
 */
Base64.encode = function(str, utf8encode) {  // http://tools.ietf.org/html/rfc4648
    utf8encode =  (typeof utf8encode == 'undefined') ? false : utf8encode;
    var o1, o2, o3, bits, h1, h2, h3, h4, e=[], pad = '', c, plain, coded;
    var b64 = Base64.code;

    plain = utf8encode ? Utf8.encode(str) : str;

    c = plain.length % 3;  // pad string to length of multiple of 3
    if (c > 0) { while (c++ < 3) { pad += '='; plain += '\0'; } }
    // note: doing padding here saves us doing special-case packing for trailing 1 or 2 chars

    for (c=0; c<plain.length; c+=3) {  // pack three octets into four hexets
        o1 = plain.charCodeAt(c);
        o2 = plain.charCodeAt(c+1);
        o3 = plain.charCodeAt(c+2);

        bits = o1<<16 | o2<<8 | o3;

        h1 = bits>>18 & 0x3f;
        h2 = bits>>12 & 0x3f;
        h3 = bits>>6 & 0x3f;
        h4 = bits & 0x3f;

        // use hextets to index into code string
        e[c/3] = b64.charAt(h1) + b64.charAt(h2) + b64.charAt(h3) + b64.charAt(h4);
    }
    coded = e.join('');  // join() is far faster than repeated string concatenation in IE

    // replace 'A's from padded nulls with '='s
    coded = coded.slice(0, coded.length-pad.length) + pad;

    return coded;
}

/**
 * Decode string from Base64, as defined by RFC 4648 [http://tools.ietf.org/html/rfc4648]
 * (instance method extending String object). As per RFC 4648, newlines are not catered for.
 *
 * @param {String} str The string to be decoded from base-64
 * @param {Boolean} [utf8decode=false] Flag to indicate whether str is Unicode string to be decoded
 *   from UTF8 after conversion from base64
 * @returns {String} decoded string
 */
Base64.decode = function(str, utf8decode) {
    utf8decode =  (typeof utf8decode == 'undefined') ? false : utf8decode;
    var o1, o2, o3, h1, h2, h3, h4, bits, d=[], plain, coded;
    var b64 = Base64.code;

    coded = utf8decode ? Utf8.decode(str) : str;

    for (var c=0; c<coded.length; c+=4) {  // unpack four hexets into three octets
        h1 = b64.indexOf(coded.charAt(c));
        h2 = b64.indexOf(coded.charAt(c+1));
        h3 = b64.indexOf(coded.charAt(c+2));
        h4 = b64.indexOf(coded.charAt(c+3));

        bits = h1<<18 | h2<<12 | h3<<6 | h4;

        o1 = bits>>>16 & 0xff;
        o2 = bits>>>8 & 0xff;
        o3 = bits & 0xff;

        d[c/4] = String.fromCharCode(o1, o2, o3);
        // check for padding
        if (h4 == 0x40) d[c/4] = String.fromCharCode(o1, o2);
        if (h3 == 0x40) d[c/4] = String.fromCharCode(o1);
    }
    plain = d.join('');  // join() is far faster than repeated string concatenation in IE

    return utf8decode ? Utf8.decode(plain) : plain;
}
//Utf8
var Utf8 = {};  // Utf8 namespace

/**
 * Encode multi-byte Unicode string into utf-8 multiple single-byte characters
 * (BMP / basic multilingual plane only)
 *
 * Chars in range U+0080 - U+07FF are encoded in 2 chars, U+0800 - U+FFFF in 3 chars
 *
 * @param {String} strUni Unicode string to be encoded as UTF-8
 * @returns {String} encoded string
 */
Utf8.encode = function(strUni) {
    // use regular expressions & String.replace callback function for better efficiency
    // than procedural approaches
    var strUtf = strUni.replace(
        /[\u0080-\u07ff]/g,  // U+0080 - U+07FF => 2 bytes 110yyyyy, 10zzzzzz
        function(c) {
            var cc = c.charCodeAt(0);
            return String.fromCharCode(0xc0 | cc>>6, 0x80 | cc&0x3f); }
    );
    strUtf = strUtf.replace(
        /[\u0800-\uffff]/g,  // U+0800 - U+FFFF => 3 bytes 1110xxxx, 10yyyyyy, 10zzzzzz
        function(c) {
            var cc = c.charCodeAt(0);
            return String.fromCharCode(0xe0 | cc>>12, 0x80 | cc>>6&0x3F, 0x80 | cc&0x3f); }
    );
    return strUtf;
}

/**
 * Decode utf-8 encoded string back into multi-byte Unicode characters
 *
 * @param {String} strUtf UTF-8 string to be decoded back to Unicode
 * @returns {String} decoded string
 */
Utf8.decode = function(strUtf) {
    var strUni = strUtf.replace(
        /[\u00c0-\u00df][\u0080-\u00bf]/g,                 // 2-byte chars
        function(c) {  // (note parentheses for precence)
            var cc = (c.charCodeAt(0)&0x1f)<<6 | c.charCodeAt(1)&0x3f;
            return String.fromCharCode(cc); }
    );
    strUni = strUni.replace(
        /[\u00e0-\u00ef][\u0080-\u00bf][\u0080-\u00bf]/g,  // 3-byte chars
        function(c) {  // (note parentheses for precence)
            var cc = ((c.charCodeAt(0)&0x0f)<<12) | ((c.charCodeAt(1)&0x3f)<<6) | ( c.charCodeAt(2)&0x3f);
            return String.fromCharCode(cc); }
    );
    return strUni;
}
//检测IP
function test_ip(ip){
    var addr = ip.split('.');
    if(addr.length != 4)return false;
    for(var i=0;i<4;i++){
        if(addr[i].length>3)return false;
        var val = parseInt(addr[i]);
        if(isNaN(val) || val<0 || val>255)return false;
    }
    return true;
}
//假定ip格式正确
function test_gateway(gateway){
    var addr3 = gateway.split('.');
    for(var i=0;i<4;i++){
        if(i==0){
            var gateway_first = parseInt(addr3[i]);
            if(gateway_first==0 || gateway_first>223){
                show_informer_text("网关第一位必须介于1-223之间！");
                return true;
            }
        }
    }
    return false
}

//假定ip格式正确
function test_ip_netmask(ip,netmask){
    var addr1 = ip.split('.');
    var addr2 = netmask.split('.');
    var val1=[0,0,0,0],val2=[0,0,0,0],val3=[0,0,0,0];
    var netmask_all_zero=true,
        zeros=true,ones=true,netmask_front=true,netmask_next_to = true;
    for(var i=0;i<4;i++){
        val1[i] = parseInt(addr1[i]);
        val2[i] = parseInt(addr2[i]);
        if(i==0){
            if(val1[i]==0 || val1[i]>223){
                show_informer_text("ip地址第一位必须介于1-223之间！");
                return true;
            }
        }
        for(var j=0;j<4;j++){
            var next = (1<<(3-j))&val2[i];
            if(netmask_front && next==0)netmask_front = false;
            else if(!netmask_front && next!=0) netmask_next_to = false;
        }
        if(val2[i]!=0) netmask_all_zero = false;
        val2[i] = (~val2[i]) & 0xff;
        val3[i] = val1[i] & val2[i];
        if(val3[i]!=0)zeros = false;
        if(val3[i]!=val2[i])ones = false;
    }
    if(netmask_all_zero){
        show_informer_text("子网掩码不能全为0！");
        return true;
    }
    if(!netmask_next_to){
        show_informer_text("子网掩码必须是相邻的！");
        return true;
    }
    if(zeros ){
        show_informer_text("ip地址和子网掩码的组合不合理，主机地址位全为0！");
        return true;
    }
    if(ones ){
        show_informer_text("ip地址和子网掩码的组合不合理！主机地址位全为1！");
        return true;
    }
    return false;
}
//检测同一网段
function test_ip_in_same_net(ip,netmask,ip2){
    var addr1 = ip.split('.');
    var addr2 = netmask.split('.');
    var addr3 = ip2.split('.');
    var val1=[0,0,0,0],val2=[0,0,0,0],val3=[0,0,0,0];
    var in_same_net = true,test_over = false;
    for(var i=0;i<4;i++){
        val1[i] = parseInt(addr1[i]);
        val2[i] = parseInt(addr2[i]);
        val3[i] = parseInt(addr3[i]);
        for(var j=0;j<4;j++){
            var bit1 = (1<<(3-j))&val1[i];
            var netmask_bit = (1<<(3-j))&val2[i];
            var bit3 = (1<<(3-j))&val3[i];
            if(netmask_bit==0){
                return true;
            }
            else if(bit1 != bit3){
                return false;
            }
        }
    }
    return true;
}
//初始化窗口大小
function init_window_size(element,callback){
    window.onresize = function(){
        setTimeout(function(){
            var width = parseInt($(element).width());
            var height = parseInt(width * 9 / 16);
            $(element).height(height);
            if(callback){
                callback();
            }
        },200)
    };
}
//初始化视频
function init_video(element,callback){
    var object_str = '<object classid="clsid:3B62E31C-B08C-4FED-83BC-6CEC8F7C1FB8" id="ax" width="100%" height="100%"></object>';
    $(element).append(object_str);
    init_window_size(element,callback);
    $(window).resize();
    play_video();
}
//获取控件
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
                return 0;
            }
        }
    }
}
//播放视频
function play_video() {
    var ie = checkIeVersion();
    if (!ie.isIE) {
        show_informer_text("当前页面含有active控件，请使用ie浏览器访问！","10000");
        return;
    }
    var manual_download = "<a href=\"/html/htmldata/RTSPStreamPlayer.msi\" style=\"font-size:18px;margin:auto;color:red;text-decoration: underline;\">如果下载没有开始，点击这里手动下载控件</a>";
    var ax = GetAX("ax");
    if(!ax){
        return;
    }
    try
    {
        var installed_version = ax.StreamClient_GetVersion();
        var cur_version = version.active;
        installed_version = installed_version.split('.');
        cur_version = cur_version.split('.');
        if (comp_version(installed_version,cur_version) < 0 ){
            show_informer_text("控件有更新，请安装新版本控件","10000");
            window.location.href = "/html/htmldata/RTSPStreamPlayer.msi";
            $("#live").html(manual_download);
            return;
        }
        var ip = location.host.split(":")[0];
        ax.StreamClient_StartPlay("rtsp://"+ip+":8557/h264");
    }
    catch(e){
        if(!confirm("没有安装控件？点击确定开始安装控件,安装时请关闭浏览器;如果已安装请点击取消并允许控件运行")){
            return;
        }
        else{
            window.location.href = "/html/htmldata/RTSPStreamPlayer.msi";
            $("#live").html(manual_download);
            return;
        }
    }
}
function trim(str){ //删除左右两端的空格
    return str.replace(/(^\s*)|(\s*$)/g, "");
}
function stop_video(){
    var ax = GetAX("ax");
    if(!ax){
        return;
    }
    try
    {
        ax.StreamClient_StopPlay();
    }
    catch(e){

    }
}

