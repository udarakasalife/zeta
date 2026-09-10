#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

typedef enum {
    T_PRINT, T_LET, T_IF, T_THEN, T_ELSE, T_END, T_WHILE, T_DO,
    T_TRUE, T_FALSE, T_AND, T_OR, T_NOT,
    T_IMPORT, T_STORE, T_FETCH, T_NODE,
    T_IDENT, T_NUMBER, T_STRING,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_PERCENT,
    T_EQ, T_EQEQ, T_NOTEQ, T_LT, T_GT, T_LTEQ, T_GTEQ,
    T_LPAREN, T_RPAREN,
    T_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    double num;
} Token;

typedef struct {
    const char *src;
    size_t pos;
    size_t len;
    Token *tokens;
    size_t tok_count;
    size_t tok_cap;
} Lexer;

static void lexer_push(Lexer *lx, TokenType type, const char *text, double num) {
    if (lx->tok_count == lx->tok_cap) {
        lx->tok_cap = lx->tok_cap ? lx->tok_cap * 2 : 64;
        lx->tokens = realloc(lx->tokens, lx->tok_cap * sizeof(Token));
    }
    Token t;
    t.type = type;
    t.text = text ? strdup(text) : NULL;
    t.num = num;
    lx->tokens[lx->tok_count++] = t;
}

typedef struct {
    TokenType type;
    const char *canonical;
    char *word;
} KeywordEntry;

static KeywordEntry keyword_table[] = {
    {T_PRINT, "print", NULL},
    {T_LET, "let", NULL},
    {T_IF, "if", NULL},
    {T_THEN, "then", NULL},
    {T_ELSE, "else", NULL},
    {T_END, "end", NULL},
    {T_WHILE, "while", NULL},
    {T_DO, "do", NULL},
    {T_TRUE, "true", NULL},
    {T_FALSE, "false", NULL},
    {T_AND, "and", NULL},
    {T_OR, "or", NULL},
    {T_NOT, "not", NULL},
    {T_IMPORT, "import", NULL},
    {T_STORE, "store", NULL},
    {T_FETCH, "fetch", NULL},
    {T_NODE, "node", NULL},
};
#define KEYWORD_COUNT (sizeof(keyword_table) / sizeof(keyword_table[0]))

static void init_keywords(void) {
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        keyword_table[i].word = strdup(keyword_table[i].canonical);
    }
}

static void load_lang_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "zeta: gak nemu paket bahasa '%s'\n", path);
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *canonical = line;
        char *local = eq + 1;
        for (size_t i = 0; i < KEYWORD_COUNT; i++) {
            if (strcmp(keyword_table[i].canonical, canonical) == 0) {
                free(keyword_table[i].word);
                keyword_table[i].word = strdup(local);
                break;
            }
        }
    }
    fclose(f);
}

