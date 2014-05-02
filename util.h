/* See LICENSE file for copyright and license details. */

#pragma once

#define UNUSED __attribute__((unused))

typedef struct {
    char *data;
    size_t len;
} buffer_t;

struct item {
    void *(*init)(void);
    void (*update)(buffer_t *buf, void *state);

    buffer_t *buffer;
    void *state;
};

char *smprintf(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void buffer_clear(buffer_t *buf);
void *buffer_new(void);
size_t buffer_printf(buffer_t *buf, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
