/*
 *  tvheadend - Codec Profiles
 *
 *  Copyright (C) 2016 Tvheadend
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "transcoding/codec/internals.h"
#include <nvmpi.h>

#define NVMPI_PRESET_DEFAULT               0
#define NVMPI_PRESET_SLOW                  1
#define NVMPI_PRESET_MEDIUM                2
#define NVMPI_PRESET_FAST                  3
#define NVMPI_PRESET_HP                    4
//#define NVMPI_PRESET_HQ                    5
//#define NVMPI_PRESET_BD                    6
//#define NVMPI_PRESET_LOW_LATENCY_DEFAULT   7
//#define NVMPI_PRESET_LOW_LATENCY_HQ        8
//#define NVMPI_PRESET_LOW_LATENCY_HP        9
//#define NVMPI_PRESET_LOSSLESS_DEFAULT      10
//#define NVMPI_PRESET_LOSSLESS_HP           11

#define NVMPI_PARAMS_RC_AUTO                  0
#define NVMPI_PARAMS_RC_CONSTQP               1
#define NVMPI_PARAMS_RC_VBR                   2
#define NVMPI_PARAMS_RC_CBR                   3
//#define NVMPI_PARAMS_RC_CBR_LD_HQ             8
//#define NVMPI_PARAMS_RC_CBR_HQ                16
//#define NVMPI_PARAMS_RC_VBR_HQ                32

#define NVMPI_PROFILE_UNKNOWN                      FF_PROFILE_UNKNOWN

#define NVMPI_H264_PROFILE_BASELINE			    0
#define NVMPI_H264_PROFILE_MAIN			        1
#define NVMPI_H264_PROFILE_HIGH			        2
#define NVMPI_H264_PROFILE_HIGH_444P           	3

#define NVMPI_HEVC_PROFILE_MAIN			        0
#define NVMPI_HEVC_PROFILE_MAIN_10 			    1
#define NVMPI_HEVC_PROFILE_REXT			        2

#define NVMPI_LEVEL_AUTOSELECT                     0

#define NVMPI_LEVEL_H264_1                         10
#define NVMPI_LEVEL_H264_1b                        9
#define NVMPI_LEVEL_H264_11                        11
#define NVMPI_LEVEL_H264_12                        12
#define NVMPI_LEVEL_H264_13                        13
#define NVMPI_LEVEL_H264_2                         20
#define NVMPI_LEVEL_H264_21                        21
#define NVMPI_LEVEL_H264_22                        22
#define NVMPI_LEVEL_H264_3                         30
#define NVMPI_LEVEL_H264_31                        31
#define NVMPI_LEVEL_H264_32                        32
#define NVMPI_LEVEL_H264_4                         40
#define NVMPI_LEVEL_H264_41                        41
#define NVMPI_LEVEL_H264_42                        42
#define NVMPI_LEVEL_H264_5                         50
#define NVMPI_LEVEL_H264_51                        51
#define NVMPI_LEVEL_H264_6                         60
#define NVMPI_LEVEL_H264_61                        61
#define NVMPI_LEVEL_H264_62                        62

#define NVMPI_LEVEL_HEVC_1                         30
#define NVMPI_LEVEL_HEVC_2                         60
#define NVMPI_LEVEL_HEVC_21                        63
#define NVMPI_LEVEL_HEVC_3                         90
#define NVMPI_LEVEL_HEVC_31                        93
#define NVMPI_LEVEL_HEVC_4                         120
#define NVMPI_LEVEL_HEVC_41                        123
#define NVMPI_LEVEL_HEVC_5                         150
#define NVMPI_LEVEL_HEVC_51                        153
#define NVMPI_LEVEL_HEVC_52                        156
#define NVMPI_LEVEL_HEVC_6                         180
#define NVMPI_LEVEL_HEVC_61                        183
#define NVMPI_LEVEL_HEVC_62                        186

/* nvmpi ====================================================================== */

typedef struct {
    TVHVideoCodecProfile;
    int quality;
    int rc;
    int preset;
    int level;
    //int qmin;
    //int qmax;
} tvh_codec_profile_nvmpi_t;