static void lex(Lexer *lx) {
    while (lx->pos < lx->len) {
        char c = lx->src[lx->pos];
        if (c == '#') {
            while (lx->pos < lx->len && lx->src[lx->pos] != '\n') lx->pos++;
            continue;
        }
        if (isspace((unsigned char)c)) { lx->pos++; continue; }
        if (isdigit((unsigned char)c)) {
            size_t start = lx->pos;
            while (lx->pos < lx->len && (isdigit((unsigned char)lx->src[lx->pos]) || lx->src[lx->pos] == '.')) lx->pos++;
            size_t n = lx->pos - start;
            char *buf = malloc(n + 1);
            memcpy(buf, lx->src + start, n);
            buf[n] = 0;
            lexer_push(lx, T_NUMBER, NULL, atof(buf));
            free(buf);
            continue;
        }
        if (c == '"') {
            lx->pos++;
            size_t start = lx->pos;
            while (lx->pos < lx->len && lx->src[lx->pos] != '"') lx->pos++;
            size_t n = lx->pos - start;
            char *buf = malloc(n + 1);
            memcpy(buf, lx->src + start, n);
            buf[n] = 0;
            lexer_push(lx, T_STRING, buf, 0);
            free(buf);
            lx->pos++;
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = lx->pos;
            while (lx->pos < lx->len && (isalnum((unsigned char)lx->src[lx->pos]) || lx->src[lx->pos] == '_')) lx->pos++;
            size_t n = lx->pos - start;
            char *buf = malloc(n + 1);
            memcpy(buf, lx->src + start, n);
            buf[n] = 0;
            {
                int matched = 0;
                for (size_t k = 0; k < KEYWORD_COUNT; k++) {
                    if (strcmp(buf, keyword_table[k].word) == 0) {
                        lexer_push(lx, keyword_table[k].type, NULL, 0);
                        matched = 1;
                        break;
                    }
                }
                if (!matched) lexer_push(lx, T_IDENT, buf, 0);
            }
            free(buf);
            continue;
        }
        switch (c) {
            case '+': lexer_push(lx, T_PLUS, NULL, 0); lx->pos++; break;
            case '-': lexer_push(lx, T_MINUS, NULL, 0); lx->pos++; break;
            case '*': lexer_push(lx, T_STAR, NULL, 0); lx->pos++; break;
            case '/': lexer_push(lx, T_SLASH, NULL, 0); lx->pos++; break;
            case '%': lexer_push(lx, T_PERCENT, NULL, 0); lx->pos++; break;
            case '(': lexer_push(lx, T_LPAREN, NULL, 0); lx->pos++; break;
            case ')': lexer_push(lx, T_RPAREN, NULL, 0); lx->pos++; break;
            case '=':
                lx->pos++;
                if (lx->pos < lx->len && lx->src[lx->pos] == '=') { lexer_push(lx, T_EQEQ, NULL, 0); lx->pos++; }
                else lexer_push(lx, T_EQ, NULL, 0);
                break;
            case '!':
                lx->pos++;
                if (lx->pos < lx->len && lx->src[lx->pos] == '=') { lexer_push(lx, T_NOTEQ, NULL, 0); lx->pos++; }
                break;
            case '<':
                lx->pos++;
                if (lx->pos < lx->len && lx->src[lx->pos] == '=') { lexer_push(lx, T_LTEQ, NULL, 0); lx->pos++; }
                else lexer_push(lx, T_LT, NULL, 0);
                break;
            case '>':
                lx->pos++;
                if (lx->pos < lx->len && lx->src[lx->pos] == '=') { lexer_push(lx, T_GTEQ, NULL, 0); lx->pos++; }
                else lexer_push(lx, T_GT, NULL, 0);
                break;
            default:
                fprintf(stderr, "zeta: karakter tidak dikenal '%c'\n", c);
                exit(1);
        }
    }
    lexer_push(lx, T_EOF, NULL, 0);
}

typedef enum {
    N_NUMBER, N_STRING, N_BOOL, N_IDENT,
    N_BINOP, N_UNARY, N_LET,
    N_PRINT, N_IF, N_WHILE, N_BLOCK,
    N_IMPORT, N_STORE, N_FETCH, N_NODE
} NodeType;

typedef struct Node {
    NodeType type;
    double num;
    char *str;
    TokenType op;
    struct Node *a, *b, *c;
    struct Node **list;
    size_t list_len, list_cap;
} Node;

static Node *node_new(NodeType type) {
    Node *n = calloc(1, sizeof(Node));
    n->type = type;
    return n;
}

static void node_list_push(Node *n, Node *child) {
    if (n->list_len == n->list_cap) {
        n->list_cap = n->list_cap ? n->list_cap * 2 : 8;
        n->list = realloc(n->list, n->list_cap * sizeof(Node *));
    }
    n->list[n->list_len++] = child;
}

typedef struct {
    Token *tokens;
    size_t pos;
} Parser;

static Token *cur(Parser *p) { return &p->tokens[p->pos]; }
static Token *advance(Parser *p) { return &p->tokens[p->pos++]; }
static int check(Parser *p, TokenType t) { return cur(p)->type == t; }

static Token *expect(Parser *p, TokenType t, const char *msg) {
    if (!check(p, t)) {
        fprintf(stderr, "zeta: error, seharusnya ada '%s'\n", msg);
        exit(1);
    }
    return advance(p);
}

