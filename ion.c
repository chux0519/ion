#include <stdio.h>
#include <stddef.h> // to use macro `offsetof`
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

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
#define buf_push(b, x) (buf__fit(b, 1), (b)[buf__hdr(b)->len++] = (x))
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
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

typedef enum TokenKind {
    TOKEN_INT = 128,
    TOKEN_NAME
    // ...
} TokenKind;


typedef struct Token {
    TokenKind kind;
    union {
        uint64_t val;
        struct {
            const char *start;
            const char *end;
        };
    };
} Token;

Token token;
const char *stream;

void next_token() {
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
            const char *start = stream++;
            while (isalnum(*stream) || *stream == '_') {
                stream++;
            }
            const char *end = stream;
            token.start = start;
            token.end = end;
            token.kind = TOKEN_NAME;
            break;
        }
        default:
            token.kind = (TokenKind) *stream++;
            break;
    }
}

void print_token(Token token) {
    printf("TOKEN : %d", token.kind);
    switch (token.kind) {
        case TOKEN_INT:
            printf(" VALUE: %llu", token.val);
            break;
        case TOKEN_NAME:
            printf(" NAME: %.*s", (int)(token.end - token.start), token.start);
            break;
        default:
            printf(" TOKEN : %c", token.kind);
            break;
    }
    printf("\n");
}

void lex_test() {
    char *source = "+()_HELLO201_,1234_994";
    stream = source;
    next_token();
    while (token.kind) {
        print_token(token);
        next_token();
    }
}

int main() {
    buf_test();
    lex_test();
    return 0;
}