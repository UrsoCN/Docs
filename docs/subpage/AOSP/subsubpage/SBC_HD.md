# Bluetooth SBC Dual Channel HD audio mode

> origin patch from Lineageos: <https://review.lineageos.org/q/topic:%22bt-sbc-hd-dualchannel%22>  
> introduction from Lineageos: <https://www.lineageos.org/engineering/Bluetooth-SBC-XQ/>  
> third-party introduction(in Chinese): <https://techie-s.work/posts/2023/01/lineage-bluetooth-sbc-xq/>  
> posts from XDA: <https://xdaforums.com/t/improve-bluetooth-audio-quality-on-headphones-without-aptx-or-ldac.3832615/>  
> report of soundexpert: <http://soundexpert.org/articles/-/blogs/audio-quality-of-sbc-xq-bluetooth-audio-codec>  
> magisk module of SBC XQ: <https://xdaforums.com/t/magisk-module-sbc-xq-bluetooth-audio-codec-and-drc-deactivator-android-10.4332799/>

## Feasibility Analysis

skip

## on develop

1. CHANNEL_MODE_DUAL_CHANNEL

Add btav_a2dp_codec_channel_mode_t.BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL

``` diff
diff --git a/system/include/hardware/bt_av.h b/system/include/hardware/bt_av.h
index 61dbe50..6888a0e 100644
--- a/system/include/hardware/bt_av.h
+++ b/system/include/hardware/bt_av.h
@@ -117,7 +117,8 @@ typedef enum {
 typedef enum {
   BTAV_A2DP_CODEC_CHANNEL_MODE_NONE = 0x0,
   BTAV_A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0,
-  BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1
+  BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1,
+  BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL = 0x1 << 2
 } btav_a2dp_codec_channel_mode_t;
 
 typedef enum {
```

2. fit for CHANNEL_MODE_DUAL_CHANNEL

> In audio encoding, channel mode refers to how audio channels are treated during the compression process. Different channel modes are used to encode and transmit audio signals in various ways. Here's a brief explanation of the differences between MONO, DUAL, STEREO, and JOINT channel modes:
>
> MONO (Monophonic):  
> In MONO mode, the audio signal is encoded as a single channel. This means that the same audio information is sent to both left and right speakers. MONO is commonly used for scenarios where stereo separation is not crucial, such as in radio broadcasting or some low-bandwidth applications.
>
> DUAL (Dual Channel):  
> DUAL mode typically involves encoding two separate audio channels independently. Each channel is often a separate MONO signal. DUAL is not as common as other modes and may not be widely supported.
>
> STEREO:  
> In STEREO mode, the audio signal is encoded into two separate channels – one for the left speaker and one for the right speaker. This provides a sense of directionality and spatial separation in the audio, allowing for a more immersive listening experience. STEREO is the most common channel mode for music and general audio playback.
>
> JOINT (Joint Stereo):  
> JOINT mode is a more advanced stereo encoding technique. It combines elements of MONO and STEREO to optimize compression efficiency. In JOINT mode, certain audio components that are common to both the left and right channels are encoded only once, saving on file size. This mode is commonly used in modern audio compression formats like MP3.  
>
> The choice of channel mode depends on the specific requirements of the application, the desired audio quality, and the available bandwidth. MONO is more bandwidth-efficient but lacks stereo separation, while STEREO provides a richer listening experience but requires more data. JOINT stereo attempts to strike a balance between efficiency and quality by intelligently encoding shared audio components.  
> When encoding audio files or streaming audio, the choice of channel mode can be an important consideration based on factors such as file size, bandwidth constraints, and the desired listening experience.

