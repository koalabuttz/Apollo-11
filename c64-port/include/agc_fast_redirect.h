/* SPDX-License-Identifier: MIT */
#ifndef AGC_FAST_REDIRECT_H
#define AGC_FAST_REDIRECT_H
#include "agc_image_ops.h"
/* Routes a translation's agc_image_ops calls to the fast forms. Define
 * AFR_EBN (the EB the whole file runs under; see agc_fast_ops.h for the
 * rule) and include this after agc_image_ops.h. A plain-cell operand goes
 * to the afo_* helper, any other operand to the one-argument wrapper of
 * the ai_* helper (agc_image_thin.h), fixed operands by value to afo_*.
 * Every entry binds the image with AFR_BIND(s); AFR_S(s) is the image the
 * ops run on. With AFR_GENERIC the calls stay on agc_image_ops. */
#ifndef AFR_GENERIC
#include "agc_fast_ops.h"
#include "agc_image_thin.h"
#define AFR_OP(n,s,a) ((void)(s),AFO_PLAIN(AFR_EBN,a)?afo_##n(AFO_CELL(AFR_EBN,a)):AIT_ARG(n,a))
#define AFR_OP2(n,s,a) ((void)(s),AFO_PLAIN2(AFR_EBN,a)?afo_##n(AFO_CELL(AFR_EBN,a)):AIT_ARG(n,a))
#define AFR_F(n,s,k) ((void)(s),afo_##n(k))
#define ai_ca(s,a) AFR_OP(ca,s,a)
#define ai_cs(s,a) AFR_OP(cs,s,a)
#define ai_ad(s,a) AFR_OP(ad,s,a)
#define ai_su(s,a) AFR_OP(su,s,a)
#define ai_mask(s,a) AFR_OP(mask,s,a)
#define ai_xch(s,a) AFR_OP(xch,s,a)
#define ai_lxch(s,a) AFR_OP(lxch,s,a)
#define ai_qxch(s,a) AFR_OP(qxch,s,a)
#define ai_ads(s,a) AFR_OP(ads,s,a)
#define ai_incr(s,a) AFR_OP(incr,s,a)
#define ai_aug(s,a) AFR_OP(aug,s,a)
#define ai_dim(s,a) AFR_OP(dim,s,a)
#define ai_das(s,a) AFR_OP2(das,s,a)
#define ai_dca(s,a) AFR_OP2(dca,s,a)
#define ai_dcs(s,a) AFR_OP2(dcs,s,a)
#define ai_dxch(s,a) AFR_OP2(dxch,s,a)
#define ai_mp(s,a) AFR_OP(mp,s,a)
#define ai_dv(s,a) AFR_OP(dv,s,a)
#define ai_ts(s,a) AFR_OP(ts,s,a)
#define ai_ccs(s,a) AFR_OP(ccs,s,a)
#define ai_caf(s,k) AFR_F(caf,s,k)
#define ai_csf(s,k) AFR_F(csf,s,k)
#define ai_adf(s,k) AFR_F(adf,s,k)
#define ai_suf(s,k) AFR_F(suf,s,k)
#define ai_maskf(s,k) AFR_F(maskf,s,k)
#define ai_mpf(s,k) AFR_F(mpf,s,k)
#define ai_dvf(s,k) AFR_F(dvf,s,k)
#define ai_ccsf(s,k) AFR_F(ccsf,s,k)
#define ai_tc(s,k) AFR_F(tc,s,k)
#define ai_dcaf(s,k) AFR_F(dcaf,s,k)
#define ai_dcsf(s,k) AFR_F(dcsf,s,k)
#define ai_bzf(s) ((void)(s),afo_bzf())
#define ai_bzmf(s) ((void)(s),afo_bzmf())
#define AFR_S(s) ((void)(s),AFO_S)
#define AFR_BIND(s) (AIT_BIND(s),AFO_BIND(s))
#else
#define AFR_S(s) (s)
#define AFR_BIND(s) ((void)0)
#endif
#endif
