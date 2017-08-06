var DrawArea = new function(){
    var isClose = false;
    var dname = 0;
    var area = 0;
    var time = 0;
    function load_data(data){
        if(precheck(data))
        {
            return false;
        }
        var jsondata = eval("(" + data + ")");
        if(jsondata.state != 200){
            show_informer_text("获取失败");
            return;
        }
        jsondata = jsondata.body;
        //加载截图
        var imagePath = jsondata.trigger_image_path.substring(jsondata.trigger_image_path.lastIndexOf("/") + 1) + "?" + new Date().getTime();
        imagePath = "../img/" + imagePath;
        $("#ic_video").attr("src",imagePath);
        if(!isClose){
            get_img();
        }
    }

    //清除canvas
    function clearCanvas(){
        var c = document.getElementById('myCanvas');
        var ctx = c.getContext('2d');
        var width =  $("#myCanvas").attr("width");
        var height =  $("#myCanvas").attr("height");
        ctx.clearRect(0,0,width,height);
    }
    //点在区域内
    function pInQuadrangle(a,b,c,d,p) {
        var dTriangle = triangleArea(a, b, p) + triangleArea(b, c, p)
            + triangleArea(c, d, p) + triangleArea(d, a, p);
        var dQuadrangle = triangleArea(a, b, c) + triangleArea(c, d, a);
        return dTriangle == dQuadrangle;
    }

    //返回三个点组成三角形的面积
    function triangleArea(a,b,c) {
//                        var result = Math.abs((a.x * b.y + b.x * c.y + c.x * a.y - b.x * a.y - c.x * b.y - a.x * c.y) / 2);
        var result = Math.abs((parseInt(a.x) * parseInt(b.y) + parseInt(b.x) * parseInt(c.y) + parseInt(c.x) * parseInt(a.y) - parseInt(b.x) * parseInt(a.y) - parseInt(c.x) * parseInt(b.y) - parseInt(a.x) * parseInt(c.y)) / 2);
        return result;
    }
    //获取a,b两点的距离
    function getLength(a,b){
        var disx = parseInt(a.x) - parseInt(b.x);
        var disy = parseInt(a.y) - parseInt(b.y);
        var num = (disx * disx) + (disy * disy);    //平方和
        return parseInt(Math.sqrt(num));
    }

    var circle_down = false;
    var canvas_down = false;
    var line_down = false;
    var downobj = null;
    var radius = 3;
    //点击圆点事件
    function circle_mousedown(){
        circle_down = true;
        downobj = $(this);
        $(this).css("background-color","green");
        return false;
    }

    var cur_area = -1;
    var old_position = null;
    var cur_line = -1;
    //点击区域事件
    function canvas_mousedown(e){
        if(g_parking_area_cfg_json == null){
            return;
        }
        var parkingframe = g_parking_area_cfg_json.parkingareacfg.parkingframe;
        var p_l = 0;
        var p_t = 60;
        if (!$("body").hasClass('body-small')) {
            p_l = $("#leftNav").width() + 20;
            p_t = 80;
        }
        var x = e.pageX - p_l;
        var y = e.pageY - p_t;
        var p = {"x":x,"y":y};
        old_position = p;
        for(var i = 0;i < parkingframe.length;i++){
            if(parkingframe[i].parkingframeenable == 1){
                var cur_pos = parkingframe[i].parkingframeposition;
                var a = {"x":cur_pos.pointx1,"y":cur_pos.pointy1};
                var b = {"x":cur_pos.pointx2,"y":cur_pos.pointy2};
                var c = {"x":cur_pos.pointx3,"y":cur_pos.pointy3};
                var d = {"x":cur_pos.pointx4,"y":cur_pos.pointy4};
                var flag = pInQuadrangle(a,b,c,d,p);
                if(flag){
                    canvas_down = true;
                    cur_area = i;
                    draw_area();
                    break;
                }
            }
        }
        if(cur_area != -1){
            return;
        }
        var parkinglotline = g_parking_area_cfg_json.parkinglotline;
        var line_a = {"x":parkinglotline.parkinglotlineX1,"y":parkinglotline.parkinglotlineY1};
        var line_b = {"x":parkinglotline.parkinglotlineX2,"y":parkinglotline.parkinglotlineY2};
        if(getLength(line_a,line_b) == getLength(line_a,p) + getLength(line_b,p)){
            line_down = true;
            cur_line = 0;
            draw_area();
        }
        return false;
    }
    //松开鼠标事件
    function mouseup(){
        if(circle_down){
            circle_down = false;
            if(downobj){
                downobj.css("background-color","red");
            }
            downobj = null;
        }
        if(canvas_down){
            canvas_down = false;
            cur_area = -1;
            draw_area();
        }
        if(line_down){
            line_down = false;
            cur_line = -1;
            draw_area();
        }
    }
    //移动鼠标事件
    function mousemove(e){
        if(!circle_down && !canvas_down && !line_down){
            return false;
        }
        var p_l = 0;
        var p_t = 60;
        if (!$("body").hasClass('body-small')) {
            p_l = $("#leftNav").width() + 20;
            p_t = 80;
        }
        var x = e.pageX - p_l;
        var y = e.pageY - p_t;

        if(circle_down){
            var parent = downobj.parent();

            if(x >  parent.width()){
                x = parent.width();
            }
            if(x < 0){
                x = 0;
            }
            if(y > parent.height()){
                y = parent.height();
            }
            if(y < 0){
                y = 0;
            }
            if(!downobj){
                return false;
            }
            downobj.css({"left":x - radius,"top":y - radius});
            if(downobj.attr("index") == 3){
                var parkinglotline = g_parking_area_cfg_json.parkinglotline;
                var pointx = downobj.attr("pointx");
                var pointy = downobj.attr("pointy");
                parkinglotline[pointx] = x;
                parkinglotline[pointy] = y;
            }else{
                var index = parseInt(downobj.attr("index"));
                var pointx = downobj.attr("pointx");
                var pointy = downobj.attr("pointy");
                var parkingframe = g_parking_area_cfg_json.parkingareacfg.parkingframe;
                parkingframe[index]["parkingframeposition"][pointx] = x;
                parkingframe[index]["parkingframeposition"][pointy] = y;
            }
            draw_area();
        }
        if(canvas_down){
            var diff_x = x - old_position.x;
            var diff_y = y - old_position.y;
            old_position.x = x;
            old_position.y = y;
            var cur_position = g_parking_area_cfg_json.parkingareacfg.parkingframe[cur_area]["parkingframeposition"];
            var cancas = $("#myCanvas");
            var canvas_wh = {"width":parseInt(cancas.attr("width")),"height":parseInt(cancas.attr("height"))}
            if(cur_position.pointx1 + diff_x > canvas_wh.width || cur_position.pointx2 + diff_x > canvas_wh.width || cur_position.pointx3 + diff_x > canvas_wh.width || cur_position.pointx4 + diff_x > canvas_wh.width){
                return;
            }
            if(cur_position.pointx1 + diff_x < 0 || cur_position.pointx2 + diff_x < 0 || cur_position.pointx3 + diff_x < 0 || cur_position.pointx4 + diff_x < 0){
                return;
            }
            if(cur_position.pointy1 + diff_y > canvas_wh.height || cur_position.pointy2 + diff_y > canvas_wh.height || cur_position.pointy3 + diff_y > canvas_wh.height || cur_position.pointy4 + diff_y > canvas_wh.height){
                return;
            }
            if(cur_position.pointy1 + diff_y < 0 || cur_position.pointy2 + diff_y < 0 || cur_position.pointy3 + diff_y < 0 || cur_position.pointy4 + diff_y < 0){
                return;
            }

            cur_position.pointx1 = cur_position.pointx1 + diff_x;
            cur_position.pointy1 = cur_position.pointy1 + diff_y;
            cur_position.pointx2 = cur_position.pointx2 + diff_x;
            cur_position.pointy2 = cur_position.pointy2 + diff_y;
            cur_position.pointx3 = cur_position.pointx3 + diff_x;
            cur_position.pointy3 = cur_position.pointy3 + diff_y;
            cur_position.pointx4 = cur_position.pointx4 + diff_x;
            cur_position.pointy4 = cur_position.pointy4 + diff_y;
            draw_area();
        }
        if(line_down){
            var diff_x = x - old_position.x;
            var diff_y = y - old_position.y;
            old_position.x = x;
            old_position.y = y;
            var cur_position = g_parking_area_cfg_json.parkinglotline;
            var cancas = $("#myCanvas");
            var canvas_wh = {"width":parseInt(cancas.attr("width")),"height":parseInt(cancas.attr("height"))}
            if(cur_position.parkinglotlineX1 + diff_x > canvas_wh.width || cur_position.parkinglotlineX2 + diff_x > canvas_wh.width){
                return;
            }
            if(cur_position.parkinglotlineX1 + diff_x < 0 || cur_position.parkinglotlineX2 + diff_x < 0){
                return;
            }
            if(cur_position.parkinglotlineY1 + diff_y > canvas_wh.height || cur_position.parkinglotlineY2 + diff_y > canvas_wh.height){
                return;
            }
            if(cur_position.parkinglotlineY1 + diff_y < 0 || cur_position.parkinglotlineY2 + diff_y < 0){
                return;
            }

            cur_position.parkinglotlineX1 = cur_position.parkinglotlineX1 + diff_x;
            cur_position.parkinglotlineY1 = cur_position.parkinglotlineY1 + diff_y;
            cur_position.parkinglotlineX2 = cur_position.parkinglotlineX2 + diff_x;
            cur_position.parkinglotlineY2 = cur_position.parkinglotlineY2 + diff_y;
            draw_area();
        }
        return false;
    }

    //获得像素值x
    function get_real_x(x){
        var canvas_width = parseInt( $("#myCanvas").attr("width"));
        return Math.round(x / 256 * canvas_width);
    }
    //获得像素值y
    function get_real_y(y){
        var canvas_height = parseInt($("#myCanvas").attr("height"));
        return Math.round(y / 256 * canvas_height);
    }
    //获得算法值x
    function get_alg_x(x){
        var canvas_width = parseInt( $("#myCanvas").attr("width"));
        return parseInt(x / canvas_width * 256);
    }
    //获得算法值y
    function get_alg_y(y){
        var canvas_height = parseInt($("#myCanvas").attr("height"));
        return parseInt(y / canvas_height * 256);
    }
    //绘制
    function draw_area(){
        clearCanvas();
        var parkingframe = g_parking_area_cfg_json.parkingareacfg.parkingframe;
        var parkinglotline = g_parking_area_cfg_json.parkinglotline;
        var str = "";
        str += "var c = document.getElementById('myCanvas');var ctx = c.getContext('2d');ctx.lineWidth='1';";
        for(var i = 0;i < parkingframe.length;i++){
            if(parkingframe[i].parkingframeenable == 1){
                var cur_position = parkingframe[i].parkingframeposition;
                $("#canvas_container .circle[index='"+i+"']").show();
                $("#canvas_container .c1[index='"+i+"']").css({"left":cur_position.pointx1 - radius,"top":cur_position.pointy1 - radius});
                $("#canvas_container .c2[index='"+i+"']").css({"left":cur_position.pointx2 - radius,"top":cur_position.pointy2 - radius});
                $("#canvas_container .c3[index='"+i+"']").css({"left":cur_position.pointx3 - radius,"top":cur_position.pointy3 - radius});
                $("#canvas_container .c4[index='"+i+"']").css({"left":cur_position.pointx4 - radius,"top":cur_position.pointy4 - radius});
                if(i == cur_area){
                    str += "ctx.strokeStyle = '#4ac7ce';"
                }else{
                    str += "ctx.strokeStyle = 'green';"
                }
                str += "ctx.fillStyle = '#4ac7ce';";
                str += "ctx.font = 'bold 14px Arial';";
                var textY = cur_position.pointy1 - 5;
                if(textY < 10){
                    textY += 20;
                }
                str += "ctx.fillText('"+parkingframe[i].parkingframename+"',"+ cur_position.pointx1 +","+textY+");";
                str +="ctx.fillStyle = 'rgba(200,0,0,0.2)';";
                str += "ctx.beginPath();";
                str += "ctx.lineTo("+ cur_position.pointx1 +","+ cur_position.pointy1 +");";
                str += "ctx.lineTo("+ cur_position.pointx2 +","+ cur_position.pointy2 +");";
                str += "ctx.lineTo("+ cur_position.pointx3 +","+ cur_position.pointy3 +");";
                str += "ctx.lineTo("+ cur_position.pointx4 +","+ cur_position.pointy4 +");";
                str += "ctx.lineTo("+ cur_position.pointx1 +","+ cur_position.pointy1 +");";
                str += "ctx.stroke();ctx.fill();";
            }else{
                $("#canvas_container .circle[index='"+i+"']").hide();
            }
        }

        //绘制占线
        if(parkinglotline.parkinglotlineenable == 1){
            $("#canvas_container .circle[index='3']").show();
            $("#canvas_container .c5").css({"left":parkinglotline.parkinglotlineX1 - radius,"top":parkinglotline.parkinglotlineY1 - radius});
            $("#canvas_container .c6").css({"left":parkinglotline.parkinglotlineX2 - radius,"top":parkinglotline.parkinglotlineY2 - radius});
            if(cur_line == 0){
                str += "ctx.strokeStyle = '#4ac7ce';"
            }else{
                str += "ctx.strokeStyle = 'green';"
            }
            str += "ctx.beginPath();";
            str += "ctx.lineTo("+ parkinglotline.parkinglotlineX1 +","+ parkinglotline.parkinglotlineY1 +");";
            str += "ctx.lineTo("+ parkinglotline.parkinglotlineX2 +","+ parkinglotline.parkinglotlineY2 +");";
            str += "ctx.lineTo("+ parkinglotline.parkinglotlineX1 +","+ parkinglotline.parkinglotlineY1 +");";
            str += "ctx.stroke();";
        }else{
            $("#canvas_container .circle[index='3']").hide();
        }
        eval(str);
    }
    //加载checkbox和input
    function load_check_and_txt(){
        var parkingframe = g_parking_area_cfg_json.parkingareacfg.parkingframe;
        for(var i = 0;i < parkingframe.length;i++){
            var enable = parkingframe[i].parkingframeenable;
            if(enable == 0){
                $("#stall_area_text"+i).attr("disabled","disabled");
            }
            $("#ivs_stall_area_cb"+i).check_val(enable == 1);
            $("#stall_area_text"+i).val(parkingframe[i].parkingframename);
        }
    }
    var g_parking_area_cfg_json = {
        "parkinglotline" : {
            "parkinglotlineenable" : 0,
            "parkinglotlineX1" : 90,
            "parkinglotlineY1" : 35,
            "parkinglotlineX2" : 90,
            "parkinglotlineY2" : 215
        },
        "parkingareacfg" : {
            "parkingframenum" : 3,
            "parkingframe" : [
                {
                    "parkingframename" : "frame1",
                    "parkingframeenable" : 1,
                    "parkingframeposition" : {
                        "pointx1" : 15,
                        "pointy1" : 35,
                        "pointx2" : 15,
                        "pointy2" : 215,
                        "pointx3" : 75,
                        "pointy3" : 215,
                        "pointx4" : 75,
                        "pointy4" : 35
                    }
                },
                {
                    "parkingframename" : "frame2",
                    "parkingframeenable" : 1,
                    "parkingframeposition" : {
                        "pointx1" : 105,
                        "pointy1" : 35,
                        "pointx2" : 105,
                        "pointy2" : 215,
                        "pointx3" : 165,
                        "pointy3" : 215,
                        "pointx4" : 165,
                        "pointy4" : 35
                    }
                },
                {
                    "parkingframename" : "frame3",
                    "parkingframeenable" : 1,
                    "parkingframeposition" : {
                        "pointx1" : 185,
                        "pointy1" : 35,
                        "pointx2" : 185,
                        "pointy2" : 215,
                        "pointx3" : 245,
                        "pointy3" : 215,
                        "pointx4" : 245,
                        "pointy4" : 35
                    }
                }
            ]
        }
    };
    //获取区域配置
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
                try{
                    g_parking_area_cfg_json = eval("("+ajaxdata+")");
                }catch(e){

                }
                if(g_parking_area_cfg_json.state != 200){
                    show_informer_text("获取失败");
                    return;
                }
                g_parking_area_cfg_json = g_parking_area_cfg_json.body;
                var frame = g_parking_area_cfg_json.parkingareacfg.parkingframe;
                var str = "";
                for(var i = 0;i < frame.length;i++){
                    str += "<tr index='"+i+"'><td class='w-m'><input type='checkbox' class='i-checks area-cb' id='ivs_stall_area_cb"+i+"'><label for='ivs_stall_area_cb"+i+"'>车位区域"+(i+1)+"</label></td><td><input type='text' class='text area-txt' id='stall_area_text"+i+"' maxlength='20'/></td></tr>";
                    var cur_pos = frame[i].parkingframeposition;
                    cur_pos.pointx1 = get_real_x(cur_pos.pointx1);
                    cur_pos.pointy1 = get_real_y(cur_pos.pointy1);
                    cur_pos.pointx2 = get_real_x(cur_pos.pointx2);
                    cur_pos.pointy2 = get_real_y(cur_pos.pointy2);
                    cur_pos.pointx3 = get_real_x(cur_pos.pointx3);
                    cur_pos.pointy3 = get_real_y(cur_pos.pointy3);
                    cur_pos.pointx4 = get_real_x(cur_pos.pointx4);
                    cur_pos.pointy4 = get_real_y(cur_pos.pointy4);
                }
                $("#area_tb #null_tr").prevAll().remove();
                $("#area_tb #null_tr").before(str);
                $("#area_tb").show();
                init_checkbox('input[type=checkbox],input[type=radio]');
                $(".area-txt").live("input",function(){
                    var framename = $(this).val();
                    var index = parseInt($(this).parents("tr").attr("index"));
                    g_parking_area_cfg_json.parkingareacfg.parkingframe[index].parkingframename = framename;
                    draw_area();
                });
                $(".area-cb").live("ifChanged",function(){
                    var enabel = $(this).check_val() ? 1 : 0;
                    var txt = $(this).parents("tr").find(".area-txt");
                    if(enabel == 1){
                        txt.removeAttr("disabled");
                    }else{
                        txt.attr("disabled","disabled");
                    }
                    var index = parseInt($(this).parents("tr").attr("index"));
                    g_parking_area_cfg_json.parkingareacfg.parkingframe[index].parkingframeenable = enabel;
                    var num = 0;
                    var parkingframe = g_parking_area_cfg_json.parkingareacfg.parkingframe;
                    for(var i = 0;i<parkingframe.length;i++){
                        if(parkingframe[i].parkingframeenable == 1){
                            num++;
                        }
                    }
                    g_parking_area_cfg_json.parkingareacfg.parkingframenum = num;
                    draw_area();
                });
                $("#ivs_busy_cb").live("ifChanged",function(){
                    var enabel = $(this).check_val() ? 1 : 0;
                    g_parking_area_cfg_json.parkinglotline.parkinglotlineenable = enabel;
                    draw_area();
                });
                var parkinglotline = g_parking_area_cfg_json.parkinglotline;
                parkinglotline.parkinglotlineX1 = get_real_x(parkinglotline.parkinglotlineX1);
                parkinglotline.parkinglotlineY1 = get_real_y(parkinglotline.parkinglotlineY1);
                parkinglotline.parkinglotlineX2 = get_real_x(parkinglotline.parkinglotlineX2);
                parkinglotline.parkinglotlineY2 = get_real_y(parkinglotline.parkinglotlineY2);
                $("#ivs_busy_cb").check_val(parkinglotline.parkinglotlineenable == 1);
                draw_area();
                load_check_and_txt();
            }
        });
    }
    //设置区域配置
    function set_parking_area_cfg(){
        var frame = g_parking_area_cfg_json.parkingareacfg.parkingframe;
        var parkinglotline = g_parking_area_cfg_json.parkinglotline;
        var area_num = 0;
        if(parkinglotline.parkinglotlineenable == 1){
            for(var j = 0;j < frame.length;j++){
                if(frame[j].parkingframeenable == 1){
                    area_num++;
                }
            }
            if(area_num > 2){
                show_informer_text("三个车位不能使用占线");
                return;
            }
        }

        var cfg = {};
        cfg.type = "set_parking_area_cfg";
        cfg.body = {};
        cfg.body.parkinglotline = {};
        cfg.body.parkinglotline.parkinglotlineenable = parkinglotline.parkinglotlineenable;
        cfg.body.parkinglotline.parkinglotlineX1 = get_alg_x(parkinglotline.parkinglotlineX1);
        cfg.body.parkinglotline.parkinglotlineY1 = get_alg_y(parkinglotline.parkinglotlineY1);
        cfg.body.parkinglotline.parkinglotlineX2 = get_alg_x(parkinglotline.parkinglotlineX2);
        cfg.body.parkinglotline.parkinglotlineY2 = get_alg_y(parkinglotline.parkinglotlineY2);
        cfg.body.parkingareacfg = {};
        cfg.body.parkingareacfg.parkingframenum = g_parking_area_cfg_json.parkingareacfg.parkingframenum;
        cfg.body.parkingareacfg.parkingframe = [];
        for(var i = 0;i < frame.length;i++){
            var json = {};
            json.parkingframename = frame[i].parkingframename;
            json.parkingframeenable = frame[i].parkingframeenable;
            json.parkingframeposition = {};
            json.parkingframeposition.pointx1 = get_alg_x(frame[i].parkingframeposition.pointx1);
            json.parkingframeposition.pointy1 = get_alg_y(frame[i].parkingframeposition.pointy1);
            json.parkingframeposition.pointx2 = get_alg_x(frame[i].parkingframeposition.pointx2);
            json.parkingframeposition.pointy2 = get_alg_y(frame[i].parkingframeposition.pointy2);
            json.parkingframeposition.pointx3 = get_alg_x(frame[i].parkingframeposition.pointx3);
            json.parkingframeposition.pointy3 = get_alg_y(frame[i].parkingframeposition.pointy3);
            json.parkingframeposition.pointx4 = get_alg_x(frame[i].parkingframeposition.pointx4);
            json.parkingframeposition.pointy4 = get_alg_y(frame[i].parkingframeposition.pointy4);
            cfg.body.parkingareacfg.parkingframe.push(json);
        }

        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url:"/avs_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                if (precheck(ajaxdata)) {
                    return false;
                }
                var json = eval("("+ajaxdata+")");
                if (json.state == 200) {
                    area = 1;
                }else{
                    area = 2;
                }
            }
        });
    }
    //初始化视频窗口
    function init_video(element){
        var width = parseInt($(element).parent().width());
        var height = parseInt(width * 9 / 16);
        $(element).attr("width",width).attr("height",height);

        $(element + " embed").attr("width",width).attr("height",height);
        $(element).siblings("#canvas_container").width(width).height(height);
        $(element).siblings("#canvas_container").find("#myCanvas").attr("width",width).attr("height",height);
    }
    //获取图片
    function get_image(){
        var cfg = {};
        cfg.type = "get_current_image";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/avs_request_message.php',
            data:cfg,
            dataType:"text",
            success: function(ajaxdata) {
                load_data(ajaxdata);
            }
        });
    }
    function get_img(){
        setTimeout(get_image,1000);
    }
    $(".circle").live("mousedown",circle_mousedown);
    function back_default(){
        var cfg = {};
        cfg.type = "AVS_RESTORE_PARKING_AREA_CFG";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/avs_request_message.php',
            data:cfg,
            dataType:"text",
            success: function(ajaxdata) {
                var jsondata = eval("(" + ajaxdata + ")");
                if(jsondata.state == 200){
                    default_ajax_handler(ajaxdata);
                    get_parking_area_cfg();
                }
            }
        });
    }
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
                if (precheck(ajaxdata)) {
                    return false;
                }
                var json = eval("("+ajaxdata+")");
                if (json.state == 200) {
                    dname = 1;
                }else{
                    dname = 2;
                }
            }
        });
    }
    function save_data(flag){
        set_parking_area_cfg();
        set_devicename();
        if(time == 0){
            time = setInterval(function(){
                if(dname != 0 && area != 0){
                    if(dname == 1 && area == 1){
                        show_informer();
                        setTimeout(function(){
                            if(flag){
                                $("#ex_light").click();
                            }
                        },1000)
                    }else if(dname == 1 && area == 2){
                        show_informer_text("失败区域保存失败");
                    }else if(dname == 2 && area == 1){
                        show_informer_text("设备名称保存失败");
                    }else{
                        show_informer_text("保存失败");
                    }
                    clearInterval(time);
                    time = 0;
                    dname = 0;
                    net = 0;
                }
            },1000)
        }
    }
    this.init = function(){
        isClose = false;
        $("#area_tb").hide();
        var old_width;
        var old_height;
        //改变窗口大小事件
        window.onresize = function(){
            setTimeout(function(){
                var c = document.getElementById('myCanvas');
                if(!c){
                    return;
                }
                init_video("#ic_video");
                var width = $("#ic_video").attr("width");
                var height = $("#ic_video").attr("height");
                var sv = height / old_height;
                var parkinglotline = g_parking_area_cfg_json.parkinglotline;
                parkinglotline.parkinglotlineX1 = parkinglotline.parkinglotlineX1 * sv;
                parkinglotline.parkinglotlineY1 = parkinglotline.parkinglotlineY1 * sv;
                parkinglotline.parkinglotlineX2 = parkinglotline.parkinglotlineX2 * sv;
                parkinglotline.parkinglotlineY2 = parkinglotline.parkinglotlineY2 * sv;

                var frame = g_parking_area_cfg_json.parkingareacfg.parkingframe;
                for(var i = 0;i < frame.length;i++){
                    var cur_pos = frame[i].parkingframeposition;
                    cur_pos.pointx1 = cur_pos.pointx1 * sv;
                    cur_pos.pointy1 = cur_pos.pointy1 * sv;
                    cur_pos.pointx2 = cur_pos.pointx2 * sv;
                    cur_pos.pointy2 = cur_pos.pointy2 * sv;
                    cur_pos.pointx3 = cur_pos.pointx3 * sv;
                    cur_pos.pointy3 = cur_pos.pointy3 * sv;
                    cur_pos.pointx4 = cur_pos.pointx4 * sv;
                    cur_pos.pointy4 = cur_pos.pointy4 * sv;
                }
                old_width = width;
                old_height = height;
                draw_area();
            },300)
        };
        init_select("#ivs_time_sel,#ivs_date_sel,#ivs_province_sel",120);
        init_video("#ic_video");
        old_width = $("#ic_video").attr("width");
        old_height = $("#ic_video").attr("height");
        $("#myCanvas").mousedown(canvas_mousedown);
        $(document).mouseup(mouseup);
        $(document).mousemove(function(e){
            mousemove(e);
        });
        $("#submit_area_btn").click(function(){
            save_data(1);
        });
        $("#save_area").click(function(){
            save_data(0);
        });
        get_image();
        get_devicename();
        get_parking_area_cfg();
        $("#back_default").click(back_default);
    }
    this.close = function(){
        isClose = true;
    }
    close_arr.push(this.close);
}