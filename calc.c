//Giorgi Janikashvili Student ID: 251ADB123
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: <every AI tool, website and book you used; details in comments where you used them>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STUDENT_FIRST "giorgi"
#define STUDENT_LAST "janikashvili"
#define STUDENT_ID "REPLACE_WITH_STUDENT_ID"

typedef struct {
    double real;
    long long integer;
    int is_float;
} Value;

typedef struct {
    char *src;
    size_t pos;
    size_t end;
    size_t error;
    int failed;
} Parser;

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Buffer;

static int is_space_char(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static void fail_at(Parser *p, size_t pos)
{
    if (!p->failed) {
        p->failed = 1;
        p->error = pos;
    }
}

static void skip_spaces(Parser *p)
{
    while (p->pos < p->end && is_space_char(p->src[p->pos])) {
        p->pos++;
    }
}

static Value make_integer(long long n)
{
    Value v;
    v.integer = n;
    v.real = (double)n;
    v.is_float = 0;
    return v;
}

static Value make_float(double n)
{
    Value v;
    v.real = n;
    v.integer = 0;
    v.is_float = 1;
    return v;
}

static Value parse_expr(Parser *p);

static Value parse_number(Parser *p)
{
    skip_spaces(p);
    size_t start = p->pos;
    Value bad = make_integer(0);

    if (p->pos >= p->end || !isdigit((unsigned char)p->src[p->pos])) {
        fail_at(p, p->pos + 1);
        return bad;
    }

    while (p->pos < p->end && isdigit((unsigned char)p->src[p->pos])) {
        p->pos++;
    }

    int floating = 0;
    if (p->pos < p->end && p->src[p->pos] == '.') {
        floating = 1;
        size_t dot = p->pos;
        p->pos++;
        if (p->pos >= p->end || !isdigit((unsigned char)p->src[p->pos])) {
            fail_at(p, dot + 1);
            return bad;
        }
        while (p->pos < p->end && isdigit((unsigned char)p->src[p->pos])) {
            p->pos++;
        }
    }

    if (!floating) {
        long long n = 0;
        for (size_t i = start; i < p->pos; i++) {
            n = n * 10 + (p->src[i] - '0');
        }
        return make_integer(n);
    }

    char saved = p->src[p->pos];
    p->src[p->pos] = '\0';
    errno = 0;
    char *after = NULL;
    double n = strtod(p->src + start, &after);
    p->src[p->pos] = saved;
    if (after != p->src + p->pos || errno == ERANGE || !isfinite(n)) {
        fail_at(p, start + 1);
        return bad;
    }
    return make_float(n);
}

static Value parse_primary(Parser *p)
{
    skip_spaces(p);
    Value bad = make_integer(0);

    if (p->pos >= p->end) {
        fail_at(p, p->pos + 1);
        return bad;
    }

    if (p->src[p->pos] == '(') {
        p->pos++;
        Value v = parse_expr(p);
        if (p->failed) {
            return bad;
        }
        skip_spaces(p);
        if (p->pos >= p->end || p->src[p->pos] != ')') {
            fail_at(p, p->pos + 1);
            return bad;
        }
        p->pos++;
        return v;
    }

    if (!isdigit((unsigned char)p->src[p->pos])) {
        fail_at(p, p->pos + 1);
        return bad;
    }
    return parse_number(p);
}

static Value parse_power(Parser *p)
{
    Value left = parse_primary(p);
    if (p->failed) {
        return left;
    }

    skip_spaces(p);
    if (p->pos + 1 >= p->end || p->src[p->pos] != '*' || p->src[p->pos + 1] != '*') {
        return left;
    }

    size_t op_pos = p->pos + 1;
    p->pos += 2;
    skip_spaces(p);
    size_t exponent_start = p->pos + 1;
    Value right = parse_power(p);
    if (p->failed) {
        return left;
    }

    if (!left.is_float && !right.is_float) {
        if (right.integer < 0) {
            fail_at(p, exponent_start);
            return make_integer(0);
        }
        long long base = left.integer;
        unsigned long long exp = (unsigned long long)right.integer;
        long long result = 1;
        while (exp > 0) {
            if (exp & 1ULL) {
                result *= base;
            }
            exp >>= 1;
            if (exp > 0) {
                base *= base;
            }
        }
        return make_integer(result);
    }

    double result = pow(left.real, right.real);
    if (!isfinite(result)) {
        fail_at(p, op_pos);
        return make_float(0.0);
    }
    return make_float(result);
}

static Value parse_term(Parser *p)
{
    Value value = parse_power(p);
    while (!p->failed) {
        skip_spaces(p);
        if (p->pos >= p->end || (p->src[p->pos] != '*' && p->src[p->pos] != '/')) {
            break;
        }
        if (p->src[p->pos] == '*' && p->pos + 1 < p->end && p->src[p->pos + 1] == '*') {
            break;
        }

        char op = p->src[p->pos];
        size_t op_pos = p->pos + 1;
        p->pos++;
        skip_spaces(p);
        size_t right_start = p->pos + 1;
        Value right = parse_power(p);
        if (p->failed) {
            break;
        }

        if (op == '/' && ((right.is_float && right.real == 0.0) || (!right.is_float && right.integer == 0))) {
            fail_at(p, right_start);
            break;
        }

        if (!value.is_float && !right.is_float) {
            if (op == '*') {
                value = make_integer(value.integer * right.integer);
            } else {
                value = make_integer(value.integer / right.integer);
            } 
        } else {
            double a = value.is_float ? value.real : (double)value.integer;
            double b = right.is_float ? right.real : (double)right.integer;
            double result = op == '*' ? a * b : a / b;
            if (!isfinite(result)) {
                fail_at(p, op_pos);
                break;
            }
            value = make_float(result);
        }
    }
    return value;
}

static Value parse_expr(Parser *p)
{
    Value value = parse_term(p);
    while (!p->failed) {
        skip_spaces(p);
        if (p->pos >= p->end || (p->src[p->pos] != '+' && p->src[p->pos] != '-')) {
            break;
        }
        char op = p->src[p->pos];
        size_t op_pos = p->pos + 1;
        p->pos++;
        skip_spaces(p);
        Value right = parse_term(p);
        if (p->failed) {
            break;
        }

        if (!value.is_float && !right.is_float) {
            value = make_integer(op == '+' ? value.integer + right.integer : value.integer - right.integer);
        } else {
            double a = value.is_float ? value.real : (double)value.integer;
            double b = right.is_float ? right.real : (double)right.integer;
            double result = op == '+' ? a + b : a - b;
            if (!isfinite(result)) {
                fail_at(p, op_pos);
                break;
            }
            value = make_float(result);
        }
    }
    return value;
}

static int buffer_reserve(Buffer *b, size_t extra)
{
    if (extra > (size_t)-1 - b->len - 1) {
        return 0;
    }
    size_t need = b->len + extra + 1;
    if (need <= b->cap) {
        return 1;
    }
    size_t cap = b->cap ? b->cap : 256;
    while (cap < need) {
        if (cap > (size_t)-1 / 2) {
            cap = need;
            break;
        }
        cap *= 2;
    }
    char *next = realloc(b->data, cap);
    if (!next) {
        return 0;
    }
    b->data = next;
    b->cap = cap;
    return 1;
}

static int buffer_append(Buffer *b, const char *s, size_t n)
{
    if (!buffer_reserve(b, n)) {
        return 0;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 1;
}

static char *read_all(FILE *f, size_t *length)
{
    size_t cap = 8192;
    size_t len = 0;
    char *data = malloc(cap + 1);
    if (!data) {
        return NULL;
    }
    for (;;) {
        if (len == cap) {
            if (cap > (size_t)-1 / 2 - 1) {
                free(data);
                return NULL;
            }
            size_t next_cap = cap * 2;
            char *next = realloc(data, next_cap + 1);
            if (!next) {
                free(data);
                return NULL;
            }
            data = next;
            cap = next_cap;
        }
        size_t got = fread(data + len, 1, cap - len, f);
        len += got;
        if (got == 0) {
            if (ferror(f)) {
                free(data);
                return NULL;
            }
            break;
        }
    }
    data[len] = '\0';
    *length = len;
    return data;
}

static int evaluate_file(const char *path, Buffer *output)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "calc: cannot open input file '%s'\n", path);
        return 0;
    }
    size_t len = 0;
    char *src = read_all(f, &len);
    int close_error = fclose(f);
    if (!src || close_error != 0) {
        free(src);
        fprintf(stderr, "calc: cannot read input file '%s'\n", path);
        return 0;
    }

    Buffer results = {0};
    size_t line_start = 0;
    size_t first_error = 0;
    while (line_start < len) {
        size_t line_end = line_start;
        while (line_end < len && src[line_end] != '\n') {
            line_end++;
        }
        size_t first = line_start;
        while (first < line_end && is_space_char(src[first])) {
            first++;
        }

        if (first < line_end && src[first] != '#') {
            Parser p;
            p.src = src;
            p.pos = line_start;
            p.end = line_end;
            p.error = 0;
            p.failed = 0;
            Value value = parse_expr(&p);
            if (!p.failed) {
                skip_spaces(&p);
                if (p.pos < p.end) {
                    fail_at(&p, p.pos + 1);
                }
            }
            if (p.failed) {
                first_error = p.error;
                break;
            }
            char line[128];
            int n;
            if (value.is_float) {
                n = snprintf(line, sizeof line, "%.15g\n", value.real);
            } else {
                n = snprintf(line, sizeof line, "%lld\n", value.integer);
            }
            if (n < 0 || (size_t)n >= sizeof line || !buffer_append(&results, line, (size_t)n)) {
                free(results.data);
                free(src);
                fprintf(stderr, "calc: out of memory while storing results\n");
                return 0;
            }
        }

        if (line_end == len) {
            line_start = len;
        } else {
            line_start = line_end + 1;
        }
    }

    if (first_error) {
        char error_line[64];
        int n = snprintf(error_line, sizeof error_line, "ERROR:%zu\n", first_error);
        results.len = 0;
        if (!buffer_append(&results, error_line, (size_t)n)) {
            free(results.data);
            free(src);
            fprintf(stderr, "calc: out of memory while storing error\n");
            return 0;
        }
    }

    if (!results.data) {
        results.data = malloc(1);
        if (!results.data) {
            free(src);
            fprintf(stderr, "calc: out of memory\n");
            return 0;
        }
        results.data[0] = '\0';
    }
    int ok = buffer_append(output, results.data, results.len);
    free(results.data);
    free(src);
    if (!ok) {
        fprintf(stderr, "calc: out of memory while collecting output\n");
        return 0;
    }
    return 1;
}

