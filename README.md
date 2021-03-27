# Fixed VHD Writer
一种使用LBA方式的、专用于固定大小的VHD虚拟磁盘的扇区写入工具

---

## 编译生成
### Linux/Unix
Linux 下直接使用 GCC 编译 writer.c (包含 writer.h) 文件就行
```shell 
gcc -o writer writer.c
```
程序使用方法:
```
Fixed VHD Writer
[-h] usage help
[-r] specify data file name (read)
[-w] specify VHD file name (write)
[-a] specify LBA to writing data
```

### Windows
Windows 除了 CLI 版本同样可以通过编译器 (MSVC、MinGW) 编译之外，也可以在 ``` win32 ``` 目录下打开 Visual Studio 解决方案 ```FixedVHDWriter.sln``` 并生成为 MFC 程序

---

## Release 下载
### Windows
* CLI 版本下载: [fvw.exe](https://github.com/yenyuloong/fixed-vhd-writer/blob/master/bin/fvw.exe?raw=true)
* GUI(MFC) 版本下载：[FixedVHDWriter.exe](https://github.com/yenyuloong/fixed-vhd-writer/blob/master/bin/FixedVHDWriter.exe?raw=true)

### Linux/Unix
请直接本地编译使用
