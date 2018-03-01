# simple name server
## 介绍
   传统的基于配置的文件的配置方式配置信息修改与同步非常复杂，有必要建立一个统一的name server, 集中管理这些配置信息。程序上线运行之后，通过name server, 能够及时获取到最新的配置信息。

   > 最新的配置信息保存在redis服务器之中, 根据需要更改其连接方式。

## protocol

### GET

#### request

```
GET\r\n
server_name\r\n
\r\n
```

#### response

```
key1:value1\r\n
key2:value2\r\n
......
\r\n
```

### DEL

#### request

```
DEL\r\n
server_name\r\n
\r\n
```

#### response

```
OK\r\n\r\n
```

### NEW

#### request

```
NEW\r\n
server_name:name\r\n
key1:value1\r\n
key2:value2\r\n
...
\r\n
```

#### response

```
OK\r\n\r\n
```

### SET

#### request

```
SET\r\n
server_name:name\r\n
key1:value1\r\n
key2:value2\r\n
...
\r\n
```

#### response

```
OK\r\n\r\n
```

### other response

1. FAIL
2. INVALID
3. NOT EXISTED
4. EMPTY
5. INVALID COMMAND
