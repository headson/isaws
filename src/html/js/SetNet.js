var SetNet = new function(){
    var dname = 0;
    var net = 0;
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
        //if (send) {
        //    send = false;
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
                        net = 1;
                    }else{
                        net = 2;
                    }
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
        //}
    }
    var time = 0;
    this.init = function(){
        $("#submit_net_btn").click(function(){
            set_devicename();
            set_netip();
            if(time == 0){
                time = setInterval(function(){
                    if(dname != 0 && net != 0){
                        if(dname == 1 && net == 1){
                            show_informer();
                        }else if(dname == 1 && net == 2){
                            show_informer_text("网络参数保存失败");
                        }else if(dname == 2 && net == 1){
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
        });
        $("#last_step_btn").click(function(){
            onLinkClick("SetOSD.html");
        });
        get_devicename();
        get_netip();
        init_video("#live");
    }
};
