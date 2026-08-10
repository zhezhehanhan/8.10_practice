# 8.10 培训内容：指针、数组、函数指针与环形队列

> 本文件是 C 语言进阶培训8.10的课程 README，工程基于当前工作区 `keil_proj_final`。
> 前置：已完成 8.6 Git/Markdown、8.7 编译烧录、8.8 基础 C 工程操作、8.9 关键字补全与结构体/枚举。
> 作业单独放在 [doc/作业_day2.md](doc/作业_day2.md)，**本 README 只负责知识讲解**。

## 本课程教学重点

- 理解指针、取地址、解引用、空指针和指针运算；
- 理解数组与指针的关系，避免越界；
- 掌握指针作为函数参数时的"输出参数"用法；
- 区分 `const` 修饰指针的三种写法；
- 掌握常用位运算：置位、清位、翻转、判断、移位；
- 理解 `volatile` 在寄存器访问和中断共享变量中的作用；
- 用环形队列实现中断产生命令、主循环处理命令的生产者消费者模型。
- 用函数指针和命令表替代大段 `switch`(了解)；


## 硬件资源

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| LED1 | PB3 | 高电平点亮 |
| LED2 | PB4 | 高电平点亮 |
| LED3 | PB5 | 高电平点亮 |
| LED4 | PB6 | 高电平点亮 |
| 蜂鸣器 | PB0 | 高电平响 |

引脚初始化代码在 `Core/Src/gpio.c` 的 `MX_GPIO_Init()` 中，由 STM32CubeMX 生成。

## 工程目录结构

```text
keil_proj_final/
├── Core/
│   ├── Inc/                 # 主程序、GPIO、定时器等头文件
│   └── Src/
│       ├── main.c           # 初始化队列、启动定时器中断、主循环处理命令
│       ├── gpio.c           # GPIO 初始化，配置 PB0、PB3~PB6 为输出
├── Drivers/
│   ├── CMSIS/               # ARM 内核相关文件
│   └── STM32H7xx_HAL_Driver/ # STM32 HAL 库
├── hardware/                # 沿用前序课程的 LED、蜂鸣器驱动模块
│   ├── inc/
│   │   ├── led.h            # LED 函数声明和引脚宏
│   │   └── buzzer.h         # 蜂鸣器函数声明和引脚宏
│   └── src/
│       ├── led.c            # LED 点亮/熄灭实现，参数支持按位或组合多颗 LED
│       └── buzzer.c         # 蜂鸣器打开/关闭实现
├── common/                  # 通用数据结构（课程中逐步搭建）
│   ├── inc/
│   │   └── ring_buffer.h    # 环形队列公共接口
│   └── src/
│       └── ring_buffer.c    # 环形队列实现
├── doc/
│   └── 作业.md              # 当天作业
├── MDK-ARM/
│   └── keil_proj_final.uvprojx
└── keil_proj_final.ioc      # CubeMX 工程配置
```

种子工程已经提供了 `Core/`、`Drivers/`、`hardware/` 和 `MDK-ARM/` 部分，`common/` 和 `app/` 目录需要在课程中逐步搭建。

目标 `main()` 结构大致是：

```c
ring_buffer_init(&cmd_queue);
command_init(&cmd_queue);
timer_start_periodic();

while (1)
{
    command_process(&cmd_queue);
}
```

## 本工程覆盖的 C 语法

基础教程已经讲过变量、函数、`#define`、`for/while/if/switch` 和枚举、结构体、关键字补全，今天不再重复。下面展开指针、数组、位运算、`volatile`、函数指针和环形队列。

### 指针基础

#### 先形象化地理解

可以把变量想象成一个带编号的储物柜，柜子里装着数据，柜子编号就是"地址"。指针是一个特殊的"纸条"，纸条上写的不是数据本身，而是**另一个柜子的编号**。

```c
uint8_t value = 42U;   // 开一个叫 value 的柜子，放入 42
uint8_t *ptr  = &value; // 开一张纸条 ptr，上面写的是 value 的柜子编号
```

当你拿着纸条去取东西（`*ptr`），实际取到的是纸条上那个柜子里的东西。当你往纸条上写新编号（`ptr = &other`），纸条就指向了另一个柜子。

#### 语法要点

```c
uint8_t value = 42U;      // value：柜子，里面放着 42
uint8_t *ptr = &value;    // ptr：纸条，上面写着 value 的编号
                          // & 是"取柜子编号"，也叫取地址

*ptr = 43U;               // * 是"顺着纸条找到柜子，动柜子里的东西"
                          // 结果：value 变成 43（原来的 42 被覆盖）
```

