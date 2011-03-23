function complete()
    {
    if (cur_name != "")
            {
	      if (substr(source_name,length(source_name)-3) == ".cpp") 
		compiler = "$(CXX) $(EXTRA_CXXFLAGS)";
	      else if (substr(source_name,length(source_name)-2) == ".cc")
		compiler = "$(CXX) $(EXTRA_CXXFLAGS)";
	      else
		compiler = "$(CC) $(EXTRA_CFLAGS)";
            printf("	%s -c -o %s $(PROFILE_COMPILE) $(COMPILER_SPECIFIC_CXXFLAGS) \\\n",compiler,cur_name);
            printf("	$(CCFLAGS) $(INCLDIRS) $(SYSINCLDIRS) $(PREPROCESSORFLAGS) \\\n");
            printf("	$(SUIF_MODULE) %s\n\n",source_name);
            }

    }

/:/ {
	complete();
	cur_name = $1;
	sub(":","",cur_name);
	source_name = $2;
	}

{
        for (i=1;i < NF; i++)
            {
            if (!match($i,":"))
                deps[$i] = 1;
            }
        print;
        }



END {
	complete();
	printf("MAKEFILE_DEPS_RULE = defined\n");
	printf("Makefile.deps: Makefile ");
	for (i in deps)
	    printf("\\\n	%s ",i);
	printf("\n");
        printf("	@echo '# Dependencies for C files' > Makefile.deps\n");
        printf("	@echo >> Makefile.deps\n");
	printf("	@rm -f dependencies\n");
        printf("	$(CC) $(DEPSFLAG) $(EXTRA_CFLAGS) $(EXTRA_CXXFLAGS) $(COMPILER_SPECIFIC_CXXFLAGS) $(CCFLAGS) \\\n");
        printf("	$(INCLDIRS) $(SYSINCLDIRS) $(PREPROCESSORFLAGS) $(SRCS) > dependencies\n");
        printf("	$(AWK) -f $(NCIHOME)/dep2make.awk < dependencies >> Makefile.deps\n");
	printf("\n");

    }
	
BEGIN {
	cur_name = "";
	printf("CPP_TO_O_RULE = defined\n");
	printf("C_TO_O_RULE = defined\n");
  	}
