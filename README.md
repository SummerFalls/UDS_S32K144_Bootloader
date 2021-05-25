# :beers: UDS_S32K144_Bootloader <a title="Hits" target="_blank" href="https://github.com/SummerFalls/UDS_S32K144_Bootloader"><img src="https://hits.b3log.org/SummerFalls/UDS_S32K144_Bootloader.svg"></a>

```c
/*
 *                        ___           ___                                     ___           ___          _____          ___           ___
 *         _____         /  /\         /  /\          ___                      /  /\         /  /\        /  /::\        /  /\         /  /\
 *        /  /::\       /  /::\       /  /::\        /  /\                    /  /::\       /  /::\      /  /:/\:\      /  /:/_       /  /::\
 *       /  /:/\:\     /  /:/\:\     /  /:/\:\      /  /:/    ___     ___    /  /:/\:\     /  /:/\:\    /  /:/  \:\    /  /:/ /\     /  /:/\:\
 *      /  /:/~/::\   /  /:/  \:\   /  /:/  \:\    /  /:/    /__/\   /  /\  /  /:/  \:\   /  /:/~/::\  /__/:/ \__\:|  /  /:/ /:/_   /  /:/~/:/
 *     /__/:/ /:/\:| /__/:/ \__\:\ /__/:/ \__\:\  /  /::\    \  \:\ /  /:/ /__/:/ \__\:\ /__/:/ /:/\:\ \  \:\ /  /:/ /__/:/ /:/ /\ /__/:/ /:/___
 *     \  \:\/:/~/:/ \  \:\ /  /:/ \  \:\ /  /:/ /__/:/\:\    \  \:\  /:/  \  \:\ /  /:/ \  \:\/:/__\/  \  \:\  /:/  \  \:\/:/ /:/ \  \:\/:::::/
 *      \  \::/ /:/   \  \:\  /:/   \  \:\  /:/  \__\/  \:\    \  \:\/:/    \  \:\  /:/   \  \::/        \  \:\/:/    \  \::/ /:/   \  \::/~~~~
 *       \  \:\/:/     \  \:\/:/     \  \:\/:/        \  \:\    \  \::/      \  \:\/:/     \  \:\         \  \::/      \  \:\/:/     \  \:\
 *        \  \::/       \  \::/       \  \::/          \__\/     \__\/        \  \::/       \  \:\         \__\/        \  \::/       \  \:\
 *         \__\/         \__\/         \__\/                                   \__\/         \__\/                       \__\/         \__\/
 */
```

## :book: 简介

S32K1xx 的 CAN 接 `周立功 USBCANFD-100U-mini`，使用 `ZCANPRO` 软件的 `ECU刷新` 功能进行测试。在加载相应的安全访问算法 DLL 文件 :package: [UDS_SecurityAccess][UDS_SecurityAccess] 之后，通过相应的 UDS 服务将 :package: [UDS_S32K144_FlashDriver][UDS_S32K144_FlashDriver] 的 hex 文件下载至 :package: [UDS_S32K144_Bootloader][UDS_S32K144_Bootloader] 在链接文件中为其预先指定起始地址的 RAM 空间中，并通过 `Flash Driver` 内实际包含的相应的 Flash 驱动函数的相对偏移量以及驱动函数本身来计算相应驱动函数的入口点在 RAM 内的偏移地址后，再通过函数指针的方式调用相应的编程、擦写、校验等 `Flash API` 以实现将 :package: [UDS_S32K144_APP][UDS_S32K144_APP] 烧写至 Flash 的 APP 片区，最终实现 `ECU刷新` 的整个 APP 更新流程。

:file_folder: `ZCANPRO_RefreshFlow_S32K144.zflash` 为可被 `ZCANPRO` 的 `ECU刷新` 功能所加载的 UDS 刷新流程文件。

:warning: 注意：S32K144-EVB 开发板需要 12V 独立供电，CAN Transceiver 方可正常工作。

