/***************************************************-*- coding: iso-8859-1 -*-
 * ratecontrol.c: h264 encoder library (Rate Control)
 *****************************************************************************
 * Copyright (C) 2005 x264 project
 * $Id: ratecontrol.c,v 1.1 2004/06/03 19:27:08 fenrir Exp $
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Michael Niedermayer <michaelni@gmx.at>
 *          M錸s Rullg錼d <mru@mru.ath.cx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#define _ISOC99_SOURCE
#undef NDEBUG // always check asserts, the speed effect is far too small to disable them
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "common/common.h"
#include "common/cpu.h"
#include "ratecontrol.h"

//////////////////////////////////////////////////////////////////////////
#define X264_RATECTRL_ENTRY_COUNT  256		// 必须是 2 的 N 次方

typedef struct
{
    int pict_type;
    int kept_as_ref;
    float qscale;
    int mv_bits;
    int i_tex_bits;
    int p_tex_bits;
    int misc_bits;
    uint64_t expected_bits;
    float new_qscale;
    int new_qp;
    int i_count;
    int p_count;
    int s_count;
    float blurred_complexity;
    char direct_mode;
	int m_nTotalBits;			// 该帧总数
	float m_fAverageBufUsage;	// 此刻平均缓冲区利用率
	float m_fAverageQ;			// 平均量化系数
} ratecontrol_entry_t;

typedef struct
{
    double coeff;
    double count;
    double decay;
} predictor_t;

struct x264_ratecontrol_t
{
    /* constants */
    int b_abr;
    int b_2pass;
    int b_vbv;
    int b_vbv_min_rate;
    double fps;
    double bitrate;
    double rate_tolerance;
    int nmb;                    /* number of macroblocks in a frame */
    int qp_constant[5];

    /* current frame */
    ratecontrol_entry_t *rce;
    int qp;                     /* qp for current frame */
    int qpm;                    /* qp for current macroblock */
    float qpa;                  /* average of macroblocks' qp */
    int qp_force;

    /* VBV stuff */
    double buffer_size;
    double buffer_fill_final;   /* real buffer as of the last finished frame */
    double buffer_fill;         /* planned buffer, if all in-progress frames hit their bit budget */
    double buffer_rate;         /* # of bits added to buffer_fill after each frame */
    predictor_t *pred;          /* predict frame size from satd */

    /* ABR stuff */
    int    last_satd;
    double last_rceq;
    double cplxr_sum;           /* sum of bits*qscale/rceq */
    double expected_bits_sum;   /* sum of qscale2bits after rceq, ratefactor, and overflow */
    double wanted_bits_window;  /* target bitrate * window */
    double cbr_decay;
    double short_term_cplxsum;
    double short_term_cplxcount;
    double rate_factor_constant;
    double ip_offset;
    double pb_offset;

    /* 2pass stuff */
    FILE *p_stat_file_out;
    char *psz_stat_file_tmpname;

    int num_entries;            /* number of ratecontrol_entry_ts */
    ratecontrol_entry_t *entry; /* FIXME: copy needed data and free this once init is done */
    double last_qscale;
    double last_qscale_for[5];  /* last qscale for a specific pict type, used for max_diff & ipb factor stuff  */
    int last_non_b_pict_type;
    double accum_p_qp;          /* for determining I-frame quant */
    double accum_p_norm;
    double last_accum_p_norm;
    double lmin[5];             /* min qscale by frame type */
    double lmax[5];
    double lstep;               /* max change (multiply) in qscale per frame */
    double i_cplx_sum[5];       /* estimated total texture bits in intra MBs at qscale=1 */
    double p_cplx_sum[5];
    double mv_bits_sum[5];
    int frame_count[5];         /* number of frames of each type */

    /* MBRC stuff */
    double frame_size_planned;
    predictor_t *row_pred;
    predictor_t row_preds[5];
    predictor_t *pred_b_from_p; /* predict B-frame size from P-frame satd */
    int bframes;                /* # consecutive B-frames before this P-frame */
    int bframe_bits;            /* total cost of those frames */

    int i_zones;
    x264_zone_t *zones;
    x264_zone_t *prev_zone;

	//  CYJ,2007-12-17 Add
#define BITRATE_WINDOW_SIZE 32
	unsigned long m_dwTotalBytesWindowed;			// 窗口中的总字节数
	unsigned long m_dwExpectBytesWindowsed;			// 期望的字节数
	unsigned long m_adwEachFrameBytesWindowed[ BITRATE_WINDOW_SIZE ];	// 以前编码的各帧的字节数
	int		m_nFrameWindowPtr;						// 窗口指针	
	float	m_fLastQP_IP;
	float	m_afUpperBPSLimit[2];					// 速率上限
	int		m_nOverflowTimes;						// 上溢次数
	float	m_fAverageQP;							// 平均的Q值
	float	m_fAverageTotalByteWindowed;			// 平均的窗口字节数
	int		m_nBitsPerFrame;						// 每帧允许的比特数
};


//////////////////////////////////////////////////////////////////////////
static int parse_zones( x264_t *h );
static int init_pass2(x264_t *);
static float rate_estimate_qscale( x264_t *h );
static void update_vbv( x264_t *h, int bits );
static void update_vbv_plan( x264_t *h );
static double predict_size( predictor_t *p, double q, double var );
static void update_predictor( predictor_t *p, double q, double var, double bits );
int  x264_rc_analyse_slice( x264_t *h );


//////////////////////////////////////////////////////////////////////////
//  CYJ,2007-12-13 ADD，作为一个常数来使用
const double g_constqp2qscale_20 = 2.14187f; // qp2qscale(20);



static ratecontrol_entry_t	s_aRateCtrlEntry[ X264_RATECTRL_ENTRY_COUNT ];
static long s_nTotalBitsInRateCtrlEntry = 0;
static long s_nExpectBitsInRateCtrlEntry = 0; 

///-------------------------------------------------------
/// CYJ,2007-12-15
/// 函数功能:
///		速率控制点入口
/// 输入参数:
///		无
/// 返回参数:
///		无
ratecontrol_entry_t * GetRateCtrlEntryAt( unsigned int nFrameIndex )
{
	nFrameIndex &= (X264_RATECTRL_ENTRY_COUNT-1);
	return s_aRateCtrlEntry + nFrameIndex;
}

/* Terminology:
 * qp = h.264's quantizer
 * qscale = linearized quantizer = Lagrange multiplier
 */
static inline double qp2qscale(double qp)
{
    return 0.85 * pow(2.0, ( qp - 12.0 ) / 6.0);
}
static inline double qscale2qp(double qscale)
{
    return 12.0 + 6.0 * log(qscale/0.85) / log(2.0);
}


/* Texture bitrate is not quite inversely proportional to qscale,
 * probably due the the changing number of SKIP blocks.
 * MV bits level off at about qp<=12, because the lambda used
 * for motion estimation is constant there. */
static inline double qscale2bits(ratecontrol_entry_t *rce, double qscale)
{
    if(qscale<0.1)
        qscale = 0.1;
    return (rce->i_tex_bits + rce->p_tex_bits + .1) * pow( rce->qscale / qscale, 1.1 )
           + rce->mv_bits * pow( X264_MAX(rce->qscale, 1) / X264_MAX(qscale, 1), 0.5 )
           + rce->misc_bits;
}


