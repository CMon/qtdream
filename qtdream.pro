include(config.pri)

TEMPLATE = subdirs

CONFIG(gui) {
	message("Building gui")
	SUBDIRS += \
		src/qtdream-gui \
		src/qtdream-dbclient \
		src/qtdream-server
}

