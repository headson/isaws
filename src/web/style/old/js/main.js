   
var onvif_li_html = "<li><a href=\"javascript:void(0);\" onclick=\"onLinkClick('htmldata/SetChannel.htm')\">通道管理</a></li>";

var onvif_li_old_html = "<li><a href=\"javascript:void(0);\" onclick=\"onLinkClick('htmldata/SetChannelOld.htm')\">通道管理</a></li>";
            
var hj212_li_html = "<li><a href=\"javascript:void(0);\" onclick=\"onLinkClick('htmldata/SetHj212.htm')\">环境监控</a></li>";
        
var people_li_html = "<li id=\"people_count_result\"><a href=\"javascript:void(0);\" onclick=\"onLinkClick('htmldata/PeopleCountResult.htm')\">人数统计结果</a></li>";                                            

function get_is_hj_212_version(){
    get_device_type();
           
}
        
function version_setup(){
    get_device_type();
			
	if(g_device_type != 11)//不是一体机
	{
	    $("#installconfig").remove();
	}
						
    if(g_device_type == -1){
            $("#menu li ul li:contains('报警')").remove();
            $("#menu li ul li:contains('录像')").remove();
            $("#menu li ul li:contains('云台')").remove();
            $("#menu li:contains('智能分析')").remove();
            $("#device_io").remove();
            $("#menu li:contains('用户管理')").remove();
			$("#set_digital_video").remove();		 
    }
    else if(g_device_type == 14){
        $("#device_io").after(hj212_li_html);
    }
    if(g_device_type == 0 || g_device_type == 1){
            $("#menu li ul li:contains('智能分析')").remove();
    }
    if(g_device_type == 16){
		$("#setrule").after(people_li_html);
    }
    get_device_support_onvif();
			
	if(g_device_type == 11)//一体机
	{
		$("#set_digital_video").hide();
		$("#set_record").hide();
		$("#setrule").hide();
		$("#device_io > a").attr("onclick", "onLinkClick('htmldata/SetPlateDeviceIO.htm')");
		$("#set_ptz").hide();
	}
	else if (g_device_type == 18) {//车位引导
	    $("#set_digital_video").hide();
	    $("#set_ptz").hide();
	}
	else {
	    $("#plateview").hide();
	    if (g_support_onvif == "1") {
	        if (g_device_type != -1) {
	            $("#set_video").hide();
	            if (g_device_type == 3) {
	                //气田6446，暂时去掉一些页面
	                $("#set_record").hide();
	                $("#set_storage").hide();
	                $("#menu li ul li:contains('网络')").after(onvif_li_old_html);
	            }
                else{
                    $("#menu li ul li:contains('网络')").after(onvif_li_html);
	            }	            
	        }
	        else {
	            $("#set_digital_video").text("数字视频");
	        }
	    }
	    else {
	        $("#set_digital_video").hide();
	    }
	}
	init_title();
}

function logout(){
    $.get("logout.php",function(ajaxdata){
        top.location.href = "login.htm";
    });
}
    function fit_window_size(){
    var height_all =  document.documentElement.clientHeight -45;
    if(height_all < 569) {
        height_all = 569;
    }
    $("#content").height(height_all-$("#footshadow").height()-$("#header").height()-$("headershadow").height()-40);
    //compatible with chrome
    $("#footshadow_center").width($("#pagecontainer").width());
}
function init_window_size(){
    fit_window_size();
    window.onresize = fit_window_size;
}

function init_main(callback) {
    $(function () {
        init_window_size();
        //--去除虚线框--
        $("a").focus(function () { $(this).blur(); });
        //退出登录
        version_setup();
        $.get("vb.htm?getauth", function (ajaxdata) {
            if (precheck(ajaxdata)) {
                return false;
            }
            var auth = parse_ajax_data(ajaxdata);
            if (auth == 0)//管理员
            {
            }
            else if (auth == 1)//操作员
            {
                $("#menu li:contains('用户管理')").remove();
            }
            else//观察员
            {
                $("#menu li:gt(0)").remove();
            }
        });
        $("#logout").click(logout);
        $("#mainFrame")[0].src = "htmldata/LiveVideo.htm";
    });
    if (callback != undefined) callback();
}