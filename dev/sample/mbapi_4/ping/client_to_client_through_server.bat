cd ../../../
REM Сервер нужен лишь для того чтобы соединять клиентов,
REM клиенты же будут пересылать сообщения друг другу.
start sample.mbapi_4.ping.srv.exe :11411 server_ep
start sample.mbapi_4.ping.cln.exe :11411 Alice Bob
start sample.mbapi_4.ping.cln.exe :11411 Bob Alice