static char *join_path(const char *a, const char *b)
{
    size_t na = strlen(a), nb = strlen(b);
    int slash = na > 0 && a[na - 1] != '/';
    if (na > (size_t)-1 - nb - (size_t)slash - 1) {
        return NULL;
    }
    char *p = malloc(na + (size_t)slash + nb + 1);
    if (!p) {
        return NULL;
    }
    memcpy(p, a, na);
    size_t at = na;
    if (slash) {
        p[at++] = '/';
    }
    memcpy(p + at, b, nb + 1);
    return p;
}

static const char *base_name(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static char *input_base(const char *path)
{
    const char *base = base_name(path);
    size_t n = strlen(base);
    if (n >= 4 && strcmp(base + n - 4, ".txt") == 0) {
        n -= 4;
    }
    char *name = malloc(n + 1);
    if (!name) {
        return NULL;
    }
    memcpy(name, base, n);
    name[n] = '\0';
    return name;
}

static int ensure_output_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 1;
        }
        fprintf(stderr, "calc: output path '%s' is not a directory\n", path);
        return 0;
    }
    if (mkdir(path, 0777) != 0) {
        fprintf(stderr, "calc: cannot create output directory '%s'\n", path);
        return 0;
    }
    return 1;
}

static char *output_filename(const char *input_path)
{
    char *base = input_base(input_path);
    if (!base) {
        return NULL;
    }
    size_t need = strlen(base) + strlen(STUDENT_FIRST) + strlen(STUDENT_LAST) + strlen(STUDENT_ID) + 8;
    char *name = malloc(need);
    if (name) {
        snprintf(name, need, "%s_%s_%s_%s.txt", base, STUDENT_FIRST, STUDENT_LAST, STUDENT_ID);
    }
    free(base);
    return name;
}

