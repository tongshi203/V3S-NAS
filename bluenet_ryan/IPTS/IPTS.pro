SOURCES += \
    main.cpp \
    udp_rec_thread.cpp \
    udp_data_port.cpp \
    dvb_crc.cpp \
    bit_stream.cpp \
    raw_udp.cpp \
    parse_content_xml.cpp \
    sub_network_encapsulation.cpp \
    udp_send.cpp

HEADERS += \
    udp_rec_thread.h \
    udp_data_port.h \
    value.h \
    dvb_crc.h \
    bit_stream.h \
    raw_udp.h \
    parse_content_xml.h \
    sub_network_encapsulation.h \
    udp_send.h

QT += xml
DEFINES += ULE

CONFIG( debug, debug | release ) {
        TARGET = $$join( TARGET , , , D )
        DEFINES +=  _DEBUG
} else {
        DEFINES += QT_NO_DEBUG_OUTPUT
        DEFINES += QT_NO_WARNING_OUTPUT
}

CONFIG += debug_and_release
Release:OBJECTS_DIR = release/obj
Release:MOC_DIR = release/moc
Debug:OBJECTS_DIR = debug/obj
Debug:MOC_DIR = debug/moc
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
