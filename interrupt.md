中断向量与非中断向量
X86 - 有中断向量
ARM - 没有中断向量

中断ISR top half 调用disable_irq() 屏蔽中断控制器中断响应。
中断ISR bottom half 调用enable_irq() 使能中断控制器中断响应。
