# VZLOGGING 日志库要求

现在我们平台内部，调试的方法非常有限，如果用户的设备出现了问题，那么我们就很难能够在现场将问题定位出现。有一些问题，是非常难以出现的，如果我们没有一个好的办法能够在定位当前设备状态下的问题的话，实践证明，我们也很难将这个问题解决。

提高调试手段是我们整个平台中亟待解决的问题。因此我们根据前面所实践设计我们自己的日志库。这个日志库有以下的一些特点。

1. 速度非常快，整个日志库release使用时,只有UDP发送，速度非常快。
2. 这个日志库需要配合一个日志服务器一起使用，日志服务器负责将日志收集并且按一定的规划保存到文件中，能够达到限制日志大小，动态显示、过滤日志的功能。
3. 日志服务器同时是一个`WatchDog`服务端，能够检测所有的模块是否运行正常。如果运行不正常将会将当前的日志保存，并且重启设备，这样针对异常的设备重启，我们都能够看到更加详细的日志记录。

## 日志使用

日志库为了方便，提供了`C`和`C++`两种语言的接口，方便用户使用，更方便用户能够快速的将自己当前的日志切换到`vzlogging`中

### `C`语言接口

C语言的库头文件应该包含`"vzlogging/include/vzlogging.h"`,有几个接口

```c

#include "vzlogging/include/vzlogging.h"

int main(int argc, char *argv[]) {

  InitVzLogging(argc, argv);

  LOG_INFO("message %d", 8);			// 类似printf格式化输出,会记录到日志文件中
  LOG_WARNING("message %d", 8);
  LOG_ERROR("message %d", 8);

  DLOG_INFO("message %d", 8);
  DLOG_WARNING("message %d", 8);
  DLOG_ERROR("message %d", 8);

  LOGB_INFO(data, 16);					// data[IN]需16进制显示数据; 16[data长度]
  LOGB_WARNING(data, 16);
  LOGB_ERROR(data, 16);

  DLOGB_INFO(data, 16);
  DLOGB_WARNING(data, 16);
  DLOGB_ERROR(data, 16);

  return 0;
}
```

首先调用`InitVzLogging(argc, argv)`来初始化日志接口(主要接收"-v"参数实现本地屏幕输出打印)。初始化之后，就可以调用vzlogging相关的接口进行传输显示了。

- 以`LOG_*`开头的打印，会记录到日志文件中。
- 以`DLOG_*`开头的打印，只会显示不会记录到日志文件中(CMake DEBUG编译时显示)。
- 以`LOGB_*`开头的打印,把传入的参数以16进制输出，会记录到日志文件中。
- 以`DLOGB_*`开头的打印,把传入的参数以16进制输出，只会显示不会记录到日志文件中(CMake DEBUG编译时显示)。

### `C++`接口

`C++`的接口，应该包含头文件`"vzlogging/include/vzloggingcpp.h"`

```c++
#include "vzlogging/include/vzloggingcpp.h"

int main(int argc, char *argv[]) {

  InitVzLogging(argc, argv);

  LOG(L_INFO) << "Hello World!";
  LOG(L_WARNING) << "Hello World!";
  LOG(L_ERROR) << "Hello World!";

  DLOG(L_INFO) << "Hello World!";
  DLOG(L_WARNING) << "Hello World!";
  DLOG(L_ERROR) << "Hello World!";

  return 0;
}
```

首先调用`InitVzLogging(argc, argv)`来初始化日志接口(主要接收`-v`参数实现本地屏幕输出打印)。初始化之后，就可以调用vzlogging相关的接口进行传输显示了。

- 以`LOG(*)`开始的接口打印会保存到日志文件中。
- 以`DLOG(*)`开始的接口只会打印，不会保存到日志文件中(CMake DEBUG编译时显示)。。


#### 调用`ShowVzLoggingAlways()`函数,无论有没有加`-v`启动参数都将打印LOG\DLOG的所有日志信息;

---

1. 使用C++实现，但不能够使用STL，不能够引用任何的第三方库，只能够使用最原始的操作系统接口。
2. 跨平台支持，需要在Windows和Linux平台上面使用。
3. 不能够使用C++ 11特性，必须能够在GCC 4.3.3版本上面支持编译。
4. 支持多线程安全，多线程下打印日志不能够乱，打印日志不能够在当前线程输出。
5. 不能够动态的分配内存，所有的内存都只能够在启动的时候分配好，未来在运行过程中不再动态的分配内存。
---

