# MISRA C Check List

---

## 引言

### 1.1 起源与目的

**MISRA（Motor Industry Software Reliability Association）于 1998 年发布针对汽车嵌入式 C 语言的软件开发规范。**

**主要目标**：

- 提高软件安全性与可靠性
- 减少 C 语言易出错特性（如隐式类型转换、未初始化变量、指针误用）对汽车 MCU 软件的影响
- 增强可维护性和可移植性

---

### 1.2 规则体系结构

- **总量**：MISRA C:2012 共 143 条规则 + 16 条指令（Directives）
- **规则编号格式**：Rule 章节.序号（例如 Rule 1.5）
  - 前半部分"1"表示章节或主题分类（如变量与类型）
  - 后半部分"5"表示该章节下具体规则序号
- **规则级别**：
  - **Mandatory（强制）**：必须遵守
  - **Required（必需）**：必须遵守，但在特殊情况下可经例外批准
  - **Advisory（建议）**：推荐遵守，不违反不会直接报错

---

## 一、变量与类型（Rule 1–9）

本章节涵盖 9 条规则，核心目标是保证变量的初始化、安全访问和类型一致性，以降低 MCU 软件运行中因未初始化、类型不匹配或错误访问导致的风险。

### Rule 1.1 — 变量必须显式初始化（Mandatory）

**核心要求**：所有变量在使用前必须显式赋初值，避免随机值。

```c
static uint16_t counter = 0; // 文件静态变量显式初始化

void InitData(void)
{
    uint8_t localFlag = 0; // 局部变量显式初始化
}
```

---

### Rule 1.2 — 使用固定宽度整数类型（Mandatory）

**核心要求**：使用明确宽度类型（如 uint8_t, uint16_t, uint32_t）。

```c
#include <stdint.h>
uint8_t  sensorStatus;   // 1 字节
uint16_t sensorValue;    // 2 字节
uint32_t timestamp;      // 4 字节
```

---

### Rule 1.3 — 避免未定义行为的类型转换（Required）

**核心要求**：禁止隐式或危险类型转换，尤其是 signed/unsigned 或不同宽度整数之间的隐式转换。

```c
uint8_t a = 200;
int8_t  b = -50;
// 错误示例：隐式转换可能溢出
int16_t c = a + b;
// 安全示例：显式转换
int16_t c_safe = (int16_t)a + (int16_t)b;
```

---

### Rule 1.4 — 常量和宏应使用 const 或枚举（Required）

**核心要求**：避免魔法数字或硬编码，通过 const 或枚举定义常量。

```c
#define MAX_SPEED 100   // 不推荐

const uint8_t maxSpeed = 100; // 推荐

typedef enum {
    MODE_OFF = 0,
    MODE_ON  = 1
} PowerMode_t;
```

---

### Rule 1.5 — 结构体字段必须显式初始化（Required）

**核心要求**：结构体声明时或使用前必须显式初始化每个字段。

```c
typedef struct {
    uint8_t  status;
    uint16_t value;
} SensorData_t;

SensorData_t sensor = { .status = 0, .value = 0 }; // 显式初始化
```

---

### Rule 1.6 — 非结构体跨文件变量通过接口访问（Required）

**核心要求**：跨文件访问标量变量应通过 getter/setter 接口，而不是直接 extern。

```c
// temp.c
static uint16_t temperature;
void Temp_Set(uint16_t v) { temperature = v; }
uint16_t Temp_Get(void) { return temperature; }
// main.c
#include "temp.h"
Temp_Set(100);
uint16_t t = Temp_Get();
```

---

### Rule 1.7 — 复杂结构体跨文件仅在必要时直接共享（Required）

**核心要求**：复杂结构体只有在确实需要跨文件共享时才使用 extern，否则通过接口封装。

```c
// shared_data.h
typedef struct {
    uint8_t  status;
    uint16_t value;
} SharedData_t;
extern SharedData_t g_sharedData;
// shared_data.c
SharedData_t g_sharedData = {0};
```

---

### Rule 1.8 — 避免变量隐藏（shadowing）（Required）

**核心要求**：禁止局部变量覆盖同名全局或静态变量。

```c
static uint16_t counter = 0;

void Func(void)
{
    uint16_t counter = 5; // 错误示例，隐藏全局 counter
}
```

---

### Rule 1.9 — 变量命名应清晰反映用途和作用域（Advisory）

**核心要求**：命名应描述变量用途和作用域，便于理解。

**嵌入式注意点**：

- 全局变量可加 `g_` 前缀
- 文件静态变量可加 `s_` 前缀
- 局部变量无前缀，使用语义化名称

