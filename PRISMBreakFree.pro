APP_NAME = PRISMBreakFree

CONFIG += qt warn_on cascades10

include(config.pri)

LIBS += -lbbdevice -lbb -lbbsystem -lbbcascadespickers -lbbpim -lbbutilityi18n -lunifieddatasourcec
LIBS += -lhuapi -lbbplatform -lbbdata -lbbnetwork
