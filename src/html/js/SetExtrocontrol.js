var SetExtrocontrol = new function(){
    var ext_log_time_id = null;
    var check_time_id = null;
    var isClose = false;
    var ckClose = false;
    //获取主控配置
    function get_ext_device_cfg(){
        var cfg = {};
        cfg.type = "get_ext_device_cfg";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                jsondata = jsondata.body;
                $("#ck_ext_device").check_val(jsondata.ext_device_enable);
                $("#ext_device_ip").val(jsondata.ext_device_ip);
                disable();
            }
        });
    }
    //设置主控配置
    function set_ext_device_cfg(){
        var ext_device_enable = $("#ck_ext_device").check_val() ? 1 : 0;
        var ext_device_ip = $("#ext_device_ip").val();
        if(ext_device_enable == 1){
            if(!test_ip(ext_device_ip)){
                show_informer_text("请输入正确的IP地址！");
                return false;
            }
        }
        var cfg = {};
        cfg.type = "set_ext_device_cfg";
        cfg.body = {};
        cfg.body.ext_device_enable = ext_device_enable;
        cfg.body.ext_device_ip = ext_device_ip;
        cfg.body.ext_device_timeout = 1;

        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    function get_str(jsondata){
        jsondata = jsondata.body;
        var index = jsondata.log_index;
        var controled_log_arr = jsondata.logs;
        if(!controled_log_arr){
            return;
        }
        var controled_log_str = "";
        var controled_log_str_frist = "";
        for(var i = 0;i < controled_log_arr.length;i++){
            var status_str = "";
            var status = controled_log_arr[i].frame_status;
            if(status == 0){
                status_str = "空闲";
            }else if(status == 1){
                status_str = "占用";
            }else if(status == 2){
                status_str = "预留";
            }else if(status == 3){
                status_str = "告警";
            }
            var msg = controled_log_arr[i].log_msg;
            var msg_str = "";
            if(msg == 7){
                msg_str = "无法连接目标";
            }else if(msg == 28){
                msg_str = "操作超时";
            }else if(msg == 200){
                msg_str = "成功";
            }else if(msg == 405){
                msg_str = "目标不接受外控";
            }else{
                msg_str = "未知错误:" + msg;
            }
            if(i < index){
                controled_log_str_frist += "<tr><td>"+controled_log_arr[i].log_ip+"</td><td>"+controled_log_arr[i].log_time+"</td><td>"+msg_str+"</td><td>"+status_str+"</td></tr>";
            }else{
                controled_log_str += "<tr><td>"+controled_log_arr[i].log_ip+"</td><td>"+controled_log_arr[i].log_time+"</td><td>"+msg_str+"</td><td>"+status_str+"</td></tr>";
            }
        }
        controled_log_str += controled_log_str_frist;
        return controled_log_str;
    }
    var check_id = 0;
    function check_controled(){
        var ext_device_ip = $("#ext_device_ip").val();
        if(!test_ip(ext_device_ip)){
            show_informer_text("请输入正确的IP地址！");
            return false;
        }
        $(this).attr("disabled","disabled");
        ckClose = false;
        $("#error_msg").css("color","gray").html("检测中...");

        var cfg = {};
        cfg.type = "check_controled_device";
        cfg.body = {};
        cfg.body.ip = ext_device_ip;
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state == 200){
                    check_id = jsondata.body.check_id;
                    start_check();
                }else{
                    $("#error_msg").css("color","red").html("检测失败");
                    $("#check_controled_btn").removeAttr("disabled");
                }
            }
        });
    }
    var count = 0;
    function get_check_status(){
        stop_check();
        count++;
        if(count == 5){
            ckClose = true;
            count = 0;
        }
        var cfg = {};
        cfg.type = "get_check_status";
        cfg.body = {};
        cfg.body.check_id = check_id;
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                var status = jsondata.body.check_status;
                if(status == 0){
                    $("#error_msg").css("color","gray").html("检测中...");
                }else if(status == 1){
                    ckClose = true;
                    $("#error_msg").css("color","red").html("检测失败");
                    $("#check_controled_btn").removeAttr("disabled");
                    setTimeout(function(){
                        $("#error_msg").html("");
                    },1000)
                }else if(status == 2){
                    ckClose = true;
                    $("#error_msg").css("color","green").html("检测成功");
                    $("#check_controled_btn").removeAttr("disabled");
                    setTimeout(function(){
                        $("#error_msg").html("");
                    },1000)
                }
                if(!ckClose){
                    start_check();
                }
            }
        });
    }
    //获取主控日志
    function quary_ext_control_log(){
        stop();
        var cfg = {};
        cfg.type = "quary_ext_control_log";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http_log_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                var str = get_str(jsondata);
                $("#ext_control_log_tb tr:gt(1)").remove();
                $("#ext_control_log_tb").append(str);
                if(!isClose){
                    start();
                }
            }
        });
    }
    function start() {
        if (!ext_log_time_id) {
            ext_log_time_id = setTimeout(quary_ext_control_log, 1000);
        }
    }
    function stop() {
        clearTimeout(ext_log_time_id);
        ext_log_time_id = null;
    }
    function start_check() {
        if (!check_time_id) {
            count = 0;
            check_time_id = setTimeout(get_check_status, 1000);
        }
    }
    function stop_check() {
        clearTimeout(check_time_id);
        check_time_id = null;
    }
    function disable(){
        var check = $("#ck_ext_device").check_val();
        if(check){
            $("#check_controled_btn").removeAttr("disabled","disabled");
            $("#ext_device_ip").removeAttr("disabled","disabled");
        }else{
            $("#check_controled_btn").attr("disabled","disabled");
            $("#ext_device_ip").attr("disabled","disabled");
        }
    }
    this.init = function(){
        isClose = false;
        ckClose = 0;
        count = 0;
        $("#set_ext_btn").click(set_ext_device_cfg);
        $("#check_controled_btn").click(check_controled);
        $("#ck_ext_device").on("ifChanged",function(){
            disable();
        });
        get_ext_device_cfg();
        start();
    };
    this.close = function(){
        isClose = true;
        ckClose = true;
    };
    close_arr.push(this.close);
};
