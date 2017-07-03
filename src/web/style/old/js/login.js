function init_login() {
    var g_rtsp_showed = false;
    $(function () {
        var outtext = $.cookie('outtext');
        if ($.cookie('remember') != null) {
            if (outtext != null) {
                passwd = "天天";
                var postdata = AesCtr.decrypt(outtext, passwd, 128);
                postdata = postdata.split(':');
                $("#username").val(postdata[0]);
                $("#usrpwd").val(postdata[1]);
                $("#check2").checkBox('changeCheckStatus', true);
            }
        }
        var rtsp_port = $.cookie('rtsp_port');
        $("#radio2").checkBox({
            'change': function (e, ui) {
                if (!$(this).is(':checked')) {
                    $("#custom_rtsp").hide();
                }
                else {
                    $("#custom_rtsp").show();
                }
            }
        });
        if (rtsp_port != null) {
            $("#radio2").checkBox('changeCheckStatus', true);
            $("#rtsp_port").val(rtsp_port);
            $("#custom_rtsp").show();
        }
        else {
            $("#radio1").checkBox('changeCheckStatus', true);
            $("#custom_rtsp").hide();
        }
        $("#submit").click(on_submit);
        $("#username").focus();
        $("#rtsp_settings").button({
            icons: {
                primary: "ui-icon-triangle-1-w"
            }
        })
		.click(function () {
		    var options;
		    if (g_rtsp_showed) {
		        options = {
		            icons: {
		                primary: "ui-icon-triangle-1-w"
		            }
		        };
		        $(this).button("option", options);
		        g_rtsp_showed = false;
		        $("#rtsp_tr").toggle();
		    } else {
		        options = {
		            icons: {
		                primary: "ui-icon-triangle-1-s"
		            }
		        };
		        $(this).button("option", options);
		        g_rtsp_showed = true;
		        $("#rtsp_tr").toggle();
		    }
		});
        $("#rtsp_tr").hide();
        init_title();
    });
}

function on_submit() {
    var username = $("#username").val();
    if (username.length == 0) {
        alert('请输入用户名');
    }
    else if (!username.match(/^.{4,33}$/)) {
        alert('提示：用户名长度必须在4-33之内');
    }
        //            else if(! username.match(/^[a-zA-Z]{1}/))
        //            {
        //                alert('提示：用户名首字母必须为字母');
        //            }
        //            else if(! username.match(/^\w{1,33}$/))
        //            {
        //                alert('提示：用户名只能包含字母数字或下划线');
        //            }
    else {
        var pwd = $("#usrpwd").val();
        if (pwd.length == 0) {
            alert("密码不能为空");
        }
        else if (pwd.length > 33) {
            alert("密码长度不能超过33个字符");
        }
        else if (pwd.match(/[:]{1}/)) {
            alert("密码中不能包含':'字符");
        }
        else {
            var postdata = username + ":" + pwd;
            passwd = "天天";
            //alert(passwd);
            var outtext = AesCtr.encrypt(postdata, passwd, 128);

            if ($("#radio2").is(':checked')) {
                var rtsp_port = $("#rtsp_port").val();
                var num_rtsp_port = parseInt(rtsp_port);
                if (isNaN(num_rtsp_port) || num_rtsp_port < 1024 || num_rtsp_port > (2 << 15) - 1) {
                    alert("请输入正确的端口号(1024-65535)！");
                    return false;
                }
            }

            //alert(outtext);
            //use our own ajax wrapper
            $.ajax({
                type: 'POST',
                url: "login.php",
                data: outtext,
                success: function (ajaxdata) {
                    if (ajaxdata == "OK") {
                        if (has_local_storage) {
                            localStorage.clear();
                        }

                        if ($("#check2").is(':checked')) {
                            $.cookie('outtext', outtext, { expires: 1, sercure: true });
                            $.cookie('remember', "1", { expires: 7, sercure: true });
                        } else {
                            $.cookie('outtext', outtext);
                            $.cookie('remember', null);
                        }
                        if ($("#radio2").is(':checked')) {
                            $.cookie('rtsp_port', rtsp_port, { expires: 1, sercure: true });
                        }
                        else {
                            $.cookie('rtsp_port', null);
                        }
                        if ((g_oem_info == 1 || g_oem_info == 5) && g_hava_custom != 0) {
                            document.location.href = "htmldata/Maintenance.htm";
                        } else {
                            document.location.href = "main.htm";
                        }
                        
                    }
                    else {
                        alert("错误的用户名或密码");
                        //alert(AesCtr.decrypt(outtext,passwd,128));
                    }
                },
                error: function (error) {
                    if (error.status == 500) {
                        alert("登录失败，请手动重启服务器再尝试！");
                    }
                    else {
                        alert("登录失败");
                    }
                }
            });
        }
    }
}

document.onkeydown = function (e) {
    var theEvent = window.event || e;
    var code = theEvent.keyCode || theEvent.which;
    if (code == 13) {
        $("#submit").click();
        return false;
    }
}
