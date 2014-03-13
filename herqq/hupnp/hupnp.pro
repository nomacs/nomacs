TEMPLATE = subdirs
CONFIG  += ordered

!CONFIG(DISABLE_QTSOAP) : SUBDIRS += lib/qtsoap-2.7-opensource/buildlib
!CONFIG(DISABLE_CORE) : SUBDIRS += src.pro
