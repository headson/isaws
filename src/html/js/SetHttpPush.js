var SetHttpPush = new function(){
    //获取推送配置
    function get_http_server(){
        var cfg = {};
        cfg.type = "get_http_server";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
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
                $("#server_addr").val(jsondata.server_addr);
                $("#port").val(jsondata.port);
                $("#timeout").val(jsondata.timeout);
                $("#push_enable").check_val(jsondata.push_enable == 1);
                $("#smallimage_enable").check_val(jsondata.smallimage_enable);
                $("#bigimage_enable").check_val(jsondata.bigimage_enable);
                $("#push_addr").val(jsondata.push_addr);
            }
        });
    }
    //设置推送配置
    function set_http_server(){
        var server_addr = $("#server_addr").val();
        var port = parseInt($("#port").val());
        var timeout = parseInt($("#timeout").val());
        var push_enable = $("#push_enable").check_val() ? 1 : 0;
        var smallimage_enable = $("#smallimage_enable").check_val() ? 1 : 0;
        var bigimage_enable = $("#bigimage_enable").check_val() ? 1 : 0;
        var push_addr = $("#push_addr").val();
        if(!test_ip(server_addr)){
            show_informer_text("请输入正确的服务器地址！");
            return false;
        }
        if(isNaN(port)){
            show_informer_text("端口号为数字！");
            return false;
        }
        if(isNaN(timeout)){
            show_informer_text("超时时间为数字！");
            return false;
        }
        if(timeout < 1 || timeout > 3){
            show_informer_text("超时时间在1~3之间！");
            return false;
        }
        if(push_addr == ""){
            show_informer_text("地址不能为空！");
            return false;
        }
        var cfg = {};
        cfg.type = "set_http_server";
        cfg.body = {};
        cfg.body.server_addr = server_addr;
        cfg.body.port = port;
        cfg.body.timeout = timeout;
        cfg.body.push_enable = push_enable;
        cfg.body.smallimage_enable = smallimage_enable;
        cfg.body.bigimage_enable = bigimage_enable;
        cfg.body.push_addr = push_addr;

        cfg = JSON.stringify(cfg);
        $.ajax({
            type:"POST",
            url:"/http_request_message.php",
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                default_ajax_handler(ajaxdata);
            }
        });
    }

    this.init = function(){
        $("#set_http_btn").click(set_http_server);
        get_http_server();
    }
};
