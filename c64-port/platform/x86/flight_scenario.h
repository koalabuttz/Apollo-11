/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_SCENARIO_H
#define FLIGHT_SCENARIO_H
/* The flight scenario inputs, on AGC time (docs/X86-FLIGHT.md), shared by
 * the x86 program and its lockstep check, with tools/flight_host_run.c's
 * meanings:
 *   KEYS            DSKY keys (and the mark buttons M, J, x, y, j), one
 *                   every half second from 2 s; @CS (decimal)
 *                   moves the next key and the ones after it to CS or later
 *                   (V37E63E@9000R presses RSET at 90 s; P is RSET too,
 *                   as in the harness: PRO is no key code)
 *   FLIGHT_CH30-33  input channels at start (octal)
 *   FLIGHT_POKE     CS:ADDR=VALUE,... erasable words (octal)
 *   FLIGHT_LOAD     CS:FILE,...: the same from files of octal "ADDR VALUE"
 *                   lines ('#' starts a comment; tools/flight_scenarios/)
 *   FLIGHT_GOJAM    CS[,CS...]: a hardware restart
 *   FLIGHT_DOWNRUPT 1: a DOWNRUPT every 20 ms (fd_state.downrupt)
 *   FLIGHT_CHAN     CS:CH=VALUE,... input channel 16 or 30-33 set (octal),
 *                   in time order
 *   FLIGHT_ENGINE, FLIGHT_ENGINE_LAG, FLIGHT_TOUCHDOWN, FLIGHT_PILOT,
 *   FLIGHT_ASCENT, FLIGHT_STAGE, FLIGHT_RCS (LM); FLIGHT_SPS, FLIGHT_AERO,
 *   FLIGHT_IMU, FLIGHT_SATURN, FLIGHT_SIVB (CM): the vehicle models
 *                   (platform/x86/vehicle_models.h)
 *   FLIGHT_RR, FLIGHT_RR_DROP: the LM's rendezvous radar or the CM's VHF
 *                   ranging and sextant (platform/x86/radar_models.h)
 *   FLIGHT_LR, FLIGHT_LR_POS2, FLIGHT_LR_DROP: the LM's landing radar
 *                   (platform/x86/landing_radar.h)
 *   FLIGHT_BUSY_SAMPLE  CS: also a sample point at a routine boundary
 *                   outside interrupts when none has come for CS
 *                   centiseconds, so inputs reach a job that busy waits
 *                   for the crew (LM R22's V06N49 loop)
 *   X86_START       PATH: start from a native state the host differential
 *                   wrote (FLIGHT_SNAPSHOT's header) in place of GOJAM;
 *                   AGC time counts from that state's own (SNAP_CS). The
 *                   scenario tools take FLIGHT_CORE scenarios this way: the
 *                   x86 program reads no yaAGC core itself.
 * At the first sample point (an idle pass through ADVAN) at or after their
 * time, in this order: pokes, the vehicle models (the radar after the
 * SPS), channel changes, then a restart or a key. With FLIGHT_ENGINE_LAG
 * the engine also runs after every routine, as do the RCS jets and a radar
 * read that starts between sample points (fs_after_routine). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flight_dispatch.h"
#include "resident_cdu.h"
#include "vehicle_models.h"
#include "radar_models.h"
#include "landing_radar.h"
#if RK_LM
#define FS_ADVAN 03214u
#else
#define FS_ADVAN 03231u
#endif
#define FS_NEWJOB 067u

/* The sample point: Z at ADVAN in bank 01, no interrupt in progress or
 * pending, NEWJOB -0. (The timers count whole periods at boundaries, so
 * there is no scaler phase to wait for, as under tools/flight_host_run.c's
 * scaled time.) */
static int fs_at_idle_advan(const fd_state *st){const agc_word *w=st->s.words;
 return !st->pending&&!st->hw.in_interrupt&&(w[FS_NEWJOB]&077777u)==077777u
        &&(w[5]&07777u)==FS_ADVAN&&agc_fixed_physical(FS_ADVAN,w[4],st->io.superbank)==1024u+(FS_ADVAN-02000u);}

