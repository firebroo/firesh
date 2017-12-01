### 安装使用
```bash
sudo yum install readline readline-static -y
make
./firesh
```

### done
* 执行系统命令, 支持CTRL+R,CTRL+N,CTRL+B等类BASH操作
* 内置命令(cd, pwd, exit)
* 执行单条命令, e.g. ./firesh -c "ls -la"
* 自动补全命令
* 后台运行

### todo
* IO重定向
* 管道运行