``` diff
diff --git a/system/audio_a2dp_hw/src/audio_a2dp_hw.cc b/system/audio_a2dp_hw/src/audio_a2dp_hw.cc
index cfa30fd..7f42113 100644
--- a/system/audio_a2dp_hw/src/audio_a2dp_hw.cc
+++ b/system/audio_a2dp_hw/src/audio_a2dp_hw.cc
@@ -620,6 +620,7 @@ static int a2dp_read_output_audio_config(
       stream_config.is_stereo_to_mono = true;
       break;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
       stream_config.channel_mask = AUDIO_CHANNEL_OUT_STEREO;
       stream_config.is_stereo_to_mono = false;
       break;
@@ -1084,6 +1085,7 @@ size_t audio_a2dp_hw_stream_compute_buffer_size(
       number_of_channels = 1;
       break;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
       number_of_channels = 2;
       break;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
@@ -1295,7 +1297,8 @@ static char* out_get_parameters(const struct audio_stream* stream,
       if (!param.empty()) param += "|";
       param += "AUDIO_CHANNEL_OUT_MONO";
     }
-    if (codec_capability.channel_mode & BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO) {
+    if (codec_capability.channel_mode &
+            (BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO | BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL)) {
       if (!param.empty()) param += "|";
       param += "AUDIO_CHANNEL_OUT_STEREO";
     }
diff --git a/system/audio_a2dp_hw/test/audio_a2dp_hw_test.cc b/system/audio_a2dp_hw/test/audio_a2dp_hw_test.cc
index 8fcbae5..ce64e6e 100644
--- a/system/audio_a2dp_hw/test/audio_a2dp_hw_test.cc
+++ b/system/audio_a2dp_hw/test/audio_a2dp_hw_test.cc
@@ -67,6 +67,7 @@ static uint32_t codec_channel_mode2value(
     case BTAV_A2DP_CODEC_CHANNEL_MODE_MONO:
       return 1;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
       return 2;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
diff --git a/system/stack/a2dp/a2dp_aac.cc b/system/stack/a2dp/a2dp_aac.cc
index 87e9cb4..5802499 100644
--- a/system/stack/a2dp/a2dp_aac.cc
+++ b/system/stack/a2dp/a2dp_aac.cc
@@ -1002,6 +1002,8 @@ static bool select_audio_channel_mode(
         return true;
       }
       break;
+    // BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL is only use for SBC, so skip it
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
   }
@@ -1281,6 +1283,8 @@ bool A2dpCodecConfigAacBase::setCodecConfig(const uint8_t* p_peer_codec_info,
         codec_config_.channel_mode = codec_user_config_.channel_mode;
       }
       break;
+    // BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL is only use for SBC, so skip it
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
diff --git a/system/stack/a2dp/a2dp_codec_config.cc b/system/stack/a2dp/a2dp_codec_config.cc
index 611531c..2c71dae 100644
--- a/system/stack/a2dp/a2dp_codec_config.cc
+++ b/system/stack/a2dp/a2dp_codec_config.cc
@@ -355,6 +355,10 @@ bool A2dpCodecConfig::setCodecUserConfig(
     uint8_t* p_result_codec_config, bool* p_restart_input,
     bool* p_restart_output, bool* p_config_updated) {
   std::lock_guard<std::recursive_mutex> lock(codec_mutex_);
+  // maybe it is not necessary
+  auto stereo_dualchannel_inv_mask =
+      ~(BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL |
+        BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO);
   *p_restart_input = false;
   *p_restart_output = false;
   *p_config_updated = false;
@@ -385,7 +389,10 @@ bool A2dpCodecConfig::setCodecUserConfig(
   if ((saved_codec_config.sample_rate != new_codec_config.sample_rate) ||
       (saved_codec_config.bits_per_sample !=
        new_codec_config.bits_per_sample) ||
-      (saved_codec_config.channel_mode != new_codec_config.channel_mode)) {
+       // maybe it is not necessary
+      ((saved_codec_config.channel_mode != new_codec_config.channel_mode) &&
+       (saved_codec_config.channel_mode & stereo_dualchannel_inv_mask) !=
+       (new_codec_config.channel_mode & stereo_dualchannel_inv_mask))) {
     *p_restart_input = true;
   }
 
@@ -501,6 +508,10 @@ std::string A2dpCodecConfig::codecChannelMode2Str(
     if (!result.empty()) result += "|";
     result += "STEREO";
   }
+  if (codec_channel_mode & BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO) {
+    if (!result.empty()) result += "|";
+    result += "DUALCHANNEL";
+  }
   if (result.empty()) {
     std::stringstream ss;
     ss << "UnknownChannelMode(0x" << std::hex << codec_channel_mode << ")";
diff --git a/system/stack/a2dp/a2dp_sbc.cc b/system/stack/a2dp/a2dp_sbc.cc
index 5a95099..c3c8e4f 100644
--- a/system/stack/a2dp/a2dp_sbc.cc
+++ b/system/stack/a2dp/a2dp_sbc.cc
@@ -55,7 +55,7 @@ typedef struct {
 /* SBC Source codec capabilities */
 static const tA2DP_SBC_CIE a2dp_sbc_source_caps = {
     (A2DP_SBC_IE_SAMP_FREQ_44),                         /* samp_freq */
-    (A2DP_SBC_IE_CH_MD_MONO | A2DP_SBC_IE_CH_MD_JOINT), /* ch_mode */
+    (A2DP_SBC_IE_CH_MD_MONO | A2DP_SBC_IE_CH_MD_JOINT | A2DP_SBC_IE_CH_MD_DUAL), /* ch_mode */
     (A2DP_SBC_IE_BLOCKS_16 | A2DP_SBC_IE_BLOCKS_12 | A2DP_SBC_IE_BLOCKS_8 |
      A2DP_SBC_IE_BLOCKS_4),            /* block_len */
     A2DP_SBC_IE_SUBBAND_8,             /* num_subbands */
@@ -858,10 +858,10 @@ UNUSED_ATTR static void build_codec_config(const tA2DP_SBC_CIE& config_cie,
   if (config_cie.ch_mode & A2DP_SBC_IE_CH_MD_MONO)
     result->channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
 
-  if (config_cie.ch_mode & (A2DP_SBC_IE_CH_MD_STEREO | A2DP_SBC_IE_CH_MD_JOINT |
-                            A2DP_SBC_IE_CH_MD_DUAL)) {
+  if (config_cie.ch_mode & (A2DP_SBC_IE_CH_MD_STEREO | A2DP_SBC_IE_CH_MD_JOINT))
     result->channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
-  }
+  if (config_cie.ch_mode & A2DP_SBC_IE_CH_MD_DUAL)
+    result->channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
 }
 
 A2dpCodecConfigSbcSource::A2dpCodecConfigSbcSource(
@@ -887,7 +887,7 @@ A2dpCodecConfigSbcSource::A2dpCodecConfigSbcSource(
     codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
   }
   if (a2dp_sbc_source_caps.ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
-    codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
+    codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
   }
 }
 
@@ -1014,7 +1014,7 @@ static bool select_best_channel_mode(uint8_t ch_mode, tA2DP_SBC_CIE* p_result,
   }
   if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
     p_result->ch_mode = A2DP_SBC_IE_CH_MD_DUAL;
-    p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
+    p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
     return true;
   }
   if (ch_mode & A2DP_SBC_IE_CH_MD_MONO) {
@@ -1055,7 +1055,14 @@ static bool select_audio_channel_mode(
       }
       if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
         p_result->ch_mode = A2DP_SBC_IE_CH_MD_DUAL;
-        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
+        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
+        return true;
+      }
+      break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
+      if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
+        p_result->ch_mode = A2DP_SBC_IE_CH_MD_DUAL;
+        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
         return true;
       }
       break;
@@ -1295,6 +1302,13 @@ bool A2dpCodecConfigSbcBase::setCodecConfig(const uint8_t* p_peer_codec_info,
         break;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
+      if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
+        result_config_cie.ch_mode = A2DP_SBC_IE_CH_MD_DUAL;
+        codec_capability_.channel_mode = codec_user_config_.channel_mode;
+        codec_config_.channel_mode = codec_user_config_.channel_mode;
+      }
+      break;
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
@@ -1318,7 +1332,7 @@ bool A2dpCodecConfigSbcBase::setCodecConfig(const uint8_t* p_peer_codec_info,
     }
     if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
       codec_selectable_capability_.channel_mode |=
-          BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
+          BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
     }
 
     if (codec_config_.channel_mode != BTAV_A2DP_CODEC_CHANNEL_MODE_NONE) break;
@@ -1529,7 +1543,7 @@ bool A2dpCodecConfigSbcBase::setPeerCodecCapabilities(
   }
   if (ch_mode & A2DP_SBC_IE_CH_MD_DUAL) {
     codec_selectable_capability_.channel_mode |=
-        BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
+        BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL;
   }
 
   status = A2DP_BuildInfoSbc(AVDT_MEDIA_TYPE_AUDIO, &peer_info_cie,
diff --git a/system/stack/a2dp/a2dp_sbc_encoder.cc b/system/stack/a2dp/a2dp_sbc_encoder.cc
index 6bd6838..340edc7 100644
--- a/system/stack/a2dp/a2dp_sbc_encoder.cc
+++ b/system/stack/a2dp/a2dp_sbc_encoder.cc
@@ -43,6 +43,14 @@
 /* High quality quality setting @ 44.1 khz */
 #define A2DP_SBC_DEFAULT_BITRATE 328
 
+/*
+ * SBC Dual Channel (SBC HD) 3DH5 bitrates.
+ * 600 kbps @ 48 khz, 551.3 kbps @ 44.1 khz.
+ * Up to 5 frames for 3DH5.
+ */
+#define A2DP_SBC_3DH5_DEFAULT_BITRATE 552
+#define A2DP_SBC_3DH5_48KHZ_BITRATE 601
+
 #define A2DP_SBC_NON_EDR_MAX_RATE 229
 
 #define A2DP_SBC_MAX_PCM_ITER_NUM_PER_TICK 3
@@ -801,7 +809,12 @@ static uint8_t calculate_max_frames_per_packet(void) {
 
 static uint16_t a2dp_sbc_source_rate(bool is_peer_edr) {
   uint16_t rate = A2DP_SBC_DEFAULT_BITRATE;
-
+  if (a2dp_sbc_encoder_cb.peer_params.peer_supports_3mbps &&
+      a2dp_sbc_encoder_cb.TxAaMtuSize >= MIN_3MBPS_AVDTP_MTU) {
+    rate = A2DP_SBC_3DH5_DEFAULT_BITRATE;
+    if (a2dp_sbc_encoder_cb.sbc_encoder_params.s16SamplingFreq == SBC_sf48000)
+      rate = A2DP_SBC_3DH5_48KHZ_BITRATE;
+  }
   /* restrict bitrate if a2dp link is non-edr */
   if (!is_peer_edr) {
     rate = A2DP_SBC_NON_EDR_MAX_RATE;
diff --git a/system/stack/a2dp/a2dp_vendor_aptx.cc b/system/stack/a2dp/a2dp_vendor_aptx.cc
index e4dae75..1c75b3f 100644
--- a/system/stack/a2dp/a2dp_vendor_aptx.cc
+++ b/system/stack/a2dp/a2dp_vendor_aptx.cc
@@ -604,6 +604,7 @@ static bool select_audio_channel_mode(
         return true;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
   }
@@ -803,6 +804,7 @@ bool A2dpCodecConfigAptx::setCodecConfig(const uint8_t* p_peer_codec_info,
         codec_config_.channel_mode = codec_user_config_.channel_mode;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
diff --git a/system/stack/a2dp/a2dp_vendor_aptx_hd.cc b/system/stack/a2dp/a2dp_vendor_aptx_hd.cc
index 3a4bba2..e716f04 100644
--- a/system/stack/a2dp/a2dp_vendor_aptx_hd.cc
+++ b/system/stack/a2dp/a2dp_vendor_aptx_hd.cc
@@ -622,6 +622,7 @@ static bool select_audio_channel_mode(
         return true;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
   }
@@ -823,6 +824,7 @@ bool A2dpCodecConfigAptxHd::setCodecConfig(const uint8_t* p_peer_codec_info,
         codec_config_.channel_mode = codec_user_config_.channel_mode;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
diff --git a/system/stack/a2dp/a2dp_vendor_ldac.cc b/system/stack/a2dp/a2dp_vendor_ldac.cc
index e8430ff..9b59cb5 100644
--- a/system/stack/a2dp/a2dp_vendor_ldac.cc
+++ b/system/stack/a2dp/a2dp_vendor_ldac.cc
@@ -938,6 +938,7 @@ static bool select_audio_channel_mode(
         return true;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
   }
@@ -1218,6 +1219,7 @@ bool A2dpCodecConfigLdacBase::setCodecConfig(const uint8_t* p_peer_codec_info,
         break;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
diff --git a/system/stack/a2dp/a2dp_vendor_opus.cc b/system/stack/a2dp/a2dp_vendor_opus.cc
index 8390035..5e4946d 100644
--- a/system/stack/a2dp/a2dp_vendor_opus.cc
+++ b/system/stack/a2dp/a2dp_vendor_opus.cc
@@ -883,6 +883,7 @@ static bool select_audio_channel_mode(
         return true;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       break;
   }
@@ -1105,6 +1106,7 @@ bool A2dpCodecConfigOpusBase::setCodecConfig(const uint8_t* p_peer_codec_info,
         codec_config_.channel_mode = codec_user_config_.channel_mode;
       }
       break;
+    case BTAV_A2DP_CODEC_CHANNEL_MODE_DUAL_CHANNEL:
     case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
       codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
       codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
diff --git a/system/stack/include/avdt_api.h b/system/stack/include/avdt_api.h
index f051b09..dc84195 100644
--- a/system/stack/include/avdt_api.h
+++ b/system/stack/include/avdt_api.h
@@ -104,6 +104,11 @@ inline std::string avdt_result_text(const tAVDT_RESULT& result) {
 */
 #define AVDT_MARKER_SET 0x80
 
+/*
+ * 3DH5 minimum safe payload size for 4 audio frames of:
+ * 817 bytes - (4 bytes L2CAP Header + 12 bytes AVDTP Header)
+ */
+#define MIN_3MBPS_AVDTP_MTU 801   // TODO: consider if it is accurate
 #define MAX_2MBPS_AVDTP_MTU 663   // 2DH5 MTU=679, -12 for AVDTP, -4 for L2CAP
 #define MAX_3MBPS_AVDTP_MTU 1005  // 3DH5 MTU=1021, -12 for AVDTP, -4 for L2CAP
```