enum { FS_KEYS = 1024, FS_POKES = 4096, FS_GOJAMS = 16, FS_CHANS = 64 };
typedef struct {
    char keys[FS_KEYS + 1]; unsigned long key_at[FS_KEYS]; unsigned pressed;
    struct { unsigned long cs; uint16_t addr, value; } pokes[FS_POKES]; unsigned npokes, poked;
    unsigned long gojam_at[FS_GOJAMS]; unsigned ngojams, gojams;
    struct {
        unsigned long cs;
        uint16_t ch, value;
    } chans[FS_CHANS];
    unsigned nchans, changed;
    /* The vehicle models' state (their settings are vm_cfg). */
    vm_engine engine;
    vm_rcs rcs;
    vm_sps sps;
    vm_aero aero;
    vm_saturn saturn;
    vm_sivb sivb;
    rm_rr rr;
    lr_state lr;
    unsigned long busy_last; /* the last sample point (FLIGHT_BUSY_SAMPLE) */
} fs_inputs;
static rm_rr_config rm_rr_cfg;
static lr_config lr_cfg;
/* The landing radar's true trajectory takes the engine's and the jets'
 * velocity changes (vm_dv_hook has no context: one fs_inputs a program). */
static lr_state *fs_lr;
static void fs_lr_dv(const double *dv, unsigned long t, double dt)
{
    lr_dv(fs_lr, dv, t, dt);
}
static unsigned long fs_busy_every;

/* A sample point: an idle pass through ADVAN, or with FLIGHT_BUSY_SAMPLE a
 * routine boundary outside interrupts with nothing pending when none has
 * come for that long. Records the time. */
static int fs_sample_point(fs_inputs *in, const fd_state *st)
{
    unsigned long t = fd_centiseconds(st);
    if (!fs_at_idle_advan(st) && !(fs_busy_every && !st->pending && !st->hw.in_interrupt &&
                                   t - in->busy_last >= fs_busy_every))
        return 0;
    in->busy_last = t;
    return 1;
}

static int fs_poke(fs_inputs *in,unsigned long cs,unsigned a,unsigned v){
 if(a>=2048u||in->npokes>=FS_POKES)return 0;
 in->pokes[in->npokes].cs=cs;in->pokes[in->npokes].addr=(uint16_t)a;in->pokes[in->npokes++].value=(uint16_t)(v&077777u);return 1;}