int x264_ratecontrol_new( x264_t *h )
{
    x264_ratecontrol_t *rc;
    int i;

    x264_cpu_restore( h->param.cpu );

    rc = h->rc = x264_malloc( h->param.i_threads * sizeof(x264_ratecontrol_t) );
    memset( rc, 0, h->param.i_threads * sizeof(x264_ratecontrol_t) );

    rc->b_abr = h->param.rc.i_rc_method != X264_RC_CQP && !h->param.rc.b_stat_read;
    rc->b_2pass = h->param.rc.i_rc_method == X264_RC_ABR && h->param.rc.b_stat_read;
    
    /* FIXME: use integers */
    if(h->param.i_fps_num > 0 && h->param.i_fps_den > 0)
        rc->fps = (float) h->param.i_fps_num / h->param.i_fps_den;
    else
        rc->fps = 25.0;

    rc->bitrate = h->param.rc.i_bitrate * 1000.;
    rc->rate_tolerance = h->param.rc.f_rate_tolerance;
    rc->nmb = h->mb.i_mb_count;
    rc->last_non_b_pict_type = -1;
    rc->cbr_decay = 1.0;

    if( h->param.rc.i_rc_method == X264_RC_CRF && h->param.rc.b_stat_read )
    {
        x264_log(h, X264_LOG_ERROR, "constant rate-factor is incompatible with 2pass.\n");
        return -1;
    }
    if( h->param.rc.i_vbv_buffer_size )
    {
        if( h->param.rc.i_rc_method == X264_RC_CQP )
            x264_log(h, X264_LOG_WARNING, "VBV is incompatible with constant QP, ignored.\n");
        else if( h->param.rc.i_vbv_max_bitrate == 0 )
        {
            x264_log( h, X264_LOG_DEBUG, "VBV maxrate unspecified, assuming CBR\n" );
            h->param.rc.i_vbv_max_bitrate = h->param.rc.i_bitrate;
        }
    }
    if( h->param.rc.i_vbv_max_bitrate < h->param.rc.i_bitrate &&
        h->param.rc.i_vbv_max_bitrate > 0)
        x264_log(h, X264_LOG_WARNING, "max bitrate less than average bitrate, ignored.\n");
    else if( h->param.rc.i_vbv_max_bitrate > 0 &&
             h->param.rc.i_vbv_buffer_size > 0 )
    {
        if( h->param.rc.i_vbv_buffer_size < 3 * h->param.rc.i_vbv_max_bitrate / rc->fps )
        {
            h->param.rc.i_vbv_buffer_size = 3 * h->param.rc.i_vbv_max_bitrate / rc->fps;
            x264_log( h, X264_LOG_WARNING, "VBV buffer size too small, using %d kbit\n",
                      h->param.rc.i_vbv_buffer_size );
        }
        if( h->param.rc.f_vbv_buffer_init > 1. )
            h->param.rc.f_vbv_buffer_init = x264_clip3f( h->param.rc.f_vbv_buffer_init / h->param.rc.i_vbv_buffer_size, 0, 1 );
        rc->buffer_rate = h->param.rc.i_vbv_max_bitrate * 1000. / rc->fps;
        rc->buffer_size = h->param.rc.i_vbv_buffer_size * 1000.;
        rc->buffer_fill_final = rc->buffer_size * h->param.rc.f_vbv_buffer_init;
        rc->cbr_decay = 1.0 - rc->buffer_rate / rc->buffer_size
                      * 0.5 * X264_MAX(0, 1.5 - rc->buffer_rate * rc->fps / rc->bitrate);
        rc->b_vbv = 1;
        rc->b_vbv_min_rate = !rc->b_2pass
                          && h->param.rc.i_rc_method == X264_RC_ABR
                          && h->param.rc.i_vbv_max_bitrate <= h->param.rc.i_bitrate;
    }
    else if( h->param.rc.i_vbv_max_bitrate )
    {
        x264_log(h, X264_LOG_WARNING, "VBV maxrate specified, but no bufsize.\n");
        h->param.rc.i_vbv_max_bitrate = 0;
    }
    if(rc->rate_tolerance < 0.01) {
        x264_log(h, X264_LOG_WARNING, "bitrate tolerance too small, using .01\n");
        rc->rate_tolerance = 0.01;
    }

    h->mb.b_variable_qp = rc->b_vbv && !rc->b_2pass;

    if( rc->b_abr )
    {
        /* FIXME ABR_INIT_QP is actually used only in CRF */
#define ABR_INIT_QP ( h->param.rc.i_rc_method == X264_RC_CRF ? h->param.rc.f_rf_constant : 24 )
        rc->accum_p_norm = .01;
        rc->accum_p_qp = ABR_INIT_QP * rc->accum_p_norm;
        /* estimated ratio that produces a reasonable QP for the first I-frame */
        rc->cplxr_sum = .01 * pow( 7.0e5, h->param.rc.f_qcompress ) * pow( h->mb.i_mb_count, 0.5 );
        rc->wanted_bits_window = 1.0 * rc->bitrate / rc->fps;
        rc->last_non_b_pict_type = SLICE_TYPE_I;
    }

    if( h->param.rc.i_rc_method == X264_RC_CRF )
    {
        /* arbitrary rescaling to make CRF somewhat similar to QP */
        double base_cplx = h->mb.i_mb_count * (h->param.i_bframe ? 120 : 80);
        rc->rate_factor_constant = pow( base_cplx, 1 - h->param.rc.f_qcompress )
                                 / qp2qscale( h->param.rc.f_rf_constant );
    }

    rc->ip_offset = 6.0 * log(h->param.rc.f_ip_factor) / log(2.0);
    rc->pb_offset = 6.0 * log(h->param.rc.f_pb_factor) / log(2.0);
    rc->qp_constant[SLICE_TYPE_P] = h->param.rc.i_qp_constant;
    rc->qp_constant[SLICE_TYPE_I] = x264_clip3( h->param.rc.i_qp_constant - rc->ip_offset + 0.5, 0, 51 );
    rc->qp_constant[SLICE_TYPE_B] = x264_clip3( h->param.rc.i_qp_constant + rc->pb_offset + 0.5, 0, 51 );

    rc->lstep = pow( 2, h->param.rc.i_qp_step / 6.0 );
    rc->last_qscale = qp2qscale(26);
    rc->pred = x264_malloc( 5*sizeof(predictor_t) );
    rc->pred_b_from_p = x264_malloc( sizeof(predictor_t) );
    for( i = 0; i < 5; i++ )
    {
        rc->last_qscale_for[i] = qp2qscale( ABR_INIT_QP );
        rc->lmin[i] = qp2qscale( h->param.rc.i_qp_min );
        rc->lmax[i] = qp2qscale( h->param.rc.i_qp_max );
        rc->pred[i].coeff= 2.0;
        rc->pred[i].count= 1.0;
        rc->pred[i].decay= 0.5;
        rc->row_preds[i].coeff= .25;
        rc->row_preds[i].count= 1.0;
        rc->row_preds[i].decay= 0.5;
    }
    *rc->pred_b_from_p = rc->pred[0];

	// copy data
    for( i=1; i<h->param.i_threads; i++ )
    {
        h->thread[i]->rc = rc+i;
        rc[i] = rc[0];
    }

	// 窗口中总的字节数；按照速率，折合成每帧的字节数；然后再乘以缓存的帧数
	rc->m_dwExpectBytesWindowsed = (BITRATE_WINDOW_SIZE * rc->bitrate) / ( rc->fps * 8 );		// 窗口中的总字节数
	rc->m_dwTotalBytesWindowed = rc->m_dwExpectBytesWindowsed;		// 期望的字节数
	rc->m_fAverageTotalByteWindowed = rc->m_dwTotalBytesWindowed;

	rc->m_adwEachFrameBytesWindowed[ 0 ] = rc->bitrate / ( rc->fps * 8 );
	rc->m_nFrameWindowPtr = 0;
	rc->m_fLastQP_IP = 26;
	rc->m_fAverageQP = 26;
	rc->m_afUpperBPSLimit[0] = 1.1f;		// 一半水平
	rc->m_afUpperBPSLimit[1] = 1.2f;	
	rc->m_nBitsPerFrame = rc->bitrate / rc->fps;

	for(i=1; i<BITRATE_WINDOW_SIZE; i++ )
	{
		rc->m_adwEachFrameBytesWindowed[ i ] = rc->m_adwEachFrameBytesWindowed[ 0 ];
	}

#ifdef _DEBUG
	{
		char szTmp[100];
		sprintf( szTmp, "my_x264_out_%d.txt", rc->b_2pass );
		rc->p_stat_file_out = fopen( szTmp, "wt" );
	}
#endif //_DEBUG
	
	// 第一遍的编码结果
	if( h->param.rc.b_stat_write )
	{
		memset( s_aRateCtrlEntry, 0, sizeof(s_aRateCtrlEntry) );
		s_nTotalBitsInRateCtrlEntry = X264_RATECTRL_ENTRY_COUNT * rc->bitrate / rc->fps;
		s_nExpectBitsInRateCtrlEntry = s_nTotalBitsInRateCtrlEntry;	// 期望的比特数；即按照设定的速率得到的比特总数
		for(i=0; i<X264_RATECTRL_ENTRY_COUNT; i++ )
		{
			s_aRateCtrlEntry[i].m_nTotalBits = s_nTotalBitsInRateCtrlEntry / X264_RATECTRL_ENTRY_COUNT;
		}
	}

    return 0;
}


x264_zone_t *get_zone( x264_t *h, int frame_num )
{    
    return NULL;
}

