Arduino und Laptop mit Patch-Kabel verbinden

    # Siehe http://superuser.com/questions/279543/connecting-two-linux-pcs-via-cross-cable
    $ sudo ifconfig eth0 10.10.11.16 netmask 255.255.255.0 up
    $ docker run --rm -ti -v `pwd`:/usr/share/nginx/html:ro --publish 10.10.11.16:8080:80 nginx

Anderes Window:

    # Zum Testen:
    $ curl http://10.10.11.16:8080/jenkins/job/mediamonitor3-quellendatenbank/api/json
    $ make upload_monitor

Zum Trennen der Verbindung zum Arduino: Ctrl-A, K
