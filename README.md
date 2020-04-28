# HandPlusServer
HandPlus的服务端

main: 入口

server:服务器主进程

room:房间类

player:玩家类

common: 日志系统，常量，mysql，protobuf等

server_sync:
    server_sync:数据包同步处理基类
    hall_sync:继承server_sync，大厅数据处理
    game_sync:继承server_sync，战斗数据处理

Hall:   Login->JoinRoom->RoomReady
    |   |       |
Clients:Login->JoinRoom(Hall)->JoinRoom(Game)->    Snyc
                |          |
Game:                 JoinRoom->RoomReady->Sync