void x264_ratecontrol_summary( x264_t *h )
{
    x264_ratecontrol_t *rc = h->rc;
    if( rc->b_abr && h->param.rc.i_rc_method == X264_RC_ABR && rc->cbr_decay > .9999 )
    {
        double base_cplx = h->mb.i_mb_count * (h->param.i_bframe ? 120 : 80);
        x264_log( h, X264_LOG_INFO, "final ratefactor: %.2f\n", 
                  qscale2qp( pow( base_cplx, 1 - h->param.rc.f_qcompress )
                             * rc->cplxr_sum / rc->wanted_bits_window ) );
    }
}

void x264_ratecontrol_delete( x264_t *h )
{
    x264_ratecontrol_t *rc = h->rc;
    int i;

    if( rc->p_stat_file_out )
    {
        fclose( rc->p_stat_file_out );
        x264_free( rc->psz_stat_file_tmpname );
    }
    x264_free( rc->pred );
    x264_free( rc->pred_b_from_p );
    x264_free( rc->entry );
    if( rc->zones )
    {
        x264_free( rc->zones[0].param );
        if( h->param.rc.psz_zones )
            for( i=1; i<rc->i_zones; i++ )
                if( rc->zones[i].param != rc->zones[0].param )
                    x264_free( rc->zones[i].param );
        x264_free( rc->zones );
    }
    x264_free( rc );
}

static void accum_p_qp_update( x264_t *h, float qp )
{
    x264_ratecontrol_t *rc = h->rc;
    rc->accum_p_qp   *= .95;
    rc->accum_p_norm *= .95;
    rc->accum_p_norm += 1;
    if( h->sh.i_type == SLICE_TYPE_I )
        rc->accum_p_qp += qp + rc->ip_offset;
    else
        rc->accum_p_qp += qp;
}

///-------------------------------------------------------
/// CYJ,2007-12-21
/// 函数功能:
///		获取 目标比特数
/// 输入参数:
///		无
/// 返回参数:
///		无
static int my_pass2_get_target_bit( x264_t *h )
{
#define GET_TARGET_BIT_FRAME_COUNT	64

	long nTotalBits = 0;
	int frame = h->fenc->i_frame;
	int nBitAvaible = GET_TARGET_BIT_FRAME_COUNT * h->rc->m_nBitsPerFrame;
	int nTargetBits;
	ratecontrol_entry_t *rce = GetRateCtrlEntryAt( h->fenc->i_frame );
	int i;

	for(i=0; i<GET_TARGET_BIT_FRAME_COUNT; i++ )
	{		
        nTotalBits += GetRateCtrlEntryAt(frame)->m_nTotalBits;
		frame ++;
	}
	
	nBitAvaible -= nTotalBits;

	nTargetBits = nBitAvaible * rce->m_nTotalBits * 2 / nTotalBits;
	
	return nTargetBits + rce->m_nTotalBits;
}

/* Before encoding a frame, choose a QP for it */
void x264_ratecontrol_start( x264_t *h, int i_force_qp )
{
    x264_ratecontrol_t *rc = h->rc;
    ratecontrol_entry_t *rce = NULL;
    x264_zone_t *zone = get_zone( h, h->fenc->i_frame );
    float q;
	int nBFrameOffset;

    x264_cpu_restore( h->param.cpu );

    if( zone && (!rc->prev_zone || zone->param != rc->prev_zone->param) )
        x264_encoder_reconfig( h, zone->param );
    rc->prev_zone = zone;

    rc->qp_force = i_force_qp;

    if( h->param.rc.b_stat_read )
    {
        int frame = h->fenc->i_frame;
        rce = h->rc->rce = GetRateCtrlEntryAt(frame);

        if( h->sh.i_type == SLICE_TYPE_B
            && h->param.analyse.i_direct_mv_pred == X264_DIRECT_PRED_AUTO )
        {
            h->sh.b_direct_spatial_mv_pred = ( rce->direct_mode == 's' );
            h->mb.b_direct_auto_read = ( rce->direct_mode == 's' || rce->direct_mode == 't' );
        }
    }

    if( rc->b_vbv )
    {
        memset( h->fdec->i_row_bits, 0, h->sps->i_mb_height * sizeof(int) );
        rc->row_pred = &rc->row_preds[h->sh.i_type];
        update_vbv_plan( h );
    }

    if( h->sh.i_type != SLICE_TYPE_B )
    {
        rc->bframes = 0;
        while( h->frames.current[rc->bframes] && IS_X264_TYPE_B(h->frames.current[rc->bframes]->i_type) )
            rc->bframes++;
    }

    rc->qpa = 0;

    if( i_force_qp )
    {
        q = i_force_qp - 1;
    }
    else if( rc->b_abr )
    {
//        q = qscale2qp( rate_estimate_qscale( h ) );
		float fRate = (float)rc->m_dwTotalBytesWindowed / rc->m_dwExpectBytesWindowsed;
		float fAverageRate = (float)rc->m_fAverageTotalByteWindowed / rc->m_dwExpectBytesWindowsed;

		if( fRate < rc->m_afUpperBPSLimit[0] )
			rc->m_nOverflowTimes = 0;
		else if( rc->m_nOverflowTimes < 4 )
			rc->m_nOverflowTimes ++;

		nBFrameOffset = fAverageRate > 0.8f ? 8 : 4;

		if( h->sh.i_type != SLICE_TYPE_B )
		{												//  最大 1.25；在 80% 左右是不做调整；超过 80% 增大；反之降低
			q = ( (rc->m_fAverageQP + rc->m_fLastQP_IP ) / 2 ) * pow( (1.07f +fAverageRate + fRate/4 )/2, 0.9f );

			// 均值限制在 20 ~ 30 之间
			rc->m_fAverageQP = x264_clip3( ( rc->m_fAverageQP * 7 + q ) / 8, 20, 36 );

			if( h->sh.i_type == SLICE_TYPE_I )
				q = x264_clip3( (int)(q + 0.5), 20, 30+rc->m_nOverflowTimes );
			else
				q = x264_clip3( (int)(q + 2.5), 22, 40+rc->m_nOverflowTimes );

			rc->m_fLastQP_IP = q;			
		}
		else if( h->fdec->b_kept_as_ref )
			q = rc->m_fLastQP_IP + ( rc->m_nOverflowTimes ? 2 : 0 );
		else
			q = rc->m_fLastQP_IP + nBFrameOffset + rc->m_nOverflowTimes;
    }
    else if( rc->b_2pass )
    {		
		float fAverageRate = (float)rc->m_fAverageTotalByteWindowed / rc->m_dwExpectBytesWindowsed;
		nBFrameOffset = fAverageRate > 0.8f ? 8 : 4;

		if( h->sh.i_type != SLICE_TYPE_B )
		{		
			float fFutureRate;
			float fRate = (float)rc->m_dwTotalBytesWindowed / rc->m_dwExpectBytesWindowsed;
			int nNewBits, nAdjustTimes = 0;
			int nTargetBits;
			int Q_Offset;
			
			// 属于第二遍扫描；应该对这个内存使用率进行修正
			// 如果将来速率教高；则现在就开始降低
			// 反之；如果将来较低；则现在可以稍微使用高一点的带宽
			fFutureRate = (float)s_nTotalBitsInRateCtrlEntry / s_nExpectBitsInRateCtrlEntry;
			fRate = ( fRate + fAverageRate + fFutureRate + rce->m_fAverageBufUsage ) / 4;

			if( fRate < 0.6f && fAverageRate < 0.6f && fFutureRate < 0.6f )
				Q_Offset = 0;			
			else if( fAverageRate < 0.7f )
				Q_Offset = 2;
			else 
				Q_Offset = 2 + (int)( ( fAverageRate - 0.7 ) * 20 ) * 2;

			if( fRate < rc->m_afUpperBPSLimit[0] )
				rc->m_nOverflowTimes = 0;
			else if( rc->m_nOverflowTimes < 8 )
				rc->m_nOverflowTimes ++;

			if( fRate < 0.95f || fAverageRate < 0.9f || fFutureRate < 0.9f)
				q = (rc->m_fAverageQP + rc->m_fLastQP_IP + rce->new_qp) / 3 * pow( fRate, 0.5f );
			else if( fAverageRate < 0.95f && fRate < rc->m_afUpperBPSLimit[0] )
				q = (rc->m_fAverageQP + rc->m_fLastQP_IP + rce->new_qp) / 3 * fRate;
			else
				q = (rc->m_fAverageQP + rc->m_fLastQP_IP + rce->new_qp) / 3 * pow( fRate, 1.5f );
			rce->new_qp = q;

			nTargetBits = ( rc->m_nBitsPerFrame + ( 2 - rce->m_fAverageBufUsage - fFutureRate ) * rc->m_nBitsPerFrame * 4 );
			nTargetBits = ( my_pass2_get_target_bit( h ) * 2 / (1+fAverageRate)  + nTargetBits ) / 2;

			nTargetBits = nTargetBits * 2 / ( fAverageRate + 1.4 );	// 将过去的带宽利用上

			if( fAverageRate > 0.95f )		// 当 fAverageRate 超过 90% 以后，将很大地降低带宽的利用
				nTargetBits = nTargetBits * 0.8f / pow( (fAverageRate+1.5f)/2, 5 );		// 0.9 * 2/(1+fAverageRate+0.2)
			else if( fAverageRate > 0.9f )		// 当 fAverageRate 超过 90% 以后，将很大地降低带宽的利用
				nTargetBits = nTargetBits * 0.85f / pow( (fAverageRate+1.3f)/2, 2 );		// 0.9 * 2/(1+fAverageRate+0.2)			
			else if( fAverageRate > 0.85f )		// 当 fAverageRate 超过 90% 以后，将很大地降低带宽的利用
				nTargetBits = nTargetBits * 0.75f;
			else if( fRate > 1.5f || fFutureRate > 0.95f )
				nTargetBits = nTargetBits * 0.9f / pow( (fRate+fFutureRate+1.4f)/3, 2 );// 0.9 * 2/(1+fAverageRate+0.2)

			nTargetBits *= 0.95f;			// 保留 5% 余量
			
			// （预置速率 － 当前速率 ）/ 帧速率 ＝ 每帧可用比特数
			nNewBits = qscale2bits( rce, qp2qscale(q) );			

			if( nNewBits < nTargetBits )
			{			// 可以增加
				while( q > 16 && nNewBits < nTargetBits && nAdjustTimes < 6 )
				{
					q --;
					nAdjustTimes ++;
					nNewBits = qscale2bits( rce, qp2qscale(q) );
				}
				if( nNewBits > nTargetBits )
					q ++;			// 超过了，还原为原来的
			}
			else
			{
				while( q < 40 && nNewBits > nTargetBits && nAdjustTimes < 6 )
				{
					q ++;
					nAdjustTimes ++;
					nNewBits = qscale2bits( rce, qp2qscale(q) );
				}
				q --;
			}
			q = ( q * 4 + rce->new_qp + rc->m_fLastQP_IP ) / 6;
			if( fAverageRate > 0.9f )
				q += 2;

			// 均值限制在 20 ~ 30 之间
			rc->m_fAverageQP = x264_clip3( ( rc->m_fAverageQP * 7 + q ) / 8, 20, 30 );

			if( h->sh.i_type == SLICE_TYPE_I )
				q = x264_clip3( (int)(q + 0.5), 14+Q_Offset, 24+rc->m_nOverflowTimes+Q_Offset );
			else
				q = x264_clip3( (int)(q + 2.5), 16+Q_Offset, 36+rc->m_nOverflowTimes+Q_Offset );

			rc->m_fLastQP_IP = q;			
		}
		else if( h->fdec->b_kept_as_ref )
			q = rc->m_fLastQP_IP + ( rc->m_nOverflowTimes ? 2 : 0 );
		else
			q = rc->m_fLastQP_IP + nBFrameOffset + rc->m_nOverflowTimes;
//        rce->new_qscale = rate_estimate_qscale( h );
//      q = qscale2qp( rce->new_qscale );
//		q = (float)rc->m_dwTotalBytesWindowed * rc->qp / rc->m_dwExpectBytesWindowsed;
//		rce->new_qscale = qp2qscale( q );
    }
    else /* CQP */
    {
        if( h->sh.i_type == SLICE_TYPE_B && h->fdec->b_kept_as_ref )
            q = ( rc->qp_constant[ SLICE_TYPE_B ] + rc->qp_constant[ SLICE_TYPE_P ] ) / 2;
        else
            q = rc->qp_constant[ h->sh.i_type ];

        if( zone )
        {
            if( zone->b_force_qp )
                q += zone->i_qp - rc->qp_constant[SLICE_TYPE_P];
            else
                q -= 6*log(zone->f_bitrate_factor)/log(2);
        }
    }

    h->fdec->f_qp_avg =
    rc->qpm =
    rc->qp = x264_clip3( (int)(q + 0.5), 16, 40 );
    if( rce )
        rce->new_qp = rc->qp;

    /* accum_p_qp needs to be here so that future frames can benefit from the
     * data before this frame is done. but this only works because threading
     * guarantees to not re-encode any frames. so the non-threaded case does
     * accum_p_qp later. */
    if( h->param.i_threads > 1 )
        accum_p_qp_update( h, rc->qp );

    if( h->sh.i_type != SLICE_TYPE_B )
        rc->last_non_b_pict_type = h->sh.i_type;
}

