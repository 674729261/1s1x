echo -e -n 'CFLAGS=-Wall -g\nall:' > Makefile
ls | tr ' ' '\n'| grep .c | sed "s/\.c//g" | sort | uniq | tr '\n' ' ' >> Makefile
echo -e -n '\nclean:\n\trm -f ' >> Makefile
ls | tr ' ' '\n'| grep .c | sed "s/\.c//g" | sort | uniq | tr '\n' ' ' >> Makefile