static const struct strtab presettab[] = {
    {"default",     NVMPI_PRESET_DEFAULT},
    {"slow",        NVMPI_PRESET_SLOW},
    {"medium",      NVMPI_PRESET_MEDIUM},
    {"fast",        NVMPI_PRESET_FAST},
    {"hp",          NVMPI_PRESET_HP},
    //{"hq",		PRESET_HQ},
    //{"bd",		PRESET_BD},
    //{"ll",		PRESET_LOW_LATENCY_DEFAULT},
    //{"llhq",	PRESET_LOW_LATENCY_HQ},
    //{"llhp",	PRESET_LOW_LATENCY_HP},
    //{"lossless",	PRESET_LOSSLESS_DEFAULT},
    //{"losslesshp",	PRESET_LOSSLESS_HP},
};

static const struct strtab rctab[] = {
    {"constqp",	      NVMPI_PARAMS_RC_CONSTQP},
    {"vbr",           NVMPI_PARAMS_RC_VBR},
    {"cbr",           NVMPI_PARAMS_RC_CBR}
    // {"cbr_ld_hq",     NVMPI_PARAMS_RC_CBR_LD_HQ},
    // {"cbr_hq",        NVMPI_PARAMS_RC_CBR_HQ},
    // {"vbr_hq",        NVMPI_PARAMS_RC_VBR_HQ},
};

static int
tvh_codec_profile_nvmpi_open(tvh_codec_profile_nvmpi_t *self, 
                                  AVDictionary **opts)
{
    const char *s;

    AV_DICT_SET_FLAGS_GLOBAL_HEADER(opts);
    //AV_DICT_SET_PIX_FMT(opts, self->pix_fmt, AV_PIX_FMT_NONE);

    if (self->preset != NVMPI_PRESET_DEFAULT &&
        (s = val2str(self->profile, presettab)) != NULL) {
        AV_DICT_SET(opts, "preset", s, 0);
    }
    if (self->rc != NVMPI_PARAMS_RC_AUTO &&
        (s = val2str(self->rc, rctab)) != NULL) {
        AV_DICT_SET(opts, "rc", s, 0);
    }

    // bit_rate
    if (self->bit_rate) {
        AV_DICT_SET_BIT_RATE(opts, self->bit_rate);
    }
    //if (self->libname && strlen(self->libname)) {
    //    AV_DICT_SET(opts, "nvmpi_libname", self->libname, 0);
    //}
    //if (self->libprefix && strlen(self->libprefix)) {
    //    AV_DICT_SET(opts, "nvmpi_libprefix", self->libprefix, 0);
    //}
    //AV_DICT_SET_INT(opts, "zerocopy", self->zerocopy, 0);
    return 0;
}


static htsmsg_t *
codec_profile_nvmpi_class_profile_list(void *obj, const char *lang)
{
    TVHCodec *codec = tvh_codec_profile_get_codec(obj);
    return tvh_codec_get_list(codec, profiles);
}

static htsmsg_t *
codec_profile_nvmpi_class_preset_list(void *obj, const char *lang)
{
    static const struct strtab tab[] = {
        {N_("Default"),		NVMPI_PRESET_DEFAULT},
        {N_("Slow"),		NVMPI_PRESET_SLOW},
        {N_("Medium"),		NVMPI_PRESET_MEDIUM},
        {N_("Fast"),		NVMPI_PRESET_FAST},
        {N_("HP"),		    NVMPI_PRESET_HP},
        //{N_("HQ"),		PRESET_HQ},
        //{N_("BD"),		PRESET_BD},
        //{N_("Low latency"),	PRESET_LOW_LATENCY_DEFAULT},
        //{N_("Low latency HQ"),	PRESET_LOW_LATENCY_HQ},
        //{N_("Low latency HP"),	PRESET_LOW_LATENCY_HP},
        //{N_("Lossless"),	PRESET_LOSSLESS_DEFAULT},
        //{N_("Lossless HP"),	PRESET_LOSSLESS_HP},
    };
    return strtab2htsmsg(tab, 1, lang);
}

static htsmsg_t *
codec_profile_nvmpi_class_rc_list(void *obj, const char *lang)
{
    static const struct strtab tab[] = {
        {N_("Auto"),				  NVMPI_PARAMS_RC_AUTO},
        {N_("Constant QP mode"),      NVMPI_PARAMS_RC_CONSTQP},
        {N_("VBR mode"),   			  NVMPI_PARAMS_RC_VBR},
        {N_("CBR mode"), 	  		  NVMPI_PARAMS_RC_CBR},
    };
    return strtab2htsmsg(tab, 1, lang);
}


