var SetSerial = new function(){
    function get_ttyio_cfg(){
        var cfg = {};
        cfg.type = "get_ttyio_cfg";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/bus_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ ajaxdata +")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                jsondata = jsondata.body;
                $("#baudrate_sel").select_val(jsondata.baudrate);
                $("#checkbit_sel").select_val(jsondata.checkbit);
                $("#stopbit_sel").select_val(jsondata.stopbit);
                $("#databit_sel").select_val(jsondata.databit);
            }
        });
    }
    function set_ttyio_cfg(){
        var cfg = {};
        cfg.type = "set_ttyio_cfg";
        cfg.body = {};
        cfg.body.ttyionum = 2;
        cfg.body.baudrate = parseInt($("#baudrate_sel").select_val());
        cfg.body.checkbit = parseInt($("#checkbit_sel").select_val());
        cfg.body.stopbit = parseInt($("#stopbit_sel").select_val());
        cfg.body.databit = parseInt($("#databit_sel").select_val());
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/bus_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    function get_ext_ttyio_cfg(){
        var cfg = {};
        cfg.type = "get_ext_ttyio_cfg";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/bus_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var jsondata = eval("("+ ajaxdata +")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                jsondata = jsondata.body;
                $("#protocol_sel").select_val(jsondata.proto);
                $("#f_enable_cb").check_val(jsondata.f_ena);
                $("#r_enable_cb").check_val(jsondata.r_ena);
                $("#l_enable_cb").check_val(jsondata.l_ena);
                $("#f_addr_txt").val(jsondata.f_addr);
                $("#r_addr_txt").val(jsondata.r_addr);
                $("#l_addr_txt").val(jsondata.l_addr);
                disable_f();
                disable_r();
                disable_l();
            }
        });
    }
    function set_ext_ttyio_cfg(){
        var f_check = $("#f_enable_cb").check_val();
        var r_check = $("#r_enable_cb").check_val();
        var l_check = $("#l_enable_cb").check_val();
        var f_addr = parseInt($("#f_addr_txt").val());
        var r_addr = parseInt($("#r_addr_txt").val());
        var l_addr = parseInt($("#l_addr_txt").val());
        if(f_check){
            if(isNaN(f_addr) || f_addr < 0 || f_addr > 255){
                show_informer_text("显示屏地址为0~255");
                return;
            }
        }
        if(r_check){
            if(isNaN(r_addr) || r_addr < 0 || r_addr > 255){
                show_informer_text("显示屏地址为0~255");
                return;
            }
        }
        if(l_check){
            if(isNaN(l_addr) || l_addr < 0 || l_addr > 255){
                show_informer_text("显示屏地址为0~255");
                return;
            }
        }
        var cfg = {};
        cfg.type = "set_ext_ttyio_cfg";
        cfg.body = {};
        cfg.body.proto = parseInt($("#protocol_sel").select_val());
        cfg.body.f_ena = f_check;
        cfg.body.f_addr = f_addr;
        cfg.body.r_ena = r_check;
        cfg.body.r_addr = r_addr;
        cfg.body.l_ena = l_check;
        cfg.body.l_addr = l_addr;
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/bus_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    function disable_f(){
        var check = $("#f_enable_cb").check_val();
        if(check){
            $("#f_addr_txt").removeAttr("disabled");
        }else{
            $("#f_addr_txt").attr("disabled","disabled");
        }
    }
    function disable_r(){
        var check = $("#r_enable_cb").check_val();
        if(check){
            $("#r_addr_txt").removeAttr("disabled");
        }else{
            $("#r_addr_txt").attr("disabled","disabled");
        }
    }
    function disable_l(){
        var check = $("#l_enable_cb").check_val();
        if(check){
            $("#l_addr_txt").removeAttr("disabled");
        }else{
            $("#l_addr_txt").attr("disabled","disabled");
        }
    }
    this.init = function(){
        init_select("#baudrate_sel,#checkbit_sel,#stopbit_sel,#databit_sel,#protocol_sel");
        $("#submit_serial_btn").click(set_ttyio_cfg);
        $("#submit_ext_btn").click(set_ext_ttyio_cfg);
        $("#f_enable_cb").on("ifChanged",function(){
            disable_f();
        });
        $("#r_enable_cb").on("ifChanged",function(){
            disable_r();
        });
        $("#l_enable_cb").on("ifChanged",function(){
            disable_l();
        });
        get_ttyio_cfg();
        get_ext_ttyio_cfg();
    }
};