```c
static uint16_t s_sensorCounter; // 文件静态
uint8_t g_systemStatus;          // 全局变量
void ProcessData(uint8_t sensorValue); // 局部参数命名清晰
```

---

## 二、条件控制（Rule 10–28）

本章节涵盖 19 条规则，核心目标是保证条件表达式的安全、逻辑清晰和可维护性，避免由于隐式类型转换、赋值、穿透逻辑或魔法数字导致的 MCU 软件运行异常或潜在风险。

### Rule 2.1 — 禁止在条件表达式中使用赋值（Mandatory）

**核心要求**：条件判断中禁止使用赋值操作，如 `if(a = b)`。

```c
if (a == b) { /* 正确 */ }
if (a = b)  { /* 错误 */ }
```

---

### Rule 2.2 — 条件表达式必须明确（Mandatory）

**核心要求**：条件必须返回确定布尔值，避免依赖实现定义行为。

```c
if ((status & FLAG_READY) != 0)
{
    /* 推荐 */
}
```

---

### Rule 2.3 — 布尔表达式应显式使用比较（Required）

**核心要求**：避免使用整数直接作为条件，必须明确比较。

```c
if (sensorReady != 0) { /* 正确 */ }
if (sensorReady)       { /* 允许，但推荐第一种 */ }
```

---

### Rule 2.4 — 三元运算符条件部分必须明确（Required）

**核心要求**：使用 `?:` 时，条件表达式必须明确布尔结果。

```c
int val = (a > b) ? 1 : 0; // 正确
int val = a ? 1 : 0;        // 建议显式比较
```

---

### Rule 2.5 — 条件中不允许混合不同类型（Required）

**核心要求**：避免整型、浮点型、枚举混用。

```c
if ((int16_t)a > (int16_t)b) { /* 正确 */ }
```

---

### Rule 2.6 — if/else 必须使用块 {} 包围（Required）

**核心要求**：所有 if/else 语句必须用花括号包围。

```c
if (flag) 
{
    doSomething();
}
else
{
    doOther();
}
```

---

### Rule 2.7 — switch 必须有默认分支（Required）

**核心要求**：每个 switch 必须提供 default 处理。

```c
switch(mode){
    case MODE_OFF: 
        handleOff(); 
        break;
    case MODE_ON: 
        handleOn(); 
        break;
    default: 
        handleDefault(); 
        break;
}
```

---

### Rule 2.8 — switch case 标签值必须唯一（Required）

**核心要求**：每个 case 的值不可重复。

---

### Rule 2.9 — switch 不允许穿透（fallthrough）（Required）

**核心要求**：每个 case 必须显式 break 或 return，禁止隐式穿透。

```c
switch(mode){
    case 0: 
        handle0(); 
        break; // 正确
    case 1: 
        handle1(); 
        return;
}
```

---

### Rule 2.10 — 循环条件必须可确定（Required）

```c
while (counter < MAX_COUNT)  // 正确
{
    counter++;
}
```

---

### Rule 2.11 — 循环计数器类型必须明确（Required）

```c
for (uint16_t i = 0; i < MAX; i++)
{
    /* 正确 */
}
```

---

### Rule 2.12 — goto 禁止跳入或跳出条件块（Required）

```c
if (flag)
{
    goto label; // 错误
} 
label: ;
```

---

### Rule 2.13 — 逻辑表达式不允许副作用（Required）

```c
if (a++ > 10) // 错误
{
}

if (a > 10)  // 正确
{
    a++;
}
```

---

### Rule 2.14 — 条件中禁止使用魔法数字（Required）

```c
if (sensorValue > MAX_SENSOR_VAL)  // 推荐
{
}
```

---

### Rule 2.15 — 对指针判断必须显式与 NULL 比较（Required）

```c
if (ptr != NULL)  /* 安全 */
{
}
```

---

### Rule 2.16 — 条件中不允许使用可变位操作（Required）

```c
if ((flags |= 0x01)) // 错误
{
}
```

---

### Rule 2.17 — 条件避免依赖实现定义行为（Required）

```c
if (sizeof(int) > 4) /* 正确 */
{
}
```

---

### Rule 2.18 — 复杂条件应拆分为子表达式（Advisory）

```c
bool cond1 = (a>0 && b<100);
bool cond2 = (c==5 && d!=0);

if (cond1 || cond2)    /* 正确 */
{
}
```

---

## 三、循环控制（Rule 29–50）

