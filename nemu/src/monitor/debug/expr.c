#include "nemu.h"
#include "cpu/reg.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define AND_PRIOR 7
#define EQUAL_PRIOR 8
#define PLUS_MINUS_PRIOR 9
#define MULTI_DIVIDE_PRIOR 10
#define UNARY_PRIOR 11
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_NEQ, TK_AND, TK_DEREF, TK_HEX, TK_REG, TK_NEG

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"0[xX][A-Fa-f0-9]+", TK_HEX},
  {"[0-9]+", TK_NUM},   //number
  {"\\$(eax|ebx|ecx|edx|ebp|esp|esi|edi|eip)",TK_REG},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\*", '*'},         // mutiple
  {"-", '-'},         // minus
  {"/", '/'},         // divide
  {"\\(", '('},
  {"\\)", ')'},         
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},         // not equal
  {"&&", TK_AND}  
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[256];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          default:
            tokens[nr_token].type = rules[i].token_type;
            strncpy (tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len]='\0';
            nr_token ++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(uint32_t p, uint32_t q) {
  if(tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  int left = 0, right = 0;
    for (int i = p + 1; i < q; i ++)
    {
      if (tokens[i].type == '(') left ++;
      if (tokens[i].type == ')') right ++;
      if (left < right)
        return false; 
    }
    if (left == right)
      return true;
  return false;
}

int primary_operator(int p, int q) {
  int pos = -1;
  int brackets = 0, current_priotity = 100;
  for (int i = p; i < q; ++i)
  {
    switch (tokens[i].type) {
      case TK_NUM : break;
      case '(' : brackets++; break;
      case ')' : brackets--; break;
      default  : 
        if (brackets != 0) break;
        else if ((tokens[i].type == '+' || tokens[i].type == '-') && (current_priotity >= PLUS_MINUS_PRIOR)) {
          pos = i;
          current_priotity = PLUS_MINUS_PRIOR;
          break;
        }
        else if ((tokens[i].type == '*' || tokens[i].type == '/') && (current_priotity >= MULTI_DIVIDE_PRIOR)) {
          pos = i;
          current_priotity = MULTI_DIVIDE_PRIOR;
          break;
        }
        else if ((tokens[i].type == TK_NEQ || tokens[i].type == TK_EQ) && (current_priotity >= EQUAL_PRIOR)) {
          pos = i;
          current_priotity = EQUAL_PRIOR;
          break;
        }
        else if ((tokens[i].type == TK_AND) && (current_priotity >= AND_PRIOR)) {
          pos = i;
          current_priotity = AND_PRIOR;
          break;
        }
        else if ((tokens[i].type == TK_DEREF || tokens[i].type == TK_NEG) && (current_priotity > UNARY_PRIOR)) {
          pos = i;
          current_priotity = UNARY_PRIOR;
          break;
        }
    }
  }
  return pos;
}

uint32_t eval(int p, int q, bool *success) {

  // p stands for the start position of the sub-expression, and q stands for the end position of it

  Assert (p <= q,"Bad expression!\n");
  if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    uint32_t num = 0;
    if (tokens[p].type == TK_NUM) {
      sscanf(tokens[p].str,"%d",&num);
      return num;
    }
    else if(tokens[p].type == TK_HEX) {
      sscanf(tokens[p].str,"%x",&num);
      return num;
    }
    else if(tokens[p].type == TK_REG) {
      return reg_content(tokens[p].str+1);
    }
    *success = false;
    Log("Fault in eval\n");
    return 0;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    int op = primary_operator(p, q);
    //Log("Primiary_operator : %s", tokens[op].str);
    if (op < 0) {
      *success = false;
      Log("fault");
      return 0;
    }
    uint32_t val1=0, val2=0, addr=0, unary=0;
    if (tokens[op].type == TK_DEREF) 
      addr = eval(op+1, q, success);
    else  if (tokens[op].type == TK_NEG) 
      unary = eval(op+1, q, success);
    else {
      val1 = eval(p, op - 1, success);
      val2 = eval(op + 1, q, success);
    }
    if(!*success)
      return 0;
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': if (val2==0) {
                  printf("The divisor is zero\n");
                  *success = false;
                  return 0;
                }
                 return val1 / val2;
      case TK_EQ : return val1==val2;
      case TK_NEQ : return val1!=val2;
      case TK_AND : return val1&&val2;
      case TK_DEREF : 
      Log("Addr : %d 0x%x\n", addr, addr);
        if(!*success)
          return 0;
        return vaddr_read(addr, 4);
      case TK_NEG : return -unary;
      default: assert(0);
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    Log("Fault in when excuting make_token\n");
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();

  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')'&& tokens[i-1].type != TK_REG&& tokens[i-1].type != TK_HEX)) ) {
      tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == '-' && (i == 0 || (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')'&& tokens[i-1].type != TK_REG&& tokens[i-1].type != TK_HEX)) ) {
      tokens[i].type = TK_NEG;
    }

  }
  return eval(0, nr_token -1, success);
}