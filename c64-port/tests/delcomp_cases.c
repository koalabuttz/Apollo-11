/* SPDX-License-Identifier: MIT
 * Input-only matrices/angles; no expected numerical results. */
#include "delcomp_cases.h"
static const uint16_t directions[16][6]={
{0020000,0000000,0000000,0000000,0000000,0000000},
{0057777,0077777,0000000,0000000,0000000,0000000},
{0000000,0000000,0020000,0000000,0000000,0000000},
{0000000,0000000,0057777,0077777,0000000,0000000},
{0000000,0000000,0000000,0000000,0020000,0000000},
{0000000,0000000,0000000,0000000,0057777,0077777},
{0013240,0023632,0013240,0023632,0000000,0000000},
{0064537,0054145,0013240,0023632,0000000,0000000},
{0011171,0024721,0011171,0024721,0011171,0024721},
{0000000,0000000,0000000,0000000,0000000,0000000},
{0077777,0077777,0077777,0077777,0077777,0077777},
{0000000,0077777,0077777,0000000,0000000,0077777},
{0037777,0040000,0000001,0077776,0020000,0057777},
{0037777,0037777,0037777,0037777,0037777,0037777},
{0040000,0040000,0040000,0040000,0040000,0040000},
{0012345,0023456,0034567,0045670,0056701,0067012},
};
enum apollo_fixed_mission delcomp_mission(uint16_t n){return n<5120u?APOLLO_FIXED_LM:APOLLO_FIXED_CM;}
static uint16_t ext(uint16_t w){return (uint16_t)(w|((w&040000u)<<1));}
void delcomp_prepare(apollo_overlay_state *s,gimbal_io *io,uint16_t n){
 static const uint16_t angles[16][2]={{0,0},{077777,077777},{1,0},{077776,077777},{020000,0},{057777,077777},{010000,0},{067777,077777},
 {037777,037777},{040000,040000},{0,1},{077777,077776},{0,040000},{077777,037777},{0142,012345},{076543,023456}};
 static const uint16_t flags[4]={0,1,077776,077777};
 uint16_t local=(uint16_t)(n%5120u),angle=(uint16_t)(local%16u),direction=(uint16_t)((local/16u)%16u),base=(uint16_t)(0401u+((local/1024u)%5u)*054u);
 uint16_t bank=(uint16_t)((local/64u)%8u),i,cof=n<5120u?03270:03342;
 for(i=0;i<2048u;++i)s->words[i]=(uint16_t)((251u*i+(local%16u)*947u+99u)&077777u);
 for(i=0;i<3u;++i)s->words[i]=(uint16_t)(ext(s->words[i])^((local&16u)?040000u:0));
 s->words[3]=(uint16_t)(bank<<8);s->words[4]=044000;s->words[6]=(uint16_t)(044000u|bank);s->words[7]=0;
 s->words[5]=n<5120u?06070:06037;s->words[023]=0;s->words[067]=0;
 s->words[0120]=base;s->words[0115]=040000;s->words[0165]=s->words[6];s->words[0164]=n<5120u?02527:02530;
 s->words[0154]=angles[angle][0];s->words[0155]=angles[angle][1];s->words[0163]=0;s->words[0121]=flags[(local/256u)%4u];
 for(i=0;i<6u;++i)s->words[cof+i]=directions[direction][i];
 s->words[base+052u]=(uint16_t)((local/1280u)%2u ? 056073u:(n<5120u?044753u:044744u));
 io->superbank=(uint16_t)(((local/320u)%8u)*020u);io->interrupt_enabled=(uint16_t)((local/640u)%2u);
}
