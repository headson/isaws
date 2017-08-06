function init_window_size(element){
    window.onresize = function(){
        var width = parseInt($(element).parent().width());
        var height = parseInt(width * 7 / 9);
        $(element).attr("width",width).attr("height",height);
        $(element).width(width).height(height);
        $(element + " embed").attr("width",width).attr("height",height);

        var b_w = parseInt($(".rec_bagimg").width());
        $(".rec_bagimg").height(b_w*9/16);
        var w = parseInt($(".rec_img").width());
        $(".rec_img").height(w/4);
    }
}
function init_video(element){
    init_window_size(element);
    window.onresize();
}
function GetAX(name)
{
    if (window.document[name])
    {
        return window.document[name];
    }
    if (navigator.appName.indexOf("Microsoft Internet")==-1)
    {
        if (document.embeds && document.embeds[name])
            return document.embeds[name];
    }
    else // if (navigator.appName.indexOf("Microsoft Internet")!=-1)
    {
        return document.getElementById(name);
    }
}

function play_video()
{
    var manual_download = "<a href=\"/html/htmldata/RTSPStreamPlayer.msi\" style=\"font-size:18px;margin:auto;color:blue;\">如果下载没有开始，点击这里手动下载控件</a>";
    var ax = GetAX("ax");
    try
    {
        var ip = location.host.split(":")[0];
        var ret = ax.StreamClient_StartPlay("rtsp://"+ip+":8557/h264");
    }
    catch(e){
        if(!confirm("没有安装控件？点击确定开始安装控件,安装时请关闭浏览器;如果已安装请点击取消并允许控件运行")){
            return;
        }
        else{
            window.location.href = "/html/htmldata/RTSPStreamPlayer.msi";
            $("#ax_container").html(manual_download);
            return;
        }
    }
}