static const codec_profile_class_t codec_profile_nvmpi_class = {
    {
        .ic_super   = (idclass_t *)&codec_profile_video_class,
        .ic_class   = "codec_profile_nvmpi",
        .ic_caption = N_("nvmpi"),
        .ic_properties = (const property_t[]){
            {
                .type     = PT_DBL,
                .id       = "bit_rate",
                .name     = N_("Bitrate (kb/s) (0=auto)"),
                .desc     = N_("Constant bitrate (CBR) mode."),
                .group    = 3,
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(TVHCodecProfile, bit_rate),
                .def.d    = 0,
            },      
            {
                .type     = PT_INT,
                .id       = "preset",
                .name     = N_("Preset"),
                .group    = 3,
                .desc     = N_("Override the preset rate control."),
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, preset),
                .list     = codec_profile_nvmpi_class_preset_list,
                .def.i    = NVMPI_PRESET_DEFAULT,
            },
            {
                .type     = PT_INT,
                .id       = "profile",
                .name     = N_("Profile"),
                .desc     = N_("Profile."),
                .group    = 4,
                .opts     = PO_ADVANCED | PO_PHIDDEN,
                .get_opts = codec_profile_class_profile_get_opts,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, profile),
                .list     = codec_profile_nvmpi_class_profile_list,
                .def.i    = NVMPI_PROFILE_UNKNOWN,
            },
            {
                .type     = PT_INT,
                .id       = "quality",
                .name     = N_("Quality (0=auto)"),
                .desc     = N_("Set encode quality (trades off against speed, "
                               "higher is faster) [0-51]."),
                .group    = 5,
                .opts     = PO_EXPERT,
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, quality),
                .intextra = INTEXTRA_RANGE(0, 51, 1),
            },
            {
                .type     = PT_INT,
                .id       = "ratecontrol",     // Don't change
                .name     = N_("Rate control"),
                .desc     = N_("Set rate control"),
                .group    = 3,
                .get_opts = codec_profile_class_get_opts,
                .list     = codec_profile_nvmpi_class_rc_list,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, rc),
                .def.i    = NVMPI_PARAMS_RC_AUTO,
            },
            //{
            //    .type     = PT_INT,
            //    .id       = "qmin",     // Don't change
            //    .name     = N_("Minimum QP"),
            //    .group    = 5,
            //    .desc     = N_("Minimum QP of P frames (from 0 to 52, 0=skip)"),
            //    .get_opts = codec_profile_class_get_opts,
            //    .off      = offsetof(tvh_codec_profile_nvmpi_t, qmin),
            //    .intextra = INTEXTRA_RANGE(0, 52, 1),
            //    .def.i    = 0,
            //},
            //{
            //    .type     = PT_INT,
            //    .id       = "qmax",     // Don't change
            //    .name     = N_("Maximum QP"),
            //    .group    = 5,
            //    .desc     = N_("Maximum QP of P frames (from 0 to 52, 0=skip)"),
            //    .get_opts = codec_profile_class_get_opts,
            //    .off      = offsetof(tvh_codec_profile_nvmpi_t, qmax),
            //    .intextra = INTEXTRA_RANGE(0, 52, 1),
            //    .def.i    = 0,
            //},
            {}
        }
    },
    .open = (codec_profile_open_meth)tvh_codec_profile_nvmpi_open,
};