本章节涵盖 22 条规则，核心目标是保证循环结构安全、可预测并易于维护，避免死循环、溢出、未定义行为或循环计数错误对 MCU 软件造成影响。

### Rule 3.1 — 循环必须保证终止条件可达（Mandatory）

```c
for (uint16_t i=0; i<MAX_COUNT; i++)    /* 正确 */
{
}
```

---

### Rule 3.2 — 循环计数器类型必须固定宽度（Mandatory）

```c
for (uint8_t idx=0; idx<10; idx++) /* 正确 */
{
}
```

---

### Rule 3.3 — 循环体内禁止修改循环变量外部依赖（Required）

```c
uint16_t cnt = MAX;

while (cnt>0)  // 正确
{
    cnt--;
}

while (cnt>0) // 错误，循环逻辑被破坏
{
    cnt = 0;
}
```

---

### Rule 3.4 — 禁止在循环条件中使用赋值（Required）

```c
while (flag = 1)  // 错误
while (flag == 1) // 正确
```

---

### Rule 3.5 — 循环中避免使用魔法数字（Required）

```c
for (uint16_t i=0; i<MAX_RETRY; i++) /* 正确 */
```

---

### Rule 3.6 — 避免无限循环，必要时加 Watchdog 或超时检测（Required）

```c
uint16_t timeout = 1000;

while (!flag && timeout--)  // 正确，防止死循环
{
}
```

---

### Rule 3.7 — 循环体必须使用花括号（Required）

```c
for (i=0; i<10; i++)
{
    doSomething();
}
```

---

### Rule 3.8 — 循环中禁止复杂表达式（Advisory）

```c
while ( ((a > 0) && (b < 100))
     || ((c == 5) && (d != 0)))     /* 拆分更易读 */
{
}
```

---

### Rule 3.9 — 避免嵌套循环过深（Advisory）

**核心要求**：嵌套循环深度应尽量 ≤2，必要时拆分函数。

---

### Rule 3.10 — 循环变量只在循环体内使用（Advisory）

```c
for (uint16_t i = 0; i < N; i++)
{
    process(i);
}
```

---

### Rule 3.11 — do-while 循环条件必须明确（Required）

```c
do
{
    readSensor();
} while (sensorReady != 0);
```

---

### Rule 3.12 — 循环体内部禁止使用 goto 跳出循环（Required）

```c
goto label; // 错误
break;       // 正确跳出
```

---

### Rule 3.13 — 避免循环中动态内存分配（Required）

```c
for (...) /* 错误 */
{
    ptr = malloc(...); 
}
```

---

### Rule 3.14 — 循环中禁止修改外部全局状态（Advisory）

```c
uint16_t localVal = g_counter;

for(...) 
{
    localVal++;  // 安全
}
g_counter++;     // 警示
```

---

### Rule 3.15 — 循环计数器避免超过类型范围（Required）

```c
for (uint8_t i=0; i<300; i++)  // 错误，uint8_t 最大 255
```

---

### Rule 3.16 — 循环退出条件必须可预测（Required）

```c
while (flagSensor) // 正确，flagSensor 定义明确
{
}
```

---

### Rule 3.17 — 循环体函数调用注意执行时间（Advisory）

**核心要求**：循环体内调用函数执行时间应可预测，避免阻塞 MCU 主循环。

---

### Rule 3.18 — 循环中禁止使用浮点变量（Required）

```c
for (i = 0; i < N; i++)
{
    float val = 0.1f; // 尽量避免
}
```

---

### Rule 3.19 — 循环计数器初始化必须明确（Mandatory）

```c
for (uint16_t i=0; i<N; i++)
{
}
```

---

### Rule 3.20 — 循环条件避免依赖外部中断触发（Required）

```c
while (g_flag)
{
    /* 安全读取局部副本 */
}
```

---

### Rule 3.21 — 避免循环体内函数多次修改同一全局变量（Advisory）

```c
for (...) 
{
    func1(); // func1/func2 不应同时修改同一全局变量
    func2();
}
```

---

### Rule 3.22 — 循环计数器尽量局部化（Advisory）

```c
for (uint16_t i = 0; i < N; i++)
{
}
```

---

## 四、函数（Rule 51–80）

本章节涵盖 30 条规则，核心目标是确保函数设计清晰、安全、可维护，并避免因参数传递、返回值或副作用导致 MCU 软件异常。

### Rule 4.1 — 每个函数应有明确的功能（Mandatory）

```c
void ReadSensor(void);       // 正确
void ReadSensorAndCompute();  // 功能混合，应拆分
```

---

### Rule 4.2 — 函数参数数量应控制（Advisory）

