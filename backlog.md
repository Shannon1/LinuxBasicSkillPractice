# blacklog参数

```c
int listen(int sockfd, int backlog);
```

`backlog`参数提示内核监听队列的最大长度。它表示出于**完全连接状态(ESTABLISHED)**的socket的上限。

监听队列的长度如果超过`backlog`设定的值，服务器将不再受理新的客户端连接，客户端也将收到**`ECONNREFUSED`**的错误信息。

