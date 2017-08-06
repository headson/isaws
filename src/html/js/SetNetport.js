var SetNetport = new function(){
    var g_ip = null;
    var g_mask = null;
    var g_gw = null;
    var g_dns= null;
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
                g_mask = jsondata.body.netmask;
                g_gw = jsondata.body.gateway;
                g_dns = jsondata.body.dnsserver;
                $("#netip_text").val(g_ip);
                $("#netmask_text").val(g_mask);
                $("#gateway_text").val(g_gw);
                $("#dnsserver_text").val(g_dns);
            }
        });
    }
    var redirect_ip_timer = 0;
    var newloc = null;
    var send = false;
    function redirect_ip_go() {
        clearTimeout(redirect_ip_timer);
        redirect_ip_timer = 0;
        $('#warning_text').html("");
        location.href = newloc;
    }
    function redirect_ip() {
        $("#warning_text").css("color", "Green");
        $('#warning_text').html("ip地址已改变，正在跳转...");
        if (redirect_ip_timer == 0) {
            redirect_ip_timer = setTimeout(redirect_ip_go, 3000);
        }
    }
    function set_netip() {
        var cfg = {};
        cfg.type = "set_device_net";
        cfg.body = {};
        var ip = $("#netip_text").val();
        var netmask = $("#netmask_text").val();
        var gateway = $("#gateway_text").val();
        var dnsserver = $("#dnsserver_text").val();
        if (!test_ip(ip)) {
            show_informer_text("请输入正确的IP地址！");
            return false;
        }
        if (!test_ip(netmask) || netmask == "0.0.0.0" || netmask == "255.255.255.255") {
            show_informer_text("请输入正确的子网掩码地址！");
            return false;
        }
        if (!test_ip(gateway)) {
            show_informer_text("请输入正确的默认网关！");
            return false;
        }
        if (!test_ip(dnsserver)) {
            show_informer_text("请输入正确的DNS服务器！");
            return false;
        }
        var ipchanged = ip != g_ip;

        if (ip != g_ip) {
            send = true;
        }
        if(netmask != g_mask ){
            send = true;
        }
        if(gateway != g_gw){
            send = true;
        }
        if(dnsserver != g_dns){
            send = true;
        }
        cfg.body.deviceip = ip;
        cfg.body.netmask = netmask;
        cfg.body.gateway =  gateway;
        cfg.body.dnsserver = dnsserver;

        cfg = JSON.stringify(cfg);
        if (send) {
            send = false;
            $.ajax({
                type:"POST",
                url: '/sys_request_message.php',
                data:cfg,
                dataType:"text",
                success:function(ajaxdata){
                    default_ajax_handler(ajaxdata);
                }
            });
            if (ipchanged) {
                var new_href = location.href;
                var realserver = new_href.split('//')[1];
                realserver = realserver.split('/')[0];
                realserver = realserver.split(':')[0];
                newloc = new_href.split(realserver);
                newloc = newloc[0] + ip + newloc[1];
                redirect_ip();
            }
        }
    }

    function get_wb_tcp_server(){
        var cfg = {};
        cfg.type = "get_wb_tcp_server";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/tcp_request_message.php",
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
                $("#connect_enable").check_val(jsondata.connect_enable == 1);
                $("#tcp_ip_text").val(jsondata.server_ip);
                $("#tcp_port_text").val(jsondata.server_port);
                disable();
            }
        });
    }

    function set_wb_tcp_server(){
        var connect_enable = $("#connect_enable").check_val() ? 1 : 0;
        var server_ip = $("#tcp_ip_text").val();
        var server_port = parseInt($("#tcp_port_text").val());
        if (!test_ip(server_ip)) {
            show_informer_text("请输入正确的IP地址！");
            return false;
        }
        if (isNaN(server_port)) {
            show_informer_text("端口号为数字！");
            return false;
        }
        var cfg = {};
        cfg.type = "set_wb_tcp_server";
        cfg.body = {};
        cfg.body.connect_enable = connect_enable;
        cfg.body.server_ip = server_ip;
        cfg.body.server_port = server_port;
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/tcp_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }
    var isClose = false;
    function get_wbtcp_connect_status(){
        stop();
        var cfg = {};
        cfg.type = "get_wbtcp_connect_status";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/tcp_request_message.php",
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
                if(jsondata.connect_status == 1){
                    $("#status_txt").css("color","green");
                    $("#status_txt").html("已连接");
                }else{
                    $("#status_txt").css("color","gray");
                    $("#status_txt").html("未连接");
                }
                if(!isClose){
                    start();
                }
            }
        });
    }
    var tcp_status_timer = 0;
    function start() {
        if (!tcp_status_timer) {
            tcp_status_timer = setTimeout(get_wbtcp_connect_status, 3000);
        }
    }
    function stop() {
        clearTimeout(tcp_status_timer);
        tcp_status_timer = 0;
    }
    function disable(){
        var check = $("#connect_enable").check_val();
        if(check){
            $("#tcp_ip_text").removeAttr("disabled");
            $("#tcp_port_text").removeAttr("disabled");
        }else{
            $("#tcp_ip_text").attr("disabled","disabled");
            $("#tcp_port_text").attr("disabled","disabled");
        }
    }

    this.init = function init(){
        isClose = false;
        $("#submit_basic_btn").click(set_netip);
        $("#submit_tcp_btn").click(set_wb_tcp_server);
        $("#connect_enable").on("ifChanged",function(){
            disable();
        });
        get_netip();
        get_wb_tcp_server();
        start();
    }
    this.close = function(){
        isClose = true;
    }
    close_arr.push(this.close);
};
