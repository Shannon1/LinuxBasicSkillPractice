# iovec

`iovec`结构体封装了一块内存的起始位置和长度。

```cpp
struct iovec
{
	void 	*iov_base;
	size_t 	iov_len;
};
```
该结构体常用于**分散读**和**集中写**。

```cpp
ssize_t readv(int fd, const struct iovec* vector, int count);
ssize_t writev(int fd, const struct iovec* vector, int count);
```

在sokcet API中提供了一对通用的数据读写系统调用，

```cpp
ssize_t recvmsg(int socket, struct msghdr *message, int flags);
ssize_t sendmsg(int socket, struct msghdr *message, int flags);
```

其中参数`struct msghdr `:

```cpp
struct msghdr 
{
	void 		 *msg_name;      /* optional address socket地址，对于TCP来说没有意义，必须置为NULL*/
	socklen_t 	 msg_namelen;    /* size of address socket地址长度*/
	struct iovec *msg_iov; 		 /* scatter/gather array 分散的内存块（数组）*/
	int  		 msg_iovlen;     /* # elements in msg_iov 分散内存块的数量（数组长度）*/
	void         *msg_control;   /* ancillary data, see below */
	socklen_t    msg_controllen; /* ancillary data buffer len */
	int          msg_flags;      /* flags on received message */
};
```