static double fs_number(const char *name)
{
    const char *v = getenv(name);
    return v ? strtod(v, 0) : 0.0;
}
static unsigned long fs_cs(const char *name)
{
    const char *v = getenv(name);
    return v ? strtoul(v, 0, 10) : 0ul;
}
/* The vehicle models' settings (vm_cfg); 0 (with a message) on a bad one. */
static int fs_parse_models(void)
{
    const char *v;
    memset(&vm_cfg, 0, sizeof vm_cfg);
#if RK_LM
    vm_cfg.engine_kg = fs_number("FLIGHT_ENGINE");
    vm_cfg.engine_lag = fs_number("FLIGHT_ENGINE_LAG");
    vm_cfg.touchdown_cs = fs_cs("FLIGHT_TOUCHDOWN");
    vm_cfg.pilot_tau = fs_number("FLIGHT_PILOT");
    vm_cfg.ascent_kg = fs_number("FLIGHT_ASCENT");
    vm_cfg.stage_cs = fs_cs("FLIGHT_STAGE");
    vm_cfg.rcs_kg = fs_number("FLIGHT_RCS");
    if (vm_cfg.stage_cs && !vm_cfg.ascent_kg) {
        fprintf(stderr, "FLIGHT_STAGE needs FLIGHT_ASCENT\n");
        return 0;
    }
#else
    if ((v = getenv("FLIGHT_SPS")) &&
        sscanf(v, "%lf,%lf,%lf", &vm_cfg.sps_kg, &vm_cfg.sps_cg[0], &vm_cfg.sps_cg[1]) < 1) {
        fprintf(stderr, "FLIGHT_SPS=KG[,PITCH,YAW]\n");
        return 0;
    }
    vm_cfg.aero_b = fs_number("FLIGHT_AERO");
    vm_cfg.imu = getenv("FLIGHT_IMU") ? atoi(getenv("FLIGHT_IMU")) : 0;
    vm_cfg.saturn_lo = fs_cs("FLIGHT_SATURN");
    if ((v = getenv("FLIGHT_SIVB")) &&
        sscanf(v, "%lu,%lf,%lf,%lf", &vm_cfg.sivb_tig, &vm_cfg.sivb_u[0], &vm_cfg.sivb_u[1],
               &vm_cfg.sivb_u[2]) != 4) {
        fprintf(stderr, "FLIGHT_SIVB=CS,UX,UY,UZ\n");
        return 0;
    }
#endif
    memset(&rm_rr_cfg, 0, sizeof rm_rr_cfg);
    if (!rm_rr_parse(&rm_rr_cfg, getenv("FLIGHT_RR"))) {
        fprintf(stderr, "FLIGHT_RR=CS:rx,ry,rz,vx,vy,vz:rx,ry,rz,vx,vy,vz\n");
        return 0;
    }
    if (!rm_drops_parse(&rm_rr_cfg.drops, getenv("FLIGHT_RR_DROP"))) {
        fprintf(stderr, "FLIGHT_RR_DROP=T0-T1[,T0-T1...]\n");
        return 0;
    }
    memset(&lr_cfg, 0, sizeof lr_cfg);
    if (!lr_parse(&lr_cfg, getenv("FLIGHT_LR"), getenv("FLIGHT_LR_POS2"),
                  getenv("FLIGHT_LR_DROP"))) {
        fprintf(stderr, "FLIGHT_LR=CS[:DH], FLIGHT_LR_POS2=CS, FLIGHT_LR_DROP=T0-T1[,T0-T1...]\n");
        return 0;
    }
#if !RK_LM
    if (lr_cfg.on || lr_cfg.pos2) {
        fprintf(stderr, "FLIGHT_LR and FLIGHT_LR_POS2 are the LM's\n");
        return 0;
    }
#endif
    lr_cfg.trace = getenv("FLIGHT_LR_TRACE") != 0;
    if (getenv("FLIGHT_CORE") && !getenv("X86_START")) {
        fprintf(stderr,
                "FLIGHT_CORE needs X86_START: the host differential's FLIGHT_SNAPSHOT of it\n");
        return 0;
    }
    fs_busy_every = fs_cs("FLIGHT_BUSY_SAMPLE");
    (void)v;
    return 1;
}

/* The keys argument and the environment above; 0 (with a message) on a bad
 * value. */