:game_die: 已做通用性适配，目前一套代码理论同时支持多个型号，已测试 `S32K144` 和 `S32K118` 可以同时支持。

![Pic_ZCANPRO_ECU_Refresh][Pic_ZCANPRO_ECU_Refresh]

## :link: 关联工程

- :package: [UDS_SecurityAccess][UDS_SecurityAccess]
- :package: [UDS_S32K144_Bootloader][UDS_S32K144_Bootloader]
- :package: [UDS_S32K144_FlashDriver][UDS_S32K144_FlashDriver]
- :package: [UDS_S32K144_APP][UDS_S32K144_APP]

## :gear: 硬件 & 软件 需求

### 硬件需求

- S32K144-EVB
- J-Link
- USBCANFD-100U-mini
- 12V External Power Supply

### 软件需求

- S32 Design Studio for ARM Version 2.2
- ZCANPRO
- J-Flash

<br/>

---

<br/>

### :warning: 注意

![Pic_ZCANPRO_ECU_Refresh_Note_ProgramSession][Pic_ZCANPRO_ECU_Refresh_Note_ProgramSession]
![Pic_ZCANPRO_ECU_Refresh_Note_ResetECU][Pic_ZCANPRO_ECU_Refresh_Note_ResetECU]

---

### :warning: 特别注意

< :ok_hand: 已解决 > 关于 `S32K144 APP 工程` 调用 `Flash_EraseFlashDriverInRAM()` 会导致程序卡死的问题进行说明记录：

- 根据 Ozone 调试得知，实际现象为出现 `HardFault` ， `HFSR` 寄存器 `FORCED` 位被置 1，所以可以判断由其它 Fault 异常提升而来，实际为 `UsageFault` ， `INVSTATE` 位被置 1，如下图所示：
  - ![LinkSetting_ForUsageFault_00][LinkSetting_ForUsageFault_00]
- 由于程序卡死的位置并不是每次都在同一位置，故初期的排查非常困难。由于同样的代码，在 `S32K118 APP 工程` 中被调用时，并不会产生该问题，所以需要从两个不同芯片的工程的不同之处寻找原因。经过比对，初步判断与两芯片的链接文件空间分配有关联，具体为两个芯片的 `Flash Driver` 所占用的 RAM 地址空间不同。其中，在 `S32K144` 的 `Bootloader 工程` 中，`m_flash_driver` 地址空间在 `APP 工程` 中会与 `m_data` 地址空间合并，从 Bootloader 跳转至 APP 时，会先调用 `Reset_Handler` 汇编函数，此汇编函数会进行相关初始化之后再跳转到 `main` 函数，其中在初始化时，会调用 `init_data_bss()` 函数，此函数其中的一个工作就是将位于 ROM 中（即 `m_interrupts` 地址空间内）的中断向量表拷贝到 RAM 中（即 `m_data` 地址空间内）的起始地址处，此时，如若在 `APP 工程` 中尝试调用 `Flash_EraseFlashDriverInRAM()` ，会对 `0x1FFF8000` 起始地址处连续 `0x800` 个字节的内容进行清零操作，而 `0x1FFF8000` ~ `0x1FFF8800` 地址空间段已经完全属于 `m_data` ，如若再执行 `Flash_EraseFlashDriverInRAM()` 则会将 RAM 中的中断向量表擦除，导致中断响应出现异常，故最终会导致程序卡死，详细配合以下截图进行分析。
  - ![LinkSetting_ForUsageFault_01][LinkSetting_ForUsageFault_01]
  - ![LinkSetting_ForUsageFault_02][LinkSetting_ForUsageFault_02]
- 解决方法有两种：APP 工程中一律不调用 `Flash_EraseFlashDriverInRAM()` 或将链接文件内 `m_flash_driver` 的地址空间从 `m_data_2` 地址空间的后半段进行划分。
  - 但经过评估，显然前者为更恰当的解决方法，因为在 APP 工程中，已不存在 `m_flash_driver` 地址空间，此时就算在 Bootloader 中将 `m_data_2` 的后半段空间划分给 `m_flash_driver` ，在 APP 工程中再对该片空间进行自行清零的操作是不妥的，因为 `m_data_2` 会用于存放 `customSectionBlock`、 `bss`、 `heap`、 `stack`， 此时如若再在 APP 中调用 `Flash_EraseFlashDriverInRAM()` ，同样有可能会将这些数据清零。

