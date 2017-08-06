var LiveVideo = new function(){
    var color_arr = ["白色","灰(银)色","黄色","粉色","红色","绿色","蓝色","棕色","黑色","橙色"];
    var cartype_arr = ["未知车牌","蓝牌小汽车","黑牌小汽车","单排黄牌","双排黄牌","警车车牌","武警车牌","个性化车牌","单排军车牌","双排军车牌","使馆车牌","香港进出中国大陆车牌","农用车牌","教练车牌","澳门进出中国大陆车牌","双层武警车牌","武警总队车牌","双层武警总队车牌"];

    var result_id = 0;
    var time_id = null;
    var parking_arr = [];
    var isClose = false;
    var isFirst = true;
    function load_data(datajson,flag,first){
        var ischange = false;
        var trigger = 0;
        if(datajson.match(/\"ischange\" : 1/)){
            ischange = true;
        }
        if(datajson.match(/\"triggertype\" : 2/)){
            trigger = 2;
        }
        var jsondata = eval("(" + datajson + ")");
        var result = jsondata.parkingresult;
        var plateresults = result.plateresult;
        if(parking_arr.length == 0){
            return;
        }
        for(var j = parking_arr.length - 1;j >= 0;j--){
            if(parking_arr[j] == 0){
                plateresults.splice(j,1);
            }
        }
        var str = "";
        for(var i = 0;i < plateresults.length;i++){
            var cur_result = plateresults[i];
            var triggerType = cur_result.triggertype;
            var time = cur_result.timestamp.Timeval;
            var dmon = time.dmon < 10 ? "0" + time.dmon : time.dmon;
            var dday = time.dday < 10 ? "0" + time.dday : time.dday;
            var dhour = time.dhour < 10 ? "0" + time.dhour : time.dhour;
            var dmin = time.dmin < 10 ? "0" + time.dmin : time.dmin;
            var dsec = time.dsec < 10 ? "0" + time.dsec : time.dsec;
            var time_str = time.dyear + "-" + dmon + "-" + dday + " " + dhour + ":" + dmin + ":" + dsec;
            var plateresultdiv = $($(".plateresult")[i]);
            if(cur_result.ischange == 1 || triggerType == 2 || !flag) {
                str += "<tr data='" + datajson + "' ";
                str += "class='ui-state-active'";
                var parkin = cur_result.parkingstatus != 0 ? "入场" : "出场";
                var triggerTypestr = triggerType == 2 ? "手动触发" : "自动触发";
                str += "><td>" + time_str + "</td><td>" + cur_result.parkingframename + "</td><td>" + parkin + "</td><td>" + Base64.decode(cur_result.license, true) + "</td><td>" + triggerTypestr + "</td></tr>";
            }
            if(cur_result.ischange == 1 || triggerType == 2 || !flag || (first && ischange)){
                var status = cur_result.parkingstatus != 0 ? "占用" : "空闲";
                plateresultdiv.find(".parkingframename").html(cur_result.parkingframename);
                plateresultdiv.find(".timestamp").html(time_str);
                plateresultdiv.find(".plate").html(Base64.decode(cur_result.license, true));
                plateresultdiv.find(".status").html(status);
                var imageSmallPath = cur_result.imageSmallPath.substring(cur_result.imageSmallPath.lastIndexOf("/") + 1);
                if (imageSmallPath == "" || cur_result.parkingstatus == 0) {
                    imageSmallPath = "../img/default_img.jpg";
                } else {
                    imageSmallPath = "../img/" + imageSmallPath + "?" + new Date().getTime();
                }
                var img = plateresultdiv.find(".platerealimg");
                var w = img.width();
                var h = w / 2;
                img.height(h);
                img.css({"background":"url('"+imageSmallPath+"') no-repeat","background-size":w+"px "+h+"px"});
            }
        }
        if(ischange || trigger == 2){
            //加载截图
            var imagePath = result.imagePath.substring(result.imagePath.lastIndexOf("/") + 1);
            imagePath = "../img/" + imagePath + "?"  + new Date().getTime();
            var w = $("#video_printscreen_img").width();
            var h = w*9/16;
            $("#video_printscreen_img").height(h);
            $("#video_printscreen_img").css({"background":"url('"+imagePath+"') no-repeat","background-size":w+"px "+h+"px"});
        }
        if(str != "" && flag){
            $("#rec_record_tb tr").removeClass("ui-state-active");
            $("#rec_record_tb tbody").append(str);
        }
        var length = $("#rec_record_tb tbody tr").length;
        if(length > 10){
            var num = length - 10;
            $("#rec_record_tb tbody tr:lt("+ num +")").remove();
        }
    }
    $("#rec_record_tb td").live("click",function(){
        var data_str = $(this).parent("tr").attr("data");
        $(this).parent("tr").addClass("ui-state-active");
        $(this).parent("tr").siblings().removeClass("ui-state-active");
        if(data_str){
            load_data(data_str,false);
        }
    });
//                    init_socket();
    function get_plate_result_poll(){
        stop();
        var cfg = {};
        cfg.plate_index = result_id;
        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url: '/get_plate_result_poll.php',
            data:cfg,
            dataType:"text",
            success: function(ajaxdata) {
                if(precheck(ajaxdata))
                {
                    return false;
                }
                var json = eval("("+ajaxdata+")");
                result_id = json.plate_index;
                var status = json.led_status;
                if(status == 0){
                    $("#park_status").html("红灯亮");
                    $("#light_color").css("background","red");
                }else if(status == 1){
                    $("#park_status").html("绿灯亮");
                    $("#light_color").css("background","green");
                }else if(status == 2){
                    $("#park_status").html("蓝灯亮");
                    $("#light_color").css("background","blue");
                }else{
                    $("#park_status").html("白灯亮");
                    $("#light_color").css("background","#fff");
                }
                if(json.state == 200){
                    if(isFirst){
                        load_data(ajaxdata,true,true);
                        isFirst = false;
                    }else{
                        load_data(ajaxdata,true);
                    }
                }
                if(!isClose){
                    start();
                }
            }
        });
    }


    function start() {
        if (!time_id) {
            time_id = setTimeout(get_plate_result_poll, 500);
        }
    }
    function stop() {
        clearTimeout(time_id);
        time_id = null;
    }
    function get_parking_area_cfg(){
        var cfg = {};
        cfg.type = "get_parking_area_cfg";
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
                var jsondata = eval("("+ajaxdata+")");
                if(jsondata.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                var frames = jsondata.body.parkingareacfg.parkingframe;
                var str = "";
                parking_arr.splice(0,parking_arr.length);
                for(var i = 0;i<frames.length;i++){
                    parking_arr.push(frames[i].parkingframeenable);
                    if(frames[i].parkingframeenable == 1){
                        str += '<div class="col-xs-12 col-sm-4"><div class="ibox float-e-margins plateresult"><div class="ibox-title"><h5 class="parkingframename"></h5></div><div> <table class="table table-bordered"> <tr> <td class="describe">时间</td> <td class="timestamp"></td> </tr> <tr> <td class="describe">车牌号</td> <td class="plate"></td></tr><tr> <td class="describe">车位状态</td> <td class="status"></td></tr><tr><td colspan="2">车头图像 </td> </tr> <tr> <td colspan="2" class="pi_par"><div class="platerealimg"></div></td></tr></table> </div> </div> </div>'
                    }
                }
                $("#rec_parking_outer").html(str);
            }
        });
    }
    this.init = function(){
        isFirst = true;
        isClose = false;
        result_id = 0;
        $("#hm_trigger_btn").click(function(){
            var cfg = {};
            cfg.type = "avs_parking_trigger";
            cfg = JSON.stringify(cfg);

            $.ajax({
                type:"POST",
                url: '/avs_request_message.php',
                data:cfg,
                dataType:"text",
                success: function(ajaxdata) {

                }
            });
        });
        get_parking_area_cfg();
        start();
        init_video("#live",function(){
            var b_w = parseInt($("#video_printscreen_img").width());
            var b_h = b_w*9/16;
            $("#video_printscreen_img").height(b_h).css("background-size",b_w+"px "+b_h+"px");
            var w = parseInt($(".platerealimg").width());
            var h = w / 2;
            $(".platerealimg").height(h).css("background-size",w+"px "+h+"px");
        });
    }
    this.close = function(){
        isClose = true;
    }
    close_arr.push(this.close);
};
