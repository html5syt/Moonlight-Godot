#ifndef MOONLIGHTSTREAM_H
#define MOONLIGHTSTREAM_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;


class LgSTREAM_CONFIGURATION : public RefCounted {
    GDCLASS(LgSTREAM_CONFIGURATION, RefCounted);

    int width;
    int height;
    int fps;
    int bitrate;
    int packetSize;
    int streamingRemotely;
    int audioConfiguration;
    int supportedVideoFormats;
    int clientRefreshRateX100;
    int colorSpace;
    int colorRange;
    int encryptionFlags;
    String remoteInputAesKey;
    String remoteInputAesIv;

protected:
    static void _bind_methods();

public:
    LgSTREAM_CONFIGURATION();
    void set_width(int p_value) { width = p_value; }
    int get_width() const { return width; }
    void set_height(int p_value) { height = p_value; }
    int get_height() const { return height; }
    void set_fps(int p_value) { fps = p_value; }
    int get_fps() const { return fps; }
    void set_bitrate(int p_value) { bitrate = p_value; }
    int get_bitrate() const { return bitrate; }
    void set_packetSize(int p_value) { packetSize = p_value; }
    int get_packetSize() const { return packetSize; }
    void set_streamingRemotely(int p_value) { streamingRemotely = p_value; }
    int get_streamingRemotely() const { return streamingRemotely; }
    void set_audioConfiguration(int p_value) { audioConfiguration = p_value; }
    int get_audioConfiguration() const { return audioConfiguration; }
    void set_supportedVideoFormats(int p_value) { supportedVideoFormats = p_value; }
    int get_supportedVideoFormats() const { return supportedVideoFormats; }
    void set_clientRefreshRateX100(int p_value) { clientRefreshRateX100 = p_value; }
    int get_clientRefreshRateX100() const { return clientRefreshRateX100; }
    void set_colorSpace(int p_value) { colorSpace = p_value; }
    int get_colorSpace() const { return colorSpace; }
    void set_colorRange(int p_value) { colorRange = p_value; }
    int get_colorRange() const { return colorRange; }
    void set_encryptionFlags(int p_value) { encryptionFlags = p_value; }
    int get_encryptionFlags() const { return encryptionFlags; }
    void set_remoteInputAesKey(String p_value) { remoteInputAesKey = p_value; }
    String get_remoteInputAesKey() const { return remoteInputAesKey; }
    void set_remoteInputAesIv(String p_value) { remoteInputAesIv = p_value; }
    String get_remoteInputAesIv() const { return remoteInputAesIv; }
};

class LgLENTRY : public RefCounted {
    GDCLASS(LgLENTRY, RefCounted);

    String data;
    int length;
    int bufferType;

protected:
    static void _bind_methods();

public:
    LgLENTRY();
    void set_data(String p_value) { data = p_value; }
    String get_data() const { return data; }
    void set_length(int p_value) { length = p_value; }
    int get_length() const { return length; }
    void set_bufferType(int p_value) { bufferType = p_value; }
    int get_bufferType() const { return bufferType; }
};

class LgDECODE_UNIT : public RefCounted {
    GDCLASS(LgDECODE_UNIT, RefCounted);

    int frameNumber;
    int frameType;
    Variant frameHostProcessingLatency;
    Variant receiveTimeUs;
    Variant enqueueTimeUs;
    Variant presentationTimeUs;
    int rtpTimestamp;
    int fullLength;
    Variant bufferList;
    bool hdrActive;
    Variant colorspace;

protected:
    static void _bind_methods();

public:
    LgDECODE_UNIT();
    void set_frameNumber(int p_value) { frameNumber = p_value; }
    int get_frameNumber() const { return frameNumber; }
    void set_frameType(int p_value) { frameType = p_value; }
    int get_frameType() const { return frameType; }
    void set_frameHostProcessingLatency(Variant p_value) { frameHostProcessingLatency = p_value; }
    Variant get_frameHostProcessingLatency() const { return frameHostProcessingLatency; }
    void set_receiveTimeUs(Variant p_value) { receiveTimeUs = p_value; }
    Variant get_receiveTimeUs() const { return receiveTimeUs; }
    void set_enqueueTimeUs(Variant p_value) { enqueueTimeUs = p_value; }
    Variant get_enqueueTimeUs() const { return enqueueTimeUs; }
    void set_presentationTimeUs(Variant p_value) { presentationTimeUs = p_value; }
    Variant get_presentationTimeUs() const { return presentationTimeUs; }
    void set_rtpTimestamp(int p_value) { rtpTimestamp = p_value; }
    int get_rtpTimestamp() const { return rtpTimestamp; }
    void set_fullLength(int p_value) { fullLength = p_value; }
    int get_fullLength() const { return fullLength; }
    void set_bufferList(Variant p_value) { bufferList = p_value; }
    Variant get_bufferList() const { return bufferList; }
    void set_hdrActive(bool p_value) { hdrActive = p_value; }
    bool get_hdrActive() const { return hdrActive; }
    void set_colorspace(Variant p_value) { colorspace = p_value; }
    Variant get_colorspace() const { return colorspace; }
};

