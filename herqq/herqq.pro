TEMPLATE = subdirs
CONFIG  += ordered

exists(hupnp/options.pri) {
    win32 {
        system(type nul > hupnp/options.pri)
    }
    else {
        system(echo "" > hupnp/options.pri)
    }
}

exists(hupnp_av/options.pri) {
    win32 {
        system(type nul > hupnp_av/options.pri)
    }
    else {
        system(echo "" > hupnp_av/options.pri)
    }
}

CONFIG(DISABLE_QTSOAP) {
    system(echo "CONFIG += DISABLE_QTSOAP" > hupnp/options.pri)
}
CONFIG(USE_QT_INSTALL_LOC) {
    system(echo "CONFIG += USE_QT_INSTALL_LOC" >> hupnp/options.pri)
	system(echo "CONFIG += USE_QT_INSTALL_LOC" >> hupnp_av/options.pri)
}

!CONFIG(DISABLE_CORE) : SUBDIRS += hupnp
!CONFIG(DISABLE_AV) : SUBDIRS += hupnp_av
!CONFIG(DISABLE_TESTAPP) : SUBDIRS += apps/simple_test-app
!CONFIG(DISABLE_AVTESTAPP) : SUBDIRS += apps/simple_avtest-app
