#### azplf: linux boot
- linux
```
$ sudo apt install gcc-4.7-arm-linux-gnueabihf
$ git clone http://github.com/atsupi/Linux-Digilent-Dev.git -b master-next
$ cd Linux-Digilent-Dev
$ export ARCH=arm
$ export CROSS_COMPILE=arm-linux-gnueabihf-
$ make xilinx_zynq_defconfig
$ make
$ make UIMAGE_LOADADDR=0x00008000 uImage
$ cp arch/arm/boot/uImage .
$ make zynq-zybo.dtb
$ cp arch/arm/boot/dts/zynq-zybo.dtb .
```
- config: linux uImage is built with following configurations to support spidev interface.
```
CONFIG_SPIDEV=y
# CONFIG_PM is not used
```
- device tree:
  following items are added to support i2c0 and spidev32766.0 interfaces.
```
i2c0: i2c0@e0004000 {
    compatible = "cdns,i2c-r1p10";
    clocks = <&clkc 38>;
    interrupt-parent = <&ps7_scugic_0>;
    interrupts = <0 25 4>;
    reg = <0xe0004000 0x1000>;
    #address-cells = <1>;
    #size-cells = <0>;
    clock-frequency = <400000>;
    status = "okay";
    ssm2603: ssm2603@1a {
      #sound-dai-cells = <0>;
      compatibel = "adi,ssm2603";
      reg = <0x1a>;
    };
};
ps7_spi_1: spi@e0007000 {
    compatible = "xlnx,zynq-spi-r1p6";
    reg = <0xe0007000 0x1000>;
    status = "okay";
    interrupt-parent = <&ps7_scugic_0>;
    interrupts = <0 26 4>;
    clocks = <&clkc 26>, <&clkc 35>;
    clock-names = "ref_clk", "pclk";
    num-cs = <3>;
    is-decoded-cs = <0>;
    #address-cells = <0x1>;
    #size-cells = <0x0>;
    spidev@0 {
        compatible = "spidev";
        spi-max-frequency = <1000000>;
        reg = <0>;
    };
};
```