3. Allow using alternative (higher) SBC HD bitrates with a property

``` diff
diff --git a/system/stack/a2dp/a2dp_sbc_encoder.cc b/system/stack/a2dp/a2dp_sbc_encoder.cc
index 340edc7..50af3d2 100644
--- a/system/stack/a2dp/a2dp_sbc_encoder.cc
+++ b/system/stack/a2dp/a2dp_sbc_encoder.cc
@@ -32,6 +32,7 @@
 #include "osi/include/allocator.h"
 #include "osi/include/log.h"
 #include "osi/include/osi.h"
+#include "osi/include/properties.h"
 #include "stack/include/bt_hdr.h"
 
 /* Buffer pool */
@@ -51,6 +52,17 @@
 #define A2DP_SBC_3DH5_DEFAULT_BITRATE 552
 #define A2DP_SBC_3DH5_48KHZ_BITRATE 601
 
+/*
+ * SBC Dual Channel (SBC HD) 2DH5 alternative bitrates.
+ * 648 kbps @ 48 khz, 595.4 kbps @ 44.1 khz.
+ * Up to 3 frames for 2DH5.
+ */
+#define A2DP_SBC_2DH5_ALT_BITRATE 596
+#define A2DP_SBC_2DH5_ALT_48KHZ_BITRATE 649
+
+// SBC HD alternative bitrate property
+#define A2DP_SBC_HD_PROP "persist.bluetooth.sbc_hd_higher_bitrate"
+
 #define A2DP_SBC_NON_EDR_MAX_RATE 229
 
 #define A2DP_SBC_MAX_PCM_ITER_NUM_PER_TICK 3
@@ -809,12 +821,23 @@ static uint8_t calculate_max_frames_per_packet(void) {
 
 static uint16_t a2dp_sbc_source_rate(bool is_peer_edr) {
   uint16_t rate = A2DP_SBC_DEFAULT_BITRATE;
+
+  /* 3DH5 maximum bitrates */
   if (a2dp_sbc_encoder_cb.peer_params.peer_supports_3mbps &&
       a2dp_sbc_encoder_cb.TxAaMtuSize >= MIN_3MBPS_AVDTP_MTU) {
     rate = A2DP_SBC_3DH5_DEFAULT_BITRATE;
     if (a2dp_sbc_encoder_cb.sbc_encoder_params.s16SamplingFreq == SBC_sf48000)
       rate = A2DP_SBC_3DH5_48KHZ_BITRATE;
   }
+
+  /* 2DH5 alternative bitrates */
+  if (!a2dp_sbc_encoder_cb.peer_params.peer_supports_3mbps &&
+      osi_property_get_int32(A2DP_SBC_HD_PROP, 0)) {
+    rate = A2DP_SBC_2DH5_ALT_BITRATE;
+    if (a2dp_sbc_encoder_cb.sbc_encoder_params.s16SamplingFreq == SBC_sf48000)
+      rate = A2DP_SBC_2DH5_ALT_48KHZ_BITRATE;
+  }
+
   /* restrict bitrate if a2dp link is non-edr */
   if (!is_peer_edr) {
     rate = A2DP_SBC_NON_EDR_MAX_RATE;
```

