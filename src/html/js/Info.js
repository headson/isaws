var Info = new function(){
    function get_devicename(){
        var cfg = {};
        cfg.type = "get_device_name";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/sys_request_message.php',
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
                $("#devicename_text").val(jsondata.body.devicename);
            }
        });
    }
    function set_devicename(){
        var name = $("#devicename_text").val();
        if(trim(name) == ""){
            show_informer_text("设备名称不能为空");
            return false;
        }
        var cfg = {};
        cfg.type = "set_device_name";
        cfg.body = {};
        cfg.body.devicename = name;
        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url: '/sys_request_message.php',
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    //获取设备信息
    function get_software_version(){
        var cfg = {};
        cfg.type = "get_device_info";
        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url: '/sys_request_message.php',
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
                $("#software_version_td").html(jsondata.software_version);
                $("#system_version_td").html(jsondata.hardware_version);
                $("#algorithm_version_td").html(jsondata.alg_version);
                $("#device_serialno_td").html(jsondata.serial_no);
                $("#device_mac_td").html(jsondata.mac);
            }
        });
    }
    this.init = function init(){
        $("#submit_name_btn").click(set_devicename);
        get_devicename();
        get_software_version();

    }
};
