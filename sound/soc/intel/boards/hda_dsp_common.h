/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright(c) 2019 Intel Corporation.
 */

/*
 * This file defines helper functions used by multiple
 * Intel HDA based machine drivers.
 */

#ifndef __HDA_DSP_COMMON_H
#define __HDA_DSP_COMMON_H

#include <sound/hda_codec.h>
#include "../../codecs/hdac_hda.h"

/*
 * Search card topology and return PCM device number
 * matching Nth HDMI device (zero-based index).
 */
static inline struct snd_pcm *hda_dsp_hdmi_pcm_handle(struct snd_soc_card *card,
						      int hdmi_idx)
{
	struct snd_soc_pcm_runtime *rtd;
	struct snd_pcm *spcm;
	int i = 0;

	for_each_card_rtds(card, rtd) {
		spcm = rtd->pcm ?
			rtd->pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].pcm : 0;
		if (spcm && strstr(spcm->id, "HDMI")) {
			if (i == hdmi_idx)
				return rtd->pcm;
			++i;
		}
	}

	return 0;
}

#if IS_ENABLED(CONFIG_SND_SOC_SOF_HDA_AUDIO_CODEC)
/*
 * Search card topology and register HDMI PCM related controls
 * to codec driver.
 */
static inline int hda_dsp_hdmi_build_controls(struct snd_soc_card *card,
					      struct snd_soc_component *comp)
{
	struct hdac_hda_priv *hda_pvt;
	struct hda_codec *hcodec;
	struct snd_pcm *spcm;
	struct hda_pcm *hpcm;
	int err = 0, i = 0;

	if (!comp)
		return -EINVAL;

	hda_pvt = snd_soc_component_get_drvdata(comp);
	hcodec = &hda_pvt->codec;

	list_for_each_entry(hpcm, &hcodec->pcm_list_head, list) {
		spcm = hda_dsp_hdmi_pcm_handle(card, i);
		if (spcm) {
			hpcm->pcm = spcm;
			hpcm->device = spcm->device;
			dev_dbg(card->dev,
				"%s: mapping HDMI converter %d to PCM %d (%p)\n",
				__func__, i, hpcm->device, spcm);
		} else {
			hpcm->pcm = 0;
			hpcm->device = SNDRV_PCM_INVALID_DEVICE;
			dev_warn(card->dev,
				 "%s: no PCM in topology for HDMI converter %d\n\n",
				 __func__, i);
		}
		i++;
	}

	err = snd_hda_codec_build_controls(hcodec);
	if (err < 0)
		dev_err(card->dev, "unable to create controls %d\n", err);

	return err;
}
#else
static inline int hda_dsp_hdmi_build_controls(struct snd_soc_card *card,
					      struct snd_soc_component *comp)
{
	return -EINVAL;
}
#endif

#endif /* __HDA_DSP_COMMON_H */