4. Add Dual Channel into Bluetooth Audio Channel Mode developer options menu

IN frameworks/base/

``` diff
diff --git a/packages/SettingsLib/res/values/arrays.xml b/packages/SettingsLib/res/values/arrays.xml
index ef664e99..acbb09ed 100644
--- a/packages/SettingsLib/res/values/arrays.xml
+++ b/packages/SettingsLib/res/values/arrays.xml
@@ -238,6 +238,7 @@
         <item>Use System Selection (Default)</item>
         <item>Mono</item>
         <item>Stereo</item>
+        <item>Dual</item>
     </string-array>
 
     <!-- Values for Bluetooth Audio Codec Channel Mode selection preference. -->
@@ -245,6 +246,7 @@
         <item>0</item>
         <item>1</item>
         <item>2</item>
+        <item>3</item>
     </string-array>
 
     <!-- Summaries for Bluetooth Audio Codec Channel Mode selection preference. [CHAR LIMIT=50]-->
@@ -252,6 +254,7 @@
         <item>Use System Selection (Default)</item>
         <item>Mono</item>
         <item>Stereo</item>
+        <item>Dual</item>
     </string-array>
 
     <!-- Titles for Bluetooth Audio Codec LDAC Playback Quality selection preference. [CHAR LIMIT=70] -->
```

