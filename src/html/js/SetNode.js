var SetNode = new function(){
    var g_ip = "";
    var dis_addr = [];
    //IP转成整型
    function ip2int(ip)
    {
        var num = 0;
        ip = ip.split(".");
        num = Number(ip[3]) * 256 * 256 * 256 + Number(ip[2]) * 256 * 256 + Number(ip[1]) * 256 + Number(ip[0]);
        num = num >>> 0;
        return num;
    }
    //整型解析为IP地址
    function int2iP(num)
    {
        var str;
        var tt = new Array();
        tt[0] = (num >>> 24) >>> 0;
        tt[1] = ((num << 8) >>> 24) >>> 0;
        tt[2] = (num << 16) >>> 24;
        tt[3] = (num << 24) >>> 24;
        str = String(tt[3]) + "." + String(tt[2]) + "." + String(tt[1]) + "." + String(tt[0]);
        return str;
    }
    function get_netip(){
        var cfg = {};
        cfg.type = "get_device_net";
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
                g_ip = jsondata.body.deviceip;
                $("#device_ip").html(g_ip);
            }
        });
    }
    function init_node_list(){
        var str = "";
        for(var i=0;i<dis_addr.length;i++){
            var pos = dis_addr[i].pos;
            var p = "";
            if(pos == 0 || pos == 3){
                p = "直行设备";
            }else if(pos == 1){
                p = "左转向设备";
            }else if(pos == 2){
                p = "右转向设备";
            }
            str += "<tr><td>"+ int2iP(dis_addr[i].s_addr) +"</td><td>"+ p +"</td></tr>";
        }
        $("#node_list tr:gt(0)").remove();
        $("#node_list").append(str);
        g_cfg_table_cur_index = -1;
    }
    function get_server(){
        var cfg = {};
        cfg.type = "get_server";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http__root.php",
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
                dis_addr = jsondata.dis_addr;
                if(dis_addr == null){
                    dis_addr = [];
                }
                init_node_list();
            }
        });
    }
    function set_server(){
        var cfg = {};
        cfg.type = "set_server";
        cfg.body = {};
        cfg.body.time = 60;
        cfg.body.dis_addr = dis_addr;

        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http__root.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    function insert_node(){
        var ip = $("#dis_addr").val();
        var pos = parseInt($("#pos").select_val());
        if(dis_addr.length >= 4){
            alert("最多配置4个节点");
            return;
        }
        if(!test_ip(ip)){
            show_informer_text("请输入正确的目的节点IP地址");
            return;
        }
        if(ip == g_ip){
            alert("不能设置自己为目的节点");
            return;
        }
        var json = {};
        json.s_addr = ip2int(ip);
        json.pos = pos;
        dis_addr.push(json);
        init_node_list();
    }
    var g_cfg_table_cur_index = -1;
    function select_comm_cfg_table_item(index) {
        if (index <= 0) return false;

        var tr_item = $("#node_list tr");
        if (tr_item.eq(index).length == 0) return false;
        if (g_cfg_table_cur_index > 0 && g_cfg_table_cur_index < tr_item.length) {
            tr_item.eq(g_cfg_table_cur_index).removeClass("ui-state-active");
        }
        tr_item.eq(index).addClass("ui-state-active");
        g_cfg_table_cur_index = index;
    }
    function delete_node(){
        if(g_cfg_table_cur_index > 0 && g_cfg_table_cur_index <= dis_addr.length){
            dis_addr.splice(g_cfg_table_cur_index - 1,1);
            init_node_list();
        }else{
            alert("请先选择要删除的节点");
        }
    }
    this.init = function(){
        g_cfg_table_cur_index = -1;
        init_select("#pos");
        $("#submit_node_btn").click(set_server);
        $("#insert_node").click(insert_node);
        $("#delete_node").click(delete_node);
        $("#node_list td").live("click",function(){
            select_comm_cfg_table_item($(this).parent().index());
        });
        get_netip();
        get_server();
    }
};