**核心要求**：推荐参数数量 ≤4。

```c
void SetConfig(uint16_t mode, uint16_t speed); // 合理
void SetConfig(uint16_t mode, uint16_t speed, uint16_t delay, uint16_t flag, uint16_t level); // 参数过多
```

---

### Rule 4.3 — 参数类型应明确且固定宽度（Mandatory）

```c
void SetThreshold(uint16_t threshold); // 正确
void SetThreshold(unsigned int threshold); // 不推荐
```

---

### Rule 4.4 — 使用 const 修饰只读参数（Required）

```c
void ProcessData(const uint8_t* data, uint16_t length);
```

---

### Rule 4.5 — 避免返回指针指向局部变量（Mandatory）

```c
uint8_t* GetBuffer(void)
{
    static uint8_t buf[10];
    return buf; // 静态变量允许
}
```

---

### Rule 4.6 — 避免返回指针指向动态分配的局部对象（Mandatory）

**核心要求**：避免返回临时 `malloc` 的地址，确保生命周期可控。

---

### Rule 4.7 — 函数应有明确返回值（Mandatory）

```c
int16_t ReadSensor(void){
    return sensorValue; // 正确
}
```

---

### Rule 4.8 — 避免函数内静态变量造成非线程安全（Required）

```c
static uint16_t counter; // 使用需加锁或仅限单任务访问
```

---

### Rule 4.9 — 函数长度应可控（Advisory）

**核心要求**：函数行数宜短，≤50 行为佳。

---

### Rule 4.10 — 避免函数副作用过多（Advisory）

```c
void UpdateStatus(Status_t* s); // 安全
void UpdateGlobalAndPeripheral(); // 副作用过多
```

---

### Rule 4.11 — 函数名应清晰反映功能（Advisory）

```c
void ReadTemperature(void); // 清晰
void DoIt(void);             // 不清晰
```

---

### Rule 4.12 — 避免递归（Required）

**核心要求**：MCU 栈有限，递归可能导致溢出。

```c
void FuncA(void)
{
    FuncA();   // 禁止
}
```

---

### Rule 4.13 — 函数参数不要依赖全局状态（Required）

```c
void Process(uint16_t value); // 安全
void ProcessGlobal(void);      // 警示
```

---

### Rule 4.14 — 函数中禁止魔法常量（Required）

```c
#define MAX_RETRY 10

for (i = 0; i < MAX_RETRY; i++)
{
}
```

---

### Rule 4.15 — 函数内变量初始化（Mandatory）

```c
void Func(void){
    uint16_t cnt = 0;
}
```

---

### Rule 4.16 — 函数接口应保持一致性（Advisory）

```c
void Sensor_SetThreshold(uint16_t t);
void Sensor_GetValue(uint16_t* val);
```

---

### Rule 4.17 — 避免函数依赖未定义行为（Required）

```c
uint8_t arr[10];
arr[10] = 0; // 错误，越界访问
```

---

### Rule 4.18 — 函数文档化（Advisory）

```c
/**
 * @brief 读取传感器值
 * @param  sensorId 传感器编号
 * @return 传感器值
 */
```

---

### Rule 4.19 — 避免函数过长生命周期副作用（Advisory）

```c
void InitModule(void); // 短生命周期，初始化后返回
```

---

### Rule 4.20 — 函数内禁止中断敏感操作（Required）

```c
uint16_t value;
EnterCritical();
value = g_counter;
ExitCritical();
```

---

### Rule 4.21 — 避免函数中使用全局指针（Required）

```c
void Process(uint16_t* ptr); // 避免指向全局指针
```

---

### Rule 4.22 — 函数内禁止动态内存分配（Required）

**核心要求**：MCU 循环调用 `malloc`/`free` 会破坏实时性。

---

### Rule 4.23 — 函数返回值应检查（Mandatory）

```c
int16_t ret = ReadSensor();
if(ret < 0){ HandleError(); }
```

---

### Rule 4.24 — 避免函数内部副作用修改全局状态（Advisory）

```c
void UpdateStatus(Status_t* s);
```

---

### Rule 4.25 — 函数尽量可重入（Required）

```c
void Process(const uint8_t* data);
```

---

### Rule 4.26 — 函数异常路径处理（Mandatory）

**核心要求**：函数必须处理异常或返回错误码。

---

### Rule 4.27 — 避免函数内长时间阻塞（Required）

```c
void Func(void){ /* 禁止长循环 */ }
```

---

### Rule 4.28 — 函数内部循环控制遵循循环章节规则（Required）

