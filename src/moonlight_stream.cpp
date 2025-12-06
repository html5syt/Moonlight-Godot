#include "moonlight_stream.h"
#include "lib/moonlight-common-c/src/Limelight.h"


void LgSTREAM_CONFIGURATION::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_width", "value"), &LgSTREAM_CONFIGURATION::set_width);
    ClassDB::bind_method(D_METHOD("get_width"), &LgSTREAM_CONFIGURATION::get_width);
    ClassDB::bind_method(D_METHOD("set_height", "value"), &LgSTREAM_CONFIGURATION::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &LgSTREAM_CONFIGURATION::get_height);
    ClassDB::bind_method(D_METHOD("set_fps", "value"), &LgSTREAM_CONFIGURATION::set_fps);
    ClassDB::bind_method(D_METHOD("get_fps"), &LgSTREAM_CONFIGURATION::get_fps);
    ClassDB::bind_method(D_METHOD("set_bitrate", "value"), &LgSTREAM_CONFIGURATION::set_bitrate);
    ClassDB::bind_method(D_METHOD("get_bitrate"), &LgSTREAM_CONFIGURATION::get_bitrate);
    ClassDB::bind_method(D_METHOD("set_packetSize", "value"), &LgSTREAM_CONFIGURATION::set_packetSize);
    ClassDB::bind_method(D_METHOD("get_packetSize"), &LgSTREAM_CONFIGURATION::get_packetSize);
    ClassDB::bind_method(D_METHOD("set_streamingRemotely", "value"), &LgSTREAM_CONFIGURATION::set_streamingRemotely);
    ClassDB::bind_method(D_METHOD("get_streamingRemotely"), &LgSTREAM_CONFIGURATION::get_streamingRemotely);
    ClassDB::bind_method(D_METHOD("set_audioConfiguration", "value"), &LgSTREAM_CONFIGURATION::set_audioConfiguration);
    ClassDB::bind_method(D_METHOD("get_audioConfiguration"), &LgSTREAM_CONFIGURATION::get_audioConfiguration);
    ClassDB::bind_method(D_METHOD("set_supportedVideoFormats", "value"), &LgSTREAM_CONFIGURATION::set_supportedVideoFormats);
    ClassDB::bind_method(D_METHOD("get_supportedVideoFormats"), &LgSTREAM_CONFIGURATION::get_supportedVideoFormats);
    ClassDB::bind_method(D_METHOD("set_clientRefreshRateX100", "value"), &LgSTREAM_CONFIGURATION::set_clientRefreshRateX100);
    ClassDB::bind_method(D_METHOD("get_clientRefreshRateX100"), &LgSTREAM_CONFIGURATION::get_clientRefreshRateX100);
    ClassDB::bind_method(D_METHOD("set_colorSpace", "value"), &LgSTREAM_CONFIGURATION::set_colorSpace);
    ClassDB::bind_method(D_METHOD("get_colorSpace"), &LgSTREAM_CONFIGURATION::get_colorSpace);
    ClassDB::bind_method(D_METHOD("set_colorRange", "value"), &LgSTREAM_CONFIGURATION::set_colorRange);
    ClassDB::bind_method(D_METHOD("get_colorRange"), &LgSTREAM_CONFIGURATION::get_colorRange);
    ClassDB::bind_method(D_METHOD("set_encryptionFlags", "value"), &LgSTREAM_CONFIGURATION::set_encryptionFlags);
    ClassDB::bind_method(D_METHOD("get_encryptionFlags"), &LgSTREAM_CONFIGURATION::get_encryptionFlags);
    ClassDB::bind_method(D_METHOD("set_remoteInputAesKey", "value"), &LgSTREAM_CONFIGURATION::set_remoteInputAesKey);
    ClassDB::bind_method(D_METHOD("get_remoteInputAesKey"), &LgSTREAM_CONFIGURATION::get_remoteInputAesKey);
    ClassDB::bind_method(D_METHOD("set_remoteInputAesIv", "value"), &LgSTREAM_CONFIGURATION::set_remoteInputAesIv);
    ClassDB::bind_method(D_METHOD("get_remoteInputAesIv"), &LgSTREAM_CONFIGURATION::get_remoteInputAesIv);
}
LgSTREAM_CONFIGURATION::LgSTREAM_CONFIGURATION() {
    width = 0;
    height = 0;
    fps = 0;
    bitrate = 0;
    packetSize = 0;
    streamingRemotely = 0;
    audioConfiguration = 0;
    supportedVideoFormats = 0;
    clientRefreshRateX100 = 0;
    colorSpace = 0;
    colorRange = 0;
    encryptionFlags = 0;
    remoteInputAesKey = String();
    remoteInputAesIv = String();
}


void LgLENTRY::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data", "value"), &LgLENTRY::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &LgLENTRY::get_data);
    ClassDB::bind_method(D_METHOD("set_length", "value"), &LgLENTRY::set_length);
    ClassDB::bind_method(D_METHOD("get_length"), &LgLENTRY::get_length);
    ClassDB::bind_method(D_METHOD("set_bufferType", "value"), &LgLENTRY::set_bufferType);
    ClassDB::bind_method(D_METHOD("get_bufferType"), &LgLENTRY::get_bufferType);
}
LgLENTRY::LgLENTRY() {
    data = String();
    length = 0;
    bufferType = 0;
}


void LgDECODE_UNIT::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_frameNumber", "value"), &LgDECODE_UNIT::set_frameNumber);
    ClassDB::bind_method(D_METHOD("get_frameNumber"), &LgDECODE_UNIT::get_frameNumber);
    ClassDB::bind_method(D_METHOD("set_frameType", "value"), &LgDECODE_UNIT::set_frameType);
    ClassDB::bind_method(D_METHOD("get_frameType"), &LgDECODE_UNIT::get_frameType);
    ClassDB::bind_method(D_METHOD("set_frameHostProcessingLatency", "value"), &LgDECODE_UNIT::set_frameHostProcessingLatency);
    ClassDB::bind_method(D_METHOD("get_frameHostProcessingLatency"), &LgDECODE_UNIT::get_frameHostProcessingLatency);
    ClassDB::bind_method(D_METHOD("set_receiveTimeUs", "value"), &LgDECODE_UNIT::set_receiveTimeUs);
    ClassDB::bind_method(D_METHOD("get_receiveTimeUs"), &LgDECODE_UNIT::get_receiveTimeUs);
    ClassDB::bind_method(D_METHOD("set_enqueueTimeUs", "value"), &LgDECODE_UNIT::set_enqueueTimeUs);
    ClassDB::bind_method(D_METHOD("get_enqueueTimeUs"), &LgDECODE_UNIT::get_enqueueTimeUs);
    ClassDB::bind_method(D_METHOD("set_presentationTimeUs", "value"), &LgDECODE_UNIT::set_presentationTimeUs);
    ClassDB::bind_method(D_METHOD("get_presentationTimeUs"), &LgDECODE_UNIT::get_presentationTimeUs);
    ClassDB::bind_method(D_METHOD("set_rtpTimestamp", "value"), &LgDECODE_UNIT::set_rtpTimestamp);
    ClassDB::bind_method(D_METHOD("get_rtpTimestamp"), &LgDECODE_UNIT::get_rtpTimestamp);
    ClassDB::bind_method(D_METHOD("set_fullLength", "value"), &LgDECODE_UNIT::set_fullLength);
    ClassDB::bind_method(D_METHOD("get_fullLength"), &LgDECODE_UNIT::get_fullLength);
    ClassDB::bind_method(D_METHOD("set_bufferList", "value"), &LgDECODE_UNIT::set_bufferList);
    ClassDB::bind_method(D_METHOD("get_bufferList"), &LgDECODE_UNIT::get_bufferList);
    ClassDB::bind_method(D_METHOD("set_hdrActive", "value"), &LgDECODE_UNIT::set_hdrActive);
    ClassDB::bind_method(D_METHOD("get_hdrActive"), &LgDECODE_UNIT::get_hdrActive);
    ClassDB::bind_method(D_METHOD("set_colorspace", "value"), &LgDECODE_UNIT::set_colorspace);
    ClassDB::bind_method(D_METHOD("get_colorspace"), &LgDECODE_UNIT::get_colorspace);
}
LgDECODE_UNIT::LgDECODE_UNIT() {
    frameNumber = 0;
    frameType = 0;
    // TODO: init frameHostProcessingLatency of type Variant
    // TODO: init receiveTimeUs of type Variant
    // TODO: init enqueueTimeUs of type Variant
    // TODO: init presentationTimeUs of type Variant
    rtpTimestamp = 0;
    fullLength = 0;
    // TODO: init bufferList of type Variant
    hdrActive = false;
    // TODO: init colorspace of type Variant
}


