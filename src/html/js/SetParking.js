var SetParking = new function(){
    //删除左右两端的空格
    function trim(str){
        return str.replace(/(^\s*)|(\s*$)/g, "");
    }

    var parkingreserve = [];
    function get_parking_cfg(){
        var cfg = {};
        cfg.type = "get_parking_reserve_cfg";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/bus_request_message.php',
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
                load_data(jsondata);
            }
        });
    }

    function load_data(jsondata){
        var parkingreserve = jsondata.body.parkingreservecfg.parkingreserve;
        var trs = "";
        for(var i=0;i<parkingreserve.length;i++){
            var data = JSON.stringify(parkingreserve[i]);
            var starttime = parkingreserve[i].parkingreservestartyear + "-" + parkingreserve[i].parkingreservestartmon + "-" + parkingreserve[i].parkingreservestartday + " " + parkingreserve[i].parkingreservestarthour + ":" + parkingreserve[i].parkingreservestartmin;
            var endtime = parkingreserve[i].parkingreserveendyear + "-" + parkingreserve[i].parkingreserveendmon + "-" + parkingreserve[i].parkingreserveendday + " " + parkingreserve[i].parkingreserveendhour + ":" + parkingreserve[i].parkingreserveendmin;
            var alarm = parkingreserve[i].parkingreserveoccupyalarm == 1 ? "是" : "否";
            trs += "<tr data='"+data+"'><td><i class='fa fa-minus'></i></td><td class='frame'>"+parkingreserve[i].parkingreserveframe+"</td><td class='plate'>"+parkingreserve[i].parkingreserveplate+"</td><td class='starttime'>"+starttime+"</td><td class='endtime'>"+endtime+"</td><td class='alarm'>"+alarm+"</td></tr>";
        }
        $("#sp_info_tb tbody").append(trs);
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
                var parkingframe = jsondata.body.parkingareacfg.parkingframe;
                var options = "";
                for(var i=0;i<parkingframe.length;i++){
                    options += "<option value='"+parkingframe[i].parkingframename+"'>"+parkingframe[i].parkingframename+"</option>";
                }
                $("#sp_stall_sel").html(options);
                init_select("#sp_stall_sel");
            }
        });
    }

    var g_cfg_table_cur_index = -1;
    //添加选中效果
    function select_comm_cfg_table_item(index) {
        if (index < 0) return false;

        var tr_item = $("#sp_info_tb tbody tr");
        if (tr_item.eq(index).length == 0) return false;
        if (g_cfg_table_cur_index != -1 && g_cfg_table_cur_index < tr_item.length) {
            tr_item.eq(g_cfg_table_cur_index).css("background","#fff");
        }
        tr_item.eq(index).css("background","#1ab394");
        g_cfg_table_cur_index = index;
    }
    //清空当前选择
    function empty_select() {
        var tr_item = $("#sp_info_tb tbody tr");
        tr_item.eq(g_cfg_table_cur_index).css("background","#fff");
        g_cfg_table_cur_index = -1;
    }
    //显示当前预留信息
    function show_parking(data){
        if(data == ""){
            return;
        }
        var jsondata = eval("("+ data +")");
        $("#sp_stall_sel").select_val(jsondata.parkingreserveframe);
        $("#sp_op_txt").val(jsondata.parkingreserveplate);
        var start_date = "";
        start_date += jsondata.parkingreservestartyear;
        start_date += "-";
        start_date += jsondata.parkingreservestartmon;
        start_date += "-";
        start_date += jsondata.parkingreservestartday;
        start_date += " ";
        start_date += jsondata.parkingreservestarthour;
        start_date += ":";
        start_date += jsondata.parkingreservestartmin;
        $("#start_time_pic").val(start_date);

        var end_date = "";
        end_date += jsondata.parkingreserveendyear;
        end_date += "-";
        end_date += jsondata.parkingreserveendmon;
        end_date += "-";
        end_date += jsondata.parkingreserveendday;
        end_date += " ";
        end_date += jsondata.parkingreserveendhour;
        end_date += ":";
        end_date += jsondata.parkingreserveendmin;
        $("#end_time_pic").val(end_date);
        $("#sp_occupy_cb").check_val(jsondata.parkingreserveoccupyalarm == 1);
    }
    //添加预留
    function add_parking_cfg(){
        var frame = $("#sp_stall_sel").select_val();
        var plate = $("#sp_op_txt").val();
        var sd = $("#start_time_pic").val();
        var ed = $("#end_time_pic").val();
        if(trim(plate) == "" || trim(sd) == "" || trim(ed) == ""){
            alert("内容不能为空");
            return;
        }
        var cfg = {};
        cfg.parkingreserveframe = frame;
        cfg.parkingreserveplate = plate;
        cfg.parkingreserveoccupyalarm = $("#sp_occupy_cb").check_val() ? 1 : 0;
        var startdate = $("#start_time_pic").datetimepicker("getDate");
        cfg.parkingreservestartyear = startdate.getFullYear();
        cfg.parkingreservestartmon = startdate.getMonth() + 1;
        cfg.parkingreservestartday = startdate.getDate();
        cfg.parkingreservestarthour = startdate.getHours();
        cfg.parkingreservestartmin = startdate.getMinutes();
        var enddate = $("#end_time_pic").datetimepicker("getDate");
        cfg.parkingreserveendyear = enddate.getFullYear();
        cfg.parkingreserveendmon = enddate.getMonth() + 1;
        cfg.parkingreserveendday = enddate.getDate();
        cfg.parkingreserveendhour = enddate.getHours();
        cfg.parkingreserveendmin = enddate.getMinutes();

        var data = JSON.stringify(cfg);
        var tr_item = $("#sp_info_tb tbody tr");

        tr_item.eq(g_cfg_table_cur_index).attr("data",data);
        tr_item.eq(g_cfg_table_cur_index).find(".frame").html(frame);
        tr_item.eq(g_cfg_table_cur_index).find(".plate").html(plate);
        tr_item.eq(g_cfg_table_cur_index).find(".starttime").html(sd);
        tr_item.eq(g_cfg_table_cur_index).find(".endtime").html(ed);
        tr_item.eq(g_cfg_table_cur_index).find(".alarm").html( $("#sp_occupy_cb").check_val() ? "是" : "否");

    }
    function empty_ele(){
        $("#sp_op_txt").val("");
        $("#start_time_pic").val("");
        $("#end_time_pic").val("");
        $("#sp_occupy_cb").check_val(false);
    }
    function add_parking_tr(){
        var length = $("#sp_info_tb tbody tr").length;
        if(length == 5){
            alert("已达上限");
            return;
        }
        var tr = "<tr data=''><td><i class='fa fa-minus'></i></td><td class='frame'></td><td class='plate'></td><td class='starttime'></td><td class='endtime'></td><td class='alarm'></td></tr>";
        $("#sp_info_tb tbody").append(tr);
    }
    function set_parking_cfg(){
        var parking_arr = [];
        var flag = false;
        $("#sp_info_tb tbody tr").each(function(){
            var jsonstr = $(this).attr("data");
            if(jsonstr == ""){
                flag = true;
                return;
            }
            var json = eval("("+ jsonstr +")");
            parking_arr.push(json);
        });
        if(flag){
            alert("预留信息不能为空");
            return;
        }
        var cfg = {};
        cfg.type = "set_parking_reserve_cfg";
        cfg.body = {};
        cfg.body.parkingreservecfg = {};
        cfg.body.parkingreservecfg.parkingreservenum = parking_arr.length;
        cfg.body.parkingreservecfg.parkingreserve = parking_arr;
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/bus_request_message.php',
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    $("#sp_info_tb .fa-minus").live("click",function(){
        empty_select();
        empty_ele();
        $(this).parent().parent().remove();
        return false;
    });
    $("#sp_info_tb tbody td").live("click",function(){
        select_comm_cfg_table_item($(this).parent().index());
        empty_ele();
        show_parking($(this).parent().attr("data"));
    });
    $("#sp_info_tb .fa-plus,#sp_info_tb .fa-minus").live("hover",function(){
        $(this).css("cursor", "pointer");
    });
    this.init = function(){
        $("#sp_info_tb .fa-plus").click(add_parking_tr);
        $("#cur_parking_btn").click(add_parking_cfg);
        $("#submit_parking_btn").click(set_parking_cfg);
        get_parking_cfg();
        get_parking_area_cfg();
    }
};