static Node *parse_expr(Parser *p);

static Node *parse_primary(Parser *p) {
    Token *t = cur(p);
    if (t->type == T_NUMBER) {
        advance(p);
        Node *n = node_new(N_NUMBER);
        n->num = t->num;
        return n;
    }
    if (t->type == T_STRING) {
        advance(p);
        Node *n = node_new(N_STRING);
        n->str = strdup(t->text);
        return n;
    }
    if (t->type == T_TRUE || t->type == T_FALSE) {
        advance(p);
        Node *n = node_new(N_BOOL);
        n->num = t->type == T_TRUE ? 1 : 0;
        return n;
    }
    if (t->type == T_IDENT) {
        advance(p);
        Node *n = node_new(N_IDENT);
        n->str = strdup(t->text);
        return n;
    }
    if (t->type == T_LPAREN) {
        advance(p);
        Node *n = parse_expr(p);
        expect(p, T_RPAREN, ")");
        return n;
    }
    if (t->type == T_MINUS || t->type == T_NOT) {
        advance(p);
        Node *n = node_new(N_UNARY);
        n->op = t->type;
        n->a = parse_primary(p);
        return n;
    }
    if (t->type == T_FETCH) {
        advance(p);
        Node *n = node_new(N_FETCH);
        n->a = parse_primary(p);
        return n;
    }
    fprintf(stderr, "zeta: ada yang salah di bagian rumus/expression\n");
    exit(1);
}

static Node *parse_binop(Parser *p, int min_prec) {
    Node *left = parse_primary(p);
    for (;;) {
        TokenType op = cur(p)->type;
        int prec;
        switch (op) {
            case T_STAR: case T_SLASH: case T_PERCENT: prec = 5; break;
            case T_PLUS: case T_MINUS: prec = 4; break;
            case T_LT: case T_GT: case T_LTEQ: case T_GTEQ: prec = 3; break;
            case T_EQEQ: case T_NOTEQ: prec = 2; break;
            case T_AND: prec = 1; break;
            case T_OR: prec = 0; break;
            default: return left;
        }
        if (prec < min_prec) return left;
        advance(p);
        Node *right = parse_binop(p, prec + 1);
        Node *n = node_new(N_BINOP);
        n->op = op;
        n->a = left;
        n->b = right;
        left = n;
    }
}

static Node *parse_expr(Parser *p) {
    return parse_binop(p, 0);
}

static Node *parse_stmt(Parser *p);

static Node *parse_block_until(Parser *p, TokenType stop1, TokenType stop2) {
    Node *block = node_new(N_BLOCK);
    while (!check(p, stop1) && !check(p, stop2) && !check(p, T_EOF)) {
        node_list_push(block, parse_stmt(p));
    }
    return block;
}

static Node *parse_if(Parser *p) {
    advance(p);
    Node *n = node_new(N_IF);
    n->a = parse_expr(p);
    expect(p, T_THEN, "then");
    n->b = parse_block_until(p, T_ELSE, T_END);
    if (check(p, T_ELSE)) {
        advance(p);
        if (check(p, T_IF)) {
            n->c = parse_if(p);
            return n;
        }
        n->c = parse_block_until(p, T_END, T_END);
        expect(p, T_END, "end");
        return n;
    }
    expect(p, T_END, "end");
    return n;
}

static Node *parse_while(Parser *p) {
    advance(p);
    Node *n = node_new(N_WHILE);
    n->a = parse_expr(p);
    expect(p, T_DO, "do");
    n->b = parse_block_until(p, T_END, T_END);
    expect(p, T_END, "end");
    return n;
}

