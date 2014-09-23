static int check_opt(char **argv, char **opt) {
  if (argv[0][2]) { *opt = argv[0]+2; return 0; }
  else if (argv[1]) { *opt = argv[1]; return 1; }
  carp("Option ",argv[0]," requires an argument");
  _exit(1);
}

#define chk_opt(argv,opt) argv+=check_opt(argv,&opt)