```c
for (i = 0; i < N; i++)
{
    ProcessData(); 
}
```

---

### Rule 4.29 — 函数内条件判断简明（Advisory）

**核心要求**：避免复杂嵌套条件影响可读性，必要时拆分函数。

---

### Rule 4.30 — 函数参数和返回值类型一致（Required）

```c
uint16_t GetValue(void); // 返回类型与使用方一致
```

---

## 五、条件控制进阶（Rule 81–120）

本章节涵盖 40 条规则，核心目标是保证条件判断安全、清晰、可预测，避免因逻辑错误、未定义行为或边界问题导致 MCU 软件异常。

### Rule 5.1 — 条件表达式必须明确（Mandatory）

```c
if ((status & 0x01) != 0)  /* 明确 */
if (status & 0x01)          /* 不推荐 */
```

---

### Rule 5.2 — 避免赋值运算出现在条件中（Required）

```c
if (x == 0)  // 正确
if (x = 0)   // 错误
```

---

### Rule 5.3 — 条件判断结果应为布尔类型（Advisory）

```c
bool flag = (x > 0);
```

---

### Rule 5.4 — 避免复杂嵌套条件（Required）

**核心要求**：条件嵌套深度推荐 **≤3** 层。

---

### Rule 5.5 — 条件判断边界应清晰（Mandatory）

```c
if ((index >= 0) && (index < ARRAY_SIZE))
{
}
```

---

### Rule 5.6 — 避免重复条件（Required）

```c
if ((x>0) && (x>0)) // 错误
```

---

### Rule 5.7 — 避免条件依赖未定义行为（Mandatory）

```c
if ((a / b) > 0) // 检查 b != 0
```

---

### Rule 5.8 — 使用括号明确逻辑运算优先级（Required）

```c
if ((a && b) || c) // 明确
if (a && b || c)   // 易混淆
```

---

### Rule 5.9 — 避免条件中函数副作用（Required）

```c
if (GetFlag())        // 安全
if (IncrementCounter()) // 不推荐
```

---

### Rule 5.10 — 条件表达式应可测试（Advisory）

```c
if ((x > 0) && (y < 100))      // 可测试
if ((x > 0) && (y < MAX_UINT16)) // 难以覆盖
```

---

### Rule 5.11 — 避免空条件语句（Required）

```c
if(flag) { DoSomething(); } // 正确
if(flag);                   // 错误
```

---

### Rule 5.12 — 条件判断逻辑应简明（Advisory）

---

### Rule 5.13 — switch 必须包含 default（Mandatory）

```c
switch(mode){
    case MODE_OFF: ...
    case MODE_ON: ...
    default: HandleError(); 
}
```

---

### Rule 5.14 — switch case 不得重复（Required）

---

### Rule 5.15 — switch case 应覆盖所有枚举（Advisory）

```c
typedef enum { RED, GREEN, BLUE } Color;
switch(c)
{
    case RED: ... 
    case GREEN: ... 
    case BLUE: ... 
}
```

---

### Rule 5.16 — switch 内禁止 fall-through（Required）

```c
case 0: 
    DoSomething();
    break;
case 1: 
    DoSomethingElse(); 
    break;
```

---

### Rule 5.17 — 条件表达式应避免副作用（Required）

```c
if ((x++) > 0) // 错误
if(x > 0) { x++; } // 安全
```

---

### Rule 5.18 — 使用逻辑常量时显式比较（Required）

```c
if (flag == true)
```

---

### Rule 5.19 — 避免条件中多层函数调用（Advisory）

```c
bool tmp = CheckA() && CheckB();
if (tmp) { }
```

---

### Rule 5.20 — 避免条件短路副作用（Required）

```c
if ((flagA && (counter++ > 0))) // 错误
```

---

### Rule 5.21 — 条件表达式避免魔法数字（Required）

```c
#define MAX_RETRY 10
if(count < MAX_RETRY) { }
```

---

### Rule 5.22 — 避免条件判断依赖未初始化变量（Mandatory）

```c
uint8_t flag;
if (flag)  // 错误
```

---

### Rule 5.23 — 条件判断分支必须可达（Required）

```c
if (a>0) { ... }
else if (a>0) { ... }  // 错误：死代码
else { }
```

---

### Rule 5.24 — 条件判断中避免强制类型转换（Required）

```c
if ((int)x > 0)  // 不推荐
```

---

### Rule 5.25 — 条件逻辑应可单元测试（Advisory）

```c
bool isValid = ((x > 0) && (y < 100));
if (isValid) { }
```

---

