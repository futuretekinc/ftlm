include ./config.mk

.PHONY: all install uninstall reallyclean clean

LIB_OBJS=ftlm_config.o\
		ftlm_client.o\
		ftlm_object.o\
		ftlm_client_msg.o\
		ftlm_server.o

all : libftlm.a libftlmapi.a ftlm ftlm_console

ftlm : ftlm.o 
	${CROSS_COMPILE}${CC} $^ -o $@ ${FTLM_LDFLAGS}

ftlm.o : ftlm.c libftlm.a
	${CROSS_COMPILE}${CC} -c $< -o $@ ${FTLM_CFLAGS}

ftlm_console : ftlm_console.o  
	${CROSS_COMPILE}${CC} $^ -o $@ ${FTLM_CONSOLE_LDFLAGS}

ftlm_console.o : ftlm_console.c libftlmapi.a
	${CROSS_COMPILE}${CC} -c $< -o $@ ${FTLM_CFLAGS}

libftlmapi.a : ftlm_server_api.o
	${CROSS_COMPILE}$(AR) cr $@ $^

ftlm_server_api.o : ftlm_server_api.c ftlm_server_api.h
	${CROSS_COMPILE}${CC} -c $< -o $@ ${FTLM_CFLAGS}

libftlm.a : ${LIB_OBJS}
	${CROSS_COMPILE}$(AR) cr $@ $^

ftlm_config.o : ftlm_config.c ftlm_config.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_client.o : ftlm_client.c ftlm_client.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_object.o : ftlm_object.c ftlm_object.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_client_msg.o : ftlm_client_msg.c ftlm_client_msg.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_server.o : ftlm_server.c ftlm_server.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

install : all
	$(INSTALL) -d ${DESTDIR}$(prefix)/bin
	$(INSTALL) -s --strip-program=${CROSS_COMPILE}${STRIP} ftlm ${DESTDIR}${prefix}/bin/ftlm

uninstall :
	-rm -f ${DESTDIR}${prefix}/bin/ftlm

reallyclean : clean

clean : 
	-rm -f *.o ftlm lib*.a

