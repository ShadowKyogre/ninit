#include "../ninitfeatures.h"
#include "../error_table.h"

int main(int argc,char **argv,char **envp) {
  errmsg_iam("argv0");
  if (argc < 3) { 
    carp("usage: argv0", " realname program [ arg ... ]");
    return 100;
  }
  pathexec_run(argv[1],argv + 2,envp);
  carp("unable to run ",argv[1],": ",error_string(table,errno));
  return 111;
}
