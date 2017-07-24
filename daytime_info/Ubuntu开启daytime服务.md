# Ubuntu 开启daytime服务

## 第一步： 需要安装xinetd服务
```shell
sudo apt-get install xinetd
```


## 第二步： 修改配置
`/etc/xinetd.d/daytime`。 将这文件中的两个 `disable` 的值 `yes` 改为 `no` 。

```bash
chmod 777 /etc/xinet.d/daytime

vim /etc/xinet.d/daytime
```

## 第三步：注销下系统或重新启动下xinetd服务。

```base
sudo /etc/init.d/xinetd restart
```
这样daytime服务便可使用。