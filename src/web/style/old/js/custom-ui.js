function create_tabs(tabs)
{
    //remove div,which is added to make it compatible to easytabs
    //var tab_panel = $(tabs + '> div').detach();
    //$(tabs).append(tab_panel.children());
    //tab_panel.remove();
    $( tabs ).tabs();
    
    $( tabs ).css("background-color","#555555");
    $( tabs ).css("background-image","none");
    
	$( tabs+" div" ).css("font-family","Helvetica, Arial, sans-serif");
	$( tabs+" div" ).css("color","black");
	$( tabs+" div" ).css("background-color","White");
	
	$( tabs+" ul li a" ).css("font-size","12px");
	$( tabs+" ul li a" ).css("padding","10px 20px 10px 20px");
	
	$( tabs+" div table input[type=submit]" ).css("font-size","12px");
	$( tabs+" div table input[type=submit]" ).css("width","70px");
}

//for old selectmenu with jquery-ui 1.8.21
//function init_selectmenu(select, width, maxHeight, change_fun) {
//    $(select).selectmenu({ style: 'dropdown', width: width, maxHeight: maxHeight, change: change_fun });
//}

//jQuery.fn.select_val = function (value) {
//    if (arguments.length) {
//        if (this.children('option[value="' + value + '"]').length > 0) {
//            this.selectmenu('value', value);
//        }
//    }
//    else {
//        return this.selectmenu('value');
//    }
//}

//selectmenu bug fix and wrap
$.widget("ui.selectmenu", $.ui.selectmenu, {
    //add maxHeight option
    options: $.extend(this.options, { maxHeight: null }),
    _resizeMenu: function () {
        if (this.options.maxHeight != null) {
            this.menu.css("maxHeight", this.options.maxHeight + "px");
        }
        return this._super();
    },
    //fix selected not visible bug
    open: function () {
        if (this.options.disabled) {
            return;
        }

        // If this is the first time the menu is being opened, render the items
        if (!this.menuItems) {
            this._refreshMenu();
        } else {
            // Menu clears focus on close, reset focus to selected item
            this.menu.find(".ui-state-focus").removeClass("ui-state-focus");
        }
        this.isOpen = true;
        this._toggleAttr();
        this._resizeMenu();
        this._position();

        this.menuInstance.focus(null, this._getSelectedItem());
        this._on(this.document, this._documentClick);
        this._trigger("open", event);
    },
    //compatable to old selectmenu disable enable function style
    disable: function (index) {        
        if (index != undefined) {
            this.element.find("option").eq(index).attr("disabled", "disabled");
            this.refresh();
        }
        else {
            this._setOption("disabled", true);
        }
    },
    enable: function (index) {
        if (index != undefined) {
            this.element.find("option").eq(index).removeAttr("disabled");
            this.refresh();
        }
        else {
            this._setOption("disabled", false);
        }
    }
});

function init_selectmenu(select, width, maxHeight, change_fun) {
    if (change_fun != undefined) {
        $(select).selectmenu({
            width: width,
            maxHeight: maxHeight,
            change: function (event, object) { change_fun(event, object.item) }
        });
    }
    else {
        $(select).selectmenu({
            width: width,
            maxHeight: maxHeight
        });
    }
}

jQuery.fn.select_val = function (value) {
    if (arguments.length) {
        if (this.children('option[value="' + value + '"]').length > 0) {
            this.val(value);
            this.selectmenu("refresh");
        }
    }
    else {
        return this.val();
    }
}

//check box wrap for bootstrap style
function init_checkbox(check) {
    $(check).checkBox();
}

jQuery.fn.check_change = function (func) {
    this.checkBox({ 'change': func });
}

jQuery.fn.check_disabled = function (value) {
    if (value == true) this.checkBox('disable');
}

jQuery.fn.check_val = function (value) {
    if (arguments.length) {
        this.checkBox('changeCheckStatus', value);
    }
    else {
        return this.is(':checked');
    }
}

$(function() {    
    init_checkbox('input[type=checkbox],input[type=radio]');
    
    $("button, input:submit, input:button").button();
	
	$("input:text,input:password").addClass("text");
	
	$(".slider").slider({
	        range: "min",
			value:128,
			min: 0,
			max: 255,
			step: 1
	});
	
	$("select").parent().css("font-size",'11px');
	
	create_tabs("#tabs");
	
	$(".datepicker").datepicker();
	$( ".datepicker" ).datepicker( $.datepicker.regional[ "zh-CN" ] );
    $( ".datepicker" ).datepicker("option","dateFormat", 'mm/dd/yy');
	
     $("body").append( '<div class="inform"></div>');
     $(".inform").css("color","Green");

	 //loading
     $("body").append( '<div class="loading"></div>');
     $(".loading").hide();
     $(".loading").bind("ajaxSend", function(){
        $(this).show();
     }).bind("ajaxComplete", function(){
        $(this).hide();
     });     
});

var inform_auto_hide_timer=0;

function hide_informer(){
    $(".inform").html("");
    if(inform_auto_hide_timer!=0){
        clearTimeout(inform_auto_hide_timer);
        inform_auto_hide_timer=0;
    }
}

function set_inform_auto_hide(){
    if(inform_auto_hide_timer == 0) {
        inform_auto_hide_timer = setTimeout(hide_informer,2000);
    }            
}

function show_informer(){
     $(".inform").html("已设置");
    set_inform_auto_hide();
}

function show_informer_text(text){
     $(".inform").html(text);
    set_inform_auto_hide();
}