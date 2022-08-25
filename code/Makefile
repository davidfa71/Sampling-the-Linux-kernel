DIRS=KconfigSampler Logic2BDD Tseitin

all:
	$(MAKE) -C cudd-3.0.0 
	$(MAKE) -C cudd-3.0.0 install
	@for i in $(DIRS); do  $(MAKE) -C $$i; done
clean:
	$(MAKE) -C cudd-3.0.0 clean
	@for i in $(DIRS); do $(MAKE) -C $$i clean; done
	@rm -f lib/libcudd*
	@rm -f include/*

