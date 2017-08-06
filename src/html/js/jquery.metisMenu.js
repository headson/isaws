/*
 * metismenu - v1.1.3
 * Easy menu jQuery plugin for Twitter Bootstrap 3
 * https://github.com/onokumus/metisMenu
 *
 * Made by Osman Nuri Okumus
 * Under MIT License
 */
;(function($, window, document, undefined) {

    var pluginName = "metisMenu",
        defaults = {
            toggle: true,
            doubleTapToGo: false
        };

    function Plugin(element, options) {
        this.element = $(element);
        this.settings = $.extend({}, defaults, options);
        this._defaults = defaults;
        this._name = pluginName;
        this.init();
    }

    Plugin.prototype = {
        init: function() {

            var $this = this.element,
                $toggle = this.settings.toggle,
                obj = this;

            if (this.isIE() <= 9) {
                $this.find("li.active").has("ul").children("ul").collapse("show");
                $this.find("li").not(".active").has("ul").children("ul").collapse("hide");

                $this.find("li.active").has("iframe").children("iframe").collapse("show");
                $this.find("li").not(".active").has("iframe").children("iframe").collapse("hide");
            } else {
                $this.find("li.active").has("ul").children("ul").addClass("collapse in");
                $this.find("li").not(".active").has("ul").children("ul").addClass("collapse");

                $this.find("li.active").has("iframe").children("iframe").addClass("collapse in");
                $this.find("li").not(".active").has("iframe").children("iframe").addClass("collapse");
            }

            //add the "doubleTapToGo" class to active items if needed
            if (obj.settings.doubleTapToGo) {
                $this.find("li.active").has("ul").children("a").addClass("doubleTapToGo");
            }

            $this.find("li").children("a").on("click" + "." + pluginName, function(e) {
                e.preventDefault();
                if($(this).siblings("ul").length != 0){
                    var lis = $(this).parent("li").children("ul").find("li");
                    var h = lis.length * (37) + 30;
                    if(!$("body").hasClass("mini-navbar")){
                        h = 1;
                    }
                    $(this).parent("li").children("iframe").attr("height",h);
                    $(this).parent("li").children("iframe").collapse("toggle");
                    $(this).parent("li").toggleClass("active").children("ul").collapse("toggle");
                    if ($toggle) {
                        $(this).parent("li").siblings().removeClass("active").children("ul.in").collapse("hide");
                        $(this).parent("li").siblings().children("iframe.in").collapse("hide");
                    }
                }else{
                    $(this).parent().addClass("active").siblings().removeClass("active");
                    if ($toggle) {
                        $(this).parent("li").siblings().removeClass("active").children("ul.in").collapse("hide");
                        $(this).parent("li").siblings().removeClass("active").children("iframe.in").collapse("hide");
                    }
                }
            });
        },

        isIE: function() { //https://gist.github.com/padolsey/527683
            var undef,
                v = 3,
                div = document.createElement("div"),
                all = div.getElementsByTagName("i");

            while (
                div.innerHTML = "<!--[if gt IE " + (++v) + "]><i></i><![endif]-->",
                    all[0]
                ) {
                return v > 4 ? v : undef;
            }
        },

        //Enable the link on the second click.
        doubleTapToGo: function(elem) {
            var $this = this.element;

            //if the class "doubleTapToGo" exists, remove it and return
            if (elem.hasClass("doubleTapToGo")) {
                elem.removeClass("doubleTapToGo");
                return true;
            }

            //does not exists, add a new class and return false
            if (elem.parent().children("ul").length) {
                //first remove all other class
                $this.find(".doubleTapToGo").removeClass("doubleTapToGo");
                //add the class on the current element
                elem.addClass("doubleTapToGo");
                return false;
            }
        },

        remove: function() {
            this.element.off("." + pluginName);
            this.element.removeData(pluginName);
        }

    };

    $.fn[pluginName] = function(options) {
        this.each(function () {
            var el = $(this);
            if (el.data(pluginName)) {
                el.data(pluginName).remove();
            }
            el.data(pluginName, new Plugin(this, options));
        });
        return this;
    };

})(jQuery, window, document);