static int fs_parse(fs_inputs *in,const char *keys){const char *t;unsigned long at=200ul;unsigned n=0,i,j;
 memset(in,0,sizeof *in);
 for(t=keys?keys:"";*t&&n<FS_KEYS;){
  if(*t=='@'){char *e;unsigned long v=strtoul(t+1,&e,10);if(e==t+1){fprintf(stderr,"bad key time in %s\n",keys);return 0;}if(v>at)at=v;t=e;continue;}
  in->keys[n]=*t++;in->key_at[n++]=at;at+=50ul;}
 /* As many keys as tools/flight_host_run.c takes (1024); never cut short. */
 for (; *t == '@';) {
     char *e;
     strtoul(t + 1, &e, 10);
     if (e == t + 1) break;
     t = e;
 }
 if (*t) {
     fprintf(stderr, "more than %d keys\n", FS_KEYS);
     return 0;
 }
 for(t=getenv("FLIGHT_POKE");t&&*t;){unsigned long cs;unsigned a,v;int k=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&a,&v,&k)<3||!k||!fs_poke(in,cs,a,v)){fprintf(stderr,"FLIGHT_POKE=CS:ADDR=VALUE,...\n");return 0;}
  t+=k;if(*t==',')++t;}
 for(t=getenv("FLIGHT_LOAD");t&&*t;){unsigned long cs;char path[512],line[256];int k=0;FILE *f;
  if(sscanf(t,"%lu:%511[^,]%n",&cs,path,&k)<2||!k){fprintf(stderr,"FLIGHT_LOAD=CS:FILE,...\n");return 0;}
  t+=k;if(*t==',')++t;
  if(!(f=fopen(path,"r"))){fprintf(stderr,"FLIGHT_LOAD: cannot open %s\n",path);return 0;}
  while(fgets(line,sizeof line,f)){unsigned a,v;char *c=strchr(line,'#');if(c)*c=0;
   if(sscanf(line,"%o %o",&a,&v)!=2)continue;
   if(!fs_poke(in,cs,a,v)){fclose(f);fprintf(stderr,"FLIGHT_LOAD: bad address or too many words in %s\n",path);return 0;}}
  fclose(f);}
 /* In time order, keeping each time's words in the order given. */
 for(i=1;i<in->npokes;++i)for(j=i;j>0&&in->pokes[j-1].cs>in->pokes[j].cs;--j){unsigned long c=in->pokes[j].cs;uint16_t a=in->pokes[j].addr,v=in->pokes[j].value;
  in->pokes[j]=in->pokes[j-1];in->pokes[j-1].cs=c;in->pokes[j-1].addr=a;in->pokes[j-1].value=v;}
 for (t = getenv("FLIGHT_CHAN"); t && *t;) {
     unsigned long cs;
     unsigned c, v;
     int k = 0;
     if (sscanf(t, "%lu:%o=%o%n", &cs, &c, &v, &k) < 3 || !k || in->nchans >= FS_CHANS ||
         (c != 016u && (c < 030u || c > 033u)) ||
         (in->nchans && cs < in->chans[in->nchans - 1].cs)) {
         fprintf(stderr, "FLIGHT_CHAN=CS:CH=VALUE,... (channel 16 or 30-33, in time order)\n");
         return 0;
     }
     in->chans[in->nchans].cs = cs;
     in->chans[in->nchans].ch = (uint16_t)c;
     in->chans[in->nchans++].value = (uint16_t)(v & 077777u);
     t += k;
     if (*t == ',') ++t;
 }
 if (!fs_parse_models()) return 0;
 fs_lr = &in->lr;
 in->engine.dv = in->rcs.dv = fs_lr_dv;
 for(t=getenv("FLIGHT_GOJAM");t&&*t&&in->ngojams<FS_GOJAMS;){char *e;in->gojam_at[in->ngojams++]=strtoul(t,&e,10);
  if(e==t){fprintf(stderr,"FLIGHT_GOJAM=CS[,CS...]\n");return 0;}t=*e==','?e+1:e;}
 return 1;}

static agc_word fs_keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}

/* The mark buttons, as channel 16 bits with a KEYRUPT2 request (MARKRUPT
 * takes it): M and J the CM's MARK and MARK REJECT, x, y and j the LM's X
 * MARK, Y MARK and MARK REJECT. 0 for a DSKY key. */
static agc_word fs_markcode(char c)
{
    return c == 'M'   ? 040u
           : c == 'J' ? 0100u
           : c == 'x' ? 04u
           : c == 'y' ? 010u
           : c == 'j' ? 020u
                      : 0u;
}

/* X86_START: the harness wrote its state after the start point's pokes,
 * channel changes, restart and key, so every input due at or before it
 * (cs) has happened already and is skipped here. */
static void fs_inputs_from(fs_inputs *in, unsigned long cs)
{
    while (in->poked < in->npokes && in->pokes[in->poked].cs <= cs) ++in->poked;
    while (in->changed < in->nchans && in->chans[in->changed].cs <= cs) ++in->changed;
    while (in->gojams < in->ngojams && in->gojam_at[in->gojams] <= cs) ++in->gojams;
    while (in->keys[in->pressed] && in->key_at[in->pressed] <= cs) ++in->pressed;
}

