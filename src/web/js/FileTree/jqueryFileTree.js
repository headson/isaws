// jQuery File Tree Plugin
//
// Version 1.01
//
// Cory S.N. LaViska
// A Beautiful Site (http://abeautifulsite.net/)
// 24 March 2008
//
// Visit http://abeautifulsite.net/notebook.php?article=58 for more information
//
// Usage: $('.fileTreeDemo').fileTree( options, callback )
//
// Options:  root           - root folder to display; default = /
//           script         - location of the serverside AJAX file to use; default = jqueryFileTree.php
//           folderEvent    - event to trigger expand/collapse; default = click
//           expandSpeed    - default = 500 (ms); use -1 for no animation
//           collapseSpeed  - default = 500 (ms); use -1 for no animation
//           expandEasing   - easing function to use on expand (optional)
//           collapseEasing - easing function to use on collapse (optional)
//           multiFolder    - whether or not to limit the browser to one subfolder at a time
//           loadMessage    - Message to display while initial tree loads (can be HTML)
//
// History:
//
// 1.01 - updated to work with foreign characters in directory/file names (12 April 2008)
// 1.00 - released (24 March 2008)
//
// TERMS OF USE
// 
// This plugin is dual-licensed under the GNU General Public License and the MIT License and
// is copyright 2008 A Beautiful Site, LLC. 
//

var desc = function(x,y)    
{    
    if (x > y)      
        return -1;  //返回一个小于0 的数即可  
    else        
        return 1;  //返回一个大于0 的数即可  
}    
//升序函数  
var asc = function(x,y)    
{    
    if (x > y)      
        return 1;  //返回一个大于0 的数即可  
    else            
        return -1;  //返回一个小于0 的数即可  
}  

function mysort(data, type)
{
	if(data.charAt(data.length-1) == ';')
	{
		return false;
	}
	else if(data.charAt(data.length-1) == ',')
	{
		data = data.substring(0,data.length-1);
		
		var filename = data.split(";");
		var g_filename = filename[0];
		var g_name = filename[1].split(",");
		
		if(Number(type) == 0)
		{
			g_name.sort();
		}
		else
		{
			g_name.sort(desc);
		}		
		
		g_show = "<html><head></head><body><ul class='jqueryFileTree' style='display: none;'>\n";
		
		for(var i = 0; i < g_name.length; i++)
		{	
			if(g_name[i].charAt(0) == 'b')//目录
			{
				var name = g_name[i].substring(1);
				var path = g_filename + "/" + name + "/";
				g_show = g_show + "<li class='directory collapsed'><a href='#' rel='"+path+"'>"+name+"</a></li>\n";
			}
			else
			{
				var name = g_name[i].substring(1);
				var path = g_filename + "/" + name;
				g_show = g_show + "<li class='file ext_jpg'><a href='#' rel='"+path+"'>"+name+"</a></li>\n";
			}
		}
		g_show = g_show + "</ul>\n</body></html>";
		return g_show;
	}
}

if(jQuery) (function($)
{
	$.extend($.fn, 
	{
		fileTree: function(o, h) 
		{
			// Defaults
			if( !o ) var o = {};
			if( o.root == undefined ) o.root = '/';
			if( o.script == undefined ) o.script = '../cgi-bin/FileTree.cgi';
			if( o.folderEvent == undefined ) o.folderEvent = 'click';
			if( o.expandSpeed == undefined ) o.expandSpeed= 500;
			if( o.collapseSpeed == undefined ) o.collapseSpeed= 500;
			if( o.expandEasing == undefined ) o.expandEasing = null;
			if( o.collapseEasing == undefined ) o.collapseEasing = null;
			if( o.multiFolder == undefined ) o.multiFolder = false;
			if( o.loadMessage == undefined ) o.loadMessage = 'Loading...';
			if( o.sort == undefined ) o.sort = 0;
			
			$(this).each( function() 
			{	
				function showTree(c, t) 
				{
					$(c).addClass('wait');
					$(".jqueryFileTree.start").remove();
				
					$.post(o.script, { dir:t }, function(data) 
					{
						$(c).find('.start').html('');
						$(c).removeClass('wait').append(mysort(data, o.sort));
						if( o.root == t ) $(c).find('UL:hidden').show(); else $(c).find('UL:hidden').slideDown({ duration: o.expandSpeed, easing: o.expandEasing });
						bindTree(c);
					});
				}
				
				function bindTree(t) 
				{
					$(t).find('LI A').bind(o.folderEvent, function() 
					{
						if( $(this).parent().hasClass('directory') ) 
						{
							if( $(this).parent().hasClass('collapsed') ) 
							{
								// Expand
								if( !o.multiFolder ) 
								{
									$(this).parent().parent().find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
									$(this).parent().parent().find('LI.directory').removeClass('expanded').addClass('collapsed');
								}
								$(this).parent().find('UL').remove(); // cleanup
								showTree( $(this).parent(), escape($(this).attr('rel').match( /.*\// )) );
								$(this).parent().removeClass('collapsed').addClass('expanded');
							} 
							else 
							{
								// Collapse
								$(this).parent().find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
								$(this).parent().removeClass('expanded').addClass('collapsed');
							}
							h($(this).attr('rel'), 1, $(this));
						} 
						else 
						{
							h($(this).attr('rel'), 0, $(this));
						}
						return false;
					});
					// Prevent A from triggering the # on non-click events
					if( o.folderEvent.toLowerCase != 'click' ) $(t).find('LI A').bind('click', function() { return false; });
				}
				// Loading message
				$(this).html('<ul class="jqueryFileTree start"><li class="wait">' + o.loadMessage + '<li></ul>');
				// Get the initial file list
				showTree( $(this), escape(o.root) );
			});
		}
	});
	
})(jQuery);