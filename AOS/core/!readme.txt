
svc.ini

Windows 使用 svp 時， -c 要使用 double-quote
sb8="./svp -c "../php5/php ../scripts/get_sbrs.php 8" -v "0.1b" -b 1 -s 9"
而不是
sb8="./svp -c '../php5/php ../scripts/get_sbrs.php 8' -v '0.1b' -b 1 -s 9"

svp

Windows 若 svp 不正常停止執行 (e.g. Ctrl-C or crashed)
不保証被呼叫的服務程式也會停止
也就是說如果啟動一個 php 的服務程式，不會因為 svp 掛掉而終止 php

