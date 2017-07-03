
        function test_ip(ip){
            var addr = ip.split('.');
            if(addr.length != 4)return false;
            for(var i=0;i<4;i++){
                if(addr[i].length>3)return false;
                var val = parseInt(addr[i]);
                if(isNaN(val) || val<0 || val>255)return false;
            }
            return true;
        }
        
        function test_ip_zero(ip){
            var addr = ip.split('.');
            if(addr.length != 4)return false;
            for(var i=0;i<4;i++){
                var val = parseInt(addr[i]);
                if(isNaN(val) || val!=0)return false;
            }
            return true;
        }
        
        //假定ip格式正确
        function test_gateway(gateway){
            var addr3 = gateway.split('.');
            for(var i=0;i<4;i++){
                if(i==0){
                    var gateway_first = parseInt(addr3[i]);
                    if(gateway_first==0 || gateway_first>223){
                        alert("网关第一位必须介于1-223之间！");
                        return true;
                    }
                }
            }
            return false
        }
        
        //假定ip格式正确
        function test_ip_netmask(ip,netmask){
            var addr1 = ip.split('.');
            var addr2 = netmask.split('.');
            var val1=[0,0,0,0],val2=[0,0,0,0],val3=[0,0,0,0];
            var netmask_all_zero=true,
                zeros=true,ones=true,netmask_front=true,netmask_next_to = true;
            for(var i=0;i<4;i++){
                val1[i] = parseInt(addr1[i]);            
                val2[i] = parseInt(addr2[i]);
                if(i==0){
                    if(val1[i]==0 || val1[i]>223){
                        alert("ip地址第一位必须介于1-223之间！");
                        return true;
                    }
                }
                for(var j=0;j<4;j++){
                    var next = (1<<(3-j))&val2[i];
                    if(netmask_front && next==0)netmask_front = false;
                    else if(!netmask_front && next!=0) netmask_next_to = false;
                }
                if(val2[i]!=0) netmask_all_zero = false;
                val2[i] = (~val2[i]) & 0xff;
                val3[i] = val1[i] & val2[i];
                if(val3[i]!=0)zeros = false;
                if(val3[i]!=val2[i])ones = false;                
            }
            if(netmask_all_zero){
                alert("子网掩码不能全为0！");
                return true;
            }
            if(!netmask_next_to){
                alert("子网掩码必须是相邻的！");
                return true;
            }
            if(zeros ){
                 alert("ip地址和子网掩码的组合不合理，主机地址位全为0！");
                return true;
            }
            if(ones ){
                 alert("ip地址和子网掩码的组合不合理！主机地址位全为1！");
                return true;
            }
            return false;
        }
        
        
        function test_ip_in_same_net(ip,netmask,ip2){
            var addr1 = ip.split('.');
            var addr2 = netmask.split('.');
            var addr3 = ip2.split('.');
            var val1=[0,0,0,0],val2=[0,0,0,0],val3=[0,0,0,0];
            var in_same_net = true,test_over = false;
            for(var i=0;i<4;i++){
                val1[i] = parseInt(addr1[i]);            
                val2[i] = parseInt(addr2[i]);
                val3[i] = parseInt(addr3[i]);
                for(var j=0;j<4;j++){
                    var bit1 = (1<<(3-j))&val1[i];
                    var netmask_bit = (1<<(3-j))&val2[i];
                    var bit3 = (1<<(3-j))&val3[i];
                    if(netmask_bit==0){
                        return true;
                    }
                    else if(bit1 != bit3){
                        return false;
                    }
                }
            }
            return true;
        }