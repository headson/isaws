Date.prototype.format = function(format) //author: meizz
{
    var o = {
        "M+" : this.getMonth()+1, //month
        "D+" : this.getDate(),    //day
        "h+" : this.getHours(),   //hour
        "m+" : this.getMinutes(), //minute
        "s+" : this.getSeconds(), //second
        "q+" : Math.floor((this.getMonth()+3)/3),  //quarter
        "S" : this.getMilliseconds() //millisecond
    }
    if(/(Y+)/.test(format)) format=format.replace(RegExp.$1,
        (this.getFullYear()+"").substr(4 - RegExp.$1.length));
    for(var k in o)if(new RegExp("("+ k +")").test(format))
        format = format.replace(RegExp.$1,
            RegExp.$1.length==1 ? o[k] :
                ("00"+ o[k]).substr((""+ o[k]).length));
    return format;
}
var SetDateTime = new function(){
    var g_systime = 0;
    function settime()
    {
        g_systime+=1000;
        var local_date = new Date();
        var ld = local_date.format("YYYY/MM/DD");
        var lt = local_date.format("hh:mm:ss");
        var sys_date = new Date(g_systime);
        var sd = sys_date.format("YYYY/MM/DD");
        var st = sys_date.format("hh:mm:ss");
        if($("#localdate").val()!=ld)
        {
            $("#localdate").val(ld);
        }
        if($("#localtime").val()!=lt)
        {
            $("#localtime").val(lt);
        }
        if($("#serverdate").val()!=sd)
        {
            $("#serverdate").val(sd);
        }
        if($("#servertime").val()!=st)
        {
            $("#servertime").val(st);
        }
    }

    function set_g_systime(ajaxdata)
    {
        if(precheck(ajaxdata))
        {
            return false;
        }
        var jsondata = eval("(" + ajaxdata + ")");
        if(jsondata.state != 200){
            show_informer_text("获取失败");
            return;
        }
        var msgdate = jsondata.body.devicetime;
        var year = msgdate.d_year;
        var month = msgdate.d_mon - 1;
        var date = msgdate.d_day;
        var hours = msgdate.d_hour;
        var minutes = msgdate.d_min;
        var seconds = msgdate.d_sec;
        var sys_date = new Date(year, month, date, hours, minutes, seconds);
        g_systime = sys_date.getTime();
        var manualdate = new Date(g_systime);
        var d = manualdate.format("YYYY/MM/DD");
        $("#manualdate").val(d);
        settime();
    }
    var settime_timer = null;
    function get_server_date(ajaxdata)
    {
        if(precheck(ajaxdata))
        {
            return false;
        }
        set_g_systime(ajaxdata);
        if (settime_timer == null) {
            settime_timer = setInterval(settime, 1000);
        }
    }
    function get_server_datatime() {
        var cfg = {};
        cfg.type = "get_device_time";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/sys_request_message.php',
            data:cfg,
            dataType:"text",
            success:get_server_date
        });
    }
    function set_sever_date(flag)
    {
        if($("#radio1").check_val())
        {
            if($("#manualdate").val()!="")
            {
                var ld = $("#manualdate").val();
                //简单的日期格式判断
                if(!ld.match(/^(19|20)\d{2}\/(0?\d|1[012])\/(0?\d|10|[12]\d|3[01])$/))
                {
                    show_informer_text("日期格式不符");
                    return;
                }
                ld = ld.split("/");
                var cfg = {};
                cfg.type = "set_device_time";
                cfg.body = {};
                cfg.body.devicetime = {};
                cfg.body.devicetime.d_timezone = 0;
                cfg.body.devicetime.d_year = parseInt(ld[0]);
                cfg.body.devicetime.d_mon = parseInt(ld[1]);
                cfg.body.devicetime.d_day = parseInt(ld[2]);
                cfg.body.devicetime.d_hour = parseInt($("#hour").val());
                cfg.body.devicetime.d_min = parseInt($("#min").val());
                cfg.body.devicetime.d_sec = parseInt($("#sec").val());
                cfg.body.devicetime.d_week = "Sun";
                cfg = JSON.stringify(cfg);

                $.ajax({
                    type:"POST",
                    url: '/sys_request_message.php',
                    data:cfg,
                    dataType:"text",
                    success:function(ajaxdata){
                        default_ajax_handler(ajaxdata);
                        get_server_datatime();
                        if(flag){
                            onLinkClick("SetIndLight.html");
                        }
                    }
                });
            }
            else
            {
                show_informer_text("请选择日期");
                return;
            }
        }
        else if($("#radio2").check_val())
        {
            var ld = $("#localdate").val();
            var lt = $("#localtime").val();
            if(!ld.match(/^(19|20)\d{2}\/(0?\d|1[012])\/(0?\d|10|[12]\d|3[01])$/))
            {
                show_informer_text("日期格式不符");
                return;
            }
            if(!lt.match(/^([01]\d|2[0-3]):([0-5]\d):([0-5]\d)$/))
            {
                show_informer_text("时间格式不符");
                return;
            }
            ld = ld.split("/");
            lt = lt.split(":");
            var cfg = {};
            cfg.type = "set_device_time";
            cfg.body = {};
            cfg.body.devicetime = {};
            cfg.body.devicetime.d_timezone = 0;
            cfg.body.devicetime.d_year = parseInt(ld[0]);
            cfg.body.devicetime.d_mon = parseInt(ld[1]);
            cfg.body.devicetime.d_day = parseInt(ld[2]);
            cfg.body.devicetime.d_hour = parseInt(lt[0]);
            cfg.body.devicetime.d_min = parseInt(lt[1]);
            cfg.body.devicetime.d_sec = parseInt(lt[2]);
            cfg.body.devicetime.d_week = "Sun";
            cfg = JSON.stringify(cfg);

            $.ajax({
                type:"POST",
                url: '/sys_request_message.php',
                data:cfg,
                dataType:"text",
                success:function(ajaxdata){
                    default_ajax_handler(ajaxdata);
                    get_server_datatime();
                    setTimeout(function(){
                        if(flag){
                            $("#ex_light").click();
                        }
                    },1000)
                }
            });
        }
    }
    this.init = function(){
        init_select("select",60);
        get_server_datatime();
        $("#submit").click(function(){
            set_sever_date(0);
        });
        $("#pre_step").click(function(){
            set_sever_date(1);
        });
    }
};
