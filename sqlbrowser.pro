TEMPLATE        = app
TARGET          = sqlbrowser

LIBS += -L/usr/lib/oracle/11.2/client64/lib/ -lclntsh -lnnz11

QT              += sql widgets core

HEADERS         = browser.h \
    connection.h
SOURCES         = main.cpp browser.cpp \
    connection.cpp

FORMS           = browserwidget.ui
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sql/sqlbrowser
INSTALLS += target



wince*: {
    DEPLOYMENT_PLUGIN += qsqlite
}
CONFIG += static
static {
    CONFIG += static
    QTPLUGIN += qsqloci
    DEFINES += STATIC
}

RESOURCES += \
    plan.qrc
