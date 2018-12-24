#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];
static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int last_op = '0';

void gen_num() {
  uint32_t num = ((uint32_t) rand()) % 5000;
  if(last_op == '/') num += 1;
  if(last_op == '*') num = num / 100 + 1;
  sprintf(buf + strlen(buf), "%d", num);
}

void gen_op() {
  switch(rand()%15) {
    case 0 ... 7: last_op = '+'; strncat(buf, "+", 1); break;
    case 8 ... 9: last_op = '-'; strncat(buf, "-", 1); break;
    case 10:      last_op = '/'; strncat(buf, "/", 1); break;
    case 11:      last_op = '*'; strncat(buf, "*", 1); break;
    case 12:      last_op = ' '; strncat(buf, "!=", 2); break;
    case 13:      last_op = ' '; strncat(buf, "==", 2); break;
    case 14:      last_op = ' '; strncat(buf, "&&", 2); break;
    default : assert(0);
  }
} 



void gen_rand_expr() {
  while(1) {
    int n = rand()%10;
    if(n < 7) {
      gen_num();
      gen_op();
    }
    else if(n < 8) {
      strncat(buf, "(", 1); gen_rand_expr(); strncat(buf, ")", 1) ; gen_op();
    }
    else {
      gen_num();
      break;
    } 
  }
}



int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    assert(fscanf(fp, "%d", &result)>0);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