double predict_row_size( x264_t *h, int y, int qp )
{
    /* average between two predictors:
     * absolute SATD, and scaled bit cost of the colocated row in the previous frame */
    x264_ratecontrol_t *rc = h->rc;
    double pred_s = predict_size( rc->row_pred, qp2qscale(qp), h->fdec->i_row_satd[y] );
    double pred_t = 0;
    if( h->sh.i_type != SLICE_TYPE_I 
        && h->fref0[0]->i_type == h->fdec->i_type
        && h->fref0[0]->i_row_satd[y] > 0 )
    {
        pred_t = h->fref0[0]->i_row_bits[y] * h->fdec->i_row_satd[y] / h->fref0[0]->i_row_satd[y]
                 * qp2qscale(h->fref0[0]->i_row_qp[y]) / qp2qscale(qp);
    }
    if( pred_t == 0 )
        pred_t = pred_s;

    return (pred_s + pred_t) / 2;
}

double predict_row_size_sum( x264_t *h, int y, int qp )
{
    int i;
    double bits = 0;
    for( i = 0; i <= y; i++ )
        bits += h->fdec->i_row_bits[i];
    for( i = y+1; i < h->sps->i_mb_height; i++ )
        bits += predict_row_size( h, i, qp );
    return bits;
}

void x264_ratecontrol_mb( x264_t *h, int bits )
{
    x264_ratecontrol_t *rc = h->rc;
    const int y = h->mb.i_mb_y;

    x264_cpu_restore( h->param.cpu );
                                                                                      
    h->fdec->i_row_bits[y] += bits;
    rc->qpa += rc->qpm;

    if( h->mb.i_mb_x != h->sps->i_mb_width - 1 || !h->mb.b_variable_qp )
        return;

    h->fdec->i_row_qp[y] = rc->qpm;

	// CYJ Add
#if 1	
	rc->qpm = rc->qp;
	if( (h->mb.i_mb_y >= h->sps->i_mb_height - 4) || ( h->mb.i_mb_y < 4 && h->mb.i_mb_x < 4 ) )
		return;
	if( h->mb.i_mb_x < 4 || h->mb.i_mb_x >= h->sps->i_mb_width - 4 || h->mb.i_mb_y < 4 )
	{
		if( h->sh.i_type == SLICE_TYPE_B )
			rc->qpm += 12;
		else
			rc->qpm += 6;
	}
	
	return;
#endif // 1

    if( h->sh.i_type == SLICE_TYPE_B )
    {
        /* B-frames shouldn't use lower QP than their reference frames */
        if( y < h->sps->i_mb_height-1 )
        {
            rc->qpm = X264_MAX( rc->qp,
                      X264_MIN( h->fref0[0]->i_row_qp[y+1],
                                h->fref1[0]->i_row_qp[y+1] ));
        }
    }
    else
    {
        update_predictor( rc->row_pred, qp2qscale(rc->qpm), h->fdec->i_row_satd[y], h->fdec->i_row_bits[y] );

        /* tweak quality based on difference from predicted size */
        if( y < h->sps->i_mb_height-1 && h->stat.i_slice_count[h->sh.i_type] > 0 )
        {
            int prev_row_qp = h->fdec->i_row_qp[y];
            int b0 = predict_row_size_sum( h, y, rc->qpm );
            int b1 = b0;
            int i_qp_max = X264_MIN( prev_row_qp + h->param.rc.i_qp_step, h->param.rc.i_qp_max );
            int i_qp_min = X264_MAX( prev_row_qp - h->param.rc.i_qp_step, h->param.rc.i_qp_min );
            float buffer_left_planned = rc->buffer_fill - rc->frame_size_planned;

            if( !rc->b_vbv_min_rate )
                i_qp_min = X264_MAX( i_qp_min, h->sh.i_qp );

            while( rc->qpm < i_qp_max
                   && (b1 > rc->frame_size_planned * 1.15
                    || (rc->buffer_fill - b1 < buffer_left_planned * 0.5)))
            {
                rc->qpm ++;
                b1 = predict_row_size_sum( h, y, rc->qpm );
            }

            while( rc->qpm > i_qp_min
                   && buffer_left_planned > rc->buffer_size * 0.4
                   && ((b1 < rc->frame_size_planned * 0.8 && rc->qpm <= prev_row_qp)
                     || b1 < (rc->buffer_fill - rc->buffer_size + rc->buffer_rate) * 1.1) )
            {
                rc->qpm --;
                b1 = predict_row_size_sum( h, y, rc->qpm );
            }
        }
    }
}

