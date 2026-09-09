/*
 * morsequantd.c — Timing‑encoded pulse daemon with speculative zero‑run prep
 * User-space daemon suitable for OpenWrt packaging.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

/* -----------------------------
   TIMING TABLE (SLOPE MAPPING)
   ----------------------------- */

#define MAX_BITS        8
#define TABLE_SIZE      (1 << MAX_BITS)

typedef struct {
    uint32_t pattern;
    uint8_t  len_bits;
    double   duration_us;
    double   slope_weight;
} timing_entry_t;

typedef struct {
    timing_entry_t entries[TABLE_SIZE];
} timing_table_t;

void build_timing_table(timing_table_t *tbl,
                        double base_us,
                        double slope_us)
{
    for (uint32_t p = 0; p < TABLE_SIZE; ++p) {
        double norm = (double)p / (double)(TABLE_SIZE - 1);

        tbl->entries[p].pattern      = p;
        tbl->entries[p].len_bits     = MAX_BITS;
        tbl->entries[p].duration_us  = base_us + slope_us * norm;
        tbl->entries[p].slope_weight = norm;
    }
}

static inline const timing_entry_t *
lookup(const timing_table_t *tbl, uint32_t pattern)
{
    return &tbl->entries[pattern];
}

/* -----------------------------
   BIT → PATTERN → PULSE ENCODER
   ----------------------------- */

uint32_t bits_to_pattern(const uint8_t *bits)
{
    uint32_t p = 0;
    for (int i = 0; i < MAX_BITS; i++) {
        p <<= 1;
        p |= bits[i] & 1;
    }
    return p;
}

size_t encode_bits_to_pulses(const timing_table_t *tbl,
                             const uint8_t *bits,
                             size_t bit_count,
                             double *out,
                             size_t max_out)
{
    size_t out_idx = 0;

    for (size_t i = 0; i + MAX_BITS <= bit_count && out_idx < max_out; i += MAX_BITS) {
        uint32_t pattern = bits_to_pattern(&bits[i]);
        const timing_entry_t *e = lookup(tbl, pattern);
        out[out_idx++] = e->duration_us;
    }

    return out_idx;
}

/* -----------------------------
   LATENCY MODEL + SPECULATION
   ----------------------------- */

typedef struct {
    double avg_latency_us;
    double avg_zero_run;
    double alpha;
} latency_model_t;

void update_latency(latency_model_t *m,
                    double observed_latency_us,
                    double observed_zero_run)
{
    m->avg_latency_us = m->alpha * observed_latency_us +
                        (1.0 - m->alpha) * m->avg_latency_us;

    m->avg_zero_run   = m->alpha * observed_zero_run +
                        (1.0 - m->alpha) * m->avg_zero_run;
}

size_t predict_zero_run(const latency_model_t *m)
{
    if (m->avg_zero_run < 1.0) return 0;
    return (size_t)round(m->avg_zero_run);
}

size_t prepare_speculative(const timing_table_t *tbl,
                           const latency_model_t *m,
                           double *out,
                           size_t max_out)
{
    size_t zr = predict_zero_run(m);
    if (!zr) return 0;

    uint8_t *zeros = calloc(zr, sizeof(uint8_t));
    if (!zeros) return 0;

    size_t pulses = encode_bits_to_pulses(tbl, zeros, zr, out, max_out);
    free(zeros);
    return pulses;
}

/* -----------------------------
   HARDWARE HOOKS (STUBS)
   ----------------------------- */

void hw_send_pulse(double duration_us)
{
    /* Replace with RF/GPIO driver calls */
    usleep((useconds_t)duration_us);
}

int hw_recv_bit(void)
{
    /* Replace with RF/GPIO sampling logic */
    return rand() & 1;
}

/* -----------------------------
   DAEMON MAIN LOOP
   ----------------------------- */

int main(void)
{
    timing_table_t table;
    latency_model_t model = {
        .avg_latency_us = 500.0,
        .avg_zero_run   = 4.0,
        .alpha          = 0.2
    };

    build_timing_table(&table, 50.0, 200.0);

    printf("morsequantd: starting daemon\n");

    while (1) {
        /* 1. Receive bits (stub) */
        uint8_t buf[256];
        size_t bit_count = 0;

        for (int i = 0; i < 256; i++) {
            buf[i] = hw_recv_bit();
            bit_count++;
        }

        /* 2. Update latency model (stub values) */
        update_latency(&model,
                       400.0 + (rand() % 50),
                       (rand() % 8));

        /* 3. Encode real bits */
        double pulses[512];
        size_t pcount = encode_bits_to_pulses(&table,
                                              buf,
                                              bit_count,
                                              pulses,
                                              512);

        /* 4. Prepare speculative pulses */
        double spec[128];
        size_t spec_count = prepare_speculative(&table,
                                                &model,
                                                spec,
                                                128);

        /* 5. Transmit speculative pulses first */
        for (size_t i = 0; i < spec_count; i++)
            hw_send_pulse(spec[i]);

        /* 6. Transmit real pulses */
        for (size_t i = 0; i < pcount; i++)
            hw_send_pulse(pulses[i]);

        /* 7. Daemon pacing */
        usleep(10000);
    }

    return 0;
}
