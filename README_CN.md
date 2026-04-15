# RW612 Flash 解锁工具

[English](README.md) | 中文

## 问题

将 **RD-RW612-BGA** 或 **FCM363X-L/FCM365X** 的固件（使用 Macronix Flash）烧录到使用 **Winbond/XMC Flash** 的模块（FCMA62N/FCM363X/FGMH63X）上，会因状态寄存器配置不兼容导致 flash 锁定问题。

总之，不同型号的固件请不要随便烧录，这就是烧错的后果之一。

## 解决方案

本工具通过 RAM 运行并清除保护位来解锁 flash。

## 使用方法

### 方法一：使用预编译文件

1. 从 [Releases](../../releases) 下载 `flexspi_nor_dma_transfer.bin`
2. 使用 J-Link 加载到 RAM：

```bash
JLink
> connect
> device FCM363X
> si JTAG              # FCMA62N 用 SWD，FCM363X/FGMH63X 用 JTAG
> speed 4000
> loadbin flexspi_nor_dma_transfer.bin 0x20000000
> setpc 0x200012fc     # 如果这个地址不行，检查 output.map 文件
> go
```

3. 通过串口（115200 波特率）监控解锁状态

### 方法二：自己编译

**推荐：使用 Debug（构建+运行）**

1. 在 **MCUXpresso for VS Code** 中打开项目
2. 选择构建配置：**ram_debug** 或 **ram_release**
3. 点击 **Debug** 按钮（自动构建并运行）

**备选：仅构建**

1. 在 **MCUXpresso for VS Code** 中打开项目
2. 选择构建配置：**ram_debug** 或 **ram_release**
3. 点击 **Build** 按钮
4. 输出：`armgcc/ram_debug/flexspi_nor_dma_transfer.bin`

## 说明

- 从 RAM 运行以安全修改 flash
- 支持 XMC flash（厂商 ID 0x20）
- **设备**：根据实际板子选择
  - FCMA62N → device FCMA62N, si SWD
  - FCM363X → device FCM363X, si JTAG
  - FGMH63X → device FGMH63X, si JTAG
- 如果 0x200012fc 不太行，检查 `armgcc/ram_debug/output.map` 中的 Reset_Handler 地址