int x264_ratecontrol_qp( x264_t *h )
{
    return h->rc->qpm;
}

/* In 2pass, force the same frame types as in the 1st pass */
int x264_ratecontrol_slice_type( x264_t *h, int frame_num )
{
    x264_ratecontrol_t *rc = h->rc;
    if( h->param.rc.b_stat_read )
    {
		ratecontrol_entry_t * pRCE = GetRateCtrlEntryAt( frame_num );
        switch( pRCE->pict_type )
        {
            case SLICE_TYPE_I:
                return pRCE->kept_as_ref ? X264_TYPE_IDR : X264_TYPE_I;

            case SLICE_TYPE_B:
                return pRCE->kept_as_ref ? X264_TYPE_BREF : X264_TYPE_B;

            case SLICE_TYPE_P:
            default:
                return X264_TYPE_P;
        }
    }
    else
    {
        return X264_TYPE_AUTO;
    }
}

/* After encoding one frame, save stats and update ratecontrol state */
void x264_ratecontrol_end( x264_t *h, int bits )
{
    x264_ratecontrol_t *rc = h->rc;
    const int *mbs = h->stat.frame.i_mb_count;
    int i;

    x264_cpu_restore( h->param.cpu );

	//  CYJ,2007-12-17 Add
	// {{{	修正速率
	rc->m_dwTotalBytesWindowed += ( ( bits / 8 ) - rc->m_adwEachFrameBytesWindowed[ rc->m_nFrameWindowPtr ] );
	rc->m_adwEachFrameBytesWindowed[ rc->m_nFrameWindowPtr ] = ( bits / 8 );
	if(  ++rc->m_nFrameWindowPtr >= BITRATE_WINDOW_SIZE )
		rc->m_nFrameWindowPtr = 0;	
	rc->m_fAverageTotalByteWindowed = ( rc->m_fAverageTotalByteWindowed * 15 + rc->m_dwTotalBytesWindowed ) / 16;
	//}}}

    h->stat.frame.i_mb_count_skip = mbs[P_SKIP] + mbs[B_SKIP];
    h->stat.frame.i_mb_count_i = mbs[I_16x16] + mbs[I_8x8] + mbs[I_4x4];
    h->stat.frame.i_mb_count_p = mbs[P_L0] + mbs[P_8x8];
    for( i = B_DIRECT; i < B_8x8; i++ )
        h->stat.frame.i_mb_count_p += mbs[i];

    if( h->mb.b_variable_qp )
        rc->qpa /= h->mb.i_mb_count;
    else
        rc->qpa = rc->qp;
    h->fdec->f_qp_avg = rc->qpa;

    if( h->param.rc.b_stat_write )
    {
        char c_type = h->sh.i_type==SLICE_TYPE_I ? (h->fenc->i_poc==0 ? 'I' : 'i')
                    : h->sh.i_type==SLICE_TYPE_P ? 'P'
                    : h->fenc->b_kept_as_ref ? 'B' : 'b';
        int dir_frame = h->stat.frame.i_direct_score[1] - h->stat.frame.i_direct_score[0];
        int dir_avg = h->stat.i_direct_score[1] - h->stat.i_direct_score[0];
        char c_direct = h->mb.b_direct_auto_write ?
                        ( dir_frame>0 ? 's' : dir_frame<0 ? 't' : 
                          dir_avg>0 ? 's' : dir_avg<0 ? 't' : '-' )
                        : '-';
		//  CYJ,2007-12-13 Add
		ratecontrol_entry_t * pRCE = GetRateCtrlEntryAt( h->fenc->i_frame );

		pRCE->pict_type = h->sh.i_type;
		if( h->sh.i_type==SLICE_TYPE_I )
			pRCE->kept_as_ref = (h->fenc->i_poc==0);
		else if( h->sh.i_type==SLICE_TYPE_B )
			pRCE->kept_as_ref = h->fenc->b_kept_as_ref;
		pRCE->qscale = qp2qscale( rc->qpa );			
		pRCE->i_tex_bits = h->stat.frame.i_itex_bits;
		pRCE->p_tex_bits = h->stat.frame.i_ptex_bits;
		pRCE->mv_bits = h->stat.frame.i_hdr_bits;
		pRCE->misc_bits = h->stat.frame.i_misc_bits;
		pRCE->i_count = h->stat.frame.i_mb_count_i;
		pRCE->p_count = h->stat.frame.i_mb_count_p;
		pRCE->s_count = h->stat.frame.i_mb_count_skip;
		pRCE->direct_mode = c_direct;

		pRCE->expected_bits = 0;
		pRCE->new_qscale = pRCE->qscale;
		pRCE->new_qp = rc->qpa;			
		pRCE->blurred_complexity = 0;			
		pRCE->m_fAverageBufUsage = rc->m_fAverageTotalByteWindowed / rc->m_dwExpectBytesWindowsed;

		s_nTotalBitsInRateCtrlEntry = s_nTotalBitsInRateCtrlEntry + bits - pRCE->m_nTotalBits;
		pRCE->m_nTotalBits = bits;
    }

#ifdef _DEBUG
	if( rc->p_stat_file_out )
	{
		char c_type = h->sh.i_type==SLICE_TYPE_I ? (h->fenc->i_poc==0 ? 'I' : 'i')
                    : h->sh.i_type==SLICE_TYPE_P ? 'P'
                    : h->fenc->b_kept_as_ref ? 'B' : 'b';
        int dir_frame = h->stat.frame.i_direct_score[1] - h->stat.frame.i_direct_score[0];
        int dir_avg = h->stat.i_direct_score[1] - h->stat.i_direct_score[0];
        char c_direct = h->mb.b_direct_auto_write ?
                        ( dir_frame>0 ? 's' : dir_frame<0 ? 't' : 
                          dir_avg>0 ? 's' : dir_avg<0 ? 't' : '-' )
                        : '-';

		fprintf( rc->p_stat_file_out,
				 "i:%6d o:%6d t:%c q:%5.2f it:%5d pt:%5d mv:%5d mi:%5d im:%5d pm:%5d sm:%5d d:%c br:%6.1f bu:%5.1f%% ab:%5.1f%% aq:%5.1f tbu:%3d%%;\n",
				 h->fenc->i_frame, h->i_frame,
				 c_type, rc->qpa,
				 h->stat.frame.i_itex_bits, h->stat.frame.i_ptex_bits,
				 h->stat.frame.i_hdr_bits, h->stat.frame.i_misc_bits,
				 h->stat.frame.i_mb_count_i,
				 h->stat.frame.i_mb_count_p,
				 h->stat.frame.i_mb_count_skip,
				 c_direct,
				 (float)rc->m_dwTotalBytesWindowed*rc->bitrate/(rc->m_dwExpectBytesWindowsed*1000),
				 (float)rc->m_dwTotalBytesWindowed*100/rc->m_dwExpectBytesWindowsed,
				 rc->m_fAverageTotalByteWindowed*100/rc->m_dwExpectBytesWindowsed,
				 rc->m_fAverageQP,
				 s_nTotalBitsInRateCtrlEntry * 100 / s_nExpectBitsInRateCtrlEntry );
	}
#endif

    if( rc->b_abr )
    {
        if( h->sh.i_type != SLICE_TYPE_B )
            rc->cplxr_sum += bits * qp2qscale(rc->qpa) / rc->last_rceq;
        else
        {
            /* Depends on the fact that B-frame's QP is an offset from the following P-frame's.
             * Not perfectly accurate with B-refs, but good enough. */
            rc->cplxr_sum += bits * qp2qscale(rc->qpa) / (rc->last_rceq * fabs(h->param.rc.f_pb_factor));
        }
        rc->cplxr_sum *= rc->cbr_decay;
        rc->wanted_bits_window += rc->bitrate / rc->fps;
        rc->wanted_bits_window *= rc->cbr_decay;

        if( h->param.i_threads == 1 )
            accum_p_qp_update( h, rc->qpa );
    }

    if( rc->b_2pass )
    {
        rc->expected_bits_sum += qscale2bits( rc->rce, qp2qscale(rc->rce->new_qp) );
    }

    if( h->mb.b_variable_qp )
    {
        if( h->sh.i_type == SLICE_TYPE_B )
        {
            rc->bframe_bits += bits;
            if( !h->frames.current[0] || !IS_X264_TYPE_B(h->frames.current[0]->i_type) )
            {
                update_predictor( rc->pred_b_from_p, qp2qscale(rc->qpa),
                                  h->fref1[h->i_ref1-1]->i_satd, rc->bframe_bits / rc->bframes );
                rc->bframe_bits = 0;
            }
        }
    }

    update_vbv( h, bits );
}

