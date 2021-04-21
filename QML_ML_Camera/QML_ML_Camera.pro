TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
           logger-core\
           camera-core\
           app\


camera-core.depends = logger-core
app.depends = camera-core