要点：

- `&` 取地址，`*` 解引用（顺着地址找到数据）；
- 指针保存的是地址，不是值本身 — `ptr` 里存的是 `value` 的地址，不是 `42`；
- 指针本身有类型（例如 `uint8_t *`），类型决定读写宽度和指针运算的步长。`uint8_t *` 一次读写 1 字节，`uint32_t *` 一次读写 4 字节；
- **`void *` 是通用指针**，可以指向任何类型的数据。它不指定读写宽度，也不能直接解引用 — 使用前必须强制转换成具体类型。HAL 库的回调参数（如 `HAL_TIM_PeriodElapsedCallback` 的 `TIM_HandleTypeDef *htim`）和标准库函数（`memset`、`memcpy`）里大量使用 `void *`，先混个脸熟即可；
- **空指针**：`NULL` 表示这个纸条还没写任何有效编号，解引用空指针会让程序崩溃；
- **未初始化指针**：纸条上写了什么就指什么，指向的可能是随机内存，极危险；
- **野指针**：纸条上曾经写了有效编号，但那块内存已经还给系统了（例如函数返回的局部变量地址），也用不了；
- 读写前先确认指针不是空指针，也不要指向已失效的内存。

### 数组与指针

#### 数组名的退化

```c
uint8_t data[4] = {1U, 2U, 3U, 4U};
uint8_t *p = data;   // data 退化成首元素地址，相当于 &data[0]
```

要点：

- 数组名在大多数表达式中会**退化**成首元素地址；
- `data[i]` 等价于 `*(data + i)`，编译器看到 `data[2]` 会翻译成"从 data 首地址往后走 2 个元素，取出那里的值"；
- 数组作为函数参数时退化为指针，所以函数内无法用 `sizeof` 得到数组长度 — 必须另传一个长度参数；
- **越界访问不会自动报错**：`data[10]` 会读写到数组外面的内存，编译不报错、运行也不一定马上爆炸，但可能踩坏其他变量，这是嵌入式最常见的 bug 之一；
- `sizeof(data)` 得到整个数组的大小（元素个数 × 每个元素的大小），`sizeof(p)` 只得到指针本身的大小（H723 上通常是 4 字节）。

#### 用宏函数获取数组元素个数

数组定义处可以用 `sizeof(数组) / sizeof(数组[0])` 自动算出元素个数：

```c
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

uint8_t data[4] = {1U, 2U, 3U, 4U};
uint8_t count = ARRAY_SIZE(data);   // count = 4
```

这样增减数组内容时不用手动改长度。但这条宏**只对定义处的数组有效**，一旦数组退化成指针（例如作为参数传入函数），`sizeof(arr)` 就变成指针大小，算出错误结果。适用场景：同一个 `.c` 文件内定义的全局或静态数组。

### 指针作为输出参数

函数参数默认是"传值"：调用时把实参的值复制一份给形参，函数内部修改形参不会影响外面。但有时候函数需要**修改调用方的变量**，这时候就要传地址：

```c
void set_value(uint8_t *dst, uint8_t value)
{
    *dst = value;   // 顺着 dst 里的地址，修改调用方的变量
}
```

调用时传变量的地址：

```c
uint8_t my_value = 0U;
set_value(&my_value, 42U);   // 函数通过指针修改 my_value
// 现在 my_value == 42
```

队列的 `pop` 就是典型的输出参数用法：调用方传入一个变量的地址，函数把取出的数据写回这个变量：

```c
uint8_t received;

if (ring_buffer_pop(&cmd_queue, &received))   // 把取到的命令写入 received
{
    command_process_one(received);//执行函数
}
```

要点：

- 输出参数的名字通常带 `p`、`ptr` 或者以 `_out` 结尾，一眼能看出来它是"传出值"还是"传入值"；
- 调用时漏写 `&` 是常见错误：把变量本身（而不是地址）传给需要指针的参数，编译器会报警告。

### const 指针

`const` 放在不同的位置，含义完全不同。HAL 库头文件和 `const` 表里经常出现：

```c
const uint8_t *a;          // 指针指向的数据不能改（"指向常量的指针"）
uint8_t * const b;         // 指针本身不能改（"常量指针"）
const uint8_t * const c;   // 两者都不能改
```

读法技巧：从变量名往左读，遇到 `*` 说"指针指向"，遇到类型说"什么类型"，遇到 `const` 说"常量"。

