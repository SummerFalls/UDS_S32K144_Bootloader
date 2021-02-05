# :beers: UDS_S32K144_Bootloader

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

## :book: ���

S32K1xx �� CAN �� `������ USBCANFD-100U-mini`��ʹ�� `ZCANPRO` ����� `ECUˢ��` ���ܽ��в��ԡ�ͨ����Ӧ�� UDS ���� :package: [UDS_S32K144_FlashDriver](https://github.com/SummerFalls/UDS_S32K144_FlashDriver) �� hex �ļ��������� `Bootloader` �������ļ���Ϊ��Ԥ��ָ����ʼ��ַ�� RAM �ռ��У���ͨ�� `Flash Driver` ��ʵ�ʰ�������Ӧ�� Flash �������������ƫ�����Լ���������������������Ӧ������������ڵ��� RAM �ڵ�ƫ�Ƶ�ַ����ͨ������ָ��ķ�ʽ���е�����Ӧ�� `Flash API` ��ʵ�ֶ�Ƭ�� Flash �ı�̡���д��У�飬����ʵ�� `ECUˢ��` ������ APP �������̡�

![Pic_ZCANPRO_ECU_Refresh][Pic_ZCANPRO_ECU_Refresh]

## :gear: Ӳ�� & ��� ����

### Ӳ������

- S32K144-EVB
- J-Link
- USBCANFD-100U-mini
- 12V External Power Supply

### �������

- S32 Design Studio for ARM Version 2.2
- ZCANPRO
- J-Flash

<br/>

---

<br/>

## :detective: UDS��Unified Diagnostic Services��ͳһ��Ϸ���

<div align="center">

## :page_with_curl: UDS Э��˵��

</div>

UDS��Unified Diagnostic Services��ͳһ��Ϸ������Э���� `ISO 15765` �� `ISO 14229` �����һ������ͨ�����Э�飬λ�� OSI ģ���е�Ӧ�ò㣬�����ڲ�ͬ���������ߣ�����CAN��LIN��Flexray��Internet��K-line����ʵ�֡�UDSЭ���Ӧ�ò㶨���� `ISO 14229-1`��Ŀǰ�󲿷��������̾����� UDS on CAN �����Э�顣

<div align="center">

![UDS_OSI_Model][UDS_OSI_Model]

<br/>
</div>

### :triangular_flag_on_post: UDS �����Ҫ

| ��д              | Ӣ��ȫ��                                                          | ���ĺ���                                                                                                |
|-------------------|-------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| UDS               | Unified Diagnostic Services                                       | ͳһ��Ϸ���                                                                                            |
| OBD               | On-Board Diagnostics                                              | �������ϵͳ                                                                                            |
| WWH-OBD           | World-wide Harmonized OBD                                         | ȫ��ͳһ�����ͷ������ĳ������ϵͳ                                                                      |
| DoCAN             | Diagnostic Communication Over CAN                                 | ����CAN�����ͨ��                                                                                       |
| OSI               | Open System Interconnection                                       | ����ʽϵͳ����                                                                                          |
| ECU               | Electronic Control Unit                                           | ���ӿ��Ƶ�Ԫ                                                                                            |
| SID               | Service ID                                                        | ��Ϸ����ʶ��                                                                                          |
| Sub-Function      | Sub-Function                                                      | �ӷ�����Ϸ���������ӹ���                                                                            |
| DTC               | Diagnostic Trouble Code                                           | ��Ϲ�����                                                                                              |
| NRC               | Negative Response Code                                            | ����Ӧ��                                                                                                |
| FTB               | Fault Type Byte                                                   | ���������ֽ�                                                                                            |
| PCI               | Protocol Control Information                                      | Э�������Ϣ                                                                                            |
| SF                | Single Frame                                                      | ��֡                                                                                                    |
| FF                | First Frame                                                       | ��֡                                                                                                    |
| CF                | Consecutive Frame                                                 | ����֡                                                                                                  |
| FC                | Flow Control Frame                                                | ����֡                                                                                                  |
| DL                | Data Length                                                       | ���ݳ���                                                                                                |
| FF_DL             | First Frame Data Length                                           | ��֡���ݳ���                                                                                            |
| SN                | Sequence Number                                                   | ���                                                                                                    |
| SPN               | Suspect Parameter Number                                          | ���ɲ������                                                                                            |
| FS                | Flow Status                                                       | ������״̬                                                                                              |
| BS                | Block Size                                                        | ��Ĵ�С                                                                                                |
| STmin             | Separation Time Minimum                                           | ��С���ʱ��                                                                                            |
| CTS               | Continue to Send                                                  | ��������                                                                                                |
| FMI               | Failure Mode Indicator                                            | ʧЧģʽָʾ                                                                                            |
| Mtype             | Message Type                                                      | ��Ϣ����                                                                                                |
| Indication        | Indication Service Primitive                                      | ָʾ����ԭ��                                                                                            |
| Confirm           | Confirm Service Primitive                                         | ȷ�Ϸ���ԭ��                                                                                            |
| N_AE              | Network Address Extension                                         | �������չ��ַ                                                                                          |
| N_AI              | Network Address Information                                       | ������ַ��Ϣ                                                                                          |
| N_Ar              | Network Layer Timing Parameter Ar                                 | ����㶨ʱ���� Ar                                                                                       |
| N_As              | Network Layer Timing Parameter As                                 | ����㶨ʱ���� As                                                                                       |
| N_Br              | Network Layer Timing Parameter Br                                 | ����㶨ʱ���� Br                                                                                       |
| N_Bs              | Network Layer Timing Parameter Bs                                 | ����㶨ʱ���� Bs                                                                                       |
| N_Cr              | Network Layer Timing Parameter Cr                                 | ����㶨ʱ���� Cr                                                                                       |
| N_Cs              | Network Layer Timing Parameter Cs                                 | ����㶨ʱ���� Cs                                                                                       |
| N_ChangeParameter | Network Layer Service Name                                        | �����ķ�������                                                                                        |
| N_Data            | Network Data                                                      | ���������                                                                                              |
| N_PCI             | Network Protocol Control Information                              | �����Э�������Ϣ                                                                                      |
| N_PCItype         | Network Protocol Control Information Type                         | �����Э�������Ϣ����                                                                                  |
| N_PDU             | Network Protocol Data Unit                                        | �����Э�����ݵ�Ԫ                                                                                      |
| N_SDU             | Network Service Data Unit                                         | �����������ݵ�Ԫ                                                                                      |
| N_SA              | Network Source Address                                            | �����Դ��ַ Դ��ַ -> �����                                                                           |
| N_TA              | Network Target Address                                            | �����Ŀ���ַ Ŀ���ַ -> ECU                                                                          |
| N_TAtype          | Network Target Address Type                                       | �����Ŀ���ַ����                                                                                      |
| N_USData          | Network Layer Unacknowledged Segmented Data Transfer Service Name | �����δȷ�Ϸֶ����ݴ����������                                                                        |
| NW                | Network                                                           | �����                                                                                                  |
| NWL               | Network Layer                                                     | �����                                                                                                  |
| UDS_PHYSREQ_CANID | Physical Address                                                  | ����Ѱַ - ����ͨѶ��ͬһ������ÿ�� ECU ������Ѱַ CAN ID ������ͬ��ֻ��һ�� ECU ����մ���             |
| UDS_FUNCREQ_CANID | Functional Address                                                | ����Ѱַ - �ಥͨѶ��ͬһ���������� ECU �Ĺ���Ѱַ CAN ID ����ͬ�����о߱���Ϲ��ܵ� ECU ������ղ����� |
| UDS_RESPOND_CANID | Response Address                                                  | ��Ӧ��ַ - �� ECU ���ͣ�����ǽ��գ��� ECU ������ͬ                                                     |

<br/>

---

<br/>

### :earth_asia: UDS ��������

���������Ҫ��Ŀ�ľ��� **������ת��������Ӧ CAN ���߹淶�ĵ�һ����֡���Ӷ����д��䡣** �����Ҫ����ı��ĳ��ȳ����� CAN ����֡�ĳ��ȣ�����Ҫ��������Ϣ���в�ֺ��䣬ÿ��������Դ���4095���ֽڳ��ȵı��ġ�

���е��������񶼾�����ͬ��ͨ�ýṹ��Ϊ�˶������ַ�����Ҫ�����������͵ķ���ԭ�

- �������**Request**��������**������㴫�ݿ��Ʊ�����Ϣ��Ҫ���͵�����**��Ӧ���ڸ��߲��Ӧ�ò㡣��������� ECU �������ݡ�
- ָʾ����**Indication**��������**����߲��Ӧ�ò㴫��״̬��Ϣ�����յ�������**��Ӧ��������㡣�� ECU �յ�������ǵ����ݣ�����Ӧ�ò㡣
- ȷ�Ϸ���**Confirm**�����������ʹ�ã���������߲��Ӧ�ò㴫��״̬��Ϣ����������յ��� ECU ��������ݡ�

����˵����Indication ǰ��Ӧ��ִ�е��ǵײ㴫�����ݵĴ�����������ȡ PDU ��Ϣ����һ֡������ SF��FC��CF ���� FF���������������������ϣ���Ӧ�ò㴫�ݡ�

����˵����**Confirm �� Indication ������Ƕ��Ǵ��������Ӧ�ò㴫����Ϣ���к������أ�**

�ڴ���Ĵ����У�Confirm ��ǰ��Ӧ��ײ��ⷢ Tx ���ݻ�ʱ����������������Ϣ����Ҫ�������ݡ��� Indication ���ݵ���Ϣ���Ϊ���֣�һ�ְ�����ʵ���ݣ���һ�ֲ�������

<br/>

---

<br/>

### :earth_asia: UDS �����Э�����ݵ�Ԫ N_PDU

�����Э�����ݵ�Ԫ��N_PDU��Network Protocol Data Unit������ `N_AI��N_PCI��N_Data`������ַ��Ϣ��Э�������Ϣ�����ݡ�

| ��������     | ��д   | ����                               |
|--------------|--------|------------------------------------|
| Ѱַ��Ϣ     | N_AI   | ����Դ��ַ��Ŀ�ĵ�ַ��Ѱַ��ʽ��Ϣ |
| Э�������Ϣ | N_PCI  | ��ʶ�����֡����                   |
| ����         | N_Data | ����Ӧ�ò�Э�������Ϣ������       |

**�����Э�����ݵ�Ԫ��N_PDU������������**������֡��SF������֡��FF��������֡��CF����������֡��FC�������ڽ����Ե�ʵ����ͨ�š�

<br/>

---

<br/>

### :earth_asia: UDS �����Э�������Ϣ N_PCI

> :warning: ע��
>> �����紫���ı��ĸ�ʽΪ��`ISO 15765-2 2004`
>>> ���� `ISO 15765-2 2016` �У���֡ SF �õ� 2 �ֽڴ�ų��� DL����֡ FF �õ� 3 ~ 6 �ֽڴ�ų��� DL���ʱ�Э��ջ�в�֧�� `ISO 15765-2 2016`

`ISO 15765-2` ������һ�� CAN ֡�ָ�����紫���ͨ��Э�飬��Э��� CAN ֡�� `Byte0 - Byte2` �����˶��壬�ɴ˲�����`��֡��SF��`��`��֡��FF��`��`����֡��CF��`��`����֡��FC��`�������Ͷ���Ҫ��������ʱ�����紫���������´��������������һ֡ CAN ���Ĵ��������װΪ��֡���䣬�������Ҫ�漰����֡������֡������֡��

**��֡**�����ڷ���С�ڵ��� 7 �ֽڵ����ݣ�����ͼ��ʾ��Byte0 �� 4bit Ϊ 0 ���ǵ�֡��Byte0 �� 4bit Ϊ������Ч�ֽ�����Byte1-Byte7 Ϊ���ݣ�������Ч������ֽڣ�������ݿ�Ϊ����ֵ��ͨ��Ϊ `AA` �� `55`��������Ӧ�ò���Ҫ�������� `[10 03]`���������紫����װ�����շ��������ߵ�ʵ������Ϊ `[02 10 03 AA AA AA AA AA]`

**��֡**������Ҫ���͵������ֽ������� 7 �ֽڣ���Ҫ�õ���֡������֡������֡�Ķ�֡���䷽ʽ

<div align="center">

![UDS_PCI_Frame][UDS_PCI_Frame]

</div>

������ͨ�� `Flow Control Frame������֡��`�Ļ��ƣ���֪�������Լ��ж��Ľ�������������ʵ����ÿ���� FC ֮�������������Ͷ��ٸ� CF��ÿ���� CF ֮���ʱ�䲻�ܹ��죩

���У�

- `DL`����Ч���ݵĳ��ȣ������� PCIbyte�����嶨��� `network_layer_private.h`
  - UDS_SF_DL_MAX = 7 ����֡ DL ȡֵ��� 7 �ֽڣ�
  - UDS_FF_DL_MIN = 8 ����֡ DL ȡֵ��С 8 �ֽڣ�
  - UDS_FF_DL_MAX = 4095 ����֡ DL ȡֵ��� 4095 ��0xFFF���ֽڣ�
- `SN`������֡�����к� 0 ~ 15��FF ֡û�� SN �򣬵��������㡱 CF ֡�����к�0�����Ե�һ֡ CF ֡�� SN = 1
- `FS`����ʾ���ͷ��Ƿ���Լ���������Ϣ��`0`��ʾ���Լ������� Continue To Send��`CTS`����`1`��ʾ�ȴ����� Wait��`WT`����`2`��ʾ������� Overflow��`OVFLW`����һ�η��͵����ݴ��ڽ��շ����ܽ��յ���
- `BS`�����շ�һ���ܽ��յ������������BS = 0������ FC ֡��CF ֡�������Ʒ���
- `STmin`����ʾ��������֮֡�����Сʱ����������ͼ1�еĶ���

<br/>

---

<br/>

### :earth_asia: UDS �����ʱ����Ʒ���

����㶨ʱ����������`N_As`��`N_Ar`��`N_Bs`��`N_Br`��`N_Cs`��`N_Cr`����������

- N_As ��ʱ�����ͷ�û�м�ʱ���� N_PDU��
- N_Ar ��ʱ�����շ�û�м�ʱ���� N_PDU��
- N_Bs ��ʱ�����ͷ�û�н��յ�����֡��
- N_Br ��ʱ�����շ�û�з�������֡��
- N_Cr ��ʱ�����շ�û���յ�����֡��
- N_Cs����STmin��������������֡��Ҫ�ȴ������ʱ�䡣

`s = �����ߣ�r = ������`

| ��ʱ���� | ����          | ����                                                                                              |
|----------|---------------|---------------------------------------------------------------------------------------------------|
| N_As     | ���ͷ������շ� | `��֡������֡`��������·�㴫����ʱ��                                                              |
| N_Ar     | ���շ������ͷ� | `����֡`��������·�㴫����ʱ��                                                                    |
| N_Bs     | ���ͷ������շ� | ���շ��յ�`��֡`ʱ������ ACK ��Ӧ�����Լ������ͷ����յ�`����֡`�ļ��ʱ��                         |
| N_Br     | ���շ������ͷ� | �Լ������շ����յ�`��֡`�����Լ���ʼ����`����֡`�ļ��ʱ��                                        |
| N_Cs     | ���ͷ������շ� | �Լ������ͷ����յ�`����֡`������`����֡�ʹ�ʱ������ ACK ��Ӧ`�����Լ���ʼ����`������֡`�ļ��ʱ�� |
| N_Cr     | ���շ������ͷ� | �Լ������շ����յ�`����֡`������һ���Լ��յ�`����֡`�ļ��ʱ��                                    |

<br/>

---

<br/>

### :id: UDS �������Э�飨ISO 14229�����з����Լ���Ӧ����֧�ֵķ���Ӧ���� NRC

�±� **ISO 14229-Part 1:Specification and requirements** ����Ϣ�������������� UDS �������Э�����з����Լ���Ӧ����֧�ֵķ���Ӧ���� NRC��

<div class="table-box">
    <table cellspacing="0">
        <tbody>
            <tr>
                <td style="text-align:center;vertical-align:middle;width:40pt;"><strong><span>���</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><strong><span>��������</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:53pt;"><strong><span>SID</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:234pt;"><strong><span>������</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:387pt;"><strong><span>����</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:103pt;"><strong><span>Ĭ�ϻỰ</span></strong></td>
                <td style="text-align:center;vertical-align:middle;width:131pt;"><strong><span>��Ĭ�ϻỰ</span></strong></td>
                <td colspan="16" style="text-align:center;vertical-align:middle;"><strong><span>֧�ֵĸ���Ӧ�� (NRC_)</span></strong></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>1</span></td>
                <td rowspan="10" style="text-align:center;vertical-align:middle;width:153pt;"><span>��Ϻ�ͨ�Ź����ܵ�Ԫ Diagnostic and Communication Management functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x10</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��ϻỰ���� DiagnosticSessionControl service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        control a diagnostic session with a server(s).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>2</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x11</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��ص�Ԫ��λ ECUReset service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client forces the
                        server(s) to perform a reset.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>3</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x27</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��ȫ���� SecurityAccess
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        unlock a secured server(s).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x35</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x36</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x37</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>4</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x28</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>ͨ�ſ��� CommunicationControl
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client controls the
                        setting of communication parameters in the server (e.g.,communication baudrate).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>5</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x3E</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>����豸���ߣ��Ự���֣�ZLG ZCANPRO��TesterPresent
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client indicates to the
                        server(s) that it is still present.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>6</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x83</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>����ʱ����� AccessTimingParameter
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client uses this service
                        to read/modify the timing parameters for an active communication.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>7</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x84</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >��ȫ���ݴ��� SecuredDataTransmission service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client uses this service
                        to perform data transmission with an extended data link security.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x38-0x4F</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>8</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x85</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��Ϲ��������ÿ��� ControlDTCSetting
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client controls the
                        setting of DTCs in the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>9</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x86</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>�¼���Ӧ ResponseOnEvent
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to setup
                        and/or control an event mechanism in the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>a</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>10</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x87</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>��·���� LinkControl
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests control
                        of the communication baudrate.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>11</span></td>
                <td rowspan="7" style="text-align:center;vertical-align:middle;width:153pt;"><span
                       >���ݴ��书�ܵ�Ԫ Data Transmission functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x22</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>����ʶ��ID��ȡ���� ReadDataByIdentifier
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the current value of a record identified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x14</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>12</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x23</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>����ַ��ȡ���� ReadMemoryByAddress
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the current value of the provided memory range.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>c</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>13</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x24</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >����ʶ��ID��ȡ�������� ReadScalingDataByIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to read
                        the scaling information of a record identified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>14</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2A</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >�������Ա�ʶ��ID��ȡ���� ReadDataByPeriodicIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        schedule data in the server for periodic transmission.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>15</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2C</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span
                       >��̬�������ݱ�ʶ��ID DynamicallyDefineDataIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        dynamically define data Identifiers that may subsequently be read by the readDataByIdentifier
                        service.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>d</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>16</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2E</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>����ʶ��IDд������ WriteDataByIdentifier
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to write
                        a record specified by a provided dataIdentifier.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>b</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>17</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x3D</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>����ַд�ڴ� WriteMemoryByAddress service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        overwrite a provided memory range.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>c</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>18</span></td>
                <td rowspan="2" style="text-align:center;vertical-align:middle;width:153pt;"><span
                       >�洢���ݴ��书�ܵ�Ԫ Stored Data Transmission functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x14</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��������Ϣ ClearDiagnosticInformation Service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>Allows the client to clear
                        diagnostic information from the server (including DTCs,captured data, etc.)</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>19</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x19</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>��ȡDTC��Ϣ ReadDTCInformation Service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>Allows the client to request
                        diagnostic information from the server (including DTCs, captured data, etc.)</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>20</span></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><span>����������ƹ��ܵ�Ԫ InputOutput Control functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x2F</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>����ʶ��ID������������� InputOutputControlByIdentifier service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        control of an input/output specific to the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>21</span></td>
                <td style="text-align:center;vertical-align:middle;width:153pt;"><span>���г����ܵ�Ԫ Routine functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x31</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>���̿��� RoutineControl service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests to
                        start, stop a routine in the server(s) or requests the routine results.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X<span>e</span></span></td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x12</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>22</span></td>
                <td rowspan="5" style="text-align:center;vertical-align:middle;width:153pt;"><span>�ϴ����ع��ܵ�Ԫ Upload Download functional unit</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x34</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>�������� RequestDownload service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a data transfer from the client to the server.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>23</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x35</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>�����ϴ� RequestUpload service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a data transfer from the server to the client.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x33</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>24</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x36</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>���ݴ��� TransferData service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client transmits data to
                        the server (download) or requests data from the server(upload).</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x71</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x73</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x92/0x93</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>25</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x37</span></strong></td>
                <td style="text-align:center;vertical-align:middle;color:red;"><span>�����˳����� RequestTransferExit service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        termination of a data transfer.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x24</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x72</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td style="text-align:center;vertical-align:middle;"><span>26</span></td>
                <td style="text-align:center;vertical-align:middle;"><strong><span>0x38</span></strong></td>
                <td style="text-align:center;vertical-align:middle;"><span>�����ļ����� RequestFileTransfer
                        service</span></td>
                <td style="vertical-align:middle;width:387pt;"><span>The client requests the
                        negotiation of a file transfer between server and client.</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>������ not applicable</span>
                </td>
                <td style="text-align:center;vertical-align:middle;"><span>X</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x13</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x22</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x31</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>0x70</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
                <td style="text-align:center;vertical-align:middle;"><span>��</span></td>
            </tr>
            <tr>
                <td colspan="23" style="text-align:left;vertical-align:middle;">
                <span>
                        a&nbsp;&nbsp;��Ĭ�ϻỰ�ڼ��Ƿ�Ҳ���� ResponseOnEvent �������ض���ʵ�ֵġ� It is implementation specific whether the ResponseOnEvent service is also allowed during the defaultSession.<br>
                        b&nbsp;&nbsp;��ȫ�����ݱ�ʶ����Ҫ SecurityAccess ���������Ҫ��Ĭ����ϻỰ�� Secured dataIdentifiers require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        c&nbsp;&nbsp;��ȫ�ڴ�������Ҫ SecurityAccess ���������Ҫ��Ĭ����ϻỰ�� Secured memory areas require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        d&nbsp;&nbsp;������Ĭ�Ϻͷ�Ĭ����ϻỰ�ж�̬���� dataIdentifier�� A dataIdentifier can be defined dynamically in the default and non-default diagnostic session.<br>
                        e&nbsp;&nbsp;��ȫ������Ҫ SecurityAccess ���������Ҫ��Ĭ����ϻỰ�� Secured routines require a SecurityAccess service and therefore a non-default diagnostic session.<br>
                        &nbsp;&nbsp;&nbsp;��Ҫ�ͻ�������ֹͣ������Ҳ��Ҫ��Ĭ�ϻỰ�� A routine that requires to be stopped actively by the client also requires a non-default session.
                </span>
                </td>
            </tr>
            <tr>
                <td colspan="23" style="text-align:left;vertical-align:middle;">
                <span style="color:red;">��ɫ��ע</span><span>�ķ���Ϊ��á�Ҳ���������UDS������Ҫ�˽���Щ����Ϊ������Gitlab�Ͽ����޷�������ɫ��ע��</span>
                </td>
            </tr>
        </tbody>
    </table>
</div>

<br/>

---

<br/>

### :construction: UDS ����Ӧ���� NRC ����

| ��� | ����Ӧ�� (NRC CODE) | ����                                        | ����                                               |
|------|---------------------|---------------------------------------------|----------------------------------------------------|
| 01   | 0x10                | NRC_GENERAL_REJECT                          | ͨ�þܾ�ָ��                                       |
| 02   | 0x11                | NRC_SERVICE_NOT_SUPPORTED                   | ����֧��                                         |
| 03   | 0x12                | NRC_SUBFUNCTION_NOT_SUPPORTED               | �ӹ��ܲ�֧��                                       |
| 04   | 0x13                | NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT        | �������Ϣ���Ȼ��ʽ                               |
| 05   | 0x22                | NRC_CONDITIONS_NOT_CORRECT                  | ����ȷ����������                                   |
| 06   | 0x24                | NRC_REQUEST_SEQUENCE_ERROR                  | ����˳�����                                       |
| 07   | 0x31                | NRC_REQUEST_OUT_OF_RANGE                    | ���󳬳���Χ                                       |
| 08   | 0x33                | NRC_SECURITY_ACCESS_DENIED                  | ��ȫ���ʾܾ�                                       |
| 09   | 0x35                | NRC_INVALID_KEY                             | ����ķ����ܳ�                                     |
| 10   | 0x36                | NRC_EXCEEDED_NUMBER_OF_ATTEMPTS             | �����˴���İ�ȫ���ʴ���                           |
| 11   | 0x37                | NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED         | ��������İ�ȫ���ʴ�������ʱ��ȴʱ��δ����         |
| 12   | 0x72                | NRC_GENERAL_PROGRAMMING_FAILURE             | ͨ�ñ�̴���һ��Ϊ��������д�����               |
| 13   | 0x78                | NRC_SERVICE_BUSY                            | ����æ���ϸ����������Ǵ���ֻ�ǹ����µķ������� |
| 14   | 0x7F                | NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION | ����֧�ֵ�ǰ�Ự                                 |

<br/>

---

<br/>

### :construction: UDS SID Sub-Function �ӷ����ֽ����

Resulting sub-function parameter byte value (bit 7 ~ 0)

| Bit 7                                               | Bit 6 ~ 0                                                                                          |
|-----------------------------------------------------|----------------------------------------------------------------------------------------------------|
| ����Ӧ��Ϣ����ָʾλ SuppressPosRspMsgIndicationBit | Sub-Function parameter value as specified in the Sub-Function parameter value table of the service |

- SuppressPosRspMsgIndicationBit = 1 ��������Ӧ
- SuppressPosRspMsgIndicationBit = 0 ����Ӧ��������

����Ӧ����λ���� Sub-Function �������ֽڵ����λ�����ǰ�����������Ӧ����λ��ֻ���������֧�� Sub-Function ��ʱ�򣬲��п���֧������Ӧ����λ������Ҫע�������ֻ����������Ӧ��������Ӧ�ǲ������Ƶġ�

[Pic_ZCANPRO_ECU_Refresh]: ./Pic_ZCANPRO_ECU_Refresh.png
[UDS_OSI_Model]: ./Pic_UDS_OSI.png
[UDS_PCI_Frame]: ./Pic_UDS_PCI_Frame.png
