@echo off
cd ../../../
REM Сервер нужен лишь для того чтобы соединять клиентов,
REM клиенты же будут пересылать сообщения друг другу.

REM Корень.
start sample.mbapi_4.ping.proxy.exe -s :10100

REM PAUSE

REM Первый уорвень.
start sample.mbapi_4.ping.proxy.exe -c :10100 -s :10110
start sample.mbapi_4.ping.proxy.exe -c :10100 -s :10120

REM PAUSE

REM Соединяются через первый уровень
start sample.mbapi_4.ping.cln.exe :10110 Alice Bob
start sample.mbapi_4.ping.cln.exe :10120 Bob Alice

REM PAUSE

REM Второй уорвень.
start sample.mbapi_4.ping.proxy.exe -c :10110 -s :10111
start sample.mbapi_4.ping.proxy.exe -c :10110 -s :10112
start sample.mbapi_4.ping.proxy.exe -c :10120 -s :10121
start sample.mbapi_4.ping.proxy.exe -c :10120 -s :10122

REM PAUSE

REM Соединяются через второй уровень, так чтобы сообщения должны
REM были пройти через корень
start sample.mbapi_4.ping.cln.exe :10112 Romeo Juliet
start sample.mbapi_4.ping.cln.exe :10121 Juliet Romeo

start sample.mbapi_4.ping.cln.exe :10111 Tristan Iseult
start sample.mbapi_4.ping.cln.exe :10122 Iseult Tristan

REM PAUSE

REM Длинная цепочка, с началом в корне
start sample.mbapi_4.ping.proxy.exe -c :10100 -s :10200
start sample.mbapi_4.ping.proxy.exe -c :10200 -s :10300
start sample.mbapi_4.ping.proxy.exe -c :10300 -s :10400
start sample.mbapi_4.ping.proxy.exe -c :10400 -s :10500
start sample.mbapi_4.ping.proxy.exe -c :10500 -s :10600
start sample.mbapi_4.ping.proxy.exe -c :10600 -s :10700
start sample.mbapi_4.ping.proxy.exe -c :10700 -s :10800

REM PAUSE

REM Сервер "Белоснежка"
start sample.mbapi_4.ping.srv.exe :11111 Snow_White
REM "Белоснежка" соединяется с корнем
start sample.mbapi_4.ping.proxy.exe -c :11111 -c :10100

REM PAUSE

REM Гномы подключаются к разным звеньям цепочки.
start sample.mbapi_4.ping.cln.exe :10100 Doc Snow_White
start sample.mbapi_4.ping.cln.exe :10200 Grumpy Snow_White
start sample.mbapi_4.ping.cln.exe :10300 Happy Snow_White
start sample.mbapi_4.ping.cln.exe :10400 Bashful Snow_White
start sample.mbapi_4.ping.cln.exe :10500 Sneezy Snow_White
start sample.mbapi_4.ping.cln.exe :10600 Sleepy Snow_White
start sample.mbapi_4.ping.cln.exe :10700 Dopey Snow_White

REM PAUSE
REM Замыкаем цепочку с другой стороны к белоснежке.
start sample.mbapi_4.ping.proxy.exe -c :10800 -с :11111