			/*
				create by zhaolei 2011-6-20 version 1.0
				company www.ingit.com
			*/
			var HH=23;
			var MM=59;
			var SS=59;
			var mode={};
			var LOOP;
			
			function loopInit(id,sign){
				LOOP=true;				
				change(id,sign);
			}
			function loopBreak(){
				LOOP=false;
			}
			function getTag(id,tagName,str,attr){
					var tagArray=document.getElementById(id).getElementsByTagName(tagName);
					for(i=0;i<tagArray.length;i++){
							if(eval("tagArray[i]."+attr)==str){
									return tagArray[i];
							}		
					}	
			}
			function setMode(id,hms){
					var selectInputPre = getTag(id,'input',mode[id+"mode"],'name');
					var selectInput = getTag(id,'input',hms,'name');
					selectInputPre.style.backgroundColor="#FFFFFF";
					mode[id+"mode"]=hms;
					selectInput.style.backgroundColor="#7FFFD4";
			}
			function change(id,sign){
					if(LOOP){
						var num=getTag(id,'input',mode[id+"mode"],'name').value-0;
						var changeNum=eval(num+sign+1)+"";
						if(changeNum>=0&changeNum<=eval(mode[id+"mode"])){	
							changeNum=complete(changeNum);
							getTag(id,'input',mode[id+"mode"],'name').value=changeNum;		
						}
						setTimeout("change('"+id+"','"+sign+"')",200);		
					}		
			}
			function complete(num){
					while(!(num.length==2)){
						num="0"+num;	
					}
			return num;		
			}
			function returnTimer(id){
					var strHH=getTag(id,'input','HH','name').value;
					var strMM=getTag(id,'input','MM','name').value;
					return strHH+':'+strMM;
			}
			function SetTimer(id,time){
				var t = time.split(":");
				getTag(id,'input','HH','name').value = t[0];
				getTag(id,'input','MM','name').value = t[1];
			}
			function checkNum(id,sign,value){
				if(value<10){
					value=complete(value);
					getTag(id,'input',sign,'name').value=value;
				}else{
					if(value>eval(sign)){
						getTag(id,'input',sign,'name').value=eval(sign);
					}
				}
			}
			function keyDown(id,key,onFocusObject){
				var nextObject;
				if(onFocusObject.name=='HH'){
					nextObject='MM';
				}else if(onFocusObject.name=='MM'){
					nextObject='HH';
				}
				if(!((key>=48 && key<=57)||(key>=96 && key<=105)||(key==8)||(key==46)||(key>=37 && key<=40))){
					event.returnValue=false;
				}
				if(key==37||key==39){
					getTag(id,'input',nextObject,'name').focus();
				}
				if(key==38){
					loopInit(id,'+');
				}
				if(key==40){
					loopInit(id,'-');
				}				
			}
			function keyUp(key){
				if(key==38||40){
					loopBreak();
				}		
			}
			function showTimer(tempId){
					mode[tempId+"mode"] = "HH";
					var timerConent='<table class="time_pligins" cellpadding="0" cellspacing="0" style="border:#ccc 1px solid;table-layout : fixed;min-width:80px;" >'+
													'<tr style="width:100px;">'+
														'<td width="60" valign="middle" style="display:block;padding:0;min-height: 0;"><input type="text"  maxlength="2" style="border:0;background:transparent;width:24px;float:left;height:22px;line-height:14px;" name="HH" value="00" onchange="checkNum(' + "'" + tempId + "'," + "'HH'," + 'this.value)" onkeydown="keyDown(' + "'" + tempId + "'" + ',event.keyCode,this)" onkeyup="keyUp(event.keyCode)" onfocus="setMode(' + "'" + tempId + "','HH'" + ')"/><input type="text"  style="border:0;background:transparent;width:10px;float:left;height:22px;line-height:14px;" readOnly=true  value=":"/><input type="text"  maxlength="2" style="border:0;background:transparent;width:24px;float:left;height:22px;line-height:14px;" name="MM" value="00" onchange="checkNum(' + "'" + tempId + "'," + "'MM'," + 'this.value)" onkeydown="keyDown(' + "'" + tempId + "'" + ',event.keyCode,this)" onkeyup="keyUp(event.keyCode)" onfocus="setMode(' + "'" + tempId + "','MM'" + ')"/></td>' +
														'<td width="14" valign="middle" style="padding:0;min-height: 0;"><span hidefocus="true" style="display:inline-block;width:0;height:0;border-bottom:5px solid #ccc;border-right:5px solid transparent;border-left:5px solid transparent;display:block;float:left;" onmouseup="loopBreak()" onmousedown="loopInit('+"'"+tempId+"','+'"+')"></span><span hidefocus="true" style="display:inline-block;width:0;height:0;border-top:5px solid #ccc;border-right:5px solid transparent;border-left:5px solid transparent;float:left;margin-top: 3px;" onmouseup="loopBreak()" onmousedown="loopInit('+"'"+tempId+"','-'"+')"></span></td>'+
													'</tr>'+
											  	'</table>';
					document.getElementById(tempId).innerHTML=timerConent;
					document.onmousedown=function(){
						$(".time_pligins input:text").css("background","#fff");
					}
			}