## 看门狗
头文件应该包含`"vzlogging/include/vzwatchdog.h"`,接口介绍:
```C
/************************************************************************/
/* Description : 注册一个喂狗KEY
/* Parameters  : descrebe[IN] 用户描述[MAX:8Byte]:进程名+描述形成唯一主键,
                              看门狗通过监听此主键喂狗频率判断是否挂掉
                 descrebe_size[IN] 用户描述大小
                 max_timeout[IN] 用户配置喂狗周期超时时长(单位秒)
/* Return      : !=NULL 注册成功,==NULL 注册失败
/************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout = DEF_MAX_TIMEOUT_SEC);
                          
/************************************************************************/
/* Description : 喂狗接口,定时调用,否则看门狗服务判断此key相关线程挂掉
/* Parameters  : key[IN] 注册看门狗时使用传入的key值
/* Return      : true 喂狗成功,false 喂狗失败
/************************************************************************/
bool FeedDog(const void *p_arg);
```

例程:
```C

#include "vzlogging/include/vzlogging.h"
#include "vzlogging/include/vzwatchdog.h"

int main(int argc, char *argv[]) {

  InitVzLogging(argc, argv);

  // "TEST" 用户描述
  // 4      用户描述字符长度
  // 20     用户配置超时时长(单位秒)
  void *p_key = RegisterWatchDogKey("TEST",4,20);
  if (NULL == p_key) {
    printf("register watchdog failed.\n");
    return -1;
  }
  
  while(true) {
    msleep(10*1000);
    
    bool bfd = FeedDog(p_key);
    if (bfd == false) {
      printf("feed dog failed.\n");
    }
  }

  return 0;
}
```
* 必须先初始化日志库,不然看门狗不能使用;
* 首先调用`InitVzLogging(argc, argv)`来初始化日志接口;
* 然后通过`RegisterWatchDogKey`注册一个看门狗的key,以后调用FeedDog许传入此key;
* 定时调用`FeedDog`实现喂狗,超过注册时传入的超时数值就看门狗服务将重启设备;建议:`FeedDog`的调用频率为 ((超时数值-2) / 2)秒


### 看门狗支持默认模块监控(无论模块有没有注册心跳,都将在看门狗的监控队列)

通过配置文件实现此功能(在`base/vzlogdef.h`中修改`DEF_WDG_MODULE_FILE`宏来指定配置文件路径)
``` define module config
#进程名[32byte];用户描述[8byte];超时值[Int 5-80]
#用户描述=RegisterWatchDogKey中的descrebe
#超时值=RegisterWatchDogKey中的max_timeout
#配置文件注释以 `#` 顶格开头字符串
#例子:
test_vzlogging.exe;TEST;30
```

### 日志模块的架构

日志服务一共分为两个主要的模块，一个日志服务器，一个日志库。日志库本身不会存放日志文件，它会将需要打印的内容通过UDP发送给日志服务器，由日志服务器来统一进行打印和保存日志。这样有几个特点

1. 平时运行过程中，可以不需要有任何打印(`printf`函数都不会调用)，包括日志服务器也不需要有任何打印。
2. 我们在运行过程中，可以重新启动日志服务器，这个时候可以看到整个系统中所有的模块的打印信息。而不需要像传统的调试方法那样，只能够将进程重启，才能够将打印重定向出来。
3. 日志服务器可以统一管理所有的日志文件，控制日志大小和相关的日志保存策略。

日志库和日志服务器之间有一个共享内存可以访问，这个共享内存由日志服务器创建，同时具有写入和读取的权限。日志库只能够读取里面的数据。这个共享内存主要有以下几个内容

1. 输出发送日志的级别。
2. 输出发送日志的服务端地址和端口

在正常的运行过程中，每一条打印，日志库都需要读取共享内存里面的东西，以决定输出的日志级别，并且将日志发送到相应的服务端地址和端口中去。

日志有三个级别，INFO、WARNING、ERROR。在正常的运行过程中，日志库不会在当前的线程输出任何东西(`printf`函数都不会调用)。只会在以`-v`运行的时候，才会在当前进程打印日志。这个时候打印就直接在当前线程打印就可以了，因为是调试的时候，不会太在意里面的效率。

## 日志服务器 vzlogserverapp

### 设计思路
* 日志库与日志服务器通过共享内存实现参数共享;
* 日志库读取共享内存,从中获取日志传输级别和服务器地址;
* 服务器接收到日志信息根据服务器启动参数来实现本地打印控制,存储日志到文件;
* 服务器接收到日志库中的看门狗注册,存储注册信息;
* 服务器接收喂狗消息,更新此进程-线程的看门狗信息;
* 服务器定时检查看门狗信息,当某一个看门狗信息超时,存储所有已注册看门狗信息,并另存一份最新日志到错误日志文件;最后重启设备;

### 服务器启动参数:
1. "-v n"(n为1\2\3数字,对应L_INGO,L_WARNING,L_ERROR)实现控制日志库给服务器传送日志级别;例如n=2,L_WARNING和L_ERROR日志都会传给服务器;
2. "-d" 本地屏幕输出打印,此输出与"-v n"相关,只有服务器能接收到的日志才会打印;
3. "-s savepath" 修改日志文件存储路径;
4. "-p port" 修改日志库与日志服务器通信端口;
5. "-f" 关闭看门狗重启设备