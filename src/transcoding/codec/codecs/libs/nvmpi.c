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


/* nvmpi ====================================================================== */

typedef struct {
    TVHVideoCodecProfile;
    int quality;
    int rc;
    int preset;
    int qmin;
    int qmax;
} tvh_codec_profile_nvmpi_t;


static int
tvh_codec_profile_nvmpi_open(tvh_codec_profile_nvmpi_t *self, 
                                  AVDictionary **opts)
{
    AV_DICT_SET_FLAGS_GLOBAL_HEADER(opts);
    //AV_DICT_SET_PIX_FMT(opts, self->pix_fmt, AV_PIX_FMT_NONE);

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

static const codec_profile_class_t codec_profile_nvmpi_class = {
    {
        .ic_super   = (idclass_t *)&codec_profile_video_class,
        .ic_class   = "codec_profile_nvmpi",
        .ic_caption = N_("nvmpi"),
        .ic_properties = (const property_t[]){
            {
                .type     = PT_DBL,
                .id       = "bitrate",
                .name     = N_("Bitrate (kb/s) (0=auto)"),
                .desc     = N_("Constant bitrate (CBR) mode."),
                .group    = 3,
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(TVHCodecProfile, bit_rate),
                .def.d    = 0,
            },
            //{
            //    .type     = PT_STR,
            //    .id       = "nvmpi_libname",
            //    .name     = N_("Library name"),
            //    .desc     = N_("NVMPI library name."),
            //    .group    = 5,
            //    .opts     = PO_EXPERT,
            //    .get_opts = codec_profile_class_get_opts,
            //    .off      = offsetof(tvh_codec_profile_nvmpi_t, libname),
            //},
            //{
            //    .type     = PT_STR,
            //    .id       = "nvmpi_libprefix",
            //    .name     = N_("Library prefix"),
            //    .desc     = N_("NVMPI library prefix."),
            //    .group    = 5,
            //    .opts     = PO_EXPERT,
            //    .get_opts = codec_profile_class_get_opts,
            //    .off      = offsetof(tvh_codec_profile_nvmpi_t, libprefix),
            //},
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
                .off      = offsetof(tvh_codec_profile_nvmpi_t, rc),
            },
            {
                .type     = PT_INT,
                .id       = "qmin",     // Don't change
                .name     = N_("Minimum QP"),
                .group    = 5,
                .desc     = N_("Minimum QP of P frames (from 0 to 52, 0=skip)"),
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, qmin),
                .intextra = INTEXTRA_RANGE(0, 52, 1),
                .def.i    = 0,
            },
            {
                .type     = PT_INT,
                .id       = "qmax",     // Don't change
                .name     = N_("Maximum QP"),
                .group    = 5,
                .desc     = N_("Maximum QP of P frames (from 0 to 52, 0=skip)"),
                .get_opts = codec_profile_class_get_opts,
                .off      = offsetof(tvh_codec_profile_nvmpi_t, qmax),
                .intextra = INTEXTRA_RANGE(0, 52, 1),
                .def.i    = 0,
            },
            {}
        }
    },
    .open = (codec_profile_open_meth)tvh_codec_profile_nvmpi_open,
};

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
    { FF_PROFILE_H264_CONSTRAINED_BASELINE, "Constrained Baseline" },
    { FF_PROFILE_H264_MAIN,                 "Main" },
    { FF_PROFILE_H264_HIGH,                 "High" },
    { FF_PROFILE_UNKNOWN },
};

TVHVideoCodec tvh_codec_nvmpi_h264 = {
    .name     = "h264_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_class,
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

TVHVideoCodec tvh_codec_nvmpi_hevc = {
    .name     = "hevc_nvmpi",
    .size     = sizeof(tvh_codec_profile_nvmpi_t),
    .idclass  = &codec_profile_nvmpi_class,
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
