#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_MAX_FIELDS 32
#define JSON_MAX_KEY_LEN 64

enum JsonType { JSON_NULL, JSON_NUMBER, JSON_STRING };

struct JsonField {
    char key[JSON_MAX_KEY_LEN];
    enum JsonType type;
    int intVal;
    char strVal[512];
};

struct JsonObject {
    struct JsonField fields[JSON_MAX_FIELDS];
    int fieldCount;
};

static const char *json_skip_ws(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char *json_parse_string(const char *p, char *out, int maxLen) {
    if (*p != '"') return NULL;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < maxLen - 1) {
        if (*p == '\\') { p++; if (*p) out[i++] = *p++; }
        else out[i++] = *p++;
    }
    out[i] = '\0';
    if (*p == '"') p++;
    return p;
}

static const char *json_parse_number(const char *p, int *out) {
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    *out = 0;
    while (*p >= '0' && *p <= '9') { *out = *out * 10 + (*p - '0'); p++; }
    if (neg) *out = -*out;
    return p;
}

static const char *json_parse_object(const char *p, struct JsonObject *obj) {
    obj->fieldCount = 0;
    p = json_skip_ws(p);
    if (*p != '{') return NULL;
    p++;
    while (1) {
        p = json_skip_ws(p);
        if (*p == '}') { p++; break; }
        if (*p == ',') p++;
        p = json_skip_ws(p);

        struct JsonField *f = &obj->fields[obj->fieldCount];
        p = json_parse_string(p, f->key, JSON_MAX_KEY_LEN);
        if (!p) return NULL;
        p = json_skip_ws(p);
        if (*p != ':') return NULL;
        p++;
        p = json_skip_ws(p);

        if (*p == '"') {
            f->type = JSON_STRING;
            p = json_parse_string(p, f->strVal, sizeof(f->strVal));
        } else if (*p == 'n') {
            f->type = JSON_NULL;
            p += 4;
        } else {
            f->type = JSON_NUMBER;
            p = json_parse_number(p, &f->intVal);
        }
        if (obj->fieldCount < JSON_MAX_FIELDS - 1) obj->fieldCount++;
    }
    return p;
}

static int json_get_int(const struct JsonObject *obj, const char *key, int defVal) {
    for (int i = 0; i < obj->fieldCount; i++)
        if (strcmp(obj->fields[i].key, key) == 0 && obj->fields[i].type == JSON_NUMBER)
            return obj->fields[i].intVal;
    return defVal;
}

static const char *json_get_str(const struct JsonObject *obj, const char *key) {
    for (int i = 0; i < obj->fieldCount; i++)
        if (strcmp(obj->fields[i].key, key) == 0 && obj->fields[i].type == JSON_STRING)
            return obj->fields[i].strVal;
    return NULL;
}

typedef void (*JsonArrayCallback)(int index, const struct JsonObject *obj, void *userData);

static int json_parse_array(const char *data, JsonArrayCallback cb, void *userData) {
    const char *p = json_skip_ws(data);
    if (*p != '[') return 0;
    p++;
    int index = 0;
    while (1) {
        p = json_skip_ws(p);
        if (*p == ']') break;
        if (*p == ',') p++;
        p = json_skip_ws(p);
        if (*p == '{') {
            struct JsonObject obj;
            p = json_parse_object(p, &obj);
            if (!p) break;
            cb(index, &obj, userData);
            index++;
        } else break;
    }
    return index;
}

static char *json_load_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

#endif