### Rule 5.26 — 条件中避免函数返回未定义值（Mandatory）

```c
if (GetValue() > 0)  // 安全前提：GetValue 返回有效值
```

---

### Rule 5.27 — 条件表达式应可读性良好（Advisory）

```c
bool result = (a>0) && (b>0) && (c<10);
```

---

### Rule 5.28 — 避免条件嵌套过深（Required）

**核心要求**：嵌套超过 3 层应拆分函数。

---

### Rule 5.29 — 条件判断中避免全局状态依赖（Required）

```c
if (GetFlag(param)) { }
```

---

### Rule 5.30 — 条件表达式结果类型一致（Required）

```c
bool flag = (x > 0);
```

---

### Rule 5.31 — 避免条件表达式重复计算（Required）

```c
bool tmp = (x > 0) && (y < 10);
if (tmp) { }
```

---

### Rule 5.32 — 避免条件中使用非标准运算符（Required）

```c
if (x ** 2 > 10) // 错误
```

---

### Rule 5.33 — 条件逻辑应保持简洁（Advisory）

```c
if(a && b && c) { }
```

---

### Rule 5.34 — 条件判断注释应清晰（Advisory）

```c
if ((a > 0) && (b < 10))  /* 检查输入有效性 */  
```

---

### Rule 5.35 — 条件表达式避免未定义宏（Required）

```c
#define FLAG_VALID 1
if (flag == FLAG_VALID) { }
```

---

### Rule 5.36 — 避免条件中使用静态变量（Required）

```c
static uint8_t counter;
if (counter > 0) { }  // 可能引起副作用
```

---

### Rule 5.37 — 条件中避免修改数组索引（Required）

```c
if (arr[i++] > 0)  // 错误
```

---

### Rule 5.38 — 条件判断中避免调用阻塞函数（Required）

```c
if (DelayMs(10)) // 不允许
```

---

### Rule 5.39 — 条件表达式结果应与期望类型匹配（Required）

```c
bool flag = (x != 0);
```

---

### Rule 5.40 — 条件判断应覆盖边界场景（Mandatory）

```c
if ((index >= 0) && (index < ARRAY_SIZE))
```

---

## 六、循环控制进阶（Rule 121–143）

本章节涵盖 23 条规则，核心目标是保证循环结构安全、可预测、边界明确，避免死循环、越界和未定义行为，提高 MCU 程序稳定性。

### Rule 6.1 — 循环必须可控（Mandatory）

```c
for (uint16_t i = 0; i < MAX_COUNT; i++)  /* 安全 */ 
```

---

### Rule 6.2 — 避免无限循环（Required）

```c
while(1) 
{
    DoSomething();
    break;  // 必须有退出机制
}
```

---

### Rule 6.3 — 循环边界应明确（Mandatory）

```c
for (uint8_t i = 0; i<10; i++) { }
```

---

### Rule 6.4 — 循环中避免修改循环计数器（Required）

**核心要求**：禁止在循环体中修改计数器，保证可预测性。

---

### Rule 6.5 — 循环条件表达式应简洁（Advisory）

```c
for (i = 0; i < MAX; i++) { }  // 简单清晰
```

---

### Rule 6.6 — 避免循环中出现副作用（Required）

```c
while (UpdateCounter())  // 不推荐
```

---

### Rule 6.7 — 循环变量类型匹配（Mandatory）

```c
for (size_t i = 0; i < ARRAY_SIZE; i++) { }
```

---

### Rule 6.8 — 避免循环体空语句（Required）

```c
for (i = 0; i < MAX; i++); // 错误
```

---

### Rule 6.9 — 循环中避免复杂条件（Advisory）

---

### Rule 6.10 — 避免循环变量溢出（Mandatory）

```c
for (uint8_t i = 0; i < 255; i++) // 注意循环上限
```

---

### Rule 6.11 — 循环退出条件应可靠（Required）

```c
while (flag)  // flag 必须初始化
```

---

### Rule 6.12 — 避免循环体中函数副作用（Required）

```c
for (i = 0;i < N; i++)
{
    ProcessData();  // 函数内自行管理状态
}
```

---

### Rule 6.13 — 循环中避免访问越界数组（Mandatory）

```c
for (i = 0; i < ARRAY_SIZE; i++)
{
    data[i] = 0;
}
```

---

### Rule 6.14 — 循环体应易读（Advisory）

```c
for (i = 0; i < N; i++)
{
    if (flag) { Process(i); }
}
```

---

### Rule 6.15 — 避免循环嵌套过深（Required）

**核心要求**：嵌套深度 > 3 应拆分函数。

