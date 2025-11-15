#include "moonlight_stream.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <map>
#include <cstring>

// Static member definition
std::map<void*, MoonlightStream*> MoonlightStream::instance_map;

// Thread-local storage for audio callback context
static thread_local MoonlightStream* current_audio_instance = nullptr;

// ========== Video Decoder Renderer Callbacks ==========

static int dr_setup(int video_format, int width, int height, int redraw_rate, void* context, int dr_flags) {
	return DR_OK;
}

static int dr_submit_decode_unit(PDECODE_UNIT decode_unit) {
	// In pull mode, we don't render here; frames are fetched via get_next_video_frame()
	return DR_OK;
}

// ========== Audio Renderer Callbacks ==========

static int ar_init(int audio_configuration, const POPUS_MULTISTREAM_CONFIGURATION opus_config, void* context, int ar_flags) {
	auto it = MoonlightStream::instance_map.find(context);
	if (it == MoonlightStream::instance_map.end()) {
		return -1;
	}
	MoonlightStream* self = it->second;
	current_audio_instance = self;

	self->audio_stream.instantiate();
	self->audio_stream->set_mix_rate(48000);
	self->audio_playback = self->audio_stream->instantiate_playback();

	return 0;
}

static void ar_decode_and_play_sample(char* sample_data, int sample_length) {
	MoonlightStream* self = current_audio_instance;
	if (!self || !self->audio_playback.is_valid()) {
		return;
	}

	int sample_count = sample_length / sizeof(int16_t);
	int frame_count = sample_count / 2; // stereo: L, R

	PackedVector2Array buffer;
	buffer.resize(frame_count);
	Vector2* write_ptr = buffer.ptrw();

	int16_t* samples = reinterpret_cast<int16_t*>(sample_data);
	for (int i = 0; i < frame_count; ++i) {
		float left = static_cast<float>(samples[i * 2 + 0]) / 32768.0f;
		float right = static_cast<float>(samples[i * 2 + 1]) / 32768.0f;
		write_ptr[i] = Vector2(left, right);
	}

	self->audio_playback->push_buffer(buffer);
}

// ========== Godot Class Binding ==========

void MoonlightStream::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_host_address", "host"), &MoonlightStream::set_host_address);
	ClassDB::bind_method(D_METHOD("get_host_address"), &MoonlightStream::get_host_address);

	ClassDB::bind_method(D_METHOD("set_app_id", "app_id"), &MoonlightStream::set_app_id);
	ClassDB::bind_method(D_METHOD("get_app_id"), &MoonlightStream::get_app_id);

	ClassDB::bind_method(D_METHOD("set_resolution", "width", "height"), &MoonlightStream::set_resolution);
	ClassDB::bind_method(D_METHOD("set_fps", "fps"), &MoonlightStream::set_fps);
	ClassDB::bind_method(D_METHOD("set_bitrate_kbps", "bitrate_kbps"), &MoonlightStream::set_bitrate_kbps);
	ClassDB::bind_method(D_METHOD("set_video_codec", "codec"), &MoonlightStream::set_video_codec);
	ClassDB::bind_method(D_METHOD("set_color_space", "color_space"), &MoonlightStream::set_color_space);
	ClassDB::bind_method(D_METHOD("set_enable_hdr", "enable"), &MoonlightStream::set_enable_hdr);

	ClassDB::bind_method(D_METHOD("get_audio_stream"), &MoonlightStream::get_audio_stream);

	ClassDB::bind_method(D_METHOD("start_connection"), &MoonlightStream::start_connection);
	ClassDB::bind_method(D_METHOD("stop_connection"), &MoonlightStream::stop_connection);
	ClassDB::bind_method(D_METHOD("get_next_video_frame"), &MoonlightStream::get_next_video_frame);

	ClassDB::bind_method(D_METHOD("send_mouse_button_event", "button", "pressed"), &MoonlightStream::send_mouse_button_event);
	ClassDB::bind_method(D_METHOD("send_mouse_move_event", "x", "y"), &MoonlightStream::send_mouse_move_event);
	ClassDB::bind_method(D_METHOD("send_key_event", "scancode", "pressed"), &MoonlightStream::send_key_event);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_address"), "set_host_address", "get_host_address");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "app_id"), "set_app_id", "get_app_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "320,3840,1"), "", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "240,2160,1"), "", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fps", PROPERTY_HINT_RANGE, "10,120,1"), "set_fps", "get_fps");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bitrate_kbps", PROPERTY_HINT_RANGE, "500,100000,1"), "set_bitrate_kbps", "get_bitrate_kbps");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "video_codec", PROPERTY_HINT_ENUM, "H264,HEVC,AV1"), "set_video_codec", "get_video_codec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "color_space", PROPERTY_HINT_ENUM, "Rec601,Rec709,Rec2020"), "set_color_space", "get_color_space");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_hdr"), "set_enable_hdr", "get_enable_hdr");

	BIND_ENUM_CONSTANT(CODEC_H264);
	BIND_ENUM_CONSTANT(CODEC_HEVC);
	BIND_ENUM_CONSTANT(CODEC_AV1);

	BIND_ENUM_CONSTANT(COLORSPACE_REC_601);
	BIND_ENUM_CONSTANT(COLORSPACE_REC_709);
	BIND_ENUM_CONSTANT(COLORSPACE_REC_2020);
}