<br/>

---

<br/>

## :detective: UDS（Unified Diagnostic Services，统一诊断服务）

<div align="center">

## :page_with_curl: UDS 协议说明

</div>

UDS（Unified Diagnostic Services，统一诊断服务）诊断协议是 `ISO 15765` 和 `ISO 14229` 定义的一种汽车通用诊断协议，位于 OSI 模型中的应用层，它可在不同的汽车总线（例如CAN、LIN、Flexray、Internet、K-line）上实现。UDS协议的应用层定义是 `ISO 14229-1`，目前大部分汽车厂商均采用 UDS on CAN 的诊断协议。

<div align="center">

![UDS_OSI_Model][UDS_OSI_Model]

<br/>
</div>

### :triangular_flag_on_post: UDS 术语概要

| 缩写              | 英文全称                                                          | 中文含义                                                                                                |
|-------------------|-------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| UDS               | Unified Diagnostic Services                                       | 统一诊断服务                                                                                            |
| OBD               | On-Board Diagnostics                                              | 车载诊断系统                                                                                            |
| WWH-OBD           | World-wide Harmonized OBD                                         | 全球统一的重型发动机的车载诊断系统                                                                      |
| DoCAN             | Diagnostic Communication Over CAN                                 | 基于CAN的诊断通信                                                                                       |
| OSI               | Open System Interconnection                                       | 开放式系统互联                                                                                          |
| ECU               | Electronic Control Unit                                           | 电子控制单元                                                                                            |
| SID               | Service ID                                                        | 诊断服务标识符                                                                                          |
| Sub-Function      | Sub-Function                                                      | 子服务，诊断服务包含的子功能                                                                            |
| DTC               | Diagnostic Trouble Code                                           | 诊断故障码                                                                                              |
| NRC               | Negative Response Code                                            | 负响应码                                                                                                |
| FTB               | Fault Type Byte                                                   | 故障类型字节                                                                                            |
| PCI               | Protocol Control Information                                      | 协议控制信息                                                                                            |
| SF                | Single Frame                                                      | 单帧                                                                                                    |
| FF                | First Frame                                                       | 首帧                                                                                                    |
| CF                | Consecutive Frame                                                 | 连续帧                                                                                                  |
| FC                | Flow Control Frame                                                | 流控帧                                                                                                  |
| DL                | Data Length                                                       | 数据长度                                                                                                |
| FF_DL             | First Frame Data Length                                           | 首帧数据长度                                                                                            |
| SN                | Sequence Number                                                   | 序号                                                                                                    |
| SPN               | Suspect Parameter Number                                          | 可疑参数序号                                                                                            |
| FS                | Flow Status                                                       | 流控制状态                                                                                              |
| BS                | Block Size                                                        | 块的大小                                                                                                |
| STmin             | Separation Time Minimum                                           | 最小间隔时间                                                                                            |
| CTS               | Continue to Send                                                  | 继续发送                                                                                                |
| FMI               | Failure Mode Indicator                                            | 失效模式指示                                                                                            |
| Mtype             | Message Type                                                      | 信息类型                                                                                                |
| Indication        | Indication Service Primitive                                      | 指示服务原语                                                                                            |
| Confirm           | Confirm Service Primitive                                         | 确认服务原语                                                                                            |
| N_AE              | Network Address Extension                                         | 网络层扩展地址                                                                                          |
| N_AI              | Network Address Information                                       | 网络层地址信息                                                                                          |
| N_Ar              | Network Layer Timing Parameter Ar                                 | 网络层定时参数 Ar                                                                                       |
| N_As              | Network Layer Timing Parameter As                                 | 网络层定时参数 As                                                                                       |
| N_Br              | Network Layer Timing Parameter Br                                 | 网络层定时参数 Br                                                                                       |
| N_Bs              | Network Layer Timing Parameter Bs                                 | 网络层定时参数 Bs                                                                                       |
| N_Cr              | Network Layer Timing Parameter Cr                                 | 网络层定时参数 Cr                                                                                       |
| N_Cs              | Network Layer Timing Parameter Cs                                 | 网络层定时参数 Cs                                                                                       |
| N_ChangeParameter | Network Layer Service Name                                        | 网络层的服务名称                                                                                        |
| N_Data            | Network Data                                                      | 网络层数据                                                                                              |
| N_PCI             | Network Protocol Control Information                              | 网络层协议控制信息                                                                                      |
| N_PCItype         | Network Protocol Control Information Type                         | 网络层协议控制信息类型                                                                                  |
| N_PDU             | Network Protocol Data Unit                                        | 网络层协议数据单元                                                                                      |
| N_SDU             | Network Service Data Unit                                         | 网络层服务数据单元                                                                                      |
| N_SA              | Network Source Address                                            | 网络层源地址 源地址 -> 诊断仪                                                                           |
| N_TA              | Network Target Address                                            | 网络层目标地址 目标地址 -> ECU                                                                          |
| N_TAtype          | Network Target Address Type                                       | 网络层目标地址类型                                                                                      |
| N_USData          | Network Layer Unacknowledged Segmented Data Transfer Service Name | 网络层未确认分段数据传输服务名称                                                                        |
| NW                | Network                                                           | 网络层                                                                                                  |
| NWL               | Network Layer                                                     | 网络层                                                                                                  |
| UDS_PHYSREQ_CANID | Physical Address                                                  | 物理寻址 - 单播通讯，同一总线上每个 ECU 的物理寻址 CAN ID 互不相同，只有一个 ECU 会接收处理             |
| UDS_FUNCREQ_CANID | Functional Address                                                | 功能寻址 - 多播通讯，同一总线上所有 ECU 的功能寻址 CAN ID 都相同，所有具备诊断功能的 ECU 都会接收并处理 |
| UDS_RESPOND_CANID | Response Address                                                  | 响应地址 - 由 ECU 发送，诊断仪接收，各 ECU 互不相同                                                     |