/* nvmpi - H264 ==========================================*/
static htsmsg_t *
codec_profile_nvmpi_class_level_list_h264(void *obj, const char *lang)
{
    static const struct strtab tab[] = {
        {N_("Auto"),	      NVMPI_LEVEL_AUTOSELECT},
        {N_("1.0"),           NVMPI_LEVEL_H264_1},
        //{N_("1.0b"),          NVMPI_LEVEL_H264_1b},
        {N_("1.1"),           NVMPI_LEVEL_H264_11},
        {N_("1.2"),           NVMPI_LEVEL_H264_12},
        {N_("1.3"),           NVMPI_LEVEL_H264_13},
        {N_("2.0"),           NVMPI_LEVEL_H264_2},
        {N_("2.1"),           NVMPI_LEVEL_H264_21},
        {N_("2.2"),           NVMPI_LEVEL_H264_22},
        {N_("3.0"),           NVMPI_LEVEL_H264_3},
        {N_("3.1"),           NVMPI_LEVEL_H264_31},
        {N_("3.2"),           NVMPI_LEVEL_H264_32},
        {N_("4.0"),           NVMPI_LEVEL_H264_4},
        {N_("4.1"),           NVMPI_LEVEL_H264_41},
        {N_("4.2"),           NVMPI_LEVEL_H264_42},
        {N_("5.0"),           NVMPI_LEVEL_H264_5},
        {N_("5.1"),           NVMPI_LEVEL_H264_51},
        //{N_("6.0"),           NVMPI_LEVEL_H264_6},
        //{N_("6.1"),           NVMPI_LEVEL_H264_61},
        //{N_("6.2"),           NVMPI_LEVEL_H264_62},
    };
    return strtab2htsmsg(tab, 1, lang);
}


/* mpeg2_nvmpi =============================================================== */

TVHVideoCodec tvh_codec_nvmpi_mpeg2 = {
    .name     = "mpeg2_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_class,
    .profile_init = tvh_codec_profile_video_init,
    .profile_destroy = tvh_codec_profile_video_destroy,
};

/* h264_nvmpi =============================================================== */

static const AVProfile nvmpi_h264_profiles[] = {
    { FF_PROFILE_H264_BASELINE, "Baseline" },
    { FF_PROFILE_H264_MAIN,     "Main" },
    { FF_PROFILE_H264_HIGH,     "High" },
    { FF_PROFILE_UNKNOWN },
};

static int
tvh_codec_profile_nvmpi_h264_open(tvh_codec_profile_nvmpi_t *self,
                                  AVDictionary **opts)
{
    static const struct strtab profiletab[] = {
        {"baseline",    NVMPI_H264_PROFILE_BASELINE},
        {"main",        NVMPI_H264_PROFILE_MAIN},
        {"high",        NVMPI_H264_PROFILE_HIGH},
        {"high444p",    NVMPI_H264_PROFILE_HIGH_444P},
    };
 
    static const struct strtab leveltab[] = {
        {"Auto",	      NVMPI_LEVEL_AUTOSELECT},
        {"1.0",           NVMPI_LEVEL_H264_1},
        {"1.0b",          NVMPI_LEVEL_H264_1b},
        {"1.1",           NVMPI_LEVEL_H264_11},
        {"1.2",           NVMPI_LEVEL_H264_12},
        {"1.3",           NVMPI_LEVEL_H264_13},
        {"2.0",           NVMPI_LEVEL_H264_2},
        {"2.1",           NVMPI_LEVEL_H264_21},
        {"2.2",           NVMPI_LEVEL_H264_22},
        {"3.0",           NVMPI_LEVEL_H264_3},
        {"3.1",           NVMPI_LEVEL_H264_31},
        {"3.2",           NVMPI_LEVEL_H264_32},
        {"4.0",           NVMPI_LEVEL_H264_4},
        {"4.1",           NVMPI_LEVEL_H264_41},
        {"4.2",           NVMPI_LEVEL_H264_42},
        {"5.0",           NVMPI_LEVEL_H264_5},
        {"5.1",           NVMPI_LEVEL_H264_51},
        {"6.0",           NVMPI_LEVEL_H264_6},
        {"6.1",           NVMPI_LEVEL_H264_61},
        {"6.2",           NVMPI_LEVEL_H264_62},
    };

    const char *s;

    if (self->level != NVMPI_LEVEL_AUTOSELECT &&
        (s = val2str(self->level, leveltab)) != NULL) {
        AV_DICT_SET(opts, "level", s, 0);
    }

    if (self->profile != NVMPI_PROFILE_UNKNOWN &&
        (s = val2str(self->profile, profiletab)) != NULL) {
        AV_DICT_SET(opts, "profile", s, 0);
    }
    
    // ------ Set Defaults ---------
    AV_DICT_SET_INT(opts, "qmin", -1, 0);
    AV_DICT_SET_INT(opts, "qmax", -1, 0);
    AV_DICT_SET_INT(opts, "qdiff", -1, 0);
    AV_DICT_SET_INT(opts, "qblur", -1, 0);
    AV_DICT_SET_INT(opts, "qcomp", -1, 0);
    AV_DICT_SET_INT(opts, "g", 250, 0);
    AV_DICT_SET_INT(opts, "bf", 0, 0);
    AV_DICT_SET_INT(opts, "refs", 0, 0);
    return 0;
}

