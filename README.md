# P2PCommunication
P2P communication through Nat penetration.



##  改进点

1. 用户信息是通过随意定义的格式进行传输的，解析也很不方便。后续可以通过json格式进行数据传输和解析。



## BUG

1. 客户端非正常退出，例如 ctrl + c强制退出，服务器中不会删除该条用户登入信息。后续会通过添加心跳，解决该问题问题。