class LgDECODER_RENDERER_CALLBACKS : public RefCounted {
    GDCLASS(LgDECODER_RENDERER_CALLBACKS, RefCounted);

    Variant setup;
    Variant start;
    Variant stop;
    Variant cleanup;
    Variant submitDecodeUnit;
    int capabilities;

protected:
    static void _bind_methods();

public:
    LgDECODER_RENDERER_CALLBACKS();
    void set_setup(Variant p_value) { setup = p_value; }
    Variant get_setup() const { return setup; }
    void set_start(Variant p_value) { start = p_value; }
    Variant get_start() const { return start; }
    void set_stop(Variant p_value) { stop = p_value; }
    Variant get_stop() const { return stop; }
    void set_cleanup(Variant p_value) { cleanup = p_value; }
    Variant get_cleanup() const { return cleanup; }
    void set_submitDecodeUnit(Variant p_value) { submitDecodeUnit = p_value; }
    Variant get_submitDecodeUnit() const { return submitDecodeUnit; }
    void set_capabilities(int p_value) { capabilities = p_value; }
    int get_capabilities() const { return capabilities; }
};

class LgOPUS_MULTISTREAM_CONFIGURATION : public RefCounted {
    GDCLASS(LgOPUS_MULTISTREAM_CONFIGURATION, RefCounted);

    int sampleRate;
    int channelCount;
    int streams;
    int coupledStreams;
    int samplesPerFrame;

protected:
    static void _bind_methods();

public:
    LgOPUS_MULTISTREAM_CONFIGURATION();
    void set_sampleRate(int p_value) { sampleRate = p_value; }
    int get_sampleRate() const { return sampleRate; }
    void set_channelCount(int p_value) { channelCount = p_value; }
    int get_channelCount() const { return channelCount; }
    void set_streams(int p_value) { streams = p_value; }
    int get_streams() const { return streams; }
    void set_coupledStreams(int p_value) { coupledStreams = p_value; }
    int get_coupledStreams() const { return coupledStreams; }
    void set_samplesPerFrame(int p_value) { samplesPerFrame = p_value; }
    int get_samplesPerFrame() const { return samplesPerFrame; }
};

class LgAUDIO_RENDERER_CALLBACKS : public RefCounted {
    GDCLASS(LgAUDIO_RENDERER_CALLBACKS, RefCounted);

    Variant init;
    Variant start;
    Variant stop;
    Variant cleanup;
    Variant decodeAndPlaySample;
    int capabilities;

protected:
    static void _bind_methods();

public:
    LgAUDIO_RENDERER_CALLBACKS();
    void set_init(Variant p_value) { init = p_value; }
    Variant get_init() const { return init; }
    void set_start(Variant p_value) { start = p_value; }
    Variant get_start() const { return start; }
    void set_stop(Variant p_value) { stop = p_value; }
    Variant get_stop() const { return stop; }
    void set_cleanup(Variant p_value) { cleanup = p_value; }
    Variant get_cleanup() const { return cleanup; }
    void set_decodeAndPlaySample(Variant p_value) { decodeAndPlaySample = p_value; }
    Variant get_decodeAndPlaySample() const { return decodeAndPlaySample; }
    void set_capabilities(int p_value) { capabilities = p_value; }
    int get_capabilities() const { return capabilities; }
};

class LgCONNECTION_LISTENER_CALLBACKS : public RefCounted {
    GDCLASS(LgCONNECTION_LISTENER_CALLBACKS, RefCounted);

