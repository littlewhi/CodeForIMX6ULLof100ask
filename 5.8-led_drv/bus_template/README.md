#BUS DRIVER TEMPLATE
>总线驱动模型，分为两层，一硬件函数层，二是bus层。
>bus层主要做一件事，就是硬件资源匹配驱动，两个结构体`struct platform_device`, `struct platform_driver`,
>硬件函数层实现具体操纵硬件的方法，结构体-`struct file_operations`，函数-`open(), read(), write(), ...`
>一个板子上的同类器件（如led）有很多，但不必为每一个具体器件都写一个驱动，实现通用的操作函数包括同类所有器件的硬件操作,通过bus模型将具体器件与具体操作方法匹配
>如同一个板的led
>>每个led的引脚，寄存器是不一样的，但是使能，输出流程是相似的，将这些硬件操作实现在硬件函数层，且通过次设备号区分区分引脚、获取寄存器灯资源和硬件操作。bus层在`struct platform_device`添加led资源，`struct platform_driver`实现自动匹配，将此设备号与引脚建立联系并创建设备文件（在此处创建设备文件，而不是在硬件函数层创建设备文件，因为led引脚有次设备号匹配是在此处）
 - - -
 * * *
>本代码的具体实现说明：
>>硬件函数层有分成了三层，第一层就是linux的普通框架的`struct file_operatons`结构体的构造，但是这层不涉及具体硬件操作，只是利用`open()`识别出次设备号-`minor`将其传递给下一层`struct led_operations`匹配硬件，第一层进行的操作都会是调用此结构体，但是第二层仍不会涉及具体的硬件操作，而是将`minor`转换成硬件索引找到所要操作的第三层硬件的`struct led_resource`来执行具体的硬件操作，将将其地址赋给一个通用指针`cur_led`，每次具体硬件操作，都是调用`cur_led`执行。
>>bus层就是实现`minor`与硬件的联系，并且创造设备文件。