/****************************************************************************
 * 2 pass functions
 ***************************************************************************/

double x264_eval( char *s, double *const_value, const char **const_name,
                  double (**func1)(void *, double), const char **func1_name,
                  double (**func2)(void *, double, double), char **func2_name,
                  void *opaque );

/**
 * modify the bitrate curve from pass1 for one frame
 */
static double get_qscale(x264_t *h, ratecontrol_entry_t *rce, double rate_factor, int frame_num)
{
    x264_ratecontrol_t *rcc= h->rc;
    const int pict_type = rce->pict_type;
    double q;
    x264_zone_t *zone = get_zone( h, frame_num );

    double const_values[]={
        rce->i_tex_bits * rce->qscale,
        rce->p_tex_bits * rce->qscale,
        (rce->i_tex_bits + rce->p_tex_bits) * rce->qscale,
        rce->mv_bits * rce->qscale,
        (double)rce->i_count / rcc->nmb,
        (double)rce->p_count / rcc->nmb,
        (double)rce->s_count / rcc->nmb,
        rce->pict_type == SLICE_TYPE_I,
        rce->pict_type == SLICE_TYPE_P,
        rce->pict_type == SLICE_TYPE_B,
        h->param.rc.f_qcompress,
        rcc->i_cplx_sum[SLICE_TYPE_I] / rcc->frame_count[SLICE_TYPE_I],
        rcc->i_cplx_sum[SLICE_TYPE_P] / rcc->frame_count[SLICE_TYPE_P],
        rcc->p_cplx_sum[SLICE_TYPE_P] / rcc->frame_count[SLICE_TYPE_P],
        rcc->p_cplx_sum[SLICE_TYPE_B] / rcc->frame_count[SLICE_TYPE_B],
        (rcc->i_cplx_sum[pict_type] + rcc->p_cplx_sum[pict_type]) / rcc->frame_count[pict_type],
        rce->blurred_complexity,
        0
    };
    static const char *const_names[]={
        "iTex",
        "pTex",
        "tex",
        "mv",
        "iCount",
        "pCount",
        "sCount",
        "isI",
        "isP",
        "isB",
        "qComp",
        "avgIITex",
        "avgPITex",
        "avgPPTex",
        "avgBPTex",
        "avgTex",
        "blurCplx",
        NULL
    };
    static double (*func1[])(void *, double)={
//      (void *)bits2qscale,
        (void *)qscale2bits,
        NULL
    };
    static const char *func1_names[]={
//      "bits2qp",
        "qp2bits",
        NULL
    };

    q = x264_eval((char*)h->param.rc.psz_rc_eq, const_values, const_names, func1, func1_names, NULL, NULL, rce);

    // avoid NaN's in the rc_eq
    if(!isfinite(q) || rce->i_tex_bits + rce->p_tex_bits + rce->mv_bits == 0)
        q = rcc->last_qscale;
    else {
        rcc->last_rceq = q;
        q /= rate_factor;
        rcc->last_qscale = q;
    }

    if( zone )
    {
        if( zone->b_force_qp )
            q = qp2qscale(zone->i_qp);
        else
            q /= zone->f_bitrate_factor;
    }

    return q;
}

static double get_diff_limited_q(x264_t *h, ratecontrol_entry_t *rce, double q)
{
    x264_ratecontrol_t *rcc = h->rc;
    const int pict_type = rce->pict_type;

    // force I/B quants as a function of P quants
    const double last_p_q    = rcc->last_qscale_for[SLICE_TYPE_P];
    const double last_non_b_q= rcc->last_qscale_for[rcc->last_non_b_pict_type];
    if( pict_type == SLICE_TYPE_I )
    {
        double iq = q;
        double pq = qp2qscale( rcc->accum_p_qp / rcc->accum_p_norm );
        double ip_factor = fabs( h->param.rc.f_ip_factor );
        /* don't apply ip_factor if the following frame is also I */
        if( rcc->accum_p_norm <= 0 )
            q = iq;
        else if( h->param.rc.f_ip_factor < 0 )
            q = iq / ip_factor;
        else if( rcc->accum_p_norm >= 1 )
            q = pq / ip_factor;
        else
            q = rcc->accum_p_norm * pq / ip_factor + (1 - rcc->accum_p_norm) * iq;
    }
    else if( pict_type == SLICE_TYPE_B )
    {
        if( h->param.rc.f_pb_factor > 0 )
            q = last_non_b_q;
        if( !rce->kept_as_ref )
            q *= fabs( h->param.rc.f_pb_factor );
    }
    else if( pict_type == SLICE_TYPE_P
             && rcc->last_non_b_pict_type == SLICE_TYPE_P
             && rce->i_tex_bits + rce->p_tex_bits == 0 )
    {
        q = last_p_q;
    }

    /* last qscale / qdiff stuff */
    if(rcc->last_non_b_pict_type==pict_type
       && (pict_type!=SLICE_TYPE_I || rcc->last_accum_p_norm < 1))
    {
        double last_q = rcc->last_qscale_for[pict_type];
        double max_qscale = last_q * rcc->lstep;
        double min_qscale = last_q / rcc->lstep;

        if     (q > max_qscale) q = max_qscale;
        else if(q < min_qscale) q = min_qscale;
    }

    rcc->last_qscale_for[pict_type] = q;
    if(pict_type!=SLICE_TYPE_B)
        rcc->last_non_b_pict_type = pict_type;
    if(pict_type==SLICE_TYPE_I)
    {
        rcc->last_accum_p_norm = rcc->accum_p_norm;
        rcc->accum_p_norm = 0;
        rcc->accum_p_qp = 0;
    }
    if(pict_type==SLICE_TYPE_P)
    {
        float mask = 1 - pow( (float)rce->i_count / rcc->nmb, 2 );
        rcc->accum_p_qp   = mask * (qscale2qp(q) + rcc->accum_p_qp);
        rcc->accum_p_norm = mask * (1 + rcc->accum_p_norm);
    }
    return q;
}

static double predict_size( predictor_t *p, double q, double var )
{
     return p->coeff*var / (q*p->count);
}

static void update_predictor( predictor_t *p, double q, double var, double bits )
{
    if( var < 10 )
        return;
    p->count *= p->decay;
    p->coeff *= p->decay;
    p->count ++;
    p->coeff += bits*q / var;
}

// update VBV after encoding a frame
static void update_vbv( x264_t *h, int bits )
{
    x264_ratecontrol_t *rcc = h->rc;
    x264_ratecontrol_t *rct = h->thread[0]->rc;

    if( rcc->last_satd >= h->mb.i_mb_count )
        update_predictor( &rct->pred[h->sh.i_type], qp2qscale(rcc->qpa), rcc->last_satd, bits );

    if( !rcc->b_vbv )
        return;

    rct->buffer_fill_final += rct->buffer_rate - bits;
    if( rct->buffer_fill_final < 0 && !rct->b_2pass )
        x264_log( h, X264_LOG_WARNING, "VBV underflow (%.0f bits)\n", rct->buffer_fill_final );
    rct->buffer_fill_final = x264_clip3f( rct->buffer_fill_final, 0, rct->buffer_size );
}

// provisionally update VBV according to the planned size of all frames currently in progress
static void update_vbv_plan( x264_t *h )
{
    x264_ratecontrol_t *rcc = h->rc;
    rcc->buffer_fill = h->thread[0]->rc->buffer_fill_final;
    if( h->param.i_threads > 1 )
    {
        int j = h->rc - h->thread[0]->rc;
        int i;
        for( i=1; i<h->param.i_threads; i++ )
        {
            x264_t *t = h->thread[ (j+i)%h->param.i_threads ];
            double bits = t->rc->frame_size_planned;
            if( !t->b_thread_active )
                continue;
            rcc->buffer_fill += rcc->buffer_rate - bits;
            rcc->buffer_fill = x264_clip3( rcc->buffer_fill, 0, rcc->buffer_size );
        }
    }
}