static Node *parse_stmt(Parser *p) {
    Token *t = cur(p);
    if (t->type == T_PRINT) {
        advance(p);
        Node *n = node_new(N_PRINT);
        n->a = parse_expr(p);
        return n;
    }
    if (t->type == T_LET) {
        advance(p);
        Token *name = expect(p, T_IDENT, "nama variabel");
        expect(p, T_EQ, "=");
        Node *n = node_new(N_LET);
        n->str = strdup(name->text);
        n->a = parse_expr(p);
        return n;
    }
    if (t->type == T_IF) return parse_if(p);
    if (t->type == T_WHILE) return parse_while(p);
    if (t->type == T_IMPORT) {
        advance(p);
        Token *name = expect(p, T_IDENT, "nama modul");
        Node *n = node_new(N_IMPORT);
        n->str = strdup(name->text);
        return n;
    }
    if (t->type == T_STORE) {
        advance(p);
        Node *n = node_new(N_STORE);
        n->a = parse_expr(p);
        expect(p, T_EQ, "=");
        n->b = parse_expr(p);
        return n;
    }
    if (t->type == T_NODE) {
        advance(p);
        Node *n = node_new(N_NODE);
        n->a = parse_expr(p);
        return n;
    }
    return parse_expr(p);
}

static Node *parse_program(Parser *p) {
    Node *block = node_new(N_BLOCK);
    while (!check(p, T_EOF)) {
        node_list_push(block, parse_stmt(p));
    }
    return block;
}

typedef enum { V_NUMBER, V_STRING, V_BOOL, V_NIL } ValueType;

typedef struct {
    ValueType type;
    double num;
    char *str;
} Value;

typedef struct {
    char *name;
    Value value;
} Binding;

typedef struct Env {
    Binding *bindings;
    size_t count, cap;
    struct Env *parent;
} Env;

static Env *env_new(Env *parent) {
    Env *e = calloc(1, sizeof(Env));
    e->parent = parent;
    return e;
}

static Value *env_find(Env *e, const char *name) {
    for (Env *c = e; c; c = c->parent) {
        for (size_t i = 0; i < c->count; i++) {
            if (strcmp(c->bindings[i].name, name) == 0) return &c->bindings[i].value;
        }
    }
    return NULL;
}

static void env_set(Env *e, const char *name, Value v) {
    Value *existing = env_find(e, name);
    if (existing) { *existing = v; return; }
    if (e->count == e->cap) {
        e->cap = e->cap ? e->cap * 2 : 8;
        e->bindings = realloc(e->bindings, e->cap * sizeof(Binding));
    }
    e->bindings[e->count].name = strdup(name);
    e->bindings[e->count].value = v;
    e->count++;
}

static Value make_number(double n) { Value v; v.type = V_NUMBER; v.num = n; v.str = NULL; return v; }
static Value make_string(const char *s) { Value v; v.type = V_STRING; v.str = strdup(s); v.num = 0; return v; }
static Value make_bool(int b) { Value v; v.type = V_BOOL; v.num = b; v.str = NULL; return v; }
static Value make_nil(void) { Value v; v.type = V_NIL; v.num = 0; v.str = NULL; return v; }

typedef struct {
    char *key;
    char *val;
} DbEntry;

static DbEntry *db_entries = NULL;
static size_t db_count = 0;
static size_t db_cap = 0;
static const char *DB_FILE = "zeta.db";

static void db_save(void) {
    FILE *f = fopen(DB_FILE, "wb");
    if (!f) return;
    for (size_t i = 0; i < db_count; i++) {
        fprintf(f, "%s=%s\n", db_entries[i].key, db_entries[i].val);
    }
    fclose(f);
}

static void db_load(void) {
    FILE *f = fopen(DB_FILE, "rb");
    if (!f) return;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        if (db_count == db_cap) {
            db_cap = db_cap ? db_cap * 2 : 16;
            db_entries = realloc(db_entries, db_cap * sizeof(DbEntry));
        }
        db_entries[db_count].key = strdup(line);
        db_entries[db_count].val = strdup(eq + 1);
        db_count++;
    }
    fclose(f);
}

static void db_set(const char *key, const char *val) {
    for (size_t i = 0; i < db_count; i++) {
        if (strcmp(db_entries[i].key, key) == 0) {
            free(db_entries[i].val);
            db_entries[i].val = strdup(val);
            db_save();
            return;
        }
    }
    if (db_count == db_cap) {
        db_cap = db_cap ? db_cap * 2 : 16;
        db_entries = realloc(db_entries, db_cap * sizeof(DbEntry));
    }
    db_entries[db_count].key = strdup(key);
    db_entries[db_count].val = strdup(val);
    db_count++;
    db_save();
}