---

### Rule 6.16 — 循环中变量应尽量局部（Advisory）

```c
for (int i = 0; i < N; i++)
{
    int tmp = i * 2;
}
```

---

### Rule 6.17 — 循环中避免使用浮点（Required）

```c
for (int i = 0; i < N; i++)
{
    int val = i * 2;  // 避免 float
}
```

---

### Rule 6.18 — 循环条件应避免魔法数字（Required）

```c
for (int i = 0; i < MAX_COUNT; i++) { }
```

---

### Rule 6.19 — 循环体应避免未使用变量（Advisory）

**核心要求**：清理未使用变量，保持代码整洁。

---

### Rule 6.20 — 循环变量不可修改其他全局状态（Required）

**核心要求**：保证循环可预测性。

---

### Rule 6.21 — 循环中避免调用阻塞函数（Required）

**核心要求**：嵌入式实时任务循环禁止阻塞调用。

---

### Rule 6.22 — 循环中避免调用可能出错的函数（Required）

**核心要求**：函数必须保证安全，避免循环中异常中断。

---

### Rule 6.23 — 循环变量结果应可单元测试（Advisory）

```c
for (int i = 0; i < N; i++)
{
    ProcessStep(i);
}
```

---

## 七、MISRA C:2012 指令（Directives）

| 指令编号 | 名称 | 核心内容 |
|----------|------|----------|
| Directive 1 | 可移植性检查 | 确保代码在不同编译器和硬件平台上具有可移植性 |
| Directive 2 | 头文件包含策略 | 每个头文件应自包含；防止重复包含（使用 include guards 或 `#pragma once`） |
| Directive 3 | 注释使用规范 | 注释必须清晰、准确，解释"为什么"而非"做什么"；避免误导性或过时注释 |
| Directive 4 | 代码可读性和结构 | 代码应模块化、清晰、易读；控制结构应尽量简单，减少嵌套层次 |
| Directive 5 | 预处理器指令使用 | 仅在必要时使用 `#define`、`#if` 等预处理器指令；避免宏滥用 |
| Directive 6 | 编译器依赖性检查 | 避免依赖特定编译器扩展或行为 |
| Directive 7 | 可测试性 | 代码应支持静态分析和单元测试；功能模块应独立、接口清晰 |
| Directive 8 | 错误处理策略 | 对可能出现的错误情况进行明确处理（如返回码、异常处理） |
| Directive 9 | 函数接口设计 | 函数参数、返回值、全局依赖清晰定义；避免副作用 |
| Directive 10 | 变量和类型使用 | 明确定义变量类型，避免隐式转换；遵循命名规范 |
| Directive 11 | 循环和控制结构 | 避免复杂嵌套和不可预测循环；控制结构尽量简单明了 |
| Directive 12 | 宏定义和常量 | 宏仅用于必要场景，优先使用常量、枚举或 inline 函数 |
| Directive 13 | 静态分析工具兼容性 | 编写代码时考虑静态分析工具的检查能力 |
| Directive 14 | 库函数使用策略 | 避免使用不可预测或非标准库函数；优先使用标准、可移植的库函数 |
| Directive 15 | 对外接口文档规范 | 所有对外接口（函数、模块、通信协议）必须有明确文档说明 |
| Directive 16 | 安全与关键功能的审查 | 安全关键功能必须经过专门审查，设计和实现遵循高可靠性标准 |

---

## 八、补充内容

### 8.1 C 语言空格使用指南

#### 空格使用规则总结

| 类别 | 规则描述 |
|------|----------|
| **1. 关键字** | `if`, `for`, `while`, `do`, `switch`, `case` 等关键字后必须加一个空格 |
| **2. 函数名** | 函数名与其后的左括号 `(` 之间不加任何空格 |
| **3. 括号** | 括号（圆括号 `()`、方括号 `[]`）的内侧不加空格，与内部内容紧贴 |
| **4. 大括号** | 通常左大括号 `{` 放在语句末尾或新行，右大括号 `}` 单独一行；内部语句缩进 |
| **5. 二元运算符** | 所有二元运算符（算术、逻辑、比较、位操作）两边都必须加空格 |
| **6. 赋值运算符** | 赋值运算符（`=`, `+=`, `-=` 等）两边都必须加空格 |
| **7. 一元运算符** | 一元运算符（`!`, `~`, `++`, `--`, `*`（指针）, `&`（取地址））与它的操作数之间不加空格 |
| **8. 逗号** | 逗号 `,` 后面必须加一个空格，前面不加空格 |
| **9. 分号** | 分号 `;` 前面不加空格（for 循环中的三个部分除外）；语句结束后换行或跟随空格 |
| **10. 结构体/指针访问** | 成员访问运算符 `.` 和 `->` 周围不加空格 |

