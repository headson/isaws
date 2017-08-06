var Support = new function(){
    function get_software_version(){
        var cfg = {};
        cfg.type = "get_device_info";
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
                jsondata = jsondata.body;
                $("#software_version_td").html(jsondata.software_version);
                $("#system_version_td").html(jsondata.hardware_version);
                $("#algorithm_version_td").html(jsondata.alg_version);
                $("#device_serialno_td").html(jsondata.serial_no);
                $("#device_mac_td").html(jsondata.mac);
            }
        });
    }
    var loaded = true;
    var error = false;
    function on_iframe_loaded() {
        if (loaded) {
            return;
        }
        loaded = true;
        var r;
        try {
            r = getIFrameContent("hidden_frame");
        }
        catch (e) {
            error = true;
        }
        if (error || r == undefined || r.length == 0) {
            $("#txt").css("color","red").html("请刷新网页或重启服务器,再尝试重新升级！");
        }
        else if (r.match(/All upload success/)) {
            $("#txt").css("color","green").html("升级成功，设备正在重启，请稍后...");
            restart();
            setInterval(check_restart,10000);
        }else{
            $("#txt").css("color","red").html("请刷新网页或重启服务器,再尝试重新升级！");
        }
        $("#update").removeAttr("disabled");
    }
    function restart(){
        $.ajax({
            url: '/reboot_device.php',
            success:function(ajaxdata){
            }
        });
    }
    function check_restart() {
        var cfg = {};
        cfg.type = "get_device_name";
        cfg = JSON.stringify(cfg);

        $.ajax({
            type:"POST",
            url: '/sys_request_message.php',
            data:cfg,
            dataType:"text",
            success:function(ajaxdata){
                location.href = "./Login.html";
                return false;
            }
        });
    }
    //取iframe的innerHTML
    function getIFrameContent(id) {
        var hidden_fr = document.getElementById(id);
        if (document.getElementById) {
            if (hidden_fr && !window.opera) {
                if (hidden_fr.contentDocument) {
                    return hidden_fr.contentDocument.body.innerHTML;
                } else if (hidden_fr.Document) {
                    return hidden_fr.Document.body.innerHTML;
                }
            }
        }
    }
    this.init = function(){
        $("#hidden_frame").on("load", on_iframe_loaded);
        $("#update").click(function(){
            loaded = false;
            error = false;
            var path = $("#file_input").val().split('\\').pop();
            var type = path.substr(path.length - 4);
            if(type != ".bin"){
                show_informer_text("请选择正确的升级文件，格式为.bin");
                return false;
            }
            $("#frmUpdate").submit();
        });
        $("#restart").click(function(){
            restart();
            setInterval(check_restart,10000);
            $("#txt").css("color","red").html("服务器已重启，请稍候...");
        });
        $("#frmUpdate").submit(function(){
            $("#txt").css("color","red").html("设备正在升级，请稍后...");
            $("#update").attr("disabled","disabled");
        });
        get_software_version();
    }
};
