### high memeory

1）high memory针对的是物理内存，不是虚拟内存，更确切的，虚拟地址空间。

2）high memory也是被内核管理的（有对应的page结构），只是没有映射到内核虚拟地址空间。当kernel需要分配high memory时，通过kmap等从预留的地址空间中动态分配一个地址，然后映射到high memory，从而访问这个物理页。

3）high memory和low memory一样，都是参与内核的物理内存分配，都可以被映射到kernel地址空间，也都可以被映射到user space地址空间。

4）物理内存<896M时，没有high memory，因为所有的内存都被kernel直接映射了。

5）64位系统下不会有high memory，因为64位虚拟地址空间非常大（分给kernel的也很大），完全能够直接映射全部物理内存。


### CMA:
https://www.slideshare.net/kerneltlv/continguous-memory-allocator-in-the-linux-kernel
