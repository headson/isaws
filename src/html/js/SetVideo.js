var SetVideo = new function(){
    function set_slider_val(name,val){
        var cfg = {};
        cfg.type = "AVS_SET_VIDEO_PARAM";
        cfg.body = {};
        cfg.body.video_param = {};
        cfg.body.video_param[name] = val;
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
    function get_slider_val(name,val){
        var slider = $("#"+name).data("ionRangeSlider");
        slider.update({
            from:val
        });
    }
    function default_slider(name,val){
        get_slider_val(name,val);
    }
    function parse_config_info(ajaxdata,flag){
        if(precheck(ajaxdata)){
            return false;
        }
        var jsondata = eval("("+ajaxdata+")");
        if(flag){
            if (jsondata.state == 200) {
                show_informer();
            }
            else {
                show_informer_text("保存失败");
                return;
            }
        }else{
            if(jsondata.state != 200){
                show_informer_text("获取失败");
                return;
            }
        }
        var vp = jsondata.body.video_param;
        get_slider_val("brightness",vp.brightness);
        get_slider_val("contrast",vp.contrast);
        get_slider_val("saturation",vp.saturation);
        get_slider_val("hue",vp.hue);
        get_slider_val("max_expose",vp.max_expose);
        get_slider_val("max_gain",vp.max_gain);
    }
    function get_video_param(){
        var cfg = {};
        cfg.type = "AVS_GET_VIDEO_PARAM";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                parse_config_info(ajaxdata);
            }
        });
    }
    function back_to_default(){
        var cfg = {};
        cfg.type = "AVS_RESTORE_VIDEO_PARAM_DEFAULT";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                parse_config_info(ajaxdata,true);
            }
        });
    }
    var d_brightness = 0;
    var d_contrast = 0;
    var d_saturation = 0;
    var d_hue = 0;
    var d_max_expose = 0;
    var d_max_gain = 0;
    function init_slider(name,json) {
        var min = json.min;
        var max = json.max;
        var str = "d_" + name + "=json.default;";
        eval(str);
        var postfix = "";
        if(name == "max_expose"){
            postfix = "ms";
        }
        $("#" + name).ionRangeSlider({
            min: min,
            max: max,
            step:1,
            postfix:postfix,
            onFinish:function(data){
                set_slider_val(name,data.from);
            }
        });
    }
    function get_video_property(){
        var cfg = {};
        cfg.type = "AVS_GET_VIDEO_PARAM_PROPERTY";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata)){
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                jsondata = jsondata.body.video_param;
                init_slider("brightness",jsondata.brightness);
                init_slider("contrast",jsondata.contrast);
                init_slider("saturation",jsondata.saturation);
                init_slider("hue",jsondata.hue);
                init_slider("max_expose",jsondata.max_expose);
                init_slider("max_gain",jsondata.max_gain);
                get_video_param();
            }
        });
    }
    function set_select(name,json){
        var arr = json.types;
        var str = "";
        for(var i = 0;i < arr.length;i++){
            str += "<option value='"+arr[i].type+"'>"+arr[i].content+"</option>";
        }
        $(name).html(str);
        init_select(name);
    }
    var min_dr = 0;
    var max_dr = 0;
    function get_encode_property(){
        var cfg = {};
        cfg.type = "AVS_GET_ENCODE_PARAM_PROPERTY";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata)){
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                jsondata = jsondata.body.encode_param;
                //分辨率
                //set_select("#resolution_sel",jsondata.resolution);
                //帧率
                var fr = jsondata.frame_rate;
                var fr_str = "";
                for(var j = fr.min;j <= fr.max;j++){
                    fr_str += "<option value='"+j+"'>"+j+"</option>";
                }
                $("#frame_rate_sel").html(fr_str);
                init_select("#frame_rate_sel");
                ////码流控制
                //set_select("#rate_type_sel",jsondata.rate_type);
                ////编码方式
                //set_select("#encode_type_sel",jsondata.encode_type);
                ////图像质量
                //set_select("#video_quality_sel",jsondata.video_quality);
                //min_dr = jsondata.data_rate.min;
                //max_dr = jsondata.data_rate.max;
                get_encode_param();
            }
        });
    }
    function get_encode_param(){
        var cfg = {};
        cfg.type = "AVS_GET_ENCODE_PARAM";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if(precheck(ajaxdata)){
                    return false;
                }
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                var ep = jsondata.body.encode_param;
                //分辨率
                //$("#resolution_sel").select_val(ep.resolution);
                //帧率
                $("#frame_rate_sel").select_val(ep.frame_rate);
                //码流控制
                //$("#rate_type_sel").select_val(ep.rate_type);
                //编码方式
                //$("#encode_type_sel").select_val(ep.encode_type);
                //图像质量
                //$("#video_quality_sel").select_val(ep.video_quality);
                //码流上限
                //$("#data_rate").val(ep.data_rate);
            }
        });
    }
    function set_encode_param(){
        //var dr = parseInt($("#data_rate").val());
        //if(dr < min_dr || dr > max_dr){
        //    show_informer_text("码流上限在"+min_dr+"~"+max_dr+"之间");
        //    return false;
        //}
        var cfg = {};
        cfg.type = "AVS_SET_ENCODE_PARAM";
        cfg.body = {};
        cfg.body.encode_param = {};
        //cfg.body.encode_param.resolution = parseInt($("#resolution_sel").select_val());
        cfg.body.encode_param.frame_rate = parseInt($("#frame_rate_sel").select_val());
        //cfg.body.encode_param.encode_type = parseInt($("#encode_type_sel").select_val());
        //cfg.body.encode_param.rate_type = parseInt($("#rate_type_sel").select_val());
        //cfg.body.encode_param.video_quality = parseInt($("#video_quality_sel").select_val());
        //cfg.body.encode_param.data_rate = dr;
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
                $("#date_coordinatex").val(osd_date.datepositionX);
                $("#date_coordinatey").val(osd_date.datepositionY);
                var osd_time = jsondata.osd_time;
                $("#ivs_time_cb").check_val(osd_time.time_enable == 1);
                $("#ivs_time_sel").select_val(osd_time.timeformat);
                $("#time_coordinatex").val(osd_time.timepositionX);
                $("#time_coordinatey").val(osd_time.timepositionY);
                var osd_text = jsondata.osd_text;
                $("#ivs_text_cb").check_val(osd_text.text_enable == 1);
                $("#ivs_text").val(osd_text.textbuf);
                $("#text_coordinatex").val(osd_text.textpositionX);
                $("#text_coordinatey").val(osd_text.textpositionY);
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
        var datepositionX = parseInt($("#date_coordinatex").val());
        var datepositionY = parseInt($("#date_coordinatey").val());

        var time_enable = $("#ivs_time_cb").check_val() ? 1 : 0;
        var timeformat = parseInt($("#ivs_time_sel").select_val());
        var timepositionX = parseInt($("#time_coordinatex").val());
        var timepositionY = parseInt($("#time_coordinatey").val());

        var text_enable = $("#ivs_text_cb").check_val() ? 1 : 0;
        var textbuf = $("#ivs_text").val();
        var textpositionX = parseInt($("#text_coordinatex").val());
        var textpositionY = parseInt($("#text_coordinatey").val());

        if(date_enable == 1){
            if(isNaN(datepositionX) || datepositionX < 0 || datepositionX > 100 || isNaN(datepositionY) || datepositionY < 0 || datepositionY > 100){
                show_informer_text("日期坐标百分比为0~100正整数");
                return false;
            }
        }
        if(time_enable == 1){
            if(isNaN(timepositionX) || timepositionX < 0 ||  timepositionX > 100 || isNaN(timepositionY) || timepositionY < 0 || timepositionY > 100){
                show_informer_text("时间坐标百分比为0~100正整数");
                return false;
            }
        }
        if(text_enable == 1){
            if(isNaN(textpositionX) || textpositionX < 0 || textpositionX > 100 || isNaN(textpositionY) || textpositionY < 0 || textpositionY > 100){
                show_informer_text("文字坐标百分比为0~100正整数");
                return false;
            }
        }

        var cfg = {};
        cfg.type = "set_osd_cfg";
        cfg.body = {};
        cfg.body.osd_date = {};
        cfg.body.osd_date.date_enable = date_enable;
        cfg.body.osd_date.dateformat = dateformat;
        cfg.body.osd_date.datepositionX = datepositionX;
        cfg.body.osd_date.datepositionY = datepositionY;
        cfg.body.osd_time = {};
        cfg.body.osd_time.time_enable = time_enable;
        cfg.body.osd_time.timeformat = timeformat;
        cfg.body.osd_time.timepositionX = timepositionX;
        cfg.body.osd_time.timepositionY = timepositionY;
        cfg.body.osd_text = {};
        cfg.body.osd_text.text_enable = text_enable;
        cfg.body.osd_text.textbuf = textbuf;
        cfg.body.osd_text.textpositionX = textpositionX;
        cfg.body.osd_text.textpositionY = textpositionY;

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
        $("#back_default_btn").click(back_to_default);
        $("#submit_encode_btn").click(set_encode_param);
        $("#submit_area_btn").click(set_osd_cfg);
        $("#ivs_date_cb").on("ifChanged",function(){
            disable_date();
        });
        $("#ivs_time_cb").on("ifChanged",function(){
            disable_time();
        });
        $("#ivs_text_cb").on("ifChanged",function(){
            disable_text();
        });
        get_video_property();
        get_osd_cfg();
        init_video("#live");
        //get_encode_property();
    }
};