enum { FS_NONE, FS_KEY, FS_GOJAM };
/* At a sample point: the pokes due, then a restart if one is due
 * (FS_GOJAM), else a key if one is due (channel 15 and a KEYRUPT1 request;
 * FS_KEY). */
static int fs_inputs_due(fs_inputs *in, fd_state *st)
{
    unsigned long t = fd_centiseconds(st);
    agc_word *w = st->s.words;
    while (in->poked < in->npokes && t >= in->pokes[in->poked].cs) {
        st->s.words[in->pokes[in->poked].addr] = (agc_word)in->pokes[in->poked].value;
        ++in->poked;
    }
    vm_engine_step(t, &in->engine, w);
    vm_sps_step(t, &in->sps, w, st->hw.channel12, st->optics);
    rm_rr_step(&rm_rr_cfg, t, &in->rr, w, st->hw.channel12, st->hw.channel13, st->hw.channel14,
               &st->hw.channel33);
    vm_aero_step(t, &in->aero, w);
    vm_imu_step(w, st->hw.channel12);
    vm_saturn_step(t, &in->saturn, w, &st->hw.channel30);
    vm_sivb_step(t, &in->sivb, w);
    lr_pos2_step(&lr_cfg, t, &in->lr, st->hw.channel12, &st->hw.channel33);
    lr_step(&lr_cfg, &in->lr, t, vm_cfg.touchdown_cs, w, st->hw.channel13, &st->hw.channel33);
    while (in->changed < in->nchans && t >= in->chans[in->changed].cs) {
        unsigned c = in->chans[in->changed].ch;
        agc_word v = (agc_word)in->chans[in->changed].value;
        ++in->changed;
        if (c == 016u)
            st->io.channel16 = v;
        else if (c == 033u) { /* the radars keep their bits */
            agc_word own = (agc_word)(rm_rr_ch33_bits(&rm_rr_cfg) | lr_ch33_bits(&lr_cfg));
            st->hw.channel33 = (agc_word)((v & ~own) | (st->hw.channel33 & own));
        } else
            *(c == 030u   ? &st->hw.channel30
              : c == 031u ? &st->hw.channel31
                          : &st->hw.channel32) = v;
    }
    if (in->gojams < in->ngojams && t >= in->gojam_at[in->gojams]) {
        while (in->gojams < in->ngojams && t >= in->gojam_at[in->gojams]) ++in->gojams;
        fd_gojam(st); /* as yaAGC's GOJAM and tools/flight_gojam.h */
        return FS_GOJAM;
    }
    if (in->keys[in->pressed] && t >= in->key_at[in->pressed]) {
        char c = in->keys[in->pressed++];
        agc_word mark = fs_markcode(c);
        if (mark) {
            st->io.channel16 = mark;
            st->pending |= RK_RUPT_BIT(RK_RUPT_KEY2);
        } else {
            fd_key(st, fs_keycode(c));
        }
        return FS_KEY;
    }
    return FS_NONE;
}

/* After each routine: the engine with a throttle lag (P66 reads the PIPAs
 * between sample points), the RCS jets, and a radar read (the landing
 * radar's, then the rendezvous radar's): one starts at
 * any routine boundary and the resident timer ends it nine TIME5 counts
 * on, often before the next sample point, so the radar answers it where
 * channel 13 bit 4 comes on and is ready again once it drops. */
static void fs_after_routine(fs_inputs *in, fd_state *st)
{
    unsigned long t = fd_centiseconds(st);
    if (vm_cfg.engine_lag) vm_engine_step(t, &in->engine, st->s.words);
    vm_rcs_step(t, &in->rcs, st->s.words, st->hw.channel5, st->hw.channel6);
    if (lr_cfg.on && !(st->hw.channel13 & 010u))
        in->lr.active = 0;
    else if (lr_cfg.on && !in->lr.active && (st->hw.channel13 & 04u))
        lr_step(&lr_cfg, &in->lr, t, vm_cfg.touchdown_cs, st->s.words, st->hw.channel13,
                &st->hw.channel33);
    if (!rm_rr_cfg.on) return;
    if (!(st->hw.channel13 & 010u))
        in->rr.active = 0;
    else if (!in->rr.active)
        rm_rr_step(&rm_rr_cfg, t, &in->rr, st->s.words, st->hw.channel12, st->hw.channel13,
                   st->hw.channel14, &st->hw.channel33);
}

