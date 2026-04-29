# 03 Device Tree 示例片段

以下片段用于展示适配思路，不可直接照搬量产。所有 pinctrl、GPIO、电源、时钟、中断必须结合 100ASK T113-Pro Base V1.3 / Core V1.2 原理图和 Tina SDK 驱动确认。

## UART / RS485

```dts
&uart1 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&uart1_pins_a>;
    pinctrl-1 = <&uart1_pins_b>;
    status = "okay";

    /* 若使用内核 RS485 自动方向控制，需确认驱动是否支持。 */
    linux,rs485-enabled-at-boot-time;
    rs485-rts-active-high;
    rs485-rts-delay = <0 1>;
};
```

关键点：

- UART1 在当前 DTS 中有 PG6/PG7/PG8/PG9 线索。
- 如果外接 485 芯片 DE/RE 接 GPIO，可由用户态或 serial rs485 ioctl 控制。
- 需确认该 UART 未被 4G、调试串口或其他外设占用。

## PWM Backlight

```dts
backlight: backlight {
    compatible = "pwm-backlight";
    pwms = <&pwm 7 50000 0>;
    brightness-levels = <0 16 32 64 128 192 255>;
    default-brightness-level = <5>;
    status = "okay";
};

&pwm7 {
    pinctrl-names = "active", "sleep";
    pinctrl-0 = <&pwm7_pin_a>;
    pinctrl-1 = <&pwm7_pin_b>;
    status = "okay";
};
```

关键点：

- 当前 `board.dts` 已存在 `pwm7_pin_a`。
- Base 原理图中可见 LCD PWM 相关线索，实际背光电源和使能脚需确认。

## RGB LCD Panel

```dts
&lcd0 {
    lcd_used = <1>;
    lcd_driver_name = "default_lcd";
    lcd_if = <0>;              /* 0 通常表示 RGB，需以 SDK 文档为准 */
    lcd_x = <1024>;
    lcd_y = <600>;
    lcd_dclk_freq = <51>;
    lcd_hbp = <160>;
    lcd_ht = <1344>;
    lcd_hspw = <20>;
    lcd_vbp = <23>;
    lcd_vt = <635>;
    lcd_vspw = <3>;
    lcd_pwm_used = <1>;
    lcd_pwm_ch = <7>;
    status = "okay";
};

&disp {
    status = "okay";
};
```

关键点：

- Core 图纸可见 RGB/LVDS 管脚，Base 图纸可见 TFT 接口选择线索。
- 分辨率、时序、电源、reset、背光必须按屏规格书确认。

## I2C Touch

```dts
&twi2 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&twi2_pins_a>;
    pinctrl-1 = <&twi2_pins_b>;
    status = "okay";

    gt911: touch@14 {
        compatible = "goodix,gt911";
        reg = <0x14>;
        interrupt-parent = <&pio>;
        interrupts = <PB 3 IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&pio PB 2 GPIO_ACTIVE_LOW>;
        status = "disabled";
    };
};
```

关键点：

- 原理图文本显示 CTP_RST/CTP_INT 与 CAN0_TX/CAN0_RX、USER KEY 有复用线索。
- 当前工程已有文档指出 PB2/PB3 与 USB detect/VBUS 也可能冲突，必须按板子实物跳线确认。

## USB Host

```dts
reg_usb1_vbus: usb1-vbus {
    compatible = "regulator-fixed";
    regulator-name = "usb1-vbus";
    regulator-min-microvolt = <5000000>;
    regulator-max-microvolt = <5000000>;
    gpio = <&pio PB 3 GPIO_ACTIVE_HIGH>;
    enable-active-high;
};

&usbc1 {
    usb_port_type = <1>;       /* host */
    usb_detect_type = <0>;
    usb_regulator_io = "nocare";
    status = "okay";
};

&ehci1 { status = "okay"; };
&ohci1 { status = "okay"; };
```

关键点：

- USB 摄像头跑通依赖 VBUS、电源限流、PHY、EHCI/OHCI。
- 若 PB3 与 CTP/CAN 复用，必须按最终硬件取舍。

## EMAC

```dts
&gmac0 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&gmac0_pins_a>;
    pinctrl-1 = <&gmac0_pins_b>;
    phy-mode = "rmii";
    phy-rst = <&pio PE 10 GPIO_ACTIVE_LOW>;
    status = "okay";
};
```

关键点：

- Base 图纸显示 DVP CAMERA 与 RMII 网络二选一。
- 如果使用 USB 摄像头，则 EMAC 可以保留；如果启用 DVP 摄像头，需重新评估 RMII 引脚冲突。

## GPIO LED / KEY

```dts
gpio-leds {
    compatible = "gpio-leds";
    status_led {
        label = "edge:green:status";
        gpios = <&pio PB 4 GPIO_ACTIVE_HIGH>;
        linux,default-trigger = "heartbeat";
    };
};

gpio-keys {
    compatible = "gpio-keys";
    user_key {
        label = "USER_KEY";
        gpios = <&pio PB 4 GPIO_ACTIVE_LOW>;
        linux,code = <KEY_ENTER>;
        debounce-interval = <20>;
    };
};
```

关键点：

- PB4 在图纸文本中出现 USER_KEY/CAN1_TX 复用线索。
- LED/KEY GPIO 只作为示例，必须确认未与 CAN、CTP、USB 冲突。

## Watchdog

```dts
&wdt {
    status = "okay";
};
```

关键点：

- Kernel 需启用对应 sunxi watchdog 驱动。
- 用户态通过 `/dev/watchdog` 控制喂狗。

## Audio Codec / DMIC / I2S

```dts
&codec {
    status = "okay";
};

&dmic {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&dmic_pins_a>;
    pinctrl-1 = <&dmic_pins_b>;
    status = "disabled";
};

&i2s0 {
    pinctrl-names = "default", "sleep";
    pinctrl-0 = <&i2s0_pins_a>;
    pinctrl-1 = <&i2s0_pins_b>;
    status = "disabled";
};
```

关键点：

- Core 图纸可见 MICIN3P/MICIN3N、HPOUTL/HPOUTR。
- DSP 音频处理的数据来源可以是 Codec/DMIC/I2S，最终通路需由 SDK 音频驱动和硬件接线确认。