| 写法 | 读法 | 含义 |
| --- | --- | --- |
| `const uint8_t *a` | a 是指针，指向 const uint8_t | `*a = 1U` 报错；`a = &other` 可以 |
| `uint8_t * const b` | b 是 const 指针，指向 uint8_t | `*b = 1U` 可以；`b = &other` 报错 |
| `const uint8_t * const c` | c 是 const 指针，指向 const uint8_t | `*c = 1U` 报错；`c = &other` 报错 |

常见使用场景：

- `const uint8_t *` 用于函数参数，表示"我只是读取你指向的数据，不会修改"，例如 `ring_buffer_is_empty(const ring_buffer *rb)`；
- `uint8_t * const` 较少单独出现；
- `const uint8_t * const` 用于全局只读配置表的指针。

### 位运算

位运算直接操作整数中的某一个或某几个二进制位，常用于寄存器配置、GPIO 引脚掩码和状态标志。

> 大部分代码处理位运算时使用十六进制（`0x0F`、`0x80`），但编者认为用二进制（`0b00001111`、`0b10000000`）能更直观地看到每一位的状态，学习阶段建议先用二进制理解，再看十六进制写法。

#### 基本操作

```c
#define BIT0 (1U << 0)   // 00000001
#define BIT1 (1U << 1)   // 00000010

uint8_t flags = 0U;

flags |= BIT1;                  // 置位：把 BIT1 变成 1
flags &= ~BIT1;                 // 清位：把 BIT1 变成 0
flags ^= BIT0;                  // 翻转：BIT0 原来是 0 就变 1，原来是 1 就变 0

if ((flags & BIT1) != 0U)       // 判断 BIT1 是否为 1
{
    /* BIT1 为 1 */
}
```

常用运算符：

| 运算符 | 含义 | 示例 | 说明 |
| --- | --- | --- | --- |
| `&` | 按位与 | `(flags & BIT1) != 0U` | 两个位都是 1 结果才为 1，常用于"读取/判断某一位" |
| `\|` | 按位或 | `flags \|= BIT1` | 有一个是 1 结果就是 1，常用于"把某一位设成 1" |
| `^` | 按位异或 | `flags ^= BIT0` | 两个位不一样结果才是 1，常用于"翻转某一位" |
| `~` | 按位取反 | `flags &= ~BIT1` | `~BIT1` 把 BIT1 那一位变成 0、其余变成 1，再 `&` 就是"只清那一位" |
| `<<` | 左移 | `1U << 3` | 把 1 往左移 3 位，也就是 `00001000` |
| `>>` | 右移 | `value >> 2` | 把 value 的位往右移 2 位 |

#### GPIO 引脚掩码

HAL 库中同时操作多个引脚也是位运算：

```c
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 | GPIO_PIN_4, GPIO_PIN_SET);
// GPIO_PIN_3 是 (1U << 3)，GPIO_PIN_4 是 (1U << 4)
// 按位或后得到 (1U << 4) | (1U << 5)，即同时选中两个引脚
```

本工程的 `led_on(LED1_PIN | LED3_PIN)` 也是同样的思路：一个字节里 bit4 控制 LED1、bit3 控制 LED3，或在一起后同时点亮两颗，无需循环或多次调用。

#### 通讯协议中的拼包

通讯协议经常把多个小字段拼进一个字节，省带宽。比如向电机发送"ID=3、方向=1、速度=5"，不用三个字节分开传，用位运算拼成一个：

```c
// 协议约定：bit7-5=ID, bit4=方向, bit3-0=速度
#define ID_POS   5U
#define DIR_POS  4U
#define SPEED_POS 0U

uint8_t packet = 0U;
packet |= (3U << ID_POS);     // ID=3    → bit7-5: 011
packet |= (1U << DIR_POS);    // 方向=1  → bit4:   1
packet |= (5U << SPEED_POS);  // 速度=5  → bit3-0: 0101
// packet = 0b011_1_0101 = 0x75
```

反过来，接收方用移位和掩码把各字段拆出来。CAN 帧 ID 的拼接、串口协议的命令字，都是这个套路。

#### 位运算符与逻辑运算符的区别

新手容易把位运算符和逻辑运算符搞混：

| | 位运算符 | 逻辑运算符 |
| --- | --- | --- |
| 运算对象 | 每一位 | 整个值的"真假" |
| 与 | `&` | `&&` |
| 或 | `\|` | `\|\|` |
| 取反 | `~` | `!` |
| 示例 | `0x03 & 0x02` = `0x02` | `3 && 2` = `1`（真） |