<br/>

---

<br/>

### :earth_asia: UDS 网络层概述

网络层最主要的目的就是 **把数据转换成能适应 CAN 总线规范的单一数据帧，从而进行传输。** 如果将要传输的报文长度超过了 CAN 数据帧的长度，则需要将报文信息进行拆分后传输，每次至多可以传输4095个字节长度的报文。

所有的网络层服务都具有相同的通用结构。为了定义这种服务，需要定义三种类型的服务原语：

- 请求服务（**Request**）：用于**向网络层传递控制报文信息及要发送的数据**，应用于更高层或应用层。如诊断仪向 ECU 发出数据。
- 指示服务（**Indication**）：用于**向更高层或应用层传递状态信息及接收到的数据**，应用于网络层。如 ECU 收到了诊断仪的数据，传至应用层。
- 确认服务（**Confirm**）：被网络层使用，用于向更高层或应用层传递状态信息。如诊断仪收到了 ECU 方面的数据。

具体说来，Indication 前端应该执行的是底层传入数据的处理函数，即读取 PDU 信息，这一帧具体是 SF、FC、CF 还是 FF。若满足条件，继续向上，即应用层传递。

具体说来，**Confirm 和 Indication 很像的是都是从网络层向应用层传递信息，有何区别呢？**

在代码的处理中，Confirm 的前端应向底层外发 Tx 数据或超时处理函数，反馈的信息不需要包含数据。而 Indication 传递的信息则分为两种，一种包含真实数据，另一种不包含。

<br/>

---

<br/>

### :earth_asia: UDS 网络层协议数据单元 N_PDU