void LgDECODER_RENDERER_CALLBACKS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_setup", "value"), &LgDECODER_RENDERER_CALLBACKS::set_setup);
    ClassDB::bind_method(D_METHOD("get_setup"), &LgDECODER_RENDERER_CALLBACKS::get_setup);
    ClassDB::bind_method(D_METHOD("set_start", "value"), &LgDECODER_RENDERER_CALLBACKS::set_start);
    ClassDB::bind_method(D_METHOD("get_start"), &LgDECODER_RENDERER_CALLBACKS::get_start);
    ClassDB::bind_method(D_METHOD("set_stop", "value"), &LgDECODER_RENDERER_CALLBACKS::set_stop);
    ClassDB::bind_method(D_METHOD("get_stop"), &LgDECODER_RENDERER_CALLBACKS::get_stop);
    ClassDB::bind_method(D_METHOD("set_cleanup", "value"), &LgDECODER_RENDERER_CALLBACKS::set_cleanup);
    ClassDB::bind_method(D_METHOD("get_cleanup"), &LgDECODER_RENDERER_CALLBACKS::get_cleanup);
    ClassDB::bind_method(D_METHOD("set_submitDecodeUnit", "value"), &LgDECODER_RENDERER_CALLBACKS::set_submitDecodeUnit);
    ClassDB::bind_method(D_METHOD("get_submitDecodeUnit"), &LgDECODER_RENDERER_CALLBACKS::get_submitDecodeUnit);
    ClassDB::bind_method(D_METHOD("set_capabilities", "value"), &LgDECODER_RENDERER_CALLBACKS::set_capabilities);
    ClassDB::bind_method(D_METHOD("get_capabilities"), &LgDECODER_RENDERER_CALLBACKS::get_capabilities);
}
LgDECODER_RENDERER_CALLBACKS::LgDECODER_RENDERER_CALLBACKS() {
    // TODO: init setup of type Variant
    // TODO: init start of type Variant
    // TODO: init stop of type Variant
    // TODO: init cleanup of type Variant
    // TODO: init submitDecodeUnit of type Variant
    capabilities = 0;
}


void LgOPUS_MULTISTREAM_CONFIGURATION::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_sampleRate", "value"), &LgOPUS_MULTISTREAM_CONFIGURATION::set_sampleRate);
    ClassDB::bind_method(D_METHOD("get_sampleRate"), &LgOPUS_MULTISTREAM_CONFIGURATION::get_sampleRate);
    ClassDB::bind_method(D_METHOD("set_channelCount", "value"), &LgOPUS_MULTISTREAM_CONFIGURATION::set_channelCount);
    ClassDB::bind_method(D_METHOD("get_channelCount"), &LgOPUS_MULTISTREAM_CONFIGURATION::get_channelCount);
    ClassDB::bind_method(D_METHOD("set_streams", "value"), &LgOPUS_MULTISTREAM_CONFIGURATION::set_streams);
    ClassDB::bind_method(D_METHOD("get_streams"), &LgOPUS_MULTISTREAM_CONFIGURATION::get_streams);
    ClassDB::bind_method(D_METHOD("set_coupledStreams", "value"), &LgOPUS_MULTISTREAM_CONFIGURATION::set_coupledStreams);
    ClassDB::bind_method(D_METHOD("get_coupledStreams"), &LgOPUS_MULTISTREAM_CONFIGURATION::get_coupledStreams);
    ClassDB::bind_method(D_METHOD("set_samplesPerFrame", "value"), &LgOPUS_MULTISTREAM_CONFIGURATION::set_samplesPerFrame);
    ClassDB::bind_method(D_METHOD("get_samplesPerFrame"), &LgOPUS_MULTISTREAM_CONFIGURATION::get_samplesPerFrame);
}
LgOPUS_MULTISTREAM_CONFIGURATION::LgOPUS_MULTISTREAM_CONFIGURATION() {
    sampleRate = 0;
    channelCount = 0;
    streams = 0;
    coupledStreams = 0;
    samplesPerFrame = 0;
}