static int process_one(const char *input, const char *outdir)
{
    Buffer output = {0};
    if (!evaluate_file(input, &output)) {
        free(output.data);
        return 0;
    }
    char *filename = output_filename(input);
    char *path = filename ? join_path(outdir, filename) : NULL;
    free(filename);
    if (!path) {
        free(output.data);
        fprintf(stderr, "calc: out of memory while making output path\n");
        return 0;
    }
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "calc: cannot write output file '%s'\n", path);
        free(path);
        free(output.data);
        return 0;
    }
    int ok = output.len == 0 || fwrite(output.data, 1, output.len, f) == output.len;
    if (fclose(f) != 0) {
        ok = 0;
    }
    if (!ok) {
        fprintf(stderr, "calc: error writing output file '%s'\n", path);
    }
    free(path);
    free(output.data);
    return ok;
}

static int has_txt_suffix(const char *name)
{
    size_t n = strlen(name);
    return n >= 4 && strcmp(name + n - 4, ".txt") == 0;
}

static int process_directory(const char *indir, const char *outdir)
{
    DIR *dir = opendir(indir);
    if (!dir) {
        fprintf(stderr, "calc: cannot open input directory '%s'\n", indir);
        return 0;
    }
    struct dirent *entry;
    int ok = 1;
    while ((entry = readdir(dir)) != NULL) {
        if (!has_txt_suffix(entry->d_name)) {
            continue;
        }
        char *input = join_path(indir, entry->d_name);
        if (!input) {
            fprintf(stderr, "calc: out of memory while making input path\n");
            ok = 0;
            break;
        }
        struct stat st;
        if (stat(input, &st) == 0 && S_ISREG(st.st_mode)) {
            if (!process_one(input, outdir)) {
                ok = 0;
                free(input);
                break;
            }
        }
        free(input);
    }
    if (closedir(dir) != 0) {
        fprintf(stderr, "calc: error closing input directory '%s'\n", indir);
        ok = 0;
    }
    return ok;
}