    Variant stageStarting;
    Variant stageComplete;
    Variant stageFailed;
    Variant connectionStarted;
    Variant connectionTerminated;
    Variant logMessage;
    Variant rumble;
    Variant connectionStatusUpdate;
    Variant setHdrMode;
    Variant rumbleTriggers;
    Variant setMotionEventState;
    Variant setControllerLED;
    Variant setAdaptiveTriggers;

protected:
    static void _bind_methods();

public:
    LgCONNECTION_LISTENER_CALLBACKS();
    void set_stageStarting(Variant p_value) { stageStarting = p_value; }
    Variant get_stageStarting() const { return stageStarting; }
    void set_stageComplete(Variant p_value) { stageComplete = p_value; }
    Variant get_stageComplete() const { return stageComplete; }
    void set_stageFailed(Variant p_value) { stageFailed = p_value; }
    Variant get_stageFailed() const { return stageFailed; }
    void set_connectionStarted(Variant p_value) { connectionStarted = p_value; }
    Variant get_connectionStarted() const { return connectionStarted; }
    void set_connectionTerminated(Variant p_value) { connectionTerminated = p_value; }
    Variant get_connectionTerminated() const { return connectionTerminated; }
    void set_logMessage(Variant p_value) { logMessage = p_value; }
    Variant get_logMessage() const { return logMessage; }
    void set_rumble(Variant p_value) { rumble = p_value; }
    Variant get_rumble() const { return rumble; }
    void set_connectionStatusUpdate(Variant p_value) { connectionStatusUpdate = p_value; }
    Variant get_connectionStatusUpdate() const { return connectionStatusUpdate; }
    void set_setHdrMode(Variant p_value) { setHdrMode = p_value; }
    Variant get_setHdrMode() const { return setHdrMode; }
    void set_rumbleTriggers(Variant p_value) { rumbleTriggers = p_value; }
    Variant get_rumbleTriggers() const { return rumbleTriggers; }
    void set_setMotionEventState(Variant p_value) { setMotionEventState = p_value; }
    Variant get_setMotionEventState() const { return setMotionEventState; }
    void set_setControllerLED(Variant p_value) { setControllerLED = p_value; }
    Variant get_setControllerLED() const { return setControllerLED; }
    void set_setAdaptiveTriggers(Variant p_value) { setAdaptiveTriggers = p_value; }
    Variant get_setAdaptiveTriggers() const { return setAdaptiveTriggers; }
};

class LgSERVER_INFORMATION : public RefCounted {
    GDCLASS(LgSERVER_INFORMATION, RefCounted);

    int serverCodecModeSupport;

protected:
    static void _bind_methods();

public:
    LgSERVER_INFORMATION();
    void set_serverCodecModeSupport(int p_value) { serverCodecModeSupport = p_value; }
    int get_serverCodecModeSupport() const { return serverCodecModeSupport; }
};

class LgRTP_AUDIO_STATS : public RefCounted {
    GDCLASS(LgRTP_AUDIO_STATS, RefCounted);

    int packetCountAudio;
    int packetCountFec;
    int packetCountFecRecovered;
    int packetCountFecFailed;
    int packetCountOOS;
    int packetCountInvalid;
    int packetCountFecInvalid;

protected:
    static void _bind_methods();

public:
    LgRTP_AUDIO_STATS();
    void set_packetCountAudio(int p_value) { packetCountAudio = p_value; }
    int get_packetCountAudio() const { return packetCountAudio; }
    void set_packetCountFec(int p_value) { packetCountFec = p_value; }
    int get_packetCountFec() const { return packetCountFec; }
    void set_packetCountFecRecovered(int p_value) { packetCountFecRecovered = p_value; }
    int get_packetCountFecRecovered() const { return packetCountFecRecovered; }
    void set_packetCountFecFailed(int p_value) { packetCountFecFailed = p_value; }
    int get_packetCountFecFailed() const { return packetCountFecFailed; }
    void set_packetCountOOS(int p_value) { packetCountOOS = p_value; }
    int get_packetCountOOS() const { return packetCountOOS; }
    void set_packetCountInvalid(int p_value) { packetCountInvalid = p_value; }
    int get_packetCountInvalid() const { return packetCountInvalid; }
    void set_packetCountFecInvalid(int p_value) { packetCountFecInvalid = p_value; }
    int get_packetCountFecInvalid() const { return packetCountFecInvalid; }
};