网络层协议数据单元（N_PDU，Network Protocol Data Unit）包含 `N_AI，N_PCI，N_Data`。即地址信息，协议控制信息和数据。

| 参数名称     | 缩写   | 描述                               |
|--------------|--------|------------------------------------|
| 寻址信息     | N_AI   | 隐含源地址、目的地址和寻址方式信息 |
| 协议控制信息 | N_PCI  | 标识网络层帧类型                   |
| 数据         | N_Data | 包含应用层协议控制信息和数据       |

**网络层协议数据单元（N_PDU）有四种类型**，即单帧（SF）、首帧（FF）、连续帧（CF）、流控制帧（FC），用于建立对等实体间的通信。

<br/>

---

<br/>

### :earth_asia: UDS 网络层协议控制信息 N_PCI

> :warning: 注意
>> 本网络传输层的报文格式为：`ISO 15765-2 2004`
>>> 而在 `ISO 15765-2 2016` 中，单帧 SF 用第 2 字节存放长度 DL，首帧 FF 用第 3 ~ 6 字节存放长度 DL，故本协议栈尚不支持 `ISO 15765-2 2016`

`ISO 15765-2` 定义了一种 CAN 帧分割的网络传输层通信协议，该协议对 CAN 帧的 `Byte0 - Byte2` 进行了定义，由此产生了`单帧（SF）`、`首帧（FF）`、`连续帧（CF）`、`流控帧（FC）`。当发送端需要发送数据时，网络传输层会做如下处理，如果数据能用一帧 CAN 报文传输完则封装为单帧传输，否则就需要涉及到首帧、连续帧和流控帧。

**单帧**：用于发送小于等于 7 字节的数据，如下图所示，Byte0 高 4bit 为 0 的是单帧，Byte0 低 4bit 为报文有效字节数。Byte1-Byte7 为数据（包括无效的填充字节，填充数据可为任意值，通常为 `AA` 或 `55`）。比如应用层需要发送数据 `[10 03]`，经过网络传输层封装，最终发送至总线的实际数据为 `[02 10 03 AA AA AA AA AA]`

**多帧**：当需要发送的数据字节数大于 7 字节，需要用到首帧、连续帧和流控帧的多帧传输方式

<div align="center">

![UDS_PCI_Frame][UDS_PCI_Frame]

</div>

接收者通过 `Flow Control Frame（流控帧）`的机制，告知发送者自己有多大的接收能力。（其实就是每两个 FC 之间允许连续发送多少个 CF，每两个 CF 之间的时间不能过快）

其中：

- `DL`：有效数据的长度，不包括 PCIbyte，具体定义见 `network_layer_private.h`
  - UDS_SF_DL_MAX = 7 （单帧 DL 取值最大 7 字节）
  - UDS_FF_DL_MIN = 8 （首帧 DL 取值最小 8 字节）
  - UDS_FF_DL_MAX = 4095 （首帧 DL 取值最大 4095 （0xFFF）字节）
- `SN`：连续帧的序列号 0 ~ 15，FF 帧没有 SN 域，但是它“算” CF 帧的序列号0，所以第一帧 CF 帧的 SN = 1
- `FS`：表示发送方是否可以继续传输消息，`0`表示可以继续发送 Continue To Send（`CTS`），`1`表示等待发送 Wait（`WT`），`2`表示数据溢出 Overflow（`OVFLW`），一次发送的数据大于接收方所能接收的量
- `BS`：接收方一次能接收的数据最大量，BS = 0，再无 FC 帧，CF 帧可无限制发送
- `STmin`：表示两个连续帧之间的最小时间间隔，参照图1中的定义

<br/>

---

<br/>

### :earth_asia: UDS 网络层时间控制分析

网络层定时参数定义了`N_As`、`N_Ar`、`N_Bs`、`N_Br`、`N_Cs`、`N_Cr`六个参数。