int main(int argc, char **argv)
{
    const char *indir = NULL;
    const char *input = NULL;
    const char *outarg = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 >= argc || indir || input) {
                fprintf(stderr, "usage: calc [-d DIR | --dir DIR] [-o OUTDIR | --output-dir OUTDIR] [input.txt]\n");
                return 1;
            }
            indir = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 >= argc || outarg) {
                fprintf(stderr, "usage: calc [-d DIR | --dir DIR] [-o OUTDIR | --output-dir OUTDIR] [input.txt]\n");
                return 1;
            }
            outarg = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "calc: unknown option '%s'\n", argv[i]);
            return 1;
        } else {
            if (input || indir) {
                fprintf(stderr, "calc: provide either one input file or one input directory\n");
                return 1;
            }
            input = argv[i];
        }
    }

    if ((!indir && !input) || (indir && input)) {
        fprintf(stderr, "usage: calc [-d DIR | --dir DIR] [-o OUTDIR | --output-dir OUTDIR] [input.txt]\n");
        return 1;
    }

    char *default_out = NULL;
    const char *outdir = outarg;
    if (!outdir) {
        const char *source = indir ? indir : input;
        char *base = input_base(source);
        if (!base) {
            fprintf(stderr, "calc: out of memory\n");
            return 1;
        }
        size_t need = strlen(base) + strlen(STUDENT_FIRST) + strlen(STUDENT_ID) + 3;
        default_out = malloc(need);
        if (default_out) {
            snprintf(default_out, need, "%s_%s_%s", base, STUDENT_FIRST, STUDENT_ID);
        }
        free(base);
        if (!default_out) {
            fprintf(stderr, "calc: out of memory\n");
            return 1;
        }
        outdir = default_out;
    }

    if (!ensure_output_dir(outdir)) {
        free(default_out);
        return 1;
    }

    int ok = indir ? process_directory(indir, outdir) : process_one(input, outdir);
    free(default_out);
    return ok ? 0 : 1;
}