// ========== Lifecycle ==========

MoonlightStream::MoonlightStream() {
	render_context = this;
	instance_map[render_context] = this;
}

MoonlightStream::~MoonlightStream() {
	instance_map.erase(render_context);
	stop_connection();
}

// ========== Connection Management ==========

bool MoonlightStream::start_connection() {
	if (streaming) {
		return false;
	}

	CharString host_str = host_address.utf8();
	SERVER_INFORMATION server_info = {};
	server_info.address = host_str.get_data();

	STREAM_CONFIGURATION stream_config = {};
	stream_config.width = width;
	stream_config.height = height;
	stream_config.fps = fps;
	stream_config.bitrate = bitrate_kbps;
	stream_config.packetSize = 1392;
	stream_config.streamingRemotely = STREAM_CFG_LOCAL;
	stream_config.audioConfiguration = AUDIO_CONFIGURATION_STEREO;
	stream_config.supportedVideoFormats = static_cast<int>(video_codec);
	stream_config.colorSpace = static_cast<int>(color_space);
	stream_config.colorRange = enable_hdr ? COLOR_RANGE_FULL : COLOR_RANGE_LIMITED;

	CONNECTION_LISTENER_CALLBACKS cl_callbacks = {};
	DECODER_RENDERER_CALLBACKS dr_callbacks = {};
	dr_callbacks.setup = dr_setup;
	dr_callbacks.submitDecodeUnit = dr_submit_decode_unit;

	AUDIO_RENDERER_CALLBACKS ar_callbacks = {};
	ar_callbacks.init = ar_init;
	ar_callbacks.decodeAndPlaySample = ar_decode_and_play_sample;
	ar_callbacks.capabilities = CAPABILITY_DIRECT_SUBMIT;

	int dr_flags = CAPABILITY_PULL_RENDERER;
	int ar_flags = 0; // Standard value for most implementations

	// ✅ 9-parameter LiStartConnection (as per your header)
	int ret = LiStartConnection(
		&server_info,
		&stream_config,
		&cl_callbacks,
		&dr_callbacks,
		&ar_callbacks,
		render_context,   // renderContext
		dr_flags,         // drFlags
		render_context,   // audioContext ← same as renderContext
		ar_flags          // arFlags
	);

	streaming = (ret == 0);
	if (!streaming) {
		UtilityFunctions::push_error(vformat("LiStartConnection failed with code: %d", ret));
	}
	return streaming;
}

void MoonlightStream::stop_connection() {
	if (streaming) {
		LiStopConnection();
		streaming = false;
		audio_playback.unref();
		audio_stream.unref();
		current_audio_instance = nullptr;
	}
}

// ========== Frame Pulling (Pull Mode) ==========

PackedByteArray MoonlightStream::get_next_video_frame() {
	if (!streaming) {
		return PackedByteArray();
	}

	VIDEO_FRAME_HANDLE handle = nullptr;
	PDECODE_UNIT du = nullptr;

	if (!LiWaitForNextVideoFrame(&handle, &du)) {
		return PackedByteArray();
	}

	size_t total_size = 0;
	for (PLENTRY entry = du->bufferList; entry != nullptr; entry = entry->next) {
		total_size += entry->length;
	}

	PackedByteArray frame_data;
	frame_data.resize(total_size);
	uint8_t* write_ptr = frame_data.ptrw();
	size_t offset = 0;
	for (PLENTRY entry = du->bufferList; entry != nullptr; entry = entry->next) {
		memcpy(write_ptr + offset, entry->data, entry->length);
		offset += entry->length;
	}

	LiCompleteVideoFrame(handle, DR_OK);
	return frame_data;
}

// ========== Input Events ==========

void MoonlightStream::send_mouse_button_event(int button, bool pressed) {
	if (streaming) {
		LiSendMouseButtonEvent(
			pressed ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE,
			button
		);
	}
}

void MoonlightStream::send_mouse_move_event(float x, float y) {
	if (streaming) {
		LiSendMouseMoveEvent(static_cast<short>(x), static_cast<short>(y));
	}
}

void MoonlightStream::send_key_event(int scancode, bool pressed) {
	if (streaming) {
		LiSendKeyboardEvent(
			static_cast<short>(scancode),
			pressed ? KEY_ACTION_DOWN : KEY_ACTION_UP,
			0 // modifiers
		);
	}
}