#### 代码示例

```c
#include <stdio.h>

int main(void)
{
    int a = 5;
    int b = 10;
    int c = 15;
    int result = 0;
    int* ptr = &a; // 一元运算符 * 和 & 紧贴操作数

    // 条件控制
    if ((a > 0) && (b < 20) || (c == 15))
    {
        result = 1;
    }

    // 循环控制
    for (int i = 0; i < 10; i++) // for 循环中的分号后加空格
    {
        result += i; // 二元运算符 += 两边加空格
    }

    while ((a < 10) && (b > 5))
    {
        a++; // 一元运算符 ++ 紧贴操作数
    }

    do
    {
        b--;
    } while ((b > 0) && (a < 20));

    // 赋值和算术运算
    a = b + c;
    result = result + (a - (b * c) / 2); // 通过括号和空格明确优先级

    // 逻辑与位运算
    if ((a > 0) && !(b == c) || (c != 0)) // 一元运算符 ! 紧贴操作数
    {
        result = 100;
    }
    int flags = (a & 0x0F) << 4; // 位运算符两边同样加空格

    // 函数调用与结构体访问
    printf("Result: %d\n", result); // 函数名后无空格，参数逗号后有空格

    return 0;
}
```

---

### 8.2 复杂条件多行分解规范

**核心原则**：清晰展示逻辑结构，让每一个逻辑单元（`&&` / `||`）都显而易见。

#### 8.2.1 方法一：运算符置于行首（最推荐）

**规则**：
- 换行后，将逻辑运算符（`&&`, `||`）作为新行的开头
- 所有后续行的逻辑运算符与第一行 `if` 关键字的位置对齐
- 使用括号将不同优先级的逻辑分组

```c
if ((system_state == STATE_READY)
    && (network_status == CONNECTED)
    && ((request_type == GET_REQUEST) || (request_type == POST_REQUEST))
    && (payload_size < MAX_PAYLOAD_SIZE)
    && !(error_flags & FATAL_ERROR_MASK))
{
    process_request();
}

// 嵌套分组的情况，使用额外缩进
if ( (a > 0)
    && ( (b_is_valid && (b < MAX_B)) // 嵌套组，多缩进一层
        || (c_is_ready && (c != LAST_VALUE)) )
    && (d == 0))
{
    do_work();
}
```

**优点**：
- 易于阅读：眼睛只需向左扫视就能看到所有的逻辑连接词
- 易于调试：注释掉任何一行都非常容易，不会破坏语法
- 易于修改：git diff 的变更记录会更清晰

---

#### 8.2.2 方法二：运算符置于行尾（较传统，不推荐）

```c
if ((system_state == STATE_READY) &&
    (network_status == CONNECTED) &&
    ((request_type == GET_REQUEST) || (request_type == POST_REQUEST)) &&
    (payload_size < MAX_PAYLOAD_SIZE) &&
    !(error_flags & FATAL_ERROR_MASK))
{
    process_request();
}
```

**缺点**：
- 可读性稍差：逻辑运算符容易被忽略，"隐藏"在行尾
- 难以修改和调试：注释掉一个条件时，必须小心处理前一行末尾的运算符

---

#### 8.2.3 方法三：使用临时布尔变量（极致可读性）

```c
bool is_system_ready     = (system_state == STATE_READY);
bool is_network_ok       = (network_status == CONNECTED);
bool is_valid_request    = (request_type == GET_REQUEST) || (request_type == POST_REQUEST);
bool is_payload_valid    = (payload_size < MAX_PAYLOAD_SIZE);
bool has_no_fatal_errors = !(error_flags & FATAL_ERROR_MASK);

// 主判断条件变得非常简单、自解释
if (is_system_ready
    && is_network_ok
    && is_valid_request
    && is_payload_valid
    && has_no_fatal_errors)
{
    process_request();
}
```

**优点**：
- **极致的可读性**：变量名本身就是注释，清楚地说明了每个条件的目的
- **便于调试**：可以在调试器中轻松查看每个中间变量的值
- **代码复用**：这些变量可能在代码的其他地方也会被用到
- **简化主逻辑**：使 `if` 语句变得非常简洁，易于理解

---

> 📌 **内容来源**：CSDN 博客「烟花的学习笔记」，作者 qq_43399763，发布于 2025-08-26，遵循 CC 4.0 BY-SA 版权协议。
