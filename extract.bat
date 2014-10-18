if exist include (rd include /s /q)
md include
md include\jyfly
md include\jyfly\net
copy jyfly\net\Acceptor.h include\jyfly\net\Acceptor.h
copy jyfly\net\Buffer.h include\jyfly\net\Buffer.h
copy jyfly\net\Connection.h include\jyfly\net\Connection.h
copy jyfly\net\Connector.h include\jyfly\net\Connector.h

if exist lib (rd lib /s /q)
md lib
copy Debug\jyfly.lib lib\jyflyd.lib
copy Release\jyfly.lib lib\jyfly.lib