static const codec_profile_class_t codec_profile_nvmpi_h264_class = {
    {
        .ic_super      = (idclass_t *)&codec_profile_nvmpi_class,
        .ic_class      = "codec_profile_nvmpi_h264",
        .ic_caption    = N_("nvmpi_h264"),
        .ic_properties = (const property_t[]){
            {
                .type     = PT_INT,
                .id       = "level",
                .name     = N_("Level"),
                .group    = 4,
                .desc     = N_("Override the preset level."),
                .opts     = PO_EXPERT,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, level),
                .list     = codec_profile_nvmpi_class_level_list_h264,
                .def.i    = NVMPI_LEVEL_AUTOSELECT,
            },
            {}
        }
    },
    .open = (codec_profile_open_meth)tvh_codec_profile_nvmpi_h264_open,
};

TVHVideoCodec tvh_codec_nvmpi_h264 = {
    .name     = "h264_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_h264_class,
    .profiles = nvmpi_h264_profiles,
    .profile_init = tvh_codec_profile_video_init,
    .profile_destroy = tvh_codec_profile_video_destroy,
};

/* h265/hevc_vaapi =============================================================== */

static const AVProfile nvmpi_hevc_profiles[] = {
    { FF_PROFILE_HEVC_MAIN,    "Main" },
    { FF_PROFILE_HEVC_MAIN_10, "Main 10" },
    { FF_PROFILE_HEVC_REXT,    "Rext" },
    { FF_PROFILE_UNKNOWN },
};

static htsmsg_t *
codec_profile_nvmpi_class_level_list_hevc(void *obj, const char *lang)
{
    static const struct strtab tab[] = {
        {N_("Auto"),	      NVMPI_LEVEL_AUTOSELECT},
        {N_("1.0"),           NVMPI_LEVEL_HEVC_1},
        //{N_("1.0b"),          NVMPI_LEVEL_H264_1b},
        //{N_("1.1"),           NVMPI_LEVEL_HEVC_11},
        //{N_("1.2"),           NVMPI_LEVEL_H264_12},
        //{N_("1.3"),           NVMPI_LEVEL_H264_13},
        {N_("2.0"),           NVMPI_LEVEL_HEVC_2},
        {N_("2.1"),           NVMPI_LEVEL_HEVC_21},
        //{N_("2.2"),           NVMPI_LEVEL_H264_22},
        {N_("3.0"),           NVMPI_LEVEL_HEVC_3},
        {N_("3.1"),           NVMPI_LEVEL_HEVC_31},
        //{N_("3.2"),           NVMPI_LEVEL_H264_32},
        {N_("4.0"),           NVMPI_LEVEL_HEVC_4},
        {N_("4.1"),           NVMPI_LEVEL_HEVC_41},
        //{N_("4.2"),           NVMPI_LEVEL_H264_42},
        {N_("5.0"),           NVMPI_LEVEL_HEVC_5},
        {N_("5.1"),           NVMPI_LEVEL_HEVC_51},
        {N_("5.2"),           NVMPI_LEVEL_HEVC_52},
        {N_("6.0"),           NVMPI_LEVEL_HEVC_6},
        {N_("6.1"),           NVMPI_LEVEL_HEVC_61},
        {N_("6.2"),           NVMPI_LEVEL_HEVC_62},
    };
    return strtab2htsmsg(tab, 1, lang);
}





