# 顶层：依次编 core → blur_plugin → host
# 与主工程 opencv.pro 完全独立，勿合并进主工程。
TEMPLATE = subdirs
SUBDIRS = core blur_plugin host
blur_plugin.depends = core
host.depends = core
