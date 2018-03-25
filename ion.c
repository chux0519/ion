#include <stdio.h>
#include <stddef.h> // to use macro `offsetof`
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <memory.h>
#include <stdbool.h>
#include <stdarg.h>

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

void *xrealloc(void *ptr, size_t num_bytes) {
    void *new_ptr = realloc(ptr, num_bytes);
    if (!new_ptr) {
        perror("xrealloc failed");
        exit(1);
    }
    return new_ptr;
}

void *xmalloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        perror("xmalloc failed");
        exit(1);
    }
    return ptr;
}


void fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: \n");
    vprintf(fmt, args);
    va_end(args);
    exit(1);
}

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = (buf__grow(b, buf_len(b) + (n), sizeof(*(b))))))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_push(b, x) (buf__fit(b, 1), (b)[buf__hdr(b)->len++] = (x))
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert((SIZE_MAX - 1) / 2 > buf_cap(buf));
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    size_t new_size = new_cap * elem_size + offsetof(BufHdr, buf);
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

void buf_test() {
    int *test_buf = NULL;
    enum {
        N = 1024
    };
    for (int i = 0; i < N; ++i) {
        buf_push(test_buf, i);
    }
    assert(buf_len(test_buf) == N);

    for (int j = 0; j < buf_len(test_buf); ++j) {
        assert(test_buf[j] == j);
    }
    buf_free(test_buf);
    assert(test_buf == NULL);
}

typedef struct InternStr {
    size_t len;
    const char *str;
} Intern;

static Intern *interns;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    for (Intern *it = interns; it != buf_end(interns); it++) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    char *str = malloc(len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    Intern new_intern_str = {len, str};
    buf_push(interns, new_intern_str);
    return new_intern_str.str;
}

const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}

void str_intern_test() {
    char x[] = "hello";
    char y[] = "hello";
    assert(x != y);
    assert(str_intern(x) == str_intern(y));
    char z[] = "hello!";
    assert(str_intern(x) != str_intern(z));
}

typedef enum {
    TOKEN_LAST_CHAR = 127,
    TOKEN_INT,
    TOKEN_NAME
    // ...
} TokenKind;

size_t copy_token_kind_str(char *dest, size_t dest_size, TokenKind kind) {
    size_t n = 0;
    switch (kind) {
        case 0:
            n = snprintf(dest, dest_size, "end of file");
            break;
        case TOKEN_INT:
            n = snprintf(dest, dest_size, "integer");
            break;
        case TOKEN_NAME:
            n = snprintf(dest, dest_size, "name");
            break;
        default:
            if (kind < 128 && isprint(kind)) {
                n = snprintf(dest, dest_size, "%c", kind);
            } else {
                n = snprintf(dest, dest_size, "ascii %d", kind);
            }

            break;
    }
    return n;
}

const char *token_kind_str(TokenKind kind) {
    static char buf[256];
    size_t n = copy_token_kind_str(buf, sizeof(buf), kind);
    assert(n + 1 >= sizeof(buf));
    return buf;
}

typedef struct Token {
    TokenKind kind;
    const char *start;
    const char *end;
    union {
        uint64_t val;
        const char *name;
    };
} Token;

Token token;
const char *stream;


const char *keyword_if;
const char *keyword_for;
const char *keyword_while;

void init_keywords() {
    keyword_if = str_intern("if");
    keyword_for = str_intern("for");
    keyword_while = str_intern("while");
}

void next_token() {
    token.start = stream;
    switch (*stream) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            uint64_t val = 0;
            while (isdigit(*stream)) {
                val *= 10;
                val += *stream++ - '0';
            }
            token.kind = TOKEN_INT;
            token.val = val;
            break;
        }
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '_': {
            while (isalnum(*stream) || *stream == '_') {
                stream++;
            }
            token.kind = TOKEN_NAME;
            token.name = str_intern_range(token.start, stream);
            break;
        }
        default:
            token.kind = (TokenKind) *stream++;
            break;
    }
    token.end = stream;
}

void init_stream(const char *str) {
    stream = str;
    next_token();
}

void print_token(Token token) {
    printf("TOKEN : %d", token.kind);
    switch (token.kind) {
        case TOKEN_INT:
            printf(" VALUE: %llu", token.val);
            break;
        case TOKEN_NAME:
            printf(" NAME: %.*s", (int) (token.end - token.start), token.start);
            break;
        default:
            printf(" TOKEN : %c", token.kind);
            break;
    }
    printf("\n");
}


void lex_test() {
    char *source = "+()_HELLO201_,1234_994,FOO,BAR,FOO";
    stream = source;
    next_token();
    while (token.kind) {
        print_token(token);
        next_token();
    }
}

// parse

bool is_token(TokenKind kind) {
    return token.kind == kind;
}

bool is_token_name(const char *name) {
    return token.kind == TOKEN_NAME && token.name == name;
}

bool match_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        return false;
    }
}

bool expect_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        fatal("expect token %s, got %s", token_kind_str(kind), token_kind_str(token.kind));
        return false;
    }
}

/*
 * expr3 = INT | '(' expr ')'
 * expr2 = [-] expr3
 * expr1 = expr2 ([/ *] expr2)
 * expr0 = expr1 ([+-] expr1)
 * expr = expr0
 */
int parse_expr();

int parse_expr3() {
    if (is_token(TOKEN_INT)) {
        int val = token.val;
        next_token();
        return val;
    } else if (match_token('(')) {
        int val = parse_expr();
        expect_token(')');
        return val;
    } else {
        fatal("expected integer or (, got %s", token_kind_str(token.kind));
        return 0;
    }
}

int parse_expr2() {
    if (match_token('-')) {
        return -parse_expr3();
    } else {
        return parse_expr3();
    }
}

int parse_expr1() {
    int val = parse_expr2();
    while (is_token('*') || is_token('/')) {
        char op = token.kind;
        next_token();
        int rval = parse_expr2();
        if (op == '*') {
            val *= rval;
        } else {
            assert(op == '/');
            assert(rval != 0);
            val /= rval;
        }
    }
    return val;
}

int parse_expr0() {
    int val = parse_expr1();
    while (is_token('+') || is_token('-')) {
        char op = token.kind;
        next_token();
        int rval = parse_expr1();
        if (op == '+') {
            val += rval;
        } else {
            assert(op == '-');
            val -= rval;
        }
    }
    return val;
}

int parse_expr() {
    return parse_expr0();
}

int parse_expr_str(const char *expr) {
    init_stream(expr);
    return parse_expr();
}

void parse_test() {
#define TEST_EXPR(x) assert(parse_expr_str(#x) == (x))
    TEST_EXPR(1);
    TEST_EXPR((1));
#undef TEST_EXPR
    assert(parse_expr_str("1+2-3") == 0);
    assert(parse_expr_str("2*3+4*5") == 26);
    assert(parse_expr_str("2*(3+4)*5") == 70);
}

int main() {
    buf_test();
    lex_test();
    str_intern_test();
    parse_test();
    return 0;
}