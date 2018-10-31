下载Linux内核
下载内核有两种方法，一种是用git直接下载内核代码树，方便后面的内核开发。另一种是直接到内核社区下载稳定版本（详见：https://www.kernel.org/pub/linux/kernel/v4.x/）。下面演示从Linux kernel主线下载代码进行编译。

git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

安装arm的交叉编译工具链
想必做嵌入式开发的朋友，对交叉编译工具链不陌生。如果你订制一个交叉编译工具链，建议你使用crosstool-ng开源软件来构建。但在这里建议直接安装arm的交叉编译工具链：
sudo apt-get install gcc-arm-linux-gnueabi

编译Linux内核
生成vexpress开发板子的config文件：

make CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm vexpress_defconfig


编译：
make CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm
生成的内核镱像位于arch/arm/boot/zImage， 后续qemu启动时需要使用该镜像。