- N_As 超时：发送方没有及时发送 N_PDU。
- N_Ar 超时：接收方没有及时发送 N_PDU。
- N_Bs 超时：发送方没有接收到流控帧。
- N_Br 超时：接收方没有发出流控帧。
- N_Cr 超时：接收方没有收到连续帧。
- N_Cs：即STmin，发送两个连续帧需要等待的最短时间。

`s = 发送者，r = 接收者`

| 定时参数 | 方向          | 解释                                                                                              |
|----------|---------------|---------------------------------------------------------------------------------------------------|
| N_As     | 发送方→接收方 | `首帧和连续帧`在数据链路层传播的时间                                                              |
| N_Ar     | 接收方→发送方 | `流控帧`在数据链路层传播的时间                                                                    |
| N_Bs     | 发送方→接收方 | 接收方收到`首帧`时发出的 ACK 响应，与自己（发送方）收到`流控帧`的间隔时间                         |
| N_Br     | 接收方→发送方 | 自己（接收方）收到`首帧`，与自己开始发出`流控帧`的间隔时间                                        |
| N_Cs     | 发送方→接收方 | 自己（发送方）收到`流控帧`，或是`连续帧送达时产生的 ACK 响应`，与自己开始发出`新连续帧`的间隔时间 |
| N_Cr     | 接收方→发送方 | 自己（接收方）收到`连续帧`，到下一次自己收到`连续帧`的间隔时间                                    |

<br/>

---

<br/>

### :id: UDS 汽车诊断协议（ISO 14229）所有服务以及对应服务支持的否定响应代码 NRC

下表将 **ISO 14229-Part 1:Specification and requirements** 中信息进行整理，梳理了 UDS 汽车诊断协议所有服务以及对应服务支持的否定响应代码 NRC：