IN packages/apps/Settings

``` diff
diff --git a/res/layout/bluetooth_audio_channel_mode_dialog.xml b/res/layout/bluetooth_audio_channel_mode_dialog.xml
index 9154ee4..ff7c689 100644
--- a/res/layout/bluetooth_audio_channel_mode_dialog.xml
+++ b/res/layout/bluetooth_audio_channel_mode_dialog.xml
@@ -42,6 +42,10 @@
             <include
                 android:id="@+id/bluetooth_audio_channel_mode_stereo"
                 layout="@layout/preference_widget_dialog_radiobutton"/>
+
+            <include
+                android:id="@+id/bluetooth_audio_channel_mode_dual"
+                layout="@layout/preference_widget_dialog_radiobutton" />
         </RadioGroup>
 
         <include
diff --git a/src/com/android/settings/development/bluetooth/AbstractBluetoothDialogPreferenceController.java b/src/com/android/settings/development/bluetooth/AbstractBluetoothDialogPreferenceController.java
index c9cc2b7..94d65a4 100644
--- a/src/com/android/settings/development/bluetooth/AbstractBluetoothDialogPreferenceController.java
+++ b/src/com/android/settings/development/bluetooth/AbstractBluetoothDialogPreferenceController.java
@@ -59,7 +59,8 @@ public abstract class AbstractBluetoothDialogPreferenceController extends
             BluetoothCodecConfig.BITS_PER_SAMPLE_24,
             BluetoothCodecConfig.BITS_PER_SAMPLE_16};
     protected static final int[] CHANNEL_MODES = {BluetoothCodecConfig.CHANNEL_MODE_STEREO,
-            BluetoothCodecConfig.CHANNEL_MODE_MONO};
+            BluetoothCodecConfig.CHANNEL_MODE_MONO,
+            BluetoothCodecConfig.CHANNEL_MODE_DUAL};
 
     protected final BluetoothA2dpConfigStore mBluetoothA2dpConfigStore;
 
diff --git a/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreference.java b/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreference.java
index a878f32..5657ed8 100644
--- a/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreference.java
+++ b/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreference.java
@@ -59,6 +59,7 @@ public class BluetoothChannelModeDialogPreference extends BaseBluetoothDialogPre
         mRadioButtonIds.add(R.id.bluetooth_audio_channel_mode_default);
         mRadioButtonIds.add(R.id.bluetooth_audio_channel_mode_mono);
         mRadioButtonIds.add(R.id.bluetooth_audio_channel_mode_stereo);
+        mRadioButtonIds.add(R.id.bluetooth_audio_channel_mode_dual);
         String[] stringArray = context.getResources().getStringArray(
                 R.array.bluetooth_a2dp_codec_channel_mode_titles);
         for (int i = 0; i < stringArray.length; i++) {
diff --git a/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreferenceController.java b/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreferenceController.java
index d156abd..556a79d 100644
--- a/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreferenceController.java
+++ b/src/com/android/settings/development/bluetooth/BluetoothChannelModeDialogPreferenceController.java
@@ -71,6 +71,8 @@ public class BluetoothChannelModeDialogPreferenceController extends
             case 2:
                 channelModeValue = BluetoothCodecConfig.CHANNEL_MODE_STEREO;
                 break;
+            case 3:
+                channelModeValue = BluetoothCodecConfig.CHANNEL_MODE_DUAL;
             default:
                 break;
         }
@@ -112,6 +114,9 @@ public class BluetoothChannelModeDialogPreferenceController extends
             case BluetoothCodecConfig.CHANNEL_MODE_STEREO:
                 index = 2;
                 break;
+            case BluetoothCodecConfig.CHANNEL_MODE_DUAL:
+                index = 3;
+                break;
             default:
                 Log.e(TAG, "Unsupported config:" + config);
                 break;
```

5. SBC Dual Channel (SBC HD Audio) support



|2023年11月26日|
|--:|