// apply VBV constraints and clip qscale to between lmin and lmax
static double clip_qscale( x264_t *h, int pict_type, double q )
{
    x264_ratecontrol_t *rcc = h->rc;
    double lmin = rcc->lmin[pict_type];
    double lmax = rcc->lmax[pict_type];
    double q0 = q;

    /* B-frames are not directly subject to VBV,
     * since they are controlled by the P-frames' QPs.
     * FIXME: in 2pass we could modify previous frames' QP too,
     *        instead of waiting for the buffer to fill */
    if( rcc->b_vbv &&
        ( pict_type == SLICE_TYPE_P ||
          ( pict_type == SLICE_TYPE_I && rcc->last_non_b_pict_type == SLICE_TYPE_I ) ) )
    {
        if( rcc->buffer_fill/rcc->buffer_size < 0.5 )
            q /= x264_clip3f( 2.0*rcc->buffer_fill/rcc->buffer_size, 0.5, 1.0 );
    }

    if( rcc->b_vbv && rcc->last_satd > 0 )
    {
        /* Now a hard threshold to make sure the frame fits in VBV.
         * This one is mostly for I-frames. */
        double bits = predict_size( &rcc->pred[h->sh.i_type], q, rcc->last_satd );
        double qf = 1.0;
        if( bits > rcc->buffer_fill/2 )
            qf = x264_clip3f( rcc->buffer_fill/(2*bits), 0.2, 1.0 );
        q /= qf;
        bits *= qf;
        if( bits < rcc->buffer_rate/2 )
            q *= bits*2/rcc->buffer_rate;
        q = X264_MAX( q0, q );

        /* Check B-frame complexity, and use up any bits that would
         * overflow before the next P-frame. */
        if( h->sh.i_type == SLICE_TYPE_P )
        {
            int nb = rcc->bframes;
            double pbbits = bits;
            double bbits = predict_size( rcc->pred_b_from_p, q * h->param.rc.f_pb_factor, rcc->last_satd );
            double space;

            if( bbits > rcc->buffer_rate )
                nb = 0;
            pbbits += nb * bbits;

            space = rcc->buffer_fill + (1+nb)*rcc->buffer_rate - rcc->buffer_size;
            if( pbbits < space )
            {
                q *= X264_MAX( pbbits / space,
                               bits / (0.5 * rcc->buffer_size) );
            }
            q = X264_MAX( q0-5, q );
        }

        if( !rcc->b_vbv_min_rate )
            q = X264_MAX( q0, q );
    }

    if(lmin==lmax)
        return lmin;
    else if(rcc->b_2pass)
    {
        double min2 = log(lmin);
        double max2 = log(lmax);
        q = (log(q) - min2)/(max2-min2) - 0.5;
        q = 1.0/(1.0 + exp(-4*q));
        q = q*(max2-min2) + min2;
        return exp(q);
    }
    else
        return x264_clip3f(q, lmin, lmax);
}

// update qscale for 1 frame based on actual bits used so far
static float rate_estimate_qscale( x264_t *h )
{
    float q;
    x264_ratecontrol_t *rcc = h->rc;
    ratecontrol_entry_t rce;
    int pict_type = h->sh.i_type;
    double lmin = rcc->lmin[pict_type];
    double lmax = rcc->lmax[pict_type];

    if( pict_type == SLICE_TYPE_B )
    {
        /* B-frames don't have independent ratecontrol, but rather get the
         * average QP of the two adjacent P-frames + an offset */

        int i0 = IS_X264_TYPE_I(h->fref0[0]->i_type);
        int i1 = IS_X264_TYPE_I(h->fref1[0]->i_type);
        int dt0 = abs(h->fenc->i_poc - h->fref0[0]->i_poc);
        int dt1 = abs(h->fenc->i_poc - h->fref1[0]->i_poc);
        float q0 = h->fref0[0]->f_qp_avg;
        float q1 = h->fref1[0]->f_qp_avg;

        if( h->fref0[0]->i_type == X264_TYPE_BREF )
            q0 -= rcc->pb_offset/2;
        if( h->fref1[0]->i_type == X264_TYPE_BREF )
            q1 -= rcc->pb_offset/2;

        if(i0 && i1)
            q = (q0 + q1) / 2 + rcc->ip_offset;
        else if(i0)
            q = q1;
        else if(i1)
            q = q0;
        else
            q = (q0*dt1 + q1*dt0) / (dt0 + dt1);

        if(h->fenc->b_kept_as_ref)
            q += rcc->pb_offset/2;
        else
            q += rcc->pb_offset;

		//  CYJ,2007-12-15； B帧的Q值放大一点
		if( q < 24 )		// 24 是否合适 ???
			q = 24;
		q += 4;

        rcc->frame_size_planned = predict_size( rcc->pred_b_from_p, q, h->fref1[h->i_ref1-1]->i_satd );
        rcc->last_satd = 0;
        return qp2qscale(q);
    }
    else
    {
        double abr_buffer = 2 * rcc->rate_tolerance * rcc->bitrate;
		// FIXME 在此插入 pass2 的代码
        /* Calculate the quantizer which would have produced the desired
         * average bitrate if it had been applied to all frames so far.
         * Then modulate that quant based on the current frame's complexity
         * relative to the average complexity so far (using the 2pass RCEQ).
         * Then bias the quant up or down if total size so far was far from
         * the target.
         * Result: Depending on the value of rate_tolerance, there is a
         * tradeoff between quality and bitrate precision. But at large
         * tolerances, the bit distribution approaches that of 2pass. */

        double wanted_bits, overflow=1, lmin, lmax;
		int i_frame_done;

        rcc->last_satd = x264_rc_analyse_slice( h );
        rcc->short_term_cplxsum *= 0.5;
        rcc->short_term_cplxcount *= 0.5;
        rcc->short_term_cplxsum += rcc->last_satd;
        rcc->short_term_cplxcount ++;

        rce.p_tex_bits = rcc->last_satd;
        rce.blurred_complexity = rcc->short_term_cplxsum / rcc->short_term_cplxcount;
        rce.i_tex_bits = 0;
        rce.mv_bits = 0;
        rce.p_count = rcc->nmb;
        rce.i_count = 0;
        rce.s_count = 0;
        rce.qscale = 1;
        rce.pict_type = pict_type;

        i_frame_done = h->fenc->i_frame + 1 - h->param.i_threads;

      
		// CYJ Add, to compare
//				q = get_qscale( h, &rce, rcc->m_dwExpectBytesWindowsed * rcc->last_qscale / rcc->m_dwTotalBytesWindowed, h->fenc->i_frame );
//			overflow = x264_clip3f( (double)rcc->m_dwExpectBytesWindowsed/rcc->m_dwTotalBytesWindowed, .5, 1.5 );
		q = rcc->m_dwExpectBytesWindowsed * g_constqp2qscale_20 / rcc->m_dwTotalBytesWindowed;

        rcc->last_qscale_for[pict_type] =
        rcc->last_qscale = q;

        if( !rcc->b_2pass && h->fenc->i_frame == 0 )
            rcc->last_qscale_for[SLICE_TYPE_P] = q;

        rcc->frame_size_planned = predict_size( &rcc->pred[h->sh.i_type], q, rcc->last_satd );
        return q;
    }
}

void x264_thread_sync_ratecontrol( x264_t *cur, x264_t *prev, x264_t *next )
{
    if( cur != prev )
    {
#define COPY(var) memcpy(&cur->rc->var, &prev->rc->var, sizeof(cur->rc->var))
        /* these vars are updated in x264_ratecontrol_start()
         * so copy them from the context that most recently started (prev)
         * to the context that's about to start (cur).
         */
        COPY(accum_p_qp);
        COPY(accum_p_norm);
        COPY(last_satd);
        COPY(last_rceq);
        COPY(last_qscale_for);
        COPY(last_non_b_pict_type);
        COPY(short_term_cplxsum);
        COPY(short_term_cplxcount);
        COPY(bframes);
        COPY(prev_zone);
#undef COPY
    }
    if( cur != next )
    {
#define COPY(var) next->rc->var = cur->rc->var
        /* these vars are updated in x264_ratecontrol_end()
         * so copy them from the context that most recently ended (cur)
         * to the context that's about to end (next)
         */
        COPY(cplxr_sum);
        COPY(expected_bits_sum);
        COPY(wanted_bits_window);
        COPY(bframe_bits);
#undef COPY
    }
    //FIXME row_preds[] (not strictly necessary, but would improve prediction)
    /* the rest of the variables are either constant or thread-local */
}