class LgRTP_VIDEO_STATS : public RefCounted {
    GDCLASS(LgRTP_VIDEO_STATS, RefCounted);

    int packetCountVideo;
    int packetCountFec;
    int packetCountFecRecovered;
    int packetCountFecFailed;
    int packetCountOOS;
    int packetCountInvalid;
    int packetCountFecInvalid;

protected:
    static void _bind_methods();

public:
    LgRTP_VIDEO_STATS();
    void set_packetCountVideo(int p_value) { packetCountVideo = p_value; }
    int get_packetCountVideo() const { return packetCountVideo; }
    void set_packetCountFec(int p_value) { packetCountFec = p_value; }
    int get_packetCountFec() const { return packetCountFec; }
    void set_packetCountFecRecovered(int p_value) { packetCountFecRecovered = p_value; }
    int get_packetCountFecRecovered() const { return packetCountFecRecovered; }
    void set_packetCountFecFailed(int p_value) { packetCountFecFailed = p_value; }
    int get_packetCountFecFailed() const { return packetCountFecFailed; }
    void set_packetCountOOS(int p_value) { packetCountOOS = p_value; }
    int get_packetCountOOS() const { return packetCountOOS; }
    void set_packetCountInvalid(int p_value) { packetCountInvalid = p_value; }
    int get_packetCountInvalid() const { return packetCountInvalid; }
    void set_packetCountFecInvalid(int p_value) { packetCountFecInvalid = p_value; }
    int get_packetCountFecInvalid() const { return packetCountFecInvalid; }
};

class LgdisplayPrimaries : public RefCounted {
    GDCLASS(LgdisplayPrimaries, RefCounted);

    Variant x;
    Variant y;

protected:
    static void _bind_methods();

public:
    LgdisplayPrimaries();
    void set_x(Variant p_value) { x = p_value; }
    Variant get_x() const { return x; }
    void set_y(Variant p_value) { y = p_value; }
    Variant get_y() const { return y; }
};

class LgMoonlightstream : public RefCounted {
    GDCLASS(LgMoonlightstream, RefCounted);

