include ./config.mk

.PHONY: all install uninstall reallyclean clean

LIB_OBJS=ftlm_config.o\
		ftlm_client.o\
		ftlm_object.o\
		ftlm_msg.o\
		ftlm_server.o

all : libftlm.a ftlm

ftlm : ftlm.o 
	${CROSS_COMPILE}${CC} $^ -o $@ ${FTLM_LDFLAGS}

ftlm.o : ftlm.c libftlm.a
	${CROSS_COMPILE}${CC} -c $< -o $@ ${FTLM_CFLAGS}

libftlm.a : ${LIB_OBJS}
	${CROSS_COMPILE}$(AR) cr $@ $^

ftlm_config.o : ftlm_config.c ftlm_config.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_client.o : ftlm_client.c ftlm_client.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_object.o : ftlm_object.c ftlm_object.h
	${CROSS_COMPILE}$(CC) $(LIB_CFLAGS) -c $< -o $@

ftlm_msg.o : ftlm_msg.c ftlm_msg.h
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
	-rm -f *.o ftlm