static const char *db_get(const char *key) {
    for (size_t i = 0; i < db_count; i++) {
        if (strcmp(db_entries[i].key, key) == 0) return db_entries[i].val;
    }
    return NULL;
}

static int truthy(Value v) {
    if (v.type == V_BOOL) return v.num != 0;
    if (v.type == V_NUMBER) return v.num != 0;
    if (v.type == V_STRING) return v.str[0] != 0;
    return 0;
}

static void print_value(Value v) {
    if (v.type == V_NUMBER) {
        if (v.num == (long long)v.num) printf("%lld\n", (long long)v.num);
        else printf("%g\n", v.num);
    } else if (v.type == V_STRING) {
        printf("%s\n", v.str);
    } else if (v.type == V_BOOL) {
        printf("%s\n", v.num ? "true" : "false");
    } else {
        printf("nil\n");
    }
}

static char *str_concat(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = 0;
    return out;
}

static char *num_to_str(double n) {
    char buf[64];
    if (n == (long long)n) snprintf(buf, sizeof(buf), "%lld", (long long)n);
    else snprintf(buf, sizeof(buf), "%g", n);
    return strdup(buf);
}

static char *value_to_cstr(Value v) {
    if (v.type == V_STRING) return strdup(v.str);
    if (v.type == V_NUMBER) return num_to_str(v.num);
    if (v.type == V_BOOL) return strdup(v.num ? "true" : "false");
    return strdup("nil");
}

static Value eval(Node *n, Env *env);

static Value eval_binop(Node *n, Env *env) {
    Value l = eval(n->a, env);
    Value r = eval(n->b, env);
    switch (n->op) {
        case T_PLUS:
            if (l.type == V_STRING || r.type == V_STRING) {
                char *ls = l.type == V_STRING ? l.str : num_to_str(l.num);
                char *rs = r.type == V_STRING ? r.str : num_to_str(r.num);
                char *out = str_concat(ls, rs);
                Value v = make_string(out);
                free(out);
                if (l.type != V_STRING) free(ls);
                if (r.type != V_STRING) free(rs);
                return v;
            }
            return make_number(l.num + r.num);
        case T_MINUS: return make_number(l.num - r.num);
        case T_STAR: return make_number(l.num * r.num);
        case T_SLASH:
            if (r.num == 0) { fprintf(stderr, "zeta: gak bisa bagi dengan nol\n"); exit(1); }
            return make_number(l.num / r.num);
        case T_PERCENT: return make_number((double)((long long)l.num % (long long)r.num));
        case T_LT: return make_bool(l.num < r.num);
        case T_GT: return make_bool(l.num > r.num);
        case T_LTEQ: return make_bool(l.num <= r.num);
        case T_GTEQ: return make_bool(l.num >= r.num);
        case T_EQEQ:
            if (l.type == V_STRING && r.type == V_STRING) return make_bool(strcmp(l.str, r.str) == 0);
            return make_bool(l.num == r.num);
        case T_NOTEQ:
            if (l.type == V_STRING && r.type == V_STRING) return make_bool(strcmp(l.str, r.str) != 0);
            return make_bool(l.num != r.num);
        case T_AND: return make_bool(truthy(l) && truthy(r));
        case T_OR: return make_bool(truthy(l) || truthy(r));
        default:
            fprintf(stderr, "zeta: operator tidak dikenal\n");
            exit(1);
    }
}