    Callable callback_DecoderRendererSetup;
    Callable callback_DecoderRendererStart;
    Callable callback_DecoderRendererStop;
    Callable callback_DecoderRendererCleanup;
    Callable callback_DecoderRendererSubmitDecodeUnit;
    Callable callback_AudioRendererInit;
    Callable callback_AudioRendererStart;
    Callable callback_AudioRendererStop;
    Callable callback_AudioRendererCleanup;
    Callable callback_AudioRendererDecodeAndPlaySample;
    Callable callback_ConnListenerStageStarting;
    Callable callback_ConnListenerStageComplete;
    Callable callback_ConnListenerStageFailed;
    Callable callback_ConnListenerConnectionStarted;
    Callable callback_ConnListenerConnectionTerminated;
    Callable callback_ConnListenerLogMessage;
    Callable callback_ConnListenerRumble;
    Callable callback_ConnListenerConnectionStatusUpdate;
    Callable callback_ConnListenerSetHdrMode;
    Callable callback_ConnListenerRumbleTriggers;
    Callable callback_ConnListenerSetMotionEventState;
    Callable callback_ConnListenerSetAdaptiveTriggers;
    Callable callback_ConnListenerSetControllerLED;

protected:
    static void _bind_methods();

public:
    static const int LGSTAGE_NONE;
    static const int LGSTAGE_PLATFORM_INIT;
    static const int LGSTAGE_NAME_RESOLUTION;
    static const int LGSTAGE_AUDIO_STREAM_INIT;
    static const int LGSTAGE_RTSP_HANDSHAKE;
    static const int LGSTAGE_CONTROL_STREAM_INIT;
    static const int LGSTAGE_VIDEO_STREAM_INIT;
    static const int LGSTAGE_INPUT_STREAM_INIT;
    static const int LGSTAGE_CONTROL_STREAM_START;
    static const int LGSTAGE_VIDEO_STREAM_START;
    static const int LGSTAGE_AUDIO_STREAM_START;
    static const int LGSTAGE_INPUT_STREAM_START;
    static const int LGSTAGE_MAX;
    static const int LGML_ERROR_GRACEFUL_TERMINATION;
    static const int LGML_ERROR_NO_VIDEO_TRAFFIC;
    static const int LGML_ERROR_NO_VIDEO_FRAME;
    static const int LGML_ERROR_UNEXPECTED_EARLY_TERMINATION;
    static const int LGML_ERROR_PROTECTED_CONTENT;
    static const int LGML_ERROR_FRAME_CONVERSION;
    static const int LGSS_KBE_FLAG_NON_NORMALIZED;
    static const int LGA_FLAG;
    static const int LGB_FLAG;
    static const int LGX_FLAG;
    static const int LGY_FLAG;
    static const int LGUP_FLAG;
    static const int LGDOWN_FLAG;
    static const int LGLEFT_FLAG;
    static const int LGRIGHT_FLAG;
    static const int LGLB_FLAG;
    static const int LGRB_FLAG;
    static const int LGPLAY_FLAG;
    static const int LGBACK_FLAG;
    static const int LGLS_CLK_FLAG;
    static const int LGRS_CLK_FLAG;
    static const int LGSPECIAL_FLAG;
    static const int LGPADDLE1_FLAG;
    static const int LGPADDLE2_FLAG;
    static const int LGPADDLE3_FLAG;
    static const int LGPADDLE4_FLAG;
    static const int LGML_PORT_FLAG_ALL;
    static const int LGML_PORT_FLAG_TCP_47984;
    static const int LGML_PORT_FLAG_TCP_47989;
    static const int LGML_PORT_FLAG_TCP_48010;
    static const int LGML_PORT_FLAG_UDP_47998;
    static const int LGML_PORT_FLAG_UDP_47999;
    static const int LGML_PORT_FLAG_UDP_48000;
    static const int LGML_PORT_FLAG_UDP_48010;
    String LgLiGetLaunchUrlQueryParameters();
    void LgLiInitializeStreamConfiguration(Variant p_streamConfig);
    void LgLiInitializeVideoCallbacks(Variant p_drCallbacks);
    void LgLiInitializeAudioCallbacks(Variant p_arCallbacks);
    void LgLiInitializeConnectionCallbacks(Variant p_clCallbacks);
    void LgLiInitializeServerInformation(Variant p_serverInfo);
    int LgLiStartConnection(Variant p_serverInfo, Variant p_streamConfig, Variant p_clCallbacks, Variant p_drCallbacks, Variant p_arCallbacks, PackedByteArray p_renderContext, int p_drFlags, PackedByteArray p_audioContext, int p_arFlags);
    void LgLiStopConnection();
    void LgLiInterruptConnection();
    String LgLiGetStageName(int p_stage);
    bool LgLiGetEstimatedRttInfo(PackedByteArray p_estimatedRtt, PackedByteArray p_estimatedRttVariance);
    int LgLiSendMouseMoveEvent(int p_deltaX, int p_deltaY);
    int LgLiSendMousePositionEvent(int p_x, int p_y, int p_referenceWidth, int p_referenceHeight);
    int LgLiSendMouseMoveAsMousePositionEvent(int p_deltaX, int p_deltaY, int p_referenceWidth, int p_referenceHeight);
    int LgLiSendTouchEvent(Variant p_eventType, int p_pointerId, double p_x, double p_y, double p_pressureOrDistance, double p_contactAreaMajor, double p_contactAreaMinor, Variant p_rotation);
    int LgLiSendPenEvent(Variant p_eventType, Variant p_toolType, Variant p_penButtons, double p_x, double p_y, double p_pressureOrDistance, double p_contactAreaMajor, double p_contactAreaMinor, Variant p_rotation, Variant p_tilt);
    int LgLiSendMouseButtonEvent(int p_action, int p_button);
    int LgLiSendKeyboardEvent(int p_keyCode, int p_keyAction, int p_modifiers);
    int LgLiSendKeyboardEvent2(int p_keyCode, int p_keyAction, int p_modifiers, int p_flags);
    int LgLiSendUtf8TextEvent(String p_arg0, int p_length);
    int LgLiSendControllerEvent(int p_buttonFlags, Variant p_leftTrigger, Variant p_rightTrigger, int p_leftStickX, int p_leftStickY, int p_rightStickX, int p_rightStickY);
    int LgLiSendMultiControllerEvent(int p_controllerNumber, int p_activeGamepadMask, int p_buttonFlags, Variant p_leftTrigger, Variant p_rightTrigger, int p_leftStickX, int p_leftStickY, int p_rightStickX, int p_rightStickY);
    int LgLiSendControllerArrivalEvent(Variant p_controllerNumber, Variant p_activeGamepadMask, Variant p_type, int p_supportedButtonFlags, Variant p_capabilities);
    int LgLiSendControllerTouchEvent(Variant p_controllerNumber, Variant p_eventType, int p_pointerId, double p_x, double p_y, double p_pressure);
    int LgLiSendControllerMotionEvent(Variant p_controllerNumber, Variant p_motionType, double p_x, double p_y, double p_z);
    int LgLiSendControllerBatteryEvent(Variant p_controllerNumber, Variant p_batteryState, Variant p_batteryPercentage);
    int LgLiSendScrollEvent(Variant p_scrollClicks);
    int LgLiSendHighResScrollEvent(int p_scrollAmount);
    int LgLiSendHScrollEvent(Variant p_scrollClicks);
    int LgLiSendHighResHScrollEvent(int p_scrollAmount);
    Variant LgLiGetMicroseconds();
    Variant LgLiGetMillis();
    Dictionary LgLiFindExternalAddressIP4(String p_stunServer, Variant p_stunPort);
    int LgLiGetPendingVideoFrames();
    int LgLiGetPendingAudioFrames();
    int LgLiGetPendingAudioDuration();
    Ref<LgRTP_AUDIO_STATS> LgLiGetRTPAudioStats();
    Ref<LgRTP_VIDEO_STATS> LgLiGetRTPVideoStats();
    int LgLiGetPortFlagsFromStage(int p_stage);
    int LgLiGetPortFlagsFromTerminationErrorCode(int p_errorCode);
    int LgLiGetProtocolFromPortFlagIndex(int p_portFlagIndex);
    Variant LgLiGetPortFromPortFlagIndex(int p_portFlagIndex);
    Dictionary LgLiStringifyPortFlags(int p_portFlags, String p_separator, int p_outputBufferLength);
    int LgLiTestClientConnectivity(String p_testServer, Variant p_referencePort, int p_testPortFlags);
    bool LgLiWaitForNextVideoFrame(PackedByteArray p_frameHandle, PackedByteArray p_decodeUnit);
    bool LgLiPollNextVideoFrame(PackedByteArray p_frameHandle, PackedByteArray p_decodeUnit);
    bool LgLiPeekNextVideoFrame(PackedByteArray p_decodeUnit);
    void LgLiWakeWaitForVideoFrame();
    void LgLiCompleteVideoFrame(Variant p_handle, int p_drStatus);
    bool LgLiGetCurrentHostDisplayHdrMode();
    bool LgLiGetHdrMetadata(Variant p_metadata);
    void LgLiRequestIdrFrame();
    int LgLiGetHostFeatureFlags();
    void set_decoderrenderersetup_callback(Callable p_callback);
    void set_decoderrendererstart_callback(Callable p_callback);
    void set_decoderrendererstop_callback(Callable p_callback);
    void set_decoderrenderercleanup_callback(Callable p_callback);
    void set_decoderrenderersubmitdecodeunit_callback(Callable p_callback);
    void set_audiorendererinit_callback(Callable p_callback);
    void set_audiorendererstart_callback(Callable p_callback);
    void set_audiorendererstop_callback(Callable p_callback);
    void set_audiorenderercleanup_callback(Callable p_callback);
    void set_audiorendererdecodeandplaysample_callback(Callable p_callback);
    void set_connlistenerstagestarting_callback(Callable p_callback);
    void set_connlistenerstagecomplete_callback(Callable p_callback);
    void set_connlistenerstagefailed_callback(Callable p_callback);
    void set_connlistenerconnectionstarted_callback(Callable p_callback);
    void set_connlistenerconnectionterminated_callback(Callable p_callback);
    void set_connlistenerlogmessage_callback(Callable p_callback);
    void set_connlistenerrumble_callback(Callable p_callback);
    void set_connlistenerconnectionstatusupdate_callback(Callable p_callback);
    void set_connlistenersethdrmode_callback(Callable p_callback);
    void set_connlistenerrumbletriggers_callback(Callable p_callback);
    void set_connlistenersetmotioneventstate_callback(Callable p_callback);
    void set_connlistenersetadaptivetriggers_callback(Callable p_callback);
    void set_connlistenersetcontrollerled_callback(Callable p_callback);
};

#endif
