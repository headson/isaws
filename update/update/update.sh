1.0.0.1001707310
mkdir -p /mnt/etc
mkdir -p /mnt/web/log
mkdir /tmp/bak/web
tar -xf /tmp/bak/html.tar -C /tmp/bak/web
mv /tmp/bak/web/* /mnt/web/
mv /tmp/bak/platform_app /root/platform_app
mv /tmp/bak/systemserverapp /root/systemserverapp
mv /tmp/bak/web_server_app /root/web_server_app
mv /tmp/bak/business_app /root/business_app
mv /tmp/bak/alg_app /root/alg_app
mv /tmp/bak/feeddog /root/feeddog
mv /tmp/bak/watchdog /root/watchdog
mv /tmp/bak/* /root/

