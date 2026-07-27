# 总工程：blocksdk → 算法插件 DLL → 主程序空壳
# 每个算法源码集中在 plugins/<名字>/，编译为 bin/*/plugins/block_*.dll
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += blocksdk plugins opencv_app

blocksdk.file = blocksdk/blocksdk.pro
plugins.file = plugins/plugins.pro
opencv_app.file = opencv_app.pro

plugins.depends = blocksdk
opencv_app.depends = blocksdk plugins
