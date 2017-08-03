1.0.0.1001707310
mkdir -p /mnt/etc
mkdir -p /mnt/web/log
tar -xf /tmp/bak/html.tar -C /mnt/web/
mv /tmp/bak/platform_app /root/platform_app
mv /tmp/bak/systemserverapp /root/systemserverapp
mv /tmp/bak/web_server_app /root/web_server_app
mv /tmp/bak/business_app /root/business_app
mv /tmp/bak/alg_app /root/alg_app
mv /tmp/bak/feeddog /root/feeddog
mv /tmp/bak/watchdog /root/watchdog
mv /tmp/bak/* /root/

