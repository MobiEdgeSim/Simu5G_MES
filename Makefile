all: checkmakefiles
	@cd src && $(MAKE)

tests: tests_lte tests_NR tests_mec
	@echo "All tests done."

tests_lte: all
	@cd src && $(MAKE) && cd ../tests/fingerprint/LTE && ./fingerprints

tests_NR: all
	@cd src && $(MAKE) && cd ../tests/fingerprint/NR && ./fingerprints

tests_mec: all
	@cd src && $(MAKE) && cd ../tests/fingerprint/mec && ./fingerprints

clean: checkmakefiles
	@cd src && $(MAKE) clean

cleanall: checkmakefiles
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile

makefiles:
	@cd src && opp_makemake --make-so -f --deep -o simu5g -O out -KINET_PROJ=../../inet -KVEINS_INET_PROJ=../../veins_inet -KVEINS_PROJ=../../veins -DINET_IMPORT -DVEINS_INET_IMPORT -I. -I/usr/lib/jvm/java-11-openjdk-amd64/include -I/usr/lib/jvm/java-11-openjdk-amd64/include/linux -I/usr/lib/jvm/java-11-openjdk-amd64/lib -I/usr/lib/jvm/java-11-openjdk-amd64/lib/server -I/opt/conda/pkgs/openjdk-11.0.1-h516909a_1016/include -I/opt/conda/pkgs/openjdk-11.0.1-h516909a_1016/include/linux -I/opt/conda/pkgs/openjdk-11.0.1-h516909a_1016/lib -I/opt/conda/pkgs/openjdk-11.0.1-h516909a_1016/lib/server -I$$\(INET_PROJ\)/src -I$$\(VEINS_INET_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -L/usr/lib/jvm/java-11-openjdk-amd64/lib/server -L/opt/conda/pkgs/openjdk-11.0.1-h516909a_1016/lib/server -L$$\(INET_PROJ\)/src -L$$\(VEINS_INET_PROJ\)/src -lINET$$\(D\) -lveins_inet$$\(D\)

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