static int
tvh_codec_profile_nvmpi_hevc_open(tvh_codec_profile_nvmpi_t *self,
                                  AVDictionary **opts)
{
    static const struct strtab profiletab[] = {
        {"main",        NVMPI_HEVC_PROFILE_MAIN},
        {"main10",      NVMPI_HEVC_PROFILE_MAIN_10},
        {"rext",        NVMPI_HEVC_PROFILE_REXT},
    };

    static const struct strtab leveltab[] = {
        {"Auto",	   NVMPI_LEVEL_AUTOSELECT},
        {"1.0",           NVMPI_LEVEL_HEVC_1},
        {"2.0",           NVMPI_LEVEL_HEVC_2},
        {"2.1",           NVMPI_LEVEL_HEVC_21},
        {"3.0",           NVMPI_LEVEL_HEVC_3},
        {"3.1",           NVMPI_LEVEL_HEVC_31},
        {"4.0",           NVMPI_LEVEL_HEVC_4},
        {"4.1",           NVMPI_LEVEL_HEVC_41},
        {"5.0",           NVMPI_LEVEL_HEVC_5},
        {"5.1",           NVMPI_LEVEL_HEVC_51},
        {"5.2",           NVMPI_LEVEL_HEVC_52},
        {"6.0",           NVMPI_LEVEL_HEVC_6},
        {"6.1",           NVMPI_LEVEL_HEVC_61},
        {"6.2",           NVMPI_LEVEL_HEVC_62},
    };

    const char *s;

    if (self->level != NVMPI_LEVEL_AUTOSELECT &&
        (s = val2str(self->level, leveltab)) != NULL) {
        AV_DICT_SET(opts, "level", s, 0);
        }

    if (self->profile != NVMPI_PROFILE_UNKNOWN &&
        (s = val2str(self->profile, profiletab)) != NULL) {
        AV_DICT_SET(opts, "profile", s, 0);
        }
    
    // ------ Set Defaults ---------
    AV_DICT_SET_INT(opts, "qmin", -1, 0);
    AV_DICT_SET_INT(opts, "qmax", -1, 0);
    AV_DICT_SET_INT(opts, "qdiff", -1, 0);
    AV_DICT_SET_INT(opts, "qblur", -1, 0);
    AV_DICT_SET_INT(opts, "qcomp", -1, 0);
    AV_DICT_SET_INT(opts, "g", 250, 0);
    AV_DICT_SET_INT(opts, "bf", 0, 0);
    AV_DICT_SET_INT(opts, "refs", 0, 0);
    return 0;
}


static const codec_profile_class_t codec_profile_nvmpi_hevc_class = {
    {
        .ic_super      = (idclass_t *)&codec_profile_nvmpi_class,
        .ic_class      = "codec_profile_nvmpi_hevc",
        .ic_caption    = N_("nvmpi_hevc"),
        .ic_properties = (const property_t[]){
            {
                .type     = PT_INT,
                .id       = "level",
                .name     = N_("Level"),
                .group    = 4,
                .desc     = N_("Override the preset level."),
                .opts     = PO_EXPERT,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, level),
                .list     = codec_profile_nvmpi_class_level_list_hevc,
                .def.i    = NVMPI_LEVEL_AUTOSELECT,
            },
            {}
        }
    },
    .open = (codec_profile_open_meth)tvh_codec_profile_nvmpi_hevc_open,
};

TVHVideoCodec tvh_codec_nvmpi_hevc = {
    .name     = "hevc_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_hevc_class,
    .profiles = nvmpi_hevc_profiles,
    .profile_init = tvh_codec_profile_video_init,
    .profile_destroy = tvh_codec_profile_video_destroy,
};

/* vp8_vaapi =============================================================== */

static const AVProfile nvmpi_vp8_profiles[] = {
    { FF_PROFILE_UNKNOWN },
};

TVHVideoCodec tvh_codec_nvmpi_vp8 = {
    .name     = "vp8_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_class,
    .profiles = nvmpi_vp8_profiles,
    .profile_init = tvh_codec_profile_video_init,
    .profile_destroy = tvh_codec_profile_video_destroy,
};

/* vp9_vaapi =============================================================== */

static const AVProfile nvmpi_vp9_profiles[] = {
    { FF_PROFILE_UNKNOWN },
};

TVHVideoCodec tvh_codec_nvmpi_vp9 = {
    .name     = "vp9_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_class,
    .profiles = nvmpi_vp9_profiles,
    .profile_init = tvh_codec_profile_video_init,
    .profile_destroy = tvh_codec_profile_video_destroy,
};