void LgAUDIO_RENDERER_CALLBACKS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_init", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_init);
    ClassDB::bind_method(D_METHOD("get_init"), &LgAUDIO_RENDERER_CALLBACKS::get_init);
    ClassDB::bind_method(D_METHOD("set_start", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_start);
    ClassDB::bind_method(D_METHOD("get_start"), &LgAUDIO_RENDERER_CALLBACKS::get_start);
    ClassDB::bind_method(D_METHOD("set_stop", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_stop);
    ClassDB::bind_method(D_METHOD("get_stop"), &LgAUDIO_RENDERER_CALLBACKS::get_stop);
    ClassDB::bind_method(D_METHOD("set_cleanup", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_cleanup);
    ClassDB::bind_method(D_METHOD("get_cleanup"), &LgAUDIO_RENDERER_CALLBACKS::get_cleanup);
    ClassDB::bind_method(D_METHOD("set_decodeAndPlaySample", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_decodeAndPlaySample);
    ClassDB::bind_method(D_METHOD("get_decodeAndPlaySample"), &LgAUDIO_RENDERER_CALLBACKS::get_decodeAndPlaySample);
    ClassDB::bind_method(D_METHOD("set_capabilities", "value"), &LgAUDIO_RENDERER_CALLBACKS::set_capabilities);
    ClassDB::bind_method(D_METHOD("get_capabilities"), &LgAUDIO_RENDERER_CALLBACKS::get_capabilities);
}
LgAUDIO_RENDERER_CALLBACKS::LgAUDIO_RENDERER_CALLBACKS() {
    // TODO: init init of type Variant
    // TODO: init start of type Variant
    // TODO: init stop of type Variant
    // TODO: init cleanup of type Variant
    // TODO: init decodeAndPlaySample of type Variant
    capabilities = 0;
}


void LgCONNECTION_LISTENER_CALLBACKS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_stageStarting", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_stageStarting);
    ClassDB::bind_method(D_METHOD("get_stageStarting"), &LgCONNECTION_LISTENER_CALLBACKS::get_stageStarting);
    ClassDB::bind_method(D_METHOD("set_stageComplete", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_stageComplete);
    ClassDB::bind_method(D_METHOD("get_stageComplete"), &LgCONNECTION_LISTENER_CALLBACKS::get_stageComplete);
    ClassDB::bind_method(D_METHOD("set_stageFailed", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_stageFailed);
    ClassDB::bind_method(D_METHOD("get_stageFailed"), &LgCONNECTION_LISTENER_CALLBACKS::get_stageFailed);
    ClassDB::bind_method(D_METHOD("set_connectionStarted", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_connectionStarted);
    ClassDB::bind_method(D_METHOD("get_connectionStarted"), &LgCONNECTION_LISTENER_CALLBACKS::get_connectionStarted);
    ClassDB::bind_method(D_METHOD("set_connectionTerminated", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_connectionTerminated);
    ClassDB::bind_method(D_METHOD("get_connectionTerminated"), &LgCONNECTION_LISTENER_CALLBACKS::get_connectionTerminated);
    ClassDB::bind_method(D_METHOD("set_logMessage", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_logMessage);
    ClassDB::bind_method(D_METHOD("get_logMessage"), &LgCONNECTION_LISTENER_CALLBACKS::get_logMessage);
    ClassDB::bind_method(D_METHOD("set_rumble", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_rumble);
    ClassDB::bind_method(D_METHOD("get_rumble"), &LgCONNECTION_LISTENER_CALLBACKS::get_rumble);
    ClassDB::bind_method(D_METHOD("set_connectionStatusUpdate", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_connectionStatusUpdate);
    ClassDB::bind_method(D_METHOD("get_connectionStatusUpdate"), &LgCONNECTION_LISTENER_CALLBACKS::get_connectionStatusUpdate);
    ClassDB::bind_method(D_METHOD("set_setHdrMode", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_setHdrMode);
    ClassDB::bind_method(D_METHOD("get_setHdrMode"), &LgCONNECTION_LISTENER_CALLBACKS::get_setHdrMode);
    ClassDB::bind_method(D_METHOD("set_rumbleTriggers", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_rumbleTriggers);
    ClassDB::bind_method(D_METHOD("get_rumbleTriggers"), &LgCONNECTION_LISTENER_CALLBACKS::get_rumbleTriggers);
    ClassDB::bind_method(D_METHOD("set_setMotionEventState", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_setMotionEventState);
    ClassDB::bind_method(D_METHOD("get_setMotionEventState"), &LgCONNECTION_LISTENER_CALLBACKS::get_setMotionEventState);
    ClassDB::bind_method(D_METHOD("set_setControllerLED", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_setControllerLED);
    ClassDB::bind_method(D_METHOD("get_setControllerLED"), &LgCONNECTION_LISTENER_CALLBACKS::get_setControllerLED);
    ClassDB::bind_method(D_METHOD("set_setAdaptiveTriggers", "value"), &LgCONNECTION_LISTENER_CALLBACKS::set_setAdaptiveTriggers);
    ClassDB::bind_method(D_METHOD("get_setAdaptiveTriggers"), &LgCONNECTION_LISTENER_CALLBACKS::get_setAdaptiveTriggers);
}
LgCONNECTION_LISTENER_CALLBACKS::LgCONNECTION_LISTENER_CALLBACKS() {
    // TODO: init stageStarting of type Variant
    // TODO: init stageComplete of type Variant
    // TODO: init stageFailed of type Variant
    // TODO: init connectionStarted of type Variant
    // TODO: init connectionTerminated of type Variant
    // TODO: init logMessage of type Variant
    // TODO: init rumble of type Variant
    // TODO: init connectionStatusUpdate of type Variant
    // TODO: init setHdrMode of type Variant
    // TODO: init rumbleTriggers of type Variant
    // TODO: init setMotionEventState of type Variant
    // TODO: init setControllerLED of type Variant
    // TODO: init setAdaptiveTriggers of type Variant
}


void LgSERVER_INFORMATION::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_serverCodecModeSupport", "value"), &LgSERVER_INFORMATION::set_serverCodecModeSupport);
    ClassDB::bind_method(D_METHOD("get_serverCodecModeSupport"), &LgSERVER_INFORMATION::get_serverCodecModeSupport);
}
LgSERVER_INFORMATION::LgSERVER_INFORMATION() {
    serverCodecModeSupport = 0;
}


void LgRTP_AUDIO_STATS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_packetCountAudio", "value"), &LgRTP_AUDIO_STATS::set_packetCountAudio);
    ClassDB::bind_method(D_METHOD("get_packetCountAudio"), &LgRTP_AUDIO_STATS::get_packetCountAudio);
    ClassDB::bind_method(D_METHOD("set_packetCountFec", "value"), &LgRTP_AUDIO_STATS::set_packetCountFec);
    ClassDB::bind_method(D_METHOD("get_packetCountFec"), &LgRTP_AUDIO_STATS::get_packetCountFec);
    ClassDB::bind_method(D_METHOD("set_packetCountFecRecovered", "value"), &LgRTP_AUDIO_STATS::set_packetCountFecRecovered);
    ClassDB::bind_method(D_METHOD("get_packetCountFecRecovered"), &LgRTP_AUDIO_STATS::get_packetCountFecRecovered);
    ClassDB::bind_method(D_METHOD("set_packetCountFecFailed", "value"), &LgRTP_AUDIO_STATS::set_packetCountFecFailed);
    ClassDB::bind_method(D_METHOD("get_packetCountFecFailed"), &LgRTP_AUDIO_STATS::get_packetCountFecFailed);
    ClassDB::bind_method(D_METHOD("set_packetCountOOS", "value"), &LgRTP_AUDIO_STATS::set_packetCountOOS);
    ClassDB::bind_method(D_METHOD("get_packetCountOOS"), &LgRTP_AUDIO_STATS::get_packetCountOOS);
    ClassDB::bind_method(D_METHOD("set_packetCountInvalid", "value"), &LgRTP_AUDIO_STATS::set_packetCountInvalid);
    ClassDB::bind_method(D_METHOD("get_packetCountInvalid"), &LgRTP_AUDIO_STATS::get_packetCountInvalid);
    ClassDB::bind_method(D_METHOD("set_packetCountFecInvalid", "value"), &LgRTP_AUDIO_STATS::set_packetCountFecInvalid);
    ClassDB::bind_method(D_METHOD("get_packetCountFecInvalid"), &LgRTP_AUDIO_STATS::get_packetCountFecInvalid);
}
LgRTP_AUDIO_STATS::LgRTP_AUDIO_STATS() {
    packetCountAudio = 0;
    packetCountFec = 0;
    packetCountFecRecovered = 0;
    packetCountFecFailed = 0;
    packetCountOOS = 0;
    packetCountInvalid = 0;
    packetCountFecInvalid = 0;
}


void LgRTP_VIDEO_STATS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_packetCountVideo", "value"), &LgRTP_VIDEO_STATS::set_packetCountVideo);
    ClassDB::bind_method(D_METHOD("get_packetCountVideo"), &LgRTP_VIDEO_STATS::get_packetCountVideo);
    ClassDB::bind_method(D_METHOD("set_packetCountFec", "value"), &LgRTP_VIDEO_STATS::set_packetCountFec);
    ClassDB::bind_method(D_METHOD("get_packetCountFec"), &LgRTP_VIDEO_STATS::get_packetCountFec);
    ClassDB::bind_method(D_METHOD("set_packetCountFecRecovered", "value"), &LgRTP_VIDEO_STATS::set_packetCountFecRecovered);
    ClassDB::bind_method(D_METHOD("get_packetCountFecRecovered"), &LgRTP_VIDEO_STATS::get_packetCountFecRecovered);
    ClassDB::bind_method(D_METHOD("set_packetCountFecFailed", "value"), &LgRTP_VIDEO_STATS::set_packetCountFecFailed);
    ClassDB::bind_method(D_METHOD("get_packetCountFecFailed"), &LgRTP_VIDEO_STATS::get_packetCountFecFailed);
    ClassDB::bind_method(D_METHOD("set_packetCountOOS", "value"), &LgRTP_VIDEO_STATS::set_packetCountOOS);
    ClassDB::bind_method(D_METHOD("get_packetCountOOS"), &LgRTP_VIDEO_STATS::get_packetCountOOS);
    ClassDB::bind_method(D_METHOD("set_packetCountInvalid", "value"), &LgRTP_VIDEO_STATS::set_packetCountInvalid);
    ClassDB::bind_method(D_METHOD("get_packetCountInvalid"), &LgRTP_VIDEO_STATS::get_packetCountInvalid);
    ClassDB::bind_method(D_METHOD("set_packetCountFecInvalid", "value"), &LgRTP_VIDEO_STATS::set_packetCountFecInvalid);
    ClassDB::bind_method(D_METHOD("get_packetCountFecInvalid"), &LgRTP_VIDEO_STATS::get_packetCountFecInvalid);
}
LgRTP_VIDEO_STATS::LgRTP_VIDEO_STATS() {
    packetCountVideo = 0;
    packetCountFec = 0;
    packetCountFecRecovered = 0;
    packetCountFecFailed = 0;
    packetCountOOS = 0;
    packetCountInvalid = 0;
    packetCountFecInvalid = 0;
}


void LgdisplayPrimaries::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_x", "value"), &LgdisplayPrimaries::set_x);
    ClassDB::bind_method(D_METHOD("get_x"), &LgdisplayPrimaries::get_x);
    ClassDB::bind_method(D_METHOD("set_y", "value"), &LgdisplayPrimaries::set_y);
    ClassDB::bind_method(D_METHOD("get_y"), &LgdisplayPrimaries::get_y);
}
LgdisplayPrimaries::LgdisplayPrimaries() {
    // TODO: init x of type Variant
    // TODO: init y of type Variant
}


const int LgMoonlightstream::LGSTAGE_NONE = 0;
const int LgMoonlightstream::LGSTAGE_PLATFORM_INIT = 1;
const int LgMoonlightstream::LGSTAGE_NAME_RESOLUTION = 2;
const int LgMoonlightstream::LGSTAGE_AUDIO_STREAM_INIT = 3;
const int LgMoonlightstream::LGSTAGE_RTSP_HANDSHAKE = 4;
const int LgMoonlightstream::LGSTAGE_CONTROL_STREAM_INIT = 5;
const int LgMoonlightstream::LGSTAGE_VIDEO_STREAM_INIT = 6;
const int LgMoonlightstream::LGSTAGE_INPUT_STREAM_INIT = 7;
const int LgMoonlightstream::LGSTAGE_CONTROL_STREAM_START = 8;
const int LgMoonlightstream::LGSTAGE_VIDEO_STREAM_START = 9;
const int LgMoonlightstream::LGSTAGE_AUDIO_STREAM_START = 10;
const int LgMoonlightstream::LGSTAGE_INPUT_STREAM_START = 11;
const int LgMoonlightstream::LGSTAGE_MAX = 12;
const int LgMoonlightstream::LGML_ERROR_GRACEFUL_TERMINATION = 0;
const int LgMoonlightstream::LGML_ERROR_NO_VIDEO_TRAFFIC = -100;
const int LgMoonlightstream::LGML_ERROR_NO_VIDEO_FRAME = -101;
const int LgMoonlightstream::LGML_ERROR_UNEXPECTED_EARLY_TERMINATION = -102;
const int LgMoonlightstream::LGML_ERROR_PROTECTED_CONTENT = -103;
const int LgMoonlightstream::LGML_ERROR_FRAME_CONVERSION = -104;
const int LgMoonlightstream::LGSS_KBE_FLAG_NON_NORMALIZED = 0x01;
const int LgMoonlightstream::LGA_FLAG = 0x1000;
const int LgMoonlightstream::LGB_FLAG = 0x2000;
const int LgMoonlightstream::LGX_FLAG = 0x4000;
const int LgMoonlightstream::LGY_FLAG = 0x8000;
const int LgMoonlightstream::LGUP_FLAG = 0x0001;
const int LgMoonlightstream::LGDOWN_FLAG = 0x0002;
const int LgMoonlightstream::LGLEFT_FLAG = 0x0004;
const int LgMoonlightstream::LGRIGHT_FLAG = 0x0008;
const int LgMoonlightstream::LGLB_FLAG = 0x0100;
const int LgMoonlightstream::LGRB_FLAG = 0x0200;
const int LgMoonlightstream::LGPLAY_FLAG = 0x0010;
const int LgMoonlightstream::LGBACK_FLAG = 0x0020;
const int LgMoonlightstream::LGLS_CLK_FLAG = 0x0040;
const int LgMoonlightstream::LGRS_CLK_FLAG = 0x0080;
const int LgMoonlightstream::LGSPECIAL_FLAG = 0x0400;
const int LgMoonlightstream::LGPADDLE1_FLAG = 0x010000;
const int LgMoonlightstream::LGPADDLE2_FLAG = 0x020000;
const int LgMoonlightstream::LGPADDLE3_FLAG = 0x040000;
const int LgMoonlightstream::LGPADDLE4_FLAG = 0x080000;
const int LgMoonlightstream::LGML_PORT_FLAG_ALL = 0xFFFFFFFF;
const int LgMoonlightstream::LGML_PORT_FLAG_TCP_47984 = 0x0001;
const int LgMoonlightstream::LGML_PORT_FLAG_TCP_47989 = 0x0002;
const int LgMoonlightstream::LGML_PORT_FLAG_TCP_48010 = 0x0004;
const int LgMoonlightstream::LGML_PORT_FLAG_UDP_47998 = 0x0100;
const int LgMoonlightstream::LGML_PORT_FLAG_UDP_47999 = 0x0200;
const int LgMoonlightstream::LGML_PORT_FLAG_UDP_48000 = 0x0400;
const int LgMoonlightstream::LGML_PORT_FLAG_UDP_48010 = 0x0800;

void LgMoonlightstream::_bind_methods() {
    BIND_CONSTANT(LGSTAGE_NONE);
    BIND_CONSTANT(LGSTAGE_PLATFORM_INIT);
    BIND_CONSTANT(LGSTAGE_NAME_RESOLUTION);
    BIND_CONSTANT(LGSTAGE_AUDIO_STREAM_INIT);
    BIND_CONSTANT(LGSTAGE_RTSP_HANDSHAKE);
    BIND_CONSTANT(LGSTAGE_CONTROL_STREAM_INIT);
    BIND_CONSTANT(LGSTAGE_VIDEO_STREAM_INIT);
    BIND_CONSTANT(LGSTAGE_INPUT_STREAM_INIT);
    BIND_CONSTANT(LGSTAGE_CONTROL_STREAM_START);
    BIND_CONSTANT(LGSTAGE_VIDEO_STREAM_START);
    BIND_CONSTANT(LGSTAGE_AUDIO_STREAM_START);
    BIND_CONSTANT(LGSTAGE_INPUT_STREAM_START);
    BIND_CONSTANT(LGSTAGE_MAX);
    BIND_CONSTANT(LGML_ERROR_GRACEFUL_TERMINATION);
    BIND_CONSTANT(LGML_ERROR_NO_VIDEO_TRAFFIC);
    BIND_CONSTANT(LGML_ERROR_NO_VIDEO_FRAME);
    BIND_CONSTANT(LGML_ERROR_UNEXPECTED_EARLY_TERMINATION);
    BIND_CONSTANT(LGML_ERROR_PROTECTED_CONTENT);
    BIND_CONSTANT(LGML_ERROR_FRAME_CONVERSION);
    BIND_CONSTANT(LGSS_KBE_FLAG_NON_NORMALIZED);
    BIND_CONSTANT(LGA_FLAG);
    BIND_CONSTANT(LGB_FLAG);
    BIND_CONSTANT(LGX_FLAG);
    BIND_CONSTANT(LGY_FLAG);
    BIND_CONSTANT(LGUP_FLAG);
    BIND_CONSTANT(LGDOWN_FLAG);
    BIND_CONSTANT(LGLEFT_FLAG);
    BIND_CONSTANT(LGRIGHT_FLAG);
    BIND_CONSTANT(LGLB_FLAG);
    BIND_CONSTANT(LGRB_FLAG);
    BIND_CONSTANT(LGPLAY_FLAG);
    BIND_CONSTANT(LGBACK_FLAG);
    BIND_CONSTANT(LGLS_CLK_FLAG);
    BIND_CONSTANT(LGRS_CLK_FLAG);
    BIND_CONSTANT(LGSPECIAL_FLAG);
    BIND_CONSTANT(LGPADDLE1_FLAG);
    BIND_CONSTANT(LGPADDLE2_FLAG);
    BIND_CONSTANT(LGPADDLE3_FLAG);
    BIND_CONSTANT(LGPADDLE4_FLAG);
    BIND_CONSTANT(LGML_PORT_FLAG_ALL);
    BIND_CONSTANT(LGML_PORT_FLAG_TCP_47984);
    BIND_CONSTANT(LGML_PORT_FLAG_TCP_47989);
    BIND_CONSTANT(LGML_PORT_FLAG_TCP_48010);
    BIND_CONSTANT(LGML_PORT_FLAG_UDP_47998);
    BIND_CONSTANT(LGML_PORT_FLAG_UDP_47999);
    BIND_CONSTANT(LGML_PORT_FLAG_UDP_48000);
    BIND_CONSTANT(LGML_PORT_FLAG_UDP_48010);
    ClassDB::bind_method(D_METHOD("LgLiGetLaunchUrlQueryParameters"), &LgMoonlightstream::LgLiGetLaunchUrlQueryParameters);
    ClassDB::bind_method(D_METHOD("LgLiInitializeStreamConfiguration", "streamConfig"), &LgMoonlightstream::LgLiInitializeStreamConfiguration);
    ClassDB::bind_method(D_METHOD("LgLiInitializeVideoCallbacks", "drCallbacks"), &LgMoonlightstream::LgLiInitializeVideoCallbacks);
    ClassDB::bind_method(D_METHOD("LgLiInitializeAudioCallbacks", "arCallbacks"), &LgMoonlightstream::LgLiInitializeAudioCallbacks);
    ClassDB::bind_method(D_METHOD("LgLiInitializeConnectionCallbacks", "clCallbacks"), &LgMoonlightstream::LgLiInitializeConnectionCallbacks);
    ClassDB::bind_method(D_METHOD("LgLiInitializeServerInformation", "serverInfo"), &LgMoonlightstream::LgLiInitializeServerInformation);
    ClassDB::bind_method(D_METHOD("LgLiStartConnection", "serverInfo, streamConfig, clCallbacks, drCallbacks, arCallbacks, renderContext, drFlags, audioContext, arFlags"), &LgMoonlightstream::LgLiStartConnection);
    ClassDB::bind_method(D_METHOD("LgLiStopConnection"), &LgMoonlightstream::LgLiStopConnection);
    ClassDB::bind_method(D_METHOD("LgLiInterruptConnection"), &LgMoonlightstream::LgLiInterruptConnection);
    ClassDB::bind_method(D_METHOD("LgLiGetStageName", "stage"), &LgMoonlightstream::LgLiGetStageName);
    ClassDB::bind_method(D_METHOD("LgLiGetEstimatedRttInfo", "estimatedRtt, estimatedRttVariance"), &LgMoonlightstream::LgLiGetEstimatedRttInfo);
    ClassDB::bind_method(D_METHOD("LgLiSendMouseMoveEvent", "deltaX, deltaY"), &LgMoonlightstream::LgLiSendMouseMoveEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendMousePositionEvent", "x, y, referenceWidth, referenceHeight"), &LgMoonlightstream::LgLiSendMousePositionEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendMouseMoveAsMousePositionEvent", "deltaX, deltaY, referenceWidth, referenceHeight"), &LgMoonlightstream::LgLiSendMouseMoveAsMousePositionEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendTouchEvent", "eventType, pointerId, x, y, pressureOrDistance, contactAreaMajor, contactAreaMinor, rotation"), &LgMoonlightstream::LgLiSendTouchEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendPenEvent", "eventType, toolType, penButtons, x, y, pressureOrDistance, contactAreaMajor, contactAreaMinor, rotation, tilt"), &LgMoonlightstream::LgLiSendPenEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendMouseButtonEvent", "action, button"), &LgMoonlightstream::LgLiSendMouseButtonEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendKeyboardEvent", "keyCode, keyAction, modifiers"), &LgMoonlightstream::LgLiSendKeyboardEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendKeyboardEvent2", "keyCode, keyAction, modifiers, flags"), &LgMoonlightstream::LgLiSendKeyboardEvent2);
    ClassDB::bind_method(D_METHOD("LgLiSendUtf8TextEvent", "arg0, length"), &LgMoonlightstream::LgLiSendUtf8TextEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendControllerEvent", "buttonFlags, leftTrigger, rightTrigger, leftStickX, leftStickY, rightStickX, rightStickY"), &LgMoonlightstream::LgLiSendControllerEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendMultiControllerEvent", "controllerNumber, activeGamepadMask, buttonFlags, leftTrigger, rightTrigger, leftStickX, leftStickY, rightStickX, rightStickY"), &LgMoonlightstream::LgLiSendMultiControllerEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendControllerArrivalEvent", "controllerNumber, activeGamepadMask, type, supportedButtonFlags, capabilities"), &LgMoonlightstream::LgLiSendControllerArrivalEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendControllerTouchEvent", "controllerNumber, eventType, pointerId, x, y, pressure"), &LgMoonlightstream::LgLiSendControllerTouchEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendControllerMotionEvent", "controllerNumber, motionType, x, y, z"), &LgMoonlightstream::LgLiSendControllerMotionEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendControllerBatteryEvent", "controllerNumber, batteryState, batteryPercentage"), &LgMoonlightstream::LgLiSendControllerBatteryEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendScrollEvent", "scrollClicks"), &LgMoonlightstream::LgLiSendScrollEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendHighResScrollEvent", "scrollAmount"), &LgMoonlightstream::LgLiSendHighResScrollEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendHScrollEvent", "scrollClicks"), &LgMoonlightstream::LgLiSendHScrollEvent);
    ClassDB::bind_method(D_METHOD("LgLiSendHighResHScrollEvent", "scrollAmount"), &LgMoonlightstream::LgLiSendHighResHScrollEvent);
    ClassDB::bind_method(D_METHOD("LgLiGetMicroseconds"), &LgMoonlightstream::LgLiGetMicroseconds);
    ClassDB::bind_method(D_METHOD("LgLiGetMillis"), &LgMoonlightstream::LgLiGetMillis);
    ClassDB::bind_method(D_METHOD("LgLiFindExternalAddressIP4", "stunServer, stunPort"), &LgMoonlightstream::LgLiFindExternalAddressIP4);
    ClassDB::bind_method(D_METHOD("LgLiGetPendingVideoFrames"), &LgMoonlightstream::LgLiGetPendingVideoFrames);
    ClassDB::bind_method(D_METHOD("LgLiGetPendingAudioFrames"), &LgMoonlightstream::LgLiGetPendingAudioFrames);
    ClassDB::bind_method(D_METHOD("LgLiGetPendingAudioDuration"), &LgMoonlightstream::LgLiGetPendingAudioDuration);
    ClassDB::bind_method(D_METHOD("LgLiGetRTPAudioStats"), &LgMoonlightstream::LgLiGetRTPAudioStats);
    ClassDB::bind_method(D_METHOD("LgLiGetRTPVideoStats"), &LgMoonlightstream::LgLiGetRTPVideoStats);
    ClassDB::bind_method(D_METHOD("LgLiGetPortFlagsFromStage", "stage"), &LgMoonlightstream::LgLiGetPortFlagsFromStage);
    ClassDB::bind_method(D_METHOD("LgLiGetPortFlagsFromTerminationErrorCode", "errorCode"), &LgMoonlightstream::LgLiGetPortFlagsFromTerminationErrorCode);
    ClassDB::bind_method(D_METHOD("LgLiGetProtocolFromPortFlagIndex", "portFlagIndex"), &LgMoonlightstream::LgLiGetProtocolFromPortFlagIndex);
    ClassDB::bind_method(D_METHOD("LgLiGetPortFromPortFlagIndex", "portFlagIndex"), &LgMoonlightstream::LgLiGetPortFromPortFlagIndex);
    ClassDB::bind_method(D_METHOD("LgLiStringifyPortFlags", "portFlags, separator, outputBufferLength"), &LgMoonlightstream::LgLiStringifyPortFlags);
    ClassDB::bind_method(D_METHOD("LgLiTestClientConnectivity", "testServer, referencePort, testPortFlags"), &LgMoonlightstream::LgLiTestClientConnectivity);
    ClassDB::bind_method(D_METHOD("LgLiWaitForNextVideoFrame", "frameHandle, decodeUnit"), &LgMoonlightstream::LgLiWaitForNextVideoFrame);
    ClassDB::bind_method(D_METHOD("LgLiPollNextVideoFrame", "frameHandle, decodeUnit"), &LgMoonlightstream::LgLiPollNextVideoFrame);
    ClassDB::bind_method(D_METHOD("LgLiPeekNextVideoFrame", "decodeUnit"), &LgMoonlightstream::LgLiPeekNextVideoFrame);
    ClassDB::bind_method(D_METHOD("LgLiWakeWaitForVideoFrame"), &LgMoonlightstream::LgLiWakeWaitForVideoFrame);
    ClassDB::bind_method(D_METHOD("LgLiCompleteVideoFrame", "handle, drStatus"), &LgMoonlightstream::LgLiCompleteVideoFrame);
    ClassDB::bind_method(D_METHOD("LgLiGetCurrentHostDisplayHdrMode"), &LgMoonlightstream::LgLiGetCurrentHostDisplayHdrMode);
    ClassDB::bind_method(D_METHOD("LgLiGetHdrMetadata", "metadata"), &LgMoonlightstream::LgLiGetHdrMetadata);
    ClassDB::bind_method(D_METHOD("LgLiRequestIdrFrame"), &LgMoonlightstream::LgLiRequestIdrFrame);
    ClassDB::bind_method(D_METHOD("LgLiGetHostFeatureFlags"), &LgMoonlightstream::LgLiGetHostFeatureFlags);
    ADD_SIGNAL(MethodInfo("lgdecoder_renderer_setup", Variant::INT, Variant::INT, Variant::INT, Variant::INT, Variant::PACKED_BYTE_ARRAY, Variant::INT));
    ADD_SIGNAL(MethodInfo("lgdecoder_renderer_start"));
    ADD_SIGNAL(MethodInfo("lgdecoder_renderer_stop"));
    ADD_SIGNAL(MethodInfo("lgdecoder_renderer_cleanup"));
    ADD_SIGNAL(MethodInfo("lgdecoder_renderer_submit_decode_unit", Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgaudio_renderer_init", Variant::INT, Variant::OBJECT, Variant::PACKED_BYTE_ARRAY, Variant::INT));
    ADD_SIGNAL(MethodInfo("lgaudio_renderer_start"));
    ADD_SIGNAL(MethodInfo("lgaudio_renderer_stop"));
    ADD_SIGNAL(MethodInfo("lgaudio_renderer_cleanup"));
    ADD_SIGNAL(MethodInfo("lgaudio_renderer_decode_and_play_sample", Variant::STRING, Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_stage_starting", Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_stage_complete", Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_stage_failed", Variant::INT, Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_connection_started"));
    ADD_SIGNAL(MethodInfo("lgconn_connection_terminated", Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_log_message", Variant::STRING, Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgconn_rumble", Variant::OBJECT, Variant::OBJECT, Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgconn_connection_status_update", Variant::INT));
    ADD_SIGNAL(MethodInfo("lgconn_set_hdr_mode", Variant::BOOL));
    ADD_SIGNAL(MethodInfo("lgconn_rumble_triggers", Variant::OBJECT, Variant::OBJECT, Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgconn_set_motion_event_state", Variant::OBJECT, Variant::OBJECT, Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgconn_set_adaptive_triggers", Variant::OBJECT, Variant::OBJECT, Variant::OBJECT, Variant::OBJECT, Variant::OBJECT, Variant::OBJECT));
    ADD_SIGNAL(MethodInfo("lgconn_set_controller_led", Variant::OBJECT, Variant::OBJECT, Variant::OBJECT, Variant::OBJECT));
}

String LgMoonlightstream::LgLiGetLaunchUrlQueryParameters() {
    return LiGetLaunchUrlQueryParameters();
}

// void LgMoonlightstream::LgLiInitializeStreamConfiguration(Variant p_streamConfig) {
//     return LiInitializeStreamConfiguration(static_cast<PSTREAM_CONFIGURATION>(p_streamConfig));
// }

// void LgMoonlightstream::LgLiInitializeVideoCallbacks(Variant p_drCallbacks) {
//     return LiInitializeVideoCallbacks(static_cast<PDECODER_RENDERER_CALLBACKS>(p_drCallbacks));
// }

// void LgMoonlightstream::LgLiInitializeAudioCallbacks(Variant p_arCallbacks) {
//     return LiInitializeAudioCallbacks(static_cast<PAUDIO_RENDERER_CALLBACKS>(p_arCallbacks));
// }

// void LgMoonlightstream::LgLiInitializeConnectionCallbacks(Variant p_clCallbacks) {
//     return LiInitializeConnectionCallbacks(static_cast<PCONNECTION_LISTENER_CALLBACKS>(p_clCallbacks));
// }

// void LgMoonlightstream::LgLiInitializeServerInformation(Variant p_serverInfo) {
//     return LiInitializeServerInformation(static_cast<PSERVER_INFORMATION>(p_serverInfo));
// }

// int LgMoonlightstream::LgLiStartConnection(Variant p_serverInfo, Variant p_streamConfig, Variant p_clCallbacks, Variant p_drCallbacks, Variant p_arCallbacks, PackedByteArray p_renderContext, int p_drFlags, PackedByteArray p_audioContext, int p_arFlags) {
//     return LiStartConnection(static_cast<PSERVER_INFORMATION>(p_serverInfo), static_cast<PSTREAM_CONFIGURATION>(p_streamConfig), static_cast<PCONNECTION_LISTENER_CALLBACKS>(p_clCallbacks), static_cast<PDECODER_RENDERER_CALLBACKS>(p_drCallbacks), static_cast<PAUDIO_RENDERER_CALLBACKS>(p_arCallbacks), static_cast<void>(p_renderContext), static_cast<int>(p_drFlags), static_cast<void>(p_audioContext), static_cast<int>(p_arFlags));
// }

void LgMoonlightstream::LgLiStopConnection() {
    return LiStopConnection();
}

void LgMoonlightstream::LgLiInterruptConnection() {
    return LiInterruptConnection();
}

String LgMoonlightstream::LgLiGetStageName(int p_stage) {
    return LiGetStageName(static_cast<int>(p_stage));
}

// bool LgMoonlightstream::LgLiGetEstimatedRttInfo(PackedByteArray p_estimatedRtt, PackedByteArray p_estimatedRttVariance) {
//     return LiGetEstimatedRttInfo(static_cast<uint32_t>(p_estimatedRtt), static_cast<uint32_t>(p_estimatedRttVariance));
// }

int LgMoonlightstream::LgLiSendMouseMoveEvent(int p_deltaX, int p_deltaY) {
    return LiSendMouseMoveEvent(static_cast<short>(p_deltaX), static_cast<short>(p_deltaY));
}

int LgMoonlightstream::LgLiSendMousePositionEvent(int p_x, int p_y, int p_referenceWidth, int p_referenceHeight) {
    return LiSendMousePositionEvent(static_cast<short>(p_x), static_cast<short>(p_y), static_cast<short>(p_referenceWidth), static_cast<short>(p_referenceHeight));
}

int LgMoonlightstream::LgLiSendMouseMoveAsMousePositionEvent(int p_deltaX, int p_deltaY, int p_referenceWidth, int p_referenceHeight) {
    return LiSendMouseMoveAsMousePositionEvent(static_cast<short>(p_deltaX), static_cast<short>(p_deltaY), static_cast<short>(p_referenceWidth), static_cast<short>(p_referenceHeight));
}

int LgMoonlightstream::LgLiSendTouchEvent(Variant p_eventType, int p_pointerId, double p_x, double p_y, double p_pressureOrDistance, double p_contactAreaMajor, double p_contactAreaMinor, Variant p_rotation) {
    return LiSendTouchEvent(static_cast<uint8_t>(p_eventType), static_cast<uint32_t>(p_pointerId), static_cast<float>(p_x), static_cast<float>(p_y), static_cast<float>(p_pressureOrDistance), static_cast<float>(p_contactAreaMajor), static_cast<float>(p_contactAreaMinor), static_cast<uint16_t>(p_rotation));
}

int LgMoonlightstream::LgLiSendPenEvent(Variant p_eventType, Variant p_toolType, Variant p_penButtons, double p_x, double p_y, double p_pressureOrDistance, double p_contactAreaMajor, double p_contactAreaMinor, Variant p_rotation, Variant p_tilt) {
    return LiSendPenEvent(static_cast<uint8_t>(p_eventType), static_cast<uint8_t>(p_toolType), static_cast<uint8_t>(p_penButtons), static_cast<float>(p_x), static_cast<float>(p_y), static_cast<float>(p_pressureOrDistance), static_cast<float>(p_contactAreaMajor), static_cast<float>(p_contactAreaMinor), static_cast<uint16_t>(p_rotation), static_cast<uint8_t>(p_tilt));
}

int LgMoonlightstream::LgLiSendMouseButtonEvent(int p_action, int p_button) {
    return LiSendMouseButtonEvent(static_cast<char>(p_action), static_cast<int>(p_button));
}

int LgMoonlightstream::LgLiSendKeyboardEvent(int p_keyCode, int p_keyAction, int p_modifiers) {
    return LiSendKeyboardEvent(static_cast<short>(p_keyCode), static_cast<char>(p_keyAction), static_cast<char>(p_modifiers));
}

int LgMoonlightstream::LgLiSendKeyboardEvent2(int p_keyCode, int p_keyAction, int p_modifiers, int p_flags) {
    return LiSendKeyboardEvent2(static_cast<short>(p_keyCode), static_cast<char>(p_keyAction), static_cast<char>(p_modifiers), static_cast<char>(p_flags));
}

int LgMoonlightstream::LgLiSendUtf8TextEvent(String p_arg0, int p_length) {
    return LiSendUtf8TextEvent(static_cast<const char*>(p_arg0.utf8().get_data()), static_cast<unsigned int>(p_length));
}

int LgMoonlightstream::LgLiSendControllerEvent(int p_buttonFlags, Variant p_leftTrigger, Variant p_rightTrigger, int p_leftStickX, int p_leftStickY, int p_rightStickX, int p_rightStickY) {
    return LiSendControllerEvent(static_cast<int>(p_buttonFlags), static_cast<unsigned char>(p_leftTrigger), static_cast<unsigned char>(p_rightTrigger), static_cast<short>(p_leftStickX), static_cast<short>(p_leftStickY), static_cast<short>(p_rightStickX), static_cast<short>(p_rightStickY));
}

int LgMoonlightstream::LgLiSendMultiControllerEvent(int p_controllerNumber, int p_activeGamepadMask, int p_buttonFlags, Variant p_leftTrigger, Variant p_rightTrigger, int p_leftStickX, int p_leftStickY, int p_rightStickX, int p_rightStickY) {
    return LiSendMultiControllerEvent(static_cast<short>(p_controllerNumber), static_cast<short>(p_activeGamepadMask), static_cast<int>(p_buttonFlags), static_cast<unsigned char>(p_leftTrigger), static_cast<unsigned char>(p_rightTrigger), static_cast<short>(p_leftStickX), static_cast<short>(p_leftStickY), static_cast<short>(p_rightStickX), static_cast<short>(p_rightStickY));
}

int LgMoonlightstream::LgLiSendControllerArrivalEvent(Variant p_controllerNumber, Variant p_activeGamepadMask, Variant p_type, int p_supportedButtonFlags, Variant p_capabilities) {
    return LiSendControllerArrivalEvent(static_cast<uint8_t>(p_controllerNumber), static_cast<uint16_t>(p_activeGamepadMask), static_cast<uint8_t>(p_type), static_cast<uint32_t>(p_supportedButtonFlags), static_cast<uint16_t>(p_capabilities));
}

int LgMoonlightstream::LgLiSendControllerTouchEvent(Variant p_controllerNumber, Variant p_eventType, int p_pointerId, double p_x, double p_y, double p_pressure) {
    return LiSendControllerTouchEvent(static_cast<uint8_t>(p_controllerNumber), static_cast<uint8_t>(p_eventType), static_cast<uint32_t>(p_pointerId), static_cast<float>(p_x), static_cast<float>(p_y), static_cast<float>(p_pressure));
}

int LgMoonlightstream::LgLiSendControllerMotionEvent(Variant p_controllerNumber, Variant p_motionType, double p_x, double p_y, double p_z) {
    return LiSendControllerMotionEvent(static_cast<uint8_t>(p_controllerNumber), static_cast<uint8_t>(p_motionType), static_cast<float>(p_x), static_cast<float>(p_y), static_cast<float>(p_z));
}

int LgMoonlightstream::LgLiSendControllerBatteryEvent(Variant p_controllerNumber, Variant p_batteryState, Variant p_batteryPercentage) {
    return LiSendControllerBatteryEvent(static_cast<uint8_t>(p_controllerNumber), static_cast<uint8_t>(p_batteryState), static_cast<uint8_t>(p_batteryPercentage));
}

int LgMoonlightstream::LgLiSendScrollEvent(Variant p_scrollClicks) {
    return LiSendScrollEvent(static_cast<signed char>(p_scrollClicks));
}

int LgMoonlightstream::LgLiSendHighResScrollEvent(int p_scrollAmount) {
    return LiSendHighResScrollEvent(static_cast<short>(p_scrollAmount));
}

int LgMoonlightstream::LgLiSendHScrollEvent(Variant p_scrollClicks) {
    return LiSendHScrollEvent(static_cast<signed char>(p_scrollClicks));
}

int LgMoonlightstream::LgLiSendHighResHScrollEvent(int p_scrollAmount) {
    return LiSendHighResHScrollEvent(static_cast<short>(p_scrollAmount));
}

Variant LgMoonlightstream::LgLiGetMicroseconds() {
    return LiGetMicroseconds();
}

Variant LgMoonlightstream::LgLiGetMillis() {
    return LiGetMillis();
}

Dictionary LgMoonlightstream::LgLiFindExternalAddressIP4(String p_stunServer, Variant p_stunPort) {
    Dictionary result;
    std::string s_stunServer = p_stunServer.utf8().get_data();
    const char* c_stunServer = s_stunServer.c_str();
    auto c_stunPort = static_cast<unsigned short>(p_stunPort);
    unsigned int c_wanAddr = 0; // 创建一个unsigned int变量
    int ret_val = LiFindExternalAddressIP4(c_stunServer, c_stunPort, &c_wanAddr); // 传递其指针
    result["error"] = ret_val;

    // 将unsigned int类型的IP地址转换为uint8_t数组
    uint8_t wanAddrBytes[4] = {
        static_cast<uint8_t>((c_wanAddr >> 24) & 0xFF),
        static_cast<uint8_t>((c_wanAddr >> 16) & 0xFF),
        static_cast<uint8_t>((c_wanAddr >> 8) & 0xFF),
        static_cast<uint8_t>(c_wanAddr & 0xFF)
    };

    // 将转换后的数组作为Variant传递给Dictionary
    result["wanAddr"] = wanAddrBytes;
    return result;
}


int LgMoonlightstream::LgLiGetPendingVideoFrames() {
    return LiGetPendingVideoFrames();
}

int LgMoonlightstream::LgLiGetPendingAudioFrames() {
    return LiGetPendingAudioFrames();
}

int LgMoonlightstream::LgLiGetPendingAudioDuration() {
    return LiGetPendingAudioDuration();
}

// Ref<LgRTP_AUDIO_STATS> LgMoonlightstream::LgLiGetRTPAudioStats() {
//     return LiGetRTPAudioStats();
// }

// Ref<LgRTP_VIDEO_STATS> LgMoonlightstream::LgLiGetRTPVideoStats() {
//     return LiGetRTPVideoStats();
// }

int LgMoonlightstream::LgLiGetPortFlagsFromStage(int p_stage) {
    return LiGetPortFlagsFromStage(static_cast<int>(p_stage));
}

int LgMoonlightstream::LgLiGetPortFlagsFromTerminationErrorCode(int p_errorCode) {
    return LiGetPortFlagsFromTerminationErrorCode(static_cast<int>(p_errorCode));
}

int LgMoonlightstream::LgLiGetProtocolFromPortFlagIndex(int p_portFlagIndex) {
    return LiGetProtocolFromPortFlagIndex(static_cast<int>(p_portFlagIndex));
}

Variant LgMoonlightstream::LgLiGetPortFromPortFlagIndex(int p_portFlagIndex) {
    return LiGetPortFromPortFlagIndex(static_cast<int>(p_portFlagIndex));
}

Dictionary LgMoonlightstream::LgLiStringifyPortFlags(int p_portFlags, String p_separator, int p_outputBufferLength) {
    Dictionary result;
    auto c_portFlags = static_cast<unsigned int>(p_portFlags);
    std::string s_separator = p_separator.utf8().get_data();
    const char* c_separator = s_separator.c_str();
    char c_outputBuffer[256] = {0};
    auto c_outputBufferLength = static_cast<int>(p_outputBufferLength);
    LiStringifyPortFlags(c_portFlags, c_separator, c_outputBuffer, c_outputBufferLength);
    // result["error"] = ret_val;
    result["outputBuffer"] = String(c_outputBuffer);
    return result;
}

int LgMoonlightstream::LgLiTestClientConnectivity(String p_testServer, Variant p_referencePort, int p_testPortFlags) {
    std::string s_testServer = p_testServer.utf8().get_data();
    return LiTestClientConnectivity(s_testServer.c_str(), static_cast<unsigned short>(p_referencePort), static_cast<unsigned int>(p_testPortFlags));
}

// bool LgMoonlightstream::LgLiWaitForNextVideoFrame(PackedByteArray p_frameHandle, PackedByteArray p_decodeUnit) {
//     return LiWaitForNextVideoFrame(static_cast<VIDEO_FRAME_HANDLE>(p_frameHandle), static_cast<PDECODE_UNIT>(p_decodeUnit));
// }

// bool LgMoonlightstream::LgLiPollNextVideoFrame(PackedByteArray p_frameHandle, PackedByteArray p_decodeUnit) {
//     return LiPollNextVideoFrame(static_cast<VIDEO_FRAME_HANDLE>(p_frameHandle), static_cast<PDECODE_UNIT>(p_decodeUnit));
// }

// bool LgMoonlightstream::LgLiPeekNextVideoFrame(PackedByteArray p_decodeUnit) {
//     return LiPeekNextVideoFrame(static_cast<PDECODE_UNIT>(p_decodeUnit));
// }

void LgMoonlightstream::LgLiWakeWaitForVideoFrame() {
    return LiWakeWaitForVideoFrame();
}

void LgMoonlightstream::LgLiCompleteVideoFrame(Variant p_handle, int p_drStatus) {
    return LiCompleteVideoFrame(static_cast<VIDEO_FRAME_HANDLE>(p_handle), static_cast<int>(p_drStatus));
}

bool LgMoonlightstream::LgLiGetCurrentHostDisplayHdrMode() {
    return LiGetCurrentHostDisplayHdrMode();
}

// bool LgMoonlightstream::LgLiGetHdrMetadata(Variant p_metadata) {
//     return LiGetHdrMetadata(static_cast<PSS_HDR_METADATA>(p_metadata));
// }

void LgMoonlightstream::LgLiRequestIdrFrame() {
    return LiRequestIdrFrame();
}

int LgMoonlightstream::LgLiGetHostFeatureFlags() {
    return LiGetHostFeatureFlags();
}

void LgMoonlightstream::set_decoderrenderersetup_callback(Callable p_callback) {
    callback_DecoderRendererSetup = p_callback;
}

void LgMoonlightstream::set_decoderrendererstart_callback(Callable p_callback) {
    callback_DecoderRendererStart = p_callback;
}

void LgMoonlightstream::set_decoderrendererstop_callback(Callable p_callback) {
    callback_DecoderRendererStop = p_callback;
}

void LgMoonlightstream::set_decoderrenderercleanup_callback(Callable p_callback) {
    callback_DecoderRendererCleanup = p_callback;
}

void LgMoonlightstream::set_decoderrenderersubmitdecodeunit_callback(Callable p_callback) {
    callback_DecoderRendererSubmitDecodeUnit = p_callback;
}

void LgMoonlightstream::set_audiorendererinit_callback(Callable p_callback) {
    callback_AudioRendererInit = p_callback;
}

void LgMoonlightstream::set_audiorendererstart_callback(Callable p_callback) {
    callback_AudioRendererStart = p_callback;
}

void LgMoonlightstream::set_audiorendererstop_callback(Callable p_callback) {
    callback_AudioRendererStop = p_callback;
}

void LgMoonlightstream::set_audiorenderercleanup_callback(Callable p_callback) {
    callback_AudioRendererCleanup = p_callback;
}

void LgMoonlightstream::set_audiorendererdecodeandplaysample_callback(Callable p_callback) {
    callback_AudioRendererDecodeAndPlaySample = p_callback;
}

void LgMoonlightstream::set_connlistenerstagestarting_callback(Callable p_callback) {
    callback_ConnListenerStageStarting = p_callback;
}

void LgMoonlightstream::set_connlistenerstagecomplete_callback(Callable p_callback) {
    callback_ConnListenerStageComplete = p_callback;
}

void LgMoonlightstream::set_connlistenerstagefailed_callback(Callable p_callback) {
    callback_ConnListenerStageFailed = p_callback;
}

void LgMoonlightstream::set_connlistenerconnectionstarted_callback(Callable p_callback) {
    callback_ConnListenerConnectionStarted = p_callback;
}

void LgMoonlightstream::set_connlistenerconnectionterminated_callback(Callable p_callback) {
    callback_ConnListenerConnectionTerminated = p_callback;
}

void LgMoonlightstream::set_connlistenerlogmessage_callback(Callable p_callback) {
    callback_ConnListenerLogMessage = p_callback;
}

void LgMoonlightstream::set_connlistenerrumble_callback(Callable p_callback) {
    callback_ConnListenerRumble = p_callback;
}

void LgMoonlightstream::set_connlistenerconnectionstatusupdate_callback(Callable p_callback) {
    callback_ConnListenerConnectionStatusUpdate = p_callback;
}

void LgMoonlightstream::set_connlistenersethdrmode_callback(Callable p_callback) {
    callback_ConnListenerSetHdrMode = p_callback;
}

void LgMoonlightstream::set_connlistenerrumbletriggers_callback(Callable p_callback) {
    callback_ConnListenerRumbleTriggers = p_callback;
}

void LgMoonlightstream::set_connlistenersetmotioneventstate_callback(Callable p_callback) {
    callback_ConnListenerSetMotionEventState = p_callback;
}

void LgMoonlightstream::set_connlistenersetadaptivetriggers_callback(Callable p_callback) {
    callback_ConnListenerSetAdaptiveTriggers = p_callback;
}

void LgMoonlightstream::set_connlistenersetcontrollerled_callback(Callable p_callback) {
    callback_ConnListenerSetControllerLED = p_callback;
}