要点：

- 位运算优先使用无符号类型，避免符号位扩展带来的意外结果；
- 宏里使用位运算时，整个表达式要用括号包起来：`#define IS_SET(x, n) ((x) & (1U << (n)))`；
- `&` 的优先级比 `==` 低，判断某一位时必须写成 `(flags & BIT1) != 0U`，不能写成 `flags & BIT1 == 1U`；
- 在读改写操作（先读寄存器值，改某一位，写回去）中，常与 `volatile` 一起使用。

### volatile

编译器开优化后，如果发现一个变量在当前代码流里没被修改，可能把它缓存到寄存器里，后续直接读寄存器，不再访问内存。这在大多数时候没问题，但在一种场景会翻车：**变量被代码流之外的东西修改了**——比如中断服务函数。

```c
volatile uint8_t flag = 0U;

// 中断里
void ISR(void)
{
    flag = 1U;
}

// 主循环
while (flag == 0U)   // 不加 volatile，编译器可能只读一次 flag 然后死循环
{
    /* 等待 */
}
```

`volatile` 就做一件事：告诉编译器"每次读写这个变量都老老实实访问内存，别缓存"。

**它不负责线程安全。** 两个地方同时写、读-改-写被打断导致丢更新——这些问题 `volatile` 一概不解决，要靠关中断、原子操作等手段保护，具体知识自行了解。

### 环形队列

#### 为什么需要队列

定时器中断里不能做耗时操作（阻塞延时、复杂计算），所以中断只负责"产生命令，丢进队列"，主循环负责"从队列取命令，慢慢处理"。这种模式叫**生产者消费者模型**：

```text
中断（生产者）         队列             主循环（消费者）
  |                     |                   |
  |--- 写入命令 ------->|                   |
  |                     |<--- 取出命令 -----|
  |                     |                   |
```

#### 数据结构

```c
typedef struct
{
    uint8_t  buf[QUEUE_SIZE];   // 存放数据的数组
    uint8_t  head;              // 下一个读位置的下标
    uint8_t  tail;              // 下一个写位置的下标
    uint8_t  count;             // 当前队列里有多少数据
    uint16_t overflow_count;    // 队列满后丢了多少次数据
} ring_buffer;
```

#### 公共接口

```c
void     ring_buffer_init(ring_buffer *rb);                  // 初始化
bool     ring_buffer_push(ring_buffer *rb, uint8_t data);    // 入队
bool     ring_buffer_pop(ring_buffer *rb, uint8_t *data);    // 出队
bool     ring_buffer_is_empty(const ring_buffer *rb);         // 判空
bool     ring_buffer_is_full(const ring_buffer *rb);          // 判满
uint16_t ring_buffer_get_overflow(const ring_buffer *rb);    // 查询溢出次数
```

如果工程使用 `bool`，需要包含 `<stdbool.h>`；也可以把 `bool` 换成 `uint8_t`，用 `0` 表示失败、`1` 表示成功。

#### 入队实现

```c
bool ring_buffer_push(ring_buffer *rb, uint8_t data)
{
    if (ring_buffer_is_full(rb))
    {
        rb->overflow_count++;   // 记录溢出次数
        return false;
    }

    rb->buf[rb->tail] = data;
    rb->tail = (rb->tail + 1U) % QUEUE_SIZE;   // tail 后移，到底后回绕
    rb->count++;
    return true;
}
```

#### 出队实现

```c
bool ring_buffer_pop(ring_buffer *rb, uint8_t *data)
{
    if (ring_buffer_is_empty(rb))
    {
        return false;
    }

    *data = rb->buf[rb->head];                   // 用输出参数传出数据
    rb->head = (rb->head + 1U) % QUEUE_SIZE;     // head 后移，到底后回绕
    rb->count--;
    return true;
}
```

#### 环形队列的运作过程

假设 `QUEUE_SIZE = 4`，初始状态 `head = tail = count = 0`：

```text
初始化：     [ _ ][ _ ][ _ ][ _ ]    head=0 tail=0 count=0

push(A)：    [ A ][ _ ][ _ ][ _ ]    head=0 tail=1 count=1
push(B)：    [ A ][ B ][ _ ][ _ ]    head=0 tail=2 count=2
pop -> A：   [ _ ][ B ][ _ ][ _ ]    head=1 tail=2 count=1
push(C)：    [ _ ][ B ][ C ][ _ ]    head=1 tail=3 count=2
push(D)：    [ _ ][ B ][ C ][ D ]    head=1 tail=0 count=3  (tail 回绕)
pop -> B：   [ _ ][ _ ][ C ][ D ]    head=2 tail=0 count=2
```