/* FLIGHT_CH30-33 (octal), defaults as yaAGC's GOJAM. */
static agc_word fs_channel(const char *name,agc_word dflt){const char *v=getenv(name);return v?(agc_word)(strtoul(v,0,8)&077777u):dflt;}
/* X86_START's file, a header tools/flight_host_run.c's FLIGHT_SNAPSHOT
 * wrote: SNAP_CS, snap_words[2048], then name=value fields of agc_hardware
 * (SNAP_HW) and flight_io (SNAP_IO, SNAP_FLIGHT). Loads them into st and
 * counts AGC time from SNAP_CS there; 0 (with a message) on a bad file. */
static int fs_load_start(fd_state *st, const char *path)
{
    static char text[1 << 16];
    static const char *const hw_names[] = {"channel11", "in_interrupt", "channel5",  "channel6",
                                           "channel13", "channel15",    "channel10", "channel32",
                                           "channel12", "channel14",    "channel30", "channel33",
                                           "channel31", "channel3",     "channel4",  "radar_gate"};
    static const char *const io_names[] = {
        "superbank",       "interrupt_enabled", "channel16", "channel34",
        "channel35",       "trap31a",           "trap31b",   "trap32",
        "executive_alarm", "waitlist_alarm",    "warning"};
    FILE *f = fopen(path, "r");
    size_t n = f ? fread(text, 1, sizeof text - 1u, f) : 0u;
    const char *p, *sec;
    unsigned long snap_cs, abs_cs, v;
    unsigned i, found = 0;
    if (f) fclose(f);
    text[n] = 0;
    if (!n || n == sizeof text - 1u || !(p = strstr(text, "#define SNAP_CS ")) ||
        sscanf(p + 16, "%lu", &snap_cs) != 1 || !(p = strstr(text, "snap_words[2048]")) ||
        !(p = strchr(p, '{'))) {
        fprintf(stderr, "X86_START: %s is no FLIGHT_SNAPSHOT header\n", path);
        return 0;
    }
    for (++p, i = 0; i < 2048u; ++i) {
        char *e;
        v = strtoul(p, &e, 10);
        if (e == p || v > 077777ul) break;
        st->s.words[i] = (agc_word)v;
        p = e;
        while (*p == 'u' || *p == ',' || *p == ' ' || *p == '\n') ++p;
    }
    if (i < 2048u) {
        fprintf(stderr, "X86_START: %s: bad snap_words\n", path);
        return 0;
    }
    /* Each field is "(h).name=Nu;" or "(io).name=Nu;" under its macro. */
    for (sec = p; (p = strstr(p, ").")) != 0; p += 2) {
        const char *m = p, *eq = strchr(p + 2, '=');
        size_t len;
        int hw;
        if (!eq) break;
        while (m > sec && strncmp(m, "#define SNAP_", 13) != 0) --m;
        hw = strncmp(m, "#define SNAP_HW", 15) == 0;
        len = (size_t)(eq - (p + 2));
        v = strtoul(eq + 1, 0, 10);
        for (i = 0; i < (hw ? 16u : 11u); ++i) {
            const char *name = hw ? hw_names[i] : io_names[i];
            if (strlen(name) != len || strncmp(name, p + 2, len) != 0) continue;
            ++found;
            if (hw) switch (i) {
                case 0: st->hw.channel11 = (agc_word)v; break;
                case 1: st->hw.in_interrupt = (uint16_t)v; break;
                case 2: st->hw.channel5 = (agc_word)v; break;
                case 3: st->hw.channel6 = (agc_word)v; break;
                case 4: st->hw.channel13 = (agc_word)v; break;
                case 5: st->hw.channel15 = (agc_word)v; break;
                case 6: st->hw.channel10 = (agc_word)v; break;
                case 7: st->hw.channel32 = (agc_word)v; break;
                case 8: st->hw.channel12 = (agc_word)v; break;
                case 9: st->hw.channel14 = (agc_word)v; break;
                case 10: st->hw.channel30 = (agc_word)v; break;
                case 11: st->hw.channel33 = (agc_word)v; break;
                case 12: st->hw.channel31 = (agc_word)v; break;
                case 13: st->hw.channel3 = (agc_word)v; break;
                case 14: st->hw.channel4 = (agc_word)v; break;
                default: st->hw.radar_gate = (uint8_t)v; break;
                }
            else
                switch (i) {
                case 0: st->io.superbank = (agc_word)v; break;
                case 1: st->io.interrupt_enabled = (agc_word)v; break;
                case 2: st->io.channel16 = (agc_word)v; break;
                case 3: st->io.channel34 = (agc_word)v; break;
                case 4: st->io.channel35 = (agc_word)v; break;
                case 5: st->io.trap31a = (uint16_t)v; break;
                case 6: st->io.trap31b = (uint16_t)v; break;
                case 7: st->io.trap32 = (uint16_t)v; break;
                case 8: st->io.executive_alarm = (agc_word)v; break;
                case 9: st->io.waitlist_alarm = (agc_word)v; break;
                default: st->io.warning = (agc_word)v; break;
                }
        }
    }
    /* SNAP_IO and SNAP_FLIGHT both give the superbank and the interrupt
     * enable: 16 + 2 + 11 fields. */
    if (found != 29u) {
        fprintf(stderr, "X86_START: %s: %u of 29 fields\n", path, found);
        return 0;
    }
    /* SNAP_CDU (headers since #786): the output drives' state, so a window
     * that starts while a drive owes pulses goes on with them. */
    if ((p = strstr(text, "#define SNAP_CDU(c) ")) != 0) {
        int owed[5];
        unsigned elapsed, active;
        if (sscanf(p + 20,
                   "(c).owed[0]=%d;(c).owed[1]=%d;(c).owed[2]=%d;(c).owed[3]=%d;(c).owed[4]=%d;"
                   "(c).elapsed=%uu;(c).active=%uu;",
                   &owed[0], &owed[1], &owed[2], &owed[3], &owed[4], &elapsed, &active) != 7) {
            fprintf(stderr, "X86_START: %s: bad SNAP_CDU\n", path);
            return 0;
        }
        for (i = 0; i < 5u; ++i) rk_cdu.owed[i] = (int16_t)owed[i];
        rk_cdu.elapsed = (uint16_t)elapsed;
        rk_cdu.active = (uint8_t)active;
    }
    abs_cs = ((unsigned long)(st->s.words[024] & 037777u) << 14) | (st->s.words[025] & 037777u);
    st->t0 = abs_cs - snap_cs;
    return 1;
}

/* Cold start with the scenario's input channels and DOWNRUPT, or X86_START's
 * state (its channels as written); 0 (with a message) on a bad start file. */
static int fs_start(fd_state *st, fs_inputs *in, uint8_t mode)
{
    const char *d = getenv("FLIGHT_DOWNRUPT"), *start = getenv("X86_START");
    fd_init(st, mode, fs_channel("FLIGHT_CH30", 037777u), fs_channel("FLIGHT_CH31", 077777u));
    st->downrupt = d && *d && strcmp(d, "0") != 0;
    if (start) {
        if (!fs_load_start(st, start)) return 0;
        fs_inputs_from(in, fd_centiseconds(st));
        return 1;
    }
    st->hw.channel32 = fs_channel("FLIGHT_CH32", 077777u);
    st->hw.channel33 = fs_channel("FLIGHT_CH33", 077777u);
    return 1;
}
#endif