static int init_pass2( x264_t *h )
{
    x264_ratecontrol_t *rcc = h->rc;
    uint64_t all_const_bits = 0;
    uint64_t all_available_bits = (uint64_t)(h->param.rc.i_bitrate * 1000. * rcc->num_entries / rcc->fps);
    double rate_factor, step, step_mult;
    double qblur = h->param.rc.f_qblur;
    double cplxblur = h->param.rc.f_complexity_blur;
    const int filter_size = (int)(qblur*4) | 1;
    double expected_bits;
    double *qscale, *blurred_qscale;
    int i;

    /* find total/average complexity & const_bits */
    for(i=0; i<rcc->num_entries; i++){
        ratecontrol_entry_t *rce = &rcc->entry[i];
        all_const_bits += rce->misc_bits;
        rcc->i_cplx_sum[rce->pict_type] += rce->i_tex_bits * rce->qscale;
        rcc->p_cplx_sum[rce->pict_type] += rce->p_tex_bits * rce->qscale;
        rcc->mv_bits_sum[rce->pict_type] += rce->mv_bits * rce->qscale;
        rcc->frame_count[rce->pict_type] ++;
    }

    if( all_available_bits < all_const_bits)
    {
        x264_log(h, X264_LOG_ERROR, "requested bitrate is too low. estimated minimum is %d kbps\n",
                 (int)(all_const_bits * rcc->fps / (rcc->num_entries * 1000.)));
        return -1;
    }

    /* Blur complexities, to reduce local fluctuation of QP.
     * We don't blur the QPs directly, because then one very simple frame
     * could drag down the QP of a nearby complex frame and give it more
     * bits than intended. */
    for(i=0; i<rcc->num_entries; i++){
        ratecontrol_entry_t *rce = &rcc->entry[i];
        double weight_sum = 0;
        double cplx_sum = 0;
        double weight = 1.0;
        int j;
        /* weighted average of cplx of future frames */
        for(j=1; j<cplxblur*2 && j<rcc->num_entries-i; j++){
            ratecontrol_entry_t *rcj = &rcc->entry[i+j];
            weight *= 1 - pow( (float)rcj->i_count / rcc->nmb, 2 );
            if(weight < .0001)
                break;
            weight_sum += weight;
            cplx_sum += weight * (qscale2bits(rcj, 1) - rcj->misc_bits);
        }
        /* weighted average of cplx of past frames */
        weight = 1.0;
        for(j=0; j<=cplxblur*2 && j<=i; j++){
            ratecontrol_entry_t *rcj = &rcc->entry[i-j];
            weight_sum += weight;
            cplx_sum += weight * (qscale2bits(rcj, 1) - rcj->misc_bits);
            weight *= 1 - pow( (float)rcj->i_count / rcc->nmb, 2 );
            if(weight < .0001)
                break;
        }
        rce->blurred_complexity = cplx_sum / weight_sum;
    }

    qscale = x264_malloc(sizeof(double)*rcc->num_entries);
    if(filter_size > 1)
        blurred_qscale = x264_malloc(sizeof(double)*rcc->num_entries);
    else
        blurred_qscale = qscale;

    /* Search for a factor which, when multiplied by the RCEQ values from
     * each frame, adds up to the desired total size.
     * There is no exact closed-form solution because of VBV constraints and
     * because qscale2bits is not invertible, but we can start with the simple
     * approximation of scaling the 1st pass by the ratio of bitrates.
     * The search range is probably overkill, but speed doesn't matter here. */

    expected_bits = 1;
    for(i=0; i<rcc->num_entries; i++)
        expected_bits += qscale2bits(&rcc->entry[i], get_qscale(h, &rcc->entry[i], 1.0, i));
    step_mult = all_available_bits / expected_bits;

    rate_factor = 0;
    for(step = 1E4 * step_mult; step > 1E-7 * step_mult; step *= 0.5){
        expected_bits = 0;
        rate_factor += step;

        rcc->last_non_b_pict_type = -1;
        rcc->last_accum_p_norm = 1;
        rcc->accum_p_norm = 0;
        rcc->buffer_fill = rcc->buffer_size * h->param.rc.f_vbv_buffer_init;

        /* find qscale */
        for(i=0; i<rcc->num_entries; i++){
            qscale[i] = get_qscale(h, &rcc->entry[i], rate_factor, i);
        }

        /* fixed I/B qscale relative to P */
        for(i=rcc->num_entries-1; i>=0; i--){
            qscale[i] = get_diff_limited_q(h, &rcc->entry[i], qscale[i]);
            assert(qscale[i] >= 0);
        }

        /* smooth curve */
        if(filter_size > 1){
            assert(filter_size%2==1);
            for(i=0; i<rcc->num_entries; i++){
                ratecontrol_entry_t *rce = &rcc->entry[i];
                int j;
                double q=0.0, sum=0.0;

                for(j=0; j<filter_size; j++){
                    int index = i+j-filter_size/2;
                    double d = index-i;
                    double coeff = qblur==0 ? 1.0 : exp(-d*d/(qblur*qblur));
                    if(index < 0 || index >= rcc->num_entries) continue;
                    if(rce->pict_type != rcc->entry[index].pict_type) continue;
                    q += qscale[index] * coeff;
                    sum += coeff;
                }
                blurred_qscale[i] = q/sum;
            }
        }

        /* find expected bits */
        for(i=0; i<rcc->num_entries; i++){
            ratecontrol_entry_t *rce = &rcc->entry[i];
            double bits;
            rce->new_qscale = clip_qscale(h, rce->pict_type, blurred_qscale[i]);
            assert(rce->new_qscale >= 0);
            bits = qscale2bits(rce, rce->new_qscale);

            rce->expected_bits = expected_bits;
            expected_bits += bits;
            update_vbv(h, bits);
            rcc->buffer_fill = rcc->buffer_fill_final;
        }

//printf("expected:%llu available:%llu factor:%lf avgQ:%lf\n", (uint64_t)expected_bits, all_available_bits, rate_factor);
        if(expected_bits > all_available_bits) rate_factor -= step;
    }

    x264_free(qscale);
    if(filter_size > 1)
        x264_free(blurred_qscale);

    if(fabs(expected_bits/all_available_bits - 1.0) > 0.01)
    {
        double avgq = 0;
        for(i=0; i<rcc->num_entries; i++)
            avgq += rcc->entry[i].new_qscale;
        avgq = qscale2qp(avgq / rcc->num_entries);

        x264_log(h, X264_LOG_WARNING, "Error: 2pass curve failed to converge\n");
        x264_log(h, X264_LOG_WARNING, "target: %.2f kbit/s, expected: %.2f kbit/s, avg QP: %.4f\n",
                 (float)h->param.rc.i_bitrate,
                 expected_bits * rcc->fps / (rcc->num_entries * 1000.),
                 avgq);
        if(expected_bits < all_available_bits && avgq < h->param.rc.i_qp_min + 2)
        {
            if(h->param.rc.i_qp_min > 0)
                x264_log(h, X264_LOG_WARNING, "try reducing target bitrate or reducing qp_min (currently %d)\n", h->param.rc.i_qp_min);
            else
                x264_log(h, X264_LOG_WARNING, "try reducing target bitrate\n");
        }
        else if(expected_bits > all_available_bits && avgq > h->param.rc.i_qp_max - 2)
        {
            if(h->param.rc.i_qp_max < 51)
                x264_log(h, X264_LOG_WARNING, "try increasing target bitrate or increasing qp_max (currently %d)\n", h->param.rc.i_qp_max);
            else
                x264_log(h, X264_LOG_WARNING, "try increasing target bitrate\n");
        }
        else
            x264_log(h, X264_LOG_WARNING, "internal error\n");
    }

    return 0;
}


///-------------------------------------------------------
/// CYJ,2007-9-11
/// 函数功能:
///		获取 H264 压缩量化系数和当前速率
/// 输入参数:
///		pnQp			输出量化系数
/// 返回参数:
///		当前的速率
long x264_cyj_rc_get_qp_bitrate( x264_t *h, long * pnQp )
{
	if( pnQp )
		*pnQp = h->rc->qpa;			// h->rc->qp;
	// 比特数/帧 X 帧/秒 => 比特数/秒
	return  (long)( h->rc->m_dwTotalBytesWindowed * 8 * h->rc->fps / BITRATE_WINDOW_SIZE );
}

///-------------------------------------------------------
/// CYJ,2007-12-13
/// 函数功能:
///		设置新的速率
/// 输入参数:
///		无
/// 返回参数:
///		无
long x264_cyj_rc_set_new_bitrate(x264_t *h, long nNewBitrateKBPS )
{
	h->rc->bitrate = nNewBitrateKBPS;
}