<div class="table-box">
    <table cellspacing="0">
        <tbody>
            <tr>
                <td style="text-align:center;vertical-align:middle;width:40pt;"><strong><span>序号</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><strong><span>服务类型</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:53pt;"><strong><span>SID</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:234pt;"><strong><span>服务名</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:387pt;"><strong><span>描述</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:103pt;"><strong><span>默认会话</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:131pt;"><strong><span>非默认会话</span></strong></td>
                <td colspan="16" style="text-align:center;vertical-align:middle;"><strong><span>支持的负响应码 (NRC_)</span></strong></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>1</span></td>
                <td rowspan="10" style="text-align:center;vertical-align:middle;width:153pt;"><span>诊断和通信管理功能单元 Diagnostic and Communication Management functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x10</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>诊断会话控制 DiagnosticSessionControl service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        control a diagnostic session with a server(s).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>2</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x11</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>电控单元复位 ECUReset service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client forces the
                        server(s) to perform a reset.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>3</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x27</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>安全访问 SecurityAccess
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        unlock a secured server(s).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x35</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x36</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x37</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>4</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x28</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>通信控制 CommunicationControl
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client controls the
                        setting of communication parameters in the server (e.g.,communication baudrate).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>5</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x3E</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>诊断设备在线，会话保持（ZLG ZCANPRO）TesterPresent
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client indicates to the
                        server(s) that it is still present.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>6</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x83</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>访问时序参数 AccessTimingParameter
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client uses this service
                        to read/modify the timing parameters for an active communication.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>7</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x84</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >安全数据传输 SecuredDataTransmission service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client uses this service
                        to perform data transmission with an extended data link security.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x38-0x4F</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>8</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x85</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>诊断故障码设置控制 ControlDTCSetting
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client controls the
                        setting of DTCs in the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>9</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x86</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>事件响应 ResponseOnEvent
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to setup
                        and/or control an event mechanism in the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>a</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>10</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x87</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>链路控制 LinkControl
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests control
                        of the communication baudrate.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>11</span></td>
                <td rowspan="7" style="text-align:center;vertical-align:middle;width:153pt;"><span
                       >数据传输功能单元 Data Transmission functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x22</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>按标识符ID读取数据 ReadDataByIdentifier
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the current value of a record identified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x14</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>12</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x23</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>按地址读取内容 ReadMemoryByAddress
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the current value of the provided memory range.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>c</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>13</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x24</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >按标识符ID读取换算数据 ReadScalingDataByIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the scaling information of a record identified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>14</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2A</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >按周期性标识符ID读取数据 ReadDataByPeriodicIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        schedule data in the server for periodic transmission.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>15</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2C</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >动态定义数据标识符ID DynamicallyDefineDataIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        dynamically define data Identifiers that may subsequently be read by the readDataByIdentifier
                        service.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>d</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>16</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2E</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>按标识符ID写入数据 WriteDataByIdentifier
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to write
                        a record specified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>17</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x3D</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>按地址写内存 WriteMemoryByAddress service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        overwrite a provided memory range.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>c</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>18</span></td>
                <td rowspan="2" style="text-align:center;vertical-align:middle;width:153pt;"><span
                       >存储数据传输功能单元 Stored Data Transmission functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x14</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>清除诊断信息 ClearDiagnosticInformation Service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>Allows the client to clear
                        diagnostic information from the server (including DTCs,captured data, etc.)</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>19</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x19</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>读取DTC信息 ReadDTCInformation Service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>Allows the client to request
                        diagnostic information from the server (including DTCs, captured data, etc.)</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>20</span></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><span>输入输出控制功能单元 InputOutput Control functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2F</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>按标识符ID的输入输出控制 InputOutputControlByIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        control of an input/output specific to the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>21</span></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><span>例行程序功能单元 Routine functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x31</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>例程控制 RoutineControl service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        start, stop a routine in the server(s) or requests the routine results.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>e</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>22</span></td>
                <td rowspan="5" style="text-align:center;vertical-align:middle;width:153pt;"><span>上传下载功能单元 Upload Download functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x34</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>请求下载 RequestDownload service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a data transfer from the client to the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>23</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x35</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>请求上传 RequestUpload service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a data transfer from the server to the client.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>24</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x36</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>数据传输 TransferData service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client transmits data to
                        the server (download) or requests data from the server(upload).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x71</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x73</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x92/0x93</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>25</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x37</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>请求退出传输 RequestTransferExit service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        termination of a data transfer.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>26</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x38</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>请求文件传输 RequestFileTransfer
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a file transfer between server and client.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>不适用 not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>　</span></td>
            </tr>
            <tr>
                <td colspan="23" style="text-align:left;vertical-align:middle;">
                <span>
                        a&nbsp;&nbsp;在默认会话期间是否也允许 ResponseOnEvent 服务是特定于实现的。 It is implementation specific whether the ResponseOnEvent service is also allowed during the defaultSession.<br>
                        b&nbsp;&nbsp;安全的数据标识符需要 SecurityAccess 服务，因此需要非默认诊断会话。 Secured dataIdentifiers require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        c&nbsp;&nbsp;安全内存区域需要 SecurityAccess 服务，因此需要非默认诊断会话。 Secured memory areas require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        d&nbsp;&nbsp;可以在默认和非默认诊断会话中动态定义 dataIdentifier。 A dataIdentifier can be defined dynamically in the default and non-default diagnostic session.<br>
                        e&nbsp;&nbsp;安全例程需要 SecurityAccess 服务，因此需要非默认诊断会话。 Secured routines require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        &nbsp;&nbsp;&nbsp;需要客户端主动停止的例程也需要非默认会话。 A routine that requires to be stopped actively by the client also requires a non-default session.
                </span>
                </td>
            </tr>
            <tr>
                <td colspan="23" style="text-align:left;vertical-align:middle;">
                <span style="color:red;">红色标注</span><span>的服务为最常用、也是最基本的UDS服务，主要了解这些服务为主。（Gitlab上可能无法看到红色标注）</span>
                </td>
            </tr>
        </tbody>
    </table>
</div>

<br/>

---

<br/>

### :construction: UDS 否定响应代码 NRC 定义