要点：

- 队列的特性总结来讲是"**先进先出**"（FIFO），先入队的数据先出队；
- `head` / `tail` 分别表示下一个读、写位置；
- `(tail + 1) % QUEUE_SIZE` 实现回绕，"%"（取模）让下标穿出数组尾部后回到 0；
- 用 `count` 区分"空"（count == 0）和"满"（count == QUEUE_SIZE）；
- 队列适合解耦"产生数据的 ISR"和"处理数据的主循环"，是嵌入式里**最常用的数据结构之一**；
- 队列满时要**明确策略**：丢弃（return false）、覆盖（覆盖最老数据），还是跳过（本课程采用丢弃并记录的方式）,不同的策略有不同的应用场景；
- 单生产者单消费者的场景下，`uint8_t` 的 `head`/`tail`/`count` 读写是原子的（Cortex-M7 上），暂时不需要关中断保护。多生产者或用更大类型时要额外注意原子性。

### 函数指针与命令表(了解思想即可)

#### 为什么需要函数指针

如果有 4 种命令要处理，一般写法通常是一大段 `switch`：

```c
void process(uint8_t cmd)
{
    switch (cmd)
    {
        case CMD_TOGGLE_LED1:  toggle_led1();  break;
        case CMD_LED_LOOP:     led_loop();     break;
        case CMD_BUZZER_SHORT: buzzer_short(); break;
        case CMD_SWITCH_SPEED: switch_speed(); break;
    }
}
```

每加一种命令就要多一个 `case`，函数越来越长。函数指针和命令表可以把"命令码 -> 处理函数"的映射集中在一张表里，处理逻辑只写一次。

#### 语法

```c
// 定义一个函数指针类型：指向"返回 void、没有参数"的函数
typedef void (*cmd_handler)(void);

// 用数组做命令表：数组下标是命令码，数组值是函数指针
// 以下赋值的对象都是函数名，函数名本身就是函数入口地址
static const cmd_handler command_table[CMD_COUNT] =
{
    [CMD_TOGGLE_LED1]  = cmd_toggle_led1,
    [CMD_LED_LOOP]     = cmd_led_loop,
    [CMD_BUZZER_SHORT] = cmd_buzzer_short,
    [CMD_SWITCH_SPEED] = cmd_switch_speed
};

// 处理命令只需要：检查边界 -> 查表 -> 调函数
void command_process_one(uint8_t cmd)
{
    if (cmd >= CMD_COUNT || command_table[cmd] == NULL)
    {
        return;
    }
    command_table[cmd]();   // 通过函数指针调用
}
```

#### 执行流程

```text
收到命令码 cmd
  -> 检查 cmd 是否在合法范围内
  -> 检查 command_table[cmd] 是不是空指针
  -> 从表中取出函数指针
  -> 通过指针执行对应的处理函数
```

要点：

- `cmd_handler` 是一个"类型"，它描述的是：指向一个返回 `void`、没有参数的函数的指针。用 `typedef` 给这个指针类型起了一个可读的名字；
- 函数名本身就是函数入口地址，`cmd_toggle_led1` 可以取地址写成 `&cmd_toggle_led1`，也可以直接写函数名；
- 函数指针表把"命令码 -> 处理函数"的关系集中管理，新增命令只需要三处改动：增加枚举值、增加处理函数、表中增加一项；
- 使用命令表前要检查数组越界和空指针，防止未实现的命令导致崩溃；
- 本课程先理解"用表代替 switch"的思想，复杂场景的函数指针（带参数、带返回值、多级表）留到后续实战中深入。



## 作业与参考资料

- 当天作业：[doc/作业.md](doc/作业.md)
- Day1 工程 README：`keil_proj_demo/README.md` — 编译原理、USER CODE 分区、基础语法
- 8.9 工程 README：`keil_proj_advance/README.md` — 关键字补全、枚举、结构体、状态机
- [C语言语法基础网课](https://www.bilibili.com/video/BV1qCSkY7EyD?p=19)，推荐带着问题去看

## 暂时不展开的内容

链表、多级指针、复杂并发保护（临界区、关中断、互斥锁），自行了解。

---
