var SetOSD = new function(){
    //获取OSD
    function get_osd_cfg(){
        var cfg = {};
        cfg.type = "get_osd_cfg";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
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
                var osd_date = jsondata.osd_date;
                $("#ivs_date_cb").check_val(osd_date.date_enable == 1);
                $("#ivs_date_sel").select_val(osd_date.dateformat);
                var osd_time = jsondata.osd_time;
                $("#ivs_time_cb").check_val(osd_time.time_enable == 1);
                $("#ivs_time_sel").select_val(osd_time.timeformat);
                var osd_text = jsondata.osd_text;
                $("#ivs_text_cb").check_val(osd_text.text_enable == 1);
                $("#ivs_text").val(osd_text.textbuf);
                disable_date();
                disable_time();
                disable_text();
            }
        });
    }
    //设置OSD
    function set_osd_cfg(){
        var date_enable = $("#ivs_date_cb").check_val() ? 1 : 0;
        var dateformat = parseInt($("#ivs_date_sel").select_val());

        var time_enable = $("#ivs_time_cb").check_val() ? 1 : 0;
        var timeformat = parseInt($("#ivs_time_sel").select_val());

        var text_enable = $("#ivs_text_cb").check_val() ? 1 : 0;
        var textbuf = $("#ivs_text").val();

        var cfg = {};
        cfg.type = "set_osd_cfg";
        cfg.body = {};
        cfg.body.osd_date = {};
        cfg.body.osd_date.date_enable = date_enable;
        cfg.body.osd_date.dateformat = dateformat;
        cfg.body.osd_date.datepositionX = 2;
        cfg.body.osd_date.datepositionY = 4;
        cfg.body.osd_time = {};
        cfg.body.osd_time.time_enable = time_enable;
        cfg.body.osd_time.timeformat = timeformat;
        cfg.body.osd_time.timepositionX = 85;
        cfg.body.osd_time.timepositionY = 4;
        cfg.body.osd_text = {};
        cfg.body.osd_text.text_enable = text_enable;
        cfg.body.osd_text.textbuf = textbuf;
        cfg.body.osd_text.textpositionX = 85;
        cfg.body.osd_text.textpositionY = 90;

        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    function disable_date(){
        var check = $("#ivs_date_cb").check_val();
        if(check){
            $("#ivs_date_sel").selectOrDie("enable");
            $("#date_coordinatex").removeAttr("disabled");
            $("#date_coordinatey").removeAttr("disabled");
        }else{
            $("#ivs_date_sel").selectOrDie("disable");
            $("#date_coordinatex").attr("disabled","disabled");
            $("#date_coordinatey").attr("disabled","disabled");
        }
    }
    function disable_time(){
        var check = $("#ivs_time_cb").check_val();
        if(check){
            $("#ivs_time_sel").selectOrDie("enable");
            $("#time_coordinatex").removeAttr("disabled");
            $("#time_coordinatey").removeAttr("disabled");
        }else{
            $("#ivs_time_sel").selectOrDie("disable");
            $("#time_coordinatex").attr("disabled","disabled");
            $("#time_coordinatey").attr("disabled","disabled");
        }
    }
    function disable_text(){
        var check = $("#ivs_text_cb").check_val();
        if(check){
            $("#ivs_text").removeAttr("disabled");
            $("#text_coordinatex").removeAttr("disabled");
            $("#text_coordinatey").removeAttr("disabled");
        }else{
            $("#ivs_text").attr("disabled","disabled");
            $("#text_coordinatex").attr("disabled","disabled");
            $("#text_coordinatey").attr("disabled","disabled");
        }
    }
    this.init = function(){
        init_select("#ivs_time_sel,#ivs_date_sel,#ivs_province_sel");
        $("#submit_osd_btn").click(function(){
            set_osd_cfg();
            onLinkClick("SetNet.html");
        });
        $("#last_step_btn").click(function(){
            onLinkClick("DrawArea.html","fast");
        });
        $("#ivs_date_cb").on("ifChanged",function(){
            disable_date();
        });
        $("#ivs_time_cb").on("ifChanged",function(){
            disable_time();
        });
        $("#ivs_text_cb").on("ifChanged",function(){
            disable_text();
        });
        get_osd_cfg();
        init_video("#live");
    }
};