| 序号 | 负响应码 (NRC CODE) | 名称                                        | 释义                                               |
|------|---------------------|---------------------------------------------|----------------------------------------------------|
| 01   | 0x10                | NRC_GENERAL_REJECT                          | 通用拒绝指令                                       |
| 02   | 0x11                | NRC_SERVICE_NOT_SUPPORTED                   | 服务不支持                                         |
| 03   | 0x12                | NRC_SUBFUNCTION_NOT_SUPPORTED               | 子功能不支持                                       |
| 04   | 0x13                | NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT        | 错误的消息长度或格式                               |
| 05   | 0x22                | NRC_CONDITIONS_NOT_CORRECT                  | 不正确的输入条件                                   |
| 06   | 0x24                | NRC_REQUEST_SEQUENCE_ERROR                  | 请求顺序错误                                       |
| 07   | 0x31                | NRC_REQUEST_OUT_OF_RANGE                    | 请求超出范围                                       |
| 08   | 0x33                | NRC_SECURITY_ACCESS_DENIED                  | 安全访问拒绝                                       |
| 09   | 0x35                | NRC_INVALID_KEY                             | 错误的访问密匙                                     |
| 10   | 0x36                | NRC_EXCEEDED_NUMBER_OF_ATTEMPTS             | 超出了错误的安全访问次数                           |
| 11   | 0x37                | NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED         | 超出错误的安全访问次数后延时冷却时间未结束         |
| 12   | 0x72                | NRC_GENERAL_PROGRAMMING_FAILURE             | 通用编程错误，一般为擦除或者写入错误               |
| 13   | 0x78                | NRC_SERVICE_BUSY                            | 服务忙（严格来讲不算是错误，只是挂起新的服务请求） |
| 14   | 0x7F                | NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION | 服务不支持当前会话                                 |

<br/>

---

<br/>

### :construction: UDS SID Sub-Function 子服务字节组成

Resulting sub-function parameter byte value (bit 7 ~ 0)

| Bit 7                                               | Bit 6 ~ 0                                                                                          |
|-----------------------------------------------------|----------------------------------------------------------------------------------------------------|
| 正响应消息抑制指示位 SuppressPosRspMsgIndicationBit | Sub-Function parameter value as specified in the Sub-Function parameter value table of the service |

- SuppressPosRspMsgIndicationBit = 1 抑制正响应
- SuppressPosRspMsgIndicationBit = 0 正响应不被抑制

正响应抑制位是在 Sub-Function 里的这个字节的最高位，我们把它叫做正响应抑制位。只有这个服务支持 Sub-Function 的时候，才有可能支持正响应抑制位。这里要注意的是它只是抑制正响应，而负响应是不被抑制的。

[Pic_ZCANPRO_ECU_Refresh]: ./Pic_ZCANPRO_ECU_Refresh.png
[Pic_ZCANPRO_ECU_Refresh_Note_ProgramSession]: ./Pic_ZCANPRO_ECU_Refresh_Note_ProgramSession.png
[Pic_ZCANPRO_ECU_Refresh_Note_ResetECU]: ./Pic_ZCANPRO_ECU_Refresh_Note_ResetECU.png
[LinkSetting_ForUsageFault_00]: ./LinkSetting_ForUsageFault_00.png
[LinkSetting_ForUsageFault_01]: ./LinkSetting_ForUsageFault_01.png
[LinkSetting_ForUsageFault_02]: ./LinkSetting_ForUsageFault_02.png
[UDS_OSI_Model]: ./Pic_UDS_OSI.png
[UDS_PCI_Frame]: ./Pic_UDS_PCI_Frame.png

[UDS_SecurityAccess]: https://github.com/SummerFalls/UDS_SecurityAccess
[UDS_S32K144_Bootloader]: https://github.com/SummerFalls/UDS_S32K144_Bootloader
[UDS_S32K144_FlashDriver]: https://github.com/SummerFalls/UDS_S32K144_FlashDriver
[UDS_S32K144_APP]: https://github.com/SummerFalls/UDS_S32K144_APP
