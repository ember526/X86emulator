#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "cpu/reg.h"
#include "memory/memory.h"
#include "nemu.h"


#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

const char *regs[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "\tDisplay informations about all supported commands", cmd_help },
  { "c", "\tContinue the execution of the program", cmd_c },
  { "q", "\tExit NEMU", cmd_q },
  { "si", "\tExecute the program for N(default is 1) more steps and pause", cmd_si },
  { "info", "\tr:Print the registers w:Print the watchpoints", cmd_info},
  { "x", "\tPrint N bytes from EXP in memory", cmd_x},
  { "p", "\tEvaluate expressions", cmd_p},
  { "w", "\tAdd an expression as watchpoint", cmd_w},
  { "d", "\tDelete the watchpoint with number N", cmd_d}

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  int i = 0;
  if(args == NULL)
    i = 1;
  else {
    char *arg = strtok(args, " ");
    if (sscanf(arg, "%d", &i) < 1) {
      printf("Invalid argument : '%s'\n", args);
      return 0;
    }
  }
  cpu_exec(i);
  return 0;
}
extern WP * head;
static int cmd_info(char *args) {
  if(args == NULL) {
    printf("Invalid argument\n");
    return 0;
  }
  char *arg = strtok(args, " ");
  if(!strcmp(arg, "r")) {
    for (int i = R_EAX; i <= R_EDI; i ++) {
      printf ("%-10s\t0x%-10x\t%-d\n", regs[i], reg_l(i), reg_l(i));
    }
    printf("\n");
    printf ("%-10s\t0x%-10x\t%-d\n", "eip", cpu.eip, cpu.eip);
    printf("\n");
    printf ("%-10s\t0x%-10x\t%-d\n", "ZF", cpu.ZF, cpu.ZF);
    printf ("%-10s\t0x%-10x\t%-d\n", "SF", cpu.SF, cpu.SF);
    printf ("%-10s\t0x%-10x\t%-d\n", "CF", cpu.CF, cpu.CF);
    printf ("%-10s\t0x%-10x\t%-d\n", "OF", cpu.OF, cpu.OF);
  }
  else if(!strcmp(arg, "w")) {
    print_wp();
  }
  return 0;
}

static int cmd_x(char *args) {
  if(args == NULL) {
    printf("Invalid argument\n");
    return 0;
  }
  char *arg = strtok(args, " ");
  int N = 0, addr;
  if(arg == NULL || sscanf(arg, "%d", &N) < 1) {
    printf("Invalid argument : '%s'\n", args);
    return 0;
  }

  arg = strtok(NULL, " ");
  if(arg == NULL || sscanf(arg, "%x", &addr) < 1) {
    printf("Invalid argument : '%s'\n", args);
    return 0;
  }
  bool success = true;
  addr = expr(arg, &success);
  if(! success) {
    printf("Invalid Expression : %s\n", arg);
    return 0;
  }
  for (int i = 0; i < N; ++i) {
    int content = vaddr_read(addr + i * 4, 4);//pmem[base] + (pmem[base+1]<<8) + (pmem[base+2]<<16) + (pmem[base+3]<<24); 
    printf("0x%-10x\t0x%-10.8x\t%-20d%c%c%c%c\n", addr+i*4, content, content, ((char *)&content)[0], ((char *)&content)[1], ((char *)&content)[2], ((char *)&content)[3]);
  }
  return 0;
  //printf("%d %x\n", N, addr);
}

int test_expr() {
  FILE *fp = fopen("./tools/gen-expr/input", "r");
  assert(fp);
  char input[65536];
  int correct_answer = 0;
  int i = 1;
  int wrong = 0;
  while(fscanf(fp, "%d %s", &correct_answer, input)== 2) {
    bool success = true;
    uint32_t result = expr(input, &success);
    if(!success) {
      printf("Test %d Invalid expression : %s\n", i, input);
    }
    else if(result != correct_answer) {
      wrong++;
      printf("Test %d Error in %s. Correct Answer : %d; Result : %d\n", i,  input, correct_answer, result);
    }
    else {
      printf("Test %d passed : %s. Correct Answer : %d; Result : %d\n", i, input, correct_answer, result);
    }
    ++i;
  }
  return wrong;
}

static int cmd_p(char *args) {
  if(!strcmp(args, "test")) {
    printf("test completed, Wrong answers : %d\n", test_expr());
    return 0; 
  }
  printf("Evaluating...\n");
  bool success = true;
  uint32_t result = expr(args, &success);
  if(!success)
    printf("Invalid expression : %s\n", args);
  else printf("%s = %u\n", args, result);
  return 0;
}

static int cmd_w(char *args) {
  if(args == NULL) {
    printf("Invalid argument\n");
    return 0;
  }
  new_wp(args);
  return 0;
}

static int cmd_d(char *args) {
  if(args == NULL) {
    printf("Invalid argument\n");
    return 0;
  }
  int N = 0;
  if(sscanf(args, "%d", &N) < 1) {
    printf("Invalid argument : '%s'\n", args);
    return 0;
  }
  free_wp(N);
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
