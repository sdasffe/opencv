【独立示例】与主工程 opencv.pro / widget.cpp 等无关，不要把本目录加进主工程。

怎么跑：
1. Qt Creator → 打开 plugin_demo/plugin_demo.pro
2. 选与你主工程相同的 Kit（如 MSVC 2019 64bit）
3. 构建全部（会编 democore.dll、blur_plugin.dll、plugin_host.exe）
4. 运行 host（目标选 host 或运行 plugin_demo/bin/plugin_host.exe）

运行后同目录应有：
  plugin_host.exe
  democore.dll
  blur_plugin.dll

操作：
  左侧应出现「模糊处理」→ 选中 →「创建选中算法块」→ 右侧出现块
  →「调用 process()」→ 看到结果文字里写着代码在 blur_plugin.dll

要点：
  host 工程里没有 BlurBlock，只有 IBlockPlugin* → create() → DemoBlock*
  BlurBlock 只存在于 blur_plugin 工程中