static Value eval(Node *n, Env *env) {
    switch (n->type) {
        case N_NUMBER: return make_number(n->num);
        case N_STRING: return make_string(n->str);
        case N_BOOL: return make_bool((int)n->num);
        case N_IDENT: {
            Value *v = env_find(env, n->str);
            if (!v) { fprintf(stderr, "zeta: variabel '%s' belum pernah dibuat\n", n->str); exit(1); }
            return *v;
        }
        case N_BINOP: return eval_binop(n, env);
        case N_UNARY: {
            Value a = eval(n->a, env);
            if (n->op == T_MINUS) return make_number(-a.num);
            if (n->op == T_NOT) return make_bool(!truthy(a));
            return a;
        }
        case N_LET: {
            Value v = eval(n->a, env);
            env_set(env, n->str, v);
            return v;
        }
        case N_PRINT: {
            Value v = eval(n->a, env);
            print_value(v);
            return v;
        }
        case N_IF: {
            if (truthy(eval(n->a, env))) eval(n->b, env);
            else if (n->c) eval(n->c, env);
            return make_number(0);
        }
        case N_WHILE: {
            while (truthy(eval(n->a, env))) eval(n->b, env);
            return make_number(0);
        }
        case N_BLOCK: {
            Value last = make_number(0);
            for (size_t i = 0; i < n->list_len; i++) last = eval(n->list[i], env);
            return last;
        }
        case N_IMPORT: {
            if (strcmp(n->str, "db") == 0) db_load();
            return make_number(0);
        }
        case N_STORE: {
            Value k = eval(n->a, env);
            Value v = eval(n->b, env);
            char *keystr = value_to_cstr(k);
            char *valstr = value_to_cstr(v);
            db_set(keystr, valstr);
            free(keystr);
            free(valstr);
            return v;
        }
        case N_FETCH: {
            Value k = eval(n->a, env);
            char *keystr = value_to_cstr(k);
            const char *found = db_get(keystr);
            free(keystr);
            return found ? make_string(found) : make_nil();
        }
        case N_NODE: {
            Value code = eval(n->a, env);
            char *codestr = value_to_cstr(code);
            char path[64];
            snprintf(path, sizeof(path), "/tmp/zeta_node_%d.js", getpid());
            FILE *f = fopen(path, "wb");
            if (f) {
                fputs(codestr, f);
                fclose(f);
                char cmd[128];
                snprintf(cmd, sizeof(cmd), "node %s", path);
                fflush(stdout);
                int rc = system(cmd);
                (void)rc;
                remove(path);
            } else {
                fprintf(stderr, "zeta: gagal siapin file buat node\n");
            }
            free(codestr);
            return make_number(0);
        }
        default:
            fprintf(stderr, "zeta: node tidak dikenal\n");
            exit(1);
    }
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "zeta: gak bisa buka file '%s'\n", path); exit(1); }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    size_t read_bytes = fread(buf, 1, size, f);
    buf[read_bytes] = 0;
    fclose(f);
    return buf;
}

static void run_source(const char *src, Env *env) {
    Lexer lx = {0};
    lx.src = src;
    lx.len = strlen(src);
    lex(&lx);
    Parser p = {0};
    p.tokens = lx.tokens;
    Node *program = parse_program(&p);
    eval(program, env);
}

static void repl(void) {
    Env *env = env_new(NULL);
    char line[4096];
    printf("zeta repl. ctrl+d buat keluar.\n");
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        run_source(line, env);
    }
}

int main(int argc, char **argv) {
    init_keywords();
    const char *lang_code = NULL;
    const char *script_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--lang=", 7) == 0) lang_code = argv[i] + 7;
        else script_path = argv[i];
    }
    if (!script_path) {
        repl();
        return 0;
    }
    char *src_original = read_file(script_path);
    char *src = src_original;
    if (!lang_code && strncmp(src, "@lang ", 6) == 0) {
        char code_buf[32];
        code_buf[0] = 0;
        sscanf(src + 6, "%31s", code_buf);
        lang_code = strdup(code_buf);
        char *nl = strchr(src, '\n');
        src = nl ? nl + 1 : src;
    }
    if (lang_code && strcmp(lang_code, "en") != 0) {
        char path[256];
        snprintf(path, sizeof(path), "langs/%s.lang", lang_code);
        load_lang_file(path);
    }
    Env *env = env_new(NULL);
    run_source(src, env);
    free(src_original);
    return 0;
}
