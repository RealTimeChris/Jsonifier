#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif
#include <jsonifier/Index.hpp>
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <chrono>

struct search_metadata_data {
	std::string next_results{};
	std::string since_id_str{};
	std::string refresh_url{};
	std::string max_id_str{};
	double completed_in{};
	std::string query{};
	int64_t since_id{};
	int64_t count{};
	double max_id{};
};

struct hashtag {
	std::vector<int64_t> indices{};
	std::string text{};
};

struct large_data {
	std::string resize{};
	int64_t w{};
	int64_t h{};
};

struct sizes_data {
	large_data large{};
	large_data medium{};
	large_data thumb{};
	large_data small{};
};

struct media_data {
	std::string source_status_id_str{};
	std::vector<int64_t> indices{};
	std::string media_url_https{};
	std::string expanded_url{};
	std::string display_url{};
	double source_status_id{};
	std::string media_url{};
	std::string id_str{};
	std::string type{};
	std::string url{};
	sizes_data sizes{};
	double id{};
};

struct url_data {
	std::vector<int64_t> indices{};
	std::string expanded_url{};
	std::string display_url{};
	std::string url{};
};

struct user_mention {
	std::vector<int64_t> indices{};
	std::string screen_name{};
	std::string id_str{};
	std::string name{};
	int64_t id{};
};

struct status_entities {
	std::vector<user_mention> user_mentions{};
	std::vector<std::string> symbols{};
	std::vector<hashtag> hashtags{};
	std::vector<media_data> media{};
	std::vector<url_data> urls{};
};

struct metadata_data {
	std::string iso_language_code{};
	std::string result_type{};
};

struct description_data  {
	std::vector<url_data> urls{};
};

struct user_entities {
	description_data description{};
	description_data url{};
};

struct user_data_twitter {
	std::string profile_background_image_url_https{};
	std::string profile_background_image_url{};
	std::string profile_sidebar_border_color{};
	std::string profile_sidebar_fill_color{};
	std::string profile_background_color{};
	std::string profile_image_url_https{};
	std::string profile_banner_url{};
	std::string profile_link_color{};
	std::string profile_text_color{};
	std::string profile_image_url{};
	std::string description{};
	int64_t favourites_count{};
	std::string screen_name{};
	user_entities entities{};
	int64_t followers_count{};
	std::string created_at{};
	int64_t statuses_count{};
	std::string location{};
	int64_t friends_count{};
	std::string time_zone{};
	int64_t listed_count{};
	std::string id_str{};
	int64_t utc_offset{};
	std::string name{};
	std::string lang{};
	std::string url{};
	int64_t id{};
	bool profile_use_background_image{};
	bool profile_background_tile{};
	bool is_translation_enabled{};
	bool contributors_enabled{};
	bool default_profile_image{};
	bool follow_request_sent{};
	bool default_profile{};
	bool user_protected{};
	bool notifications{};
	bool is_translator{};
	bool geo_enabled{};
	bool following{};
	bool verified{};
	uint8_t padding[3]{};
};

struct status {
	std::string in_reply_to_status_id_str{};
	std::string in_reply_to_screen_name{};
	std::string in_reply_to_user_id_str{};
	double in_reply_to_status_id{};
	std::string contributors{};
	status_entities entities{};
	int64_t in_reply_to_user_id{};
	std::string coordinates{};
	std::string created_at{};
	int64_t favorite_count{};
	metadata_data metadata{};
	int64_t retweet_count{};
	std::string source{};
	std::string id_str{};
	std::string place{};
	std::string text{};
	std::string lang{};
	std::string geo{};
	user_data_twitter user{};
	double id{};
	bool possibly_sensitive{};
	bool truncated{};
	bool favorited{};
	bool retweeted{};
};

struct twitter_message {
	search_metadata_data metadata{};
	std::vector<status> statuses{};
};

struct available_tag {
	std::string name;
	bool moderated;
	std::string id;
	std::string emoji_name;
	std::string emoji_id;
};

struct default_reaction_emoji_data {
	std::string emoji_name;
	std::string emoji_id;
};

struct icon_emoji_data {
	std::string name;
	std::string id;
};

struct permission_overwrite {
	int64_t type;
	std::string id;
	std::string deny;
	std::string allow;
};

struct channel_data {
	int64_t version;
	int64_t type;
	int64_t position;
	std::vector<permission_overwrite> permission_overwrites;
	std::string name;
	std::string id;
	int64_t flags;
	std::string topic;
	int64_t rate_limit_per_user;
	std::string parent_id;
	std::string last_message_id;
	std::optional<icon_emoji_data> icon_emoji;
	int64_t user_limit;
	std::string status;
	std::string rtc_region;
	int64_t bitrate;
	std::string last_pin_timestamp;
	std::string channel_template;
	std::optional<default_reaction_emoji_data> default_reaction_emoji;
	std::optional<std::vector<available_tag>> available_tags;
	std::string theme_color;
	std::optional<bool> nsfw;
};

struct emoji_element {
	int64_t version;
	std::vector<std::string> roles;
	bool require_colons;
	std::string name;
	bool managed;
	std::string id;
	bool available;
	bool animated;
};

struct avatar_decoration_data_data {
	std::string sku_id;
	std::string asset;
};

struct member_user {
	std::string username;
	int64_t public_flags;
	std::string id;
	std::string global_name;
	std::string display_name;
	std::string discriminator;
	bool bot;
	std::optional<avatar_decoration_data_data> avatar_decoration_data;
	std::string avatar;
};

struct guild_member_data {
	member_user user;
	std::vector<std::string> roles;
	std::string premium_since;
	bool pending;
	std::string nick;
	bool mute;
	std::string joined_at;
	int64_t flags;
	bool deaf;
	std::string communication_disabled_until;
	std::string avatar;
};

struct activity_emoji {
	std::string name;
	std::string id;
	bool animated;
};

struct activity {
	int64_t type;
	std::string state;
	std::string name;
	std::string id;
	std::optional<activity_emoji> emoji;
	int64_t created_at;
	std::string url;
};

struct client_status_data {
	std::string web;
	std::string desktop;
	std::string mobile;
};

struct presence_user {
	std::string id;
};

struct presence {
	presence_user user;
	std::string status;
	client_status_data client_status;
	std::string broadcast;
	std::vector<activity> activities;
};

struct tags_data {
	std::string bot_id;
	std::string premium_subscriber;
};

struct role_data {
	int64_t version;
	std::string unicode_emoji;
	tags_data tags;
	int64_t position;
	std::string permissions;
	std::string name;
	bool mentionable;
	bool managed;
	std::string id;
	std::string icon;
	bool hoist;
	int64_t flags;
	int64_t color;
};

struct soundboard_sound {
	double volume;
	std::string user_id;
	std::string sound_id;
	std::string name;
	std::string guild_id;
	std::string emoji_name;
	std::string emoji_id;
	bool available;
};

struct sticker {
	int64_t version;
	int64_t type;
	std::string tags;
	std::string name;
	std::string id;
	std::string guild_id;
	int64_t format_type;
	std::string description;
	bool available;
};

struct guild_data {
	bool lazy;
	int64_t system_channel_flags;
	std::vector<guild_member_data> members;
	std::string joined_at;
	int64_t premium_subscription_count;
	int64_t explicit_content_filter;
	std::string discovery_splash;
	std::string hub_type;
	std::vector<std::string> guild_scheduled_events;
	int64_t mfa_level;
	std::string id;
	bool unavailable;
	std::vector<std::string> voice_states;
	bool nsfw;
	int64_t verification_level;
	std::string name;
	std::string safety_alerts_channel_id;
	std::string preferred_locale;
	int64_t nsfw_level;
	std::string latest_onboarding_question_id;
	int64_t default_message_notifications;
	std::vector<presence> presences;
	std::vector<std::string> threads;
	int64_t max_stage_video_channel_users;
	bool premium_progress_bar_enabled;
	int64_t afk_timeout;
	std::string region;
	std::string description;
	bool large;
	std::string inventory_settings;
	int64_t max_members;
	std::vector<sticker> stickers;
	std::vector<std::string> features;
	std::string public_updates_channel_id;
	std::string splash;
	std::string rules_channel_id;
	int64_t max_video_channel_users;
	std::string application_id;
	std::vector<role_data> roles;
	std::string vanity_url_code;
	std::vector<soundboard_sound> soundboard_sounds;
	std::string incidents_data;
	std::vector<channel_data> channels;
	std::string system_channel_id;
	int64_t member_count;
	std::vector<std::string> embedded_activities;
	std::string afk_channel_id;
	std::string home_header;
	int64_t premium_tier;
	std::map<std::string, int64_t> application_command_counts;
	std::vector<std::string> stage_instances;
	int64_t version;
	std::string banner;
	std::string owner_id;
	std::string icon;
	std::vector<emoji_element> emojis;
};

struct discord_message {
	std::string t;
	int64_t s;
	int64_t op;
	guild_data d;
};

template<> struct jsonifier::core<search_metadata_data> {
	using value_type				 = search_metadata_data;
	static constexpr auto parseValue = createValue<&value_type::completed_in, &value_type::max_id, &value_type::max_id_str, &value_type::next_results, &value_type::query,
		&value_type::refresh_url, &value_type::count, &value_type::since_id, &value_type::since_id_str>();
};

template<> struct jsonifier::core<hashtag> {
	using value_type				 = hashtag;
	static constexpr auto parseValue = createValue<&value_type::text, &value_type::indices>();
};

template<> struct jsonifier::core<large_data> {
	using value_type				 = large_data;
	static constexpr auto parseValue = createValue<&value_type::w, &value_type::h, &value_type::resize>();
};

template<> struct jsonifier::core<sizes_data> {
	using value_type				 = sizes_data;
	static constexpr auto parseValue = createValue<&value_type::medium, &value_type::small, &value_type::thumb, &value_type::large>();
};

template<> struct jsonifier::core<media_data> {
	using value_type				 = media_data;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::id_str, &value_type::indices, &value_type::media_url, &value_type::media_url_https,
		&value_type::display_url, &value_type::expanded_url, &value_type::type, &value_type::sizes, &value_type::source_status_id, &value_type::source_status_id_str>();
};

template<> struct jsonifier::core<url_data> {
	using value_type				 = url_data;
	static constexpr auto parseValue = createValue<&value_type::expanded_url, &value_type::display_url, &value_type::indices>();
};

template<> struct jsonifier::core<user_mention> {
	using value_type				 = user_mention;
	static constexpr auto parseValue = createValue<&value_type::screen_name, &value_type::name, &value_type::id, &value_type::id_str, &value_type::indices>();
};

template<> struct jsonifier::core<status_entities> {
	using value_type				 = status_entities;
	static constexpr auto parseValue = createValue<&value_type::hashtags, &value_type::symbols, &value_type::urls, &value_type::user_mentions, &value_type::media>();
};

template<> struct jsonifier::core<metadata_data> {
	using value_type				 = metadata_data;
	static constexpr auto parseValue = createValue<&value_type::result_type, &value_type::iso_language_code>();
};

template<> struct jsonifier::core<description_data> {
	using value_type				 = description_data;
	static constexpr auto parseValue = createValue<&value_type::urls>();
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue<&value_type::description>();
};

template<> struct jsonifier::core<user_data_twitter> {
	using value_type				 = user_data_twitter;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::id_str, &value_type::name, &value_type::screen_name, &value_type::location,
		&value_type::description, &value_type::entities, &value_type::user_protected, &value_type::followers_count, &value_type::friends_count, &value_type::listed_count,
		&value_type::created_at, &value_type::favourites_count, &value_type::geo_enabled, &value_type::verified, &value_type::statuses_count, &value_type::lang,
		&value_type::contributors_enabled, &value_type::is_translator, &value_type::is_translation_enabled, &value_type::profile_background_color,
		&value_type::profile_background_image_url, &value_type::profile_background_image_url_https, &value_type::profile_background_tile, &value_type::profile_image_url,
		&value_type::profile_image_url_https, &value_type::profile_banner_url, &value_type::profile_link_color, &value_type::profile_sidebar_border_color,
		&value_type::profile_sidebar_fill_color, &value_type::profile_text_color, &value_type::profile_use_background_image, &value_type::default_profile,
		&value_type::default_profile_image, &value_type::following, &value_type::follow_request_sent, &value_type::notifications>();
};

template<> struct jsonifier::core<status> {
	using value_type				 = status;
	static constexpr auto parseValue = createValue<&value_type::metadata, &value_type::created_at, &value_type::id, &value_type::id_str, &value_type::text,
		&value_type::source, &value_type::truncated, &value_type::user, &value_type::retweet_count, &value_type::favorite_count, &value_type::entities, &value_type::favorited,
		&value_type::retweeted, &value_type::lang, &value_type::possibly_sensitive>();
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue<&value_type::statuses, &value_type::metadata>();
};

#if !defined(ASAN)
template<> struct glz::meta<search_metadata_data> {
	using value_type = search_metadata_data;
	static constexpr auto value =
		object("completed_in", &value_type::completed_in, "max_id", &value_type::max_id, "max_id_str", &value_type::max_id_str, "next_results", &value_type::next_results, "query",
			&value_type::query, "refresh_url", &value_type::refresh_url, "count", &value_type::count, "since_id", &value_type::since_id, "since_id_str", &value_type::since_id_str);
};

template<> struct glz::meta<hashtag> {
	using value_type			= hashtag;
	static constexpr auto value = object("text", &value_type::text, "indices", &value_type::indices);
};

template<> struct glz::meta<large_data> {
	using value_type			= large_data;
	static constexpr auto value = object("w", &value_type::w, "h", &value_type::h, "resize", &value_type::resize);
};

template<> struct glz::meta<sizes_data> {
	using value_type			= sizes_data;
	static constexpr auto value = object("medium", &value_type::medium, "small", &value_type::small, "thumb", &value_type::thumb, "large", &value_type::large);
};

template<> struct glz::meta<media_data> {
	using value_type			= media_data;
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::id_str, "indices", &value_type::indices, "media_url", &value_type::media_url,
		"media_url_https", &value_type::media_url_https, "display_url", &value_type::display_url, "expanded_url", &value_type::expanded_url, "type", &value_type::type, "sizes",
		&value_type::sizes, "source_status_id", &value_type::source_status_id, "source_status_id_str", &value_type::source_status_id_str);
};

template<> struct glz::meta<url_data> {
	using value_type			= url_data;
	static constexpr auto value = object("expanded_url", &value_type::expanded_url, "display_url", &value_type::display_url, "indices", &value_type::indices);
};

template<> struct glz::meta<user_mention> {
	using value_type = user_mention;
	static constexpr auto value =
		object("screen_name", &value_type::screen_name, "name", &value_type::name, "id", &value_type::id, "id_str", &value_type::id_str, "indices", &value_type::indices);
};

template<> struct glz::meta<status_entities> {
	using value_type			= status_entities;
	static constexpr auto value = object("hashtags", &value_type::hashtags, "symbols", &value_type::symbols, "urls", &value_type::urls, "user_mentions", &value_type::user_mentions,
		"media", &value_type::media);
};

template<> struct glz::meta<metadata_data> {
	using value_type			= metadata_data;
	static constexpr auto value = object("result_type", &value_type::result_type, "iso_language_code", &value_type::iso_language_code);
};

template<> struct glz::meta<description_data> {
	using value_type			= description_data;
	static constexpr auto value = object("urls", &value_type::urls);
};

template<> struct glz::meta<user_entities> {
	using value_type			= user_entities;
	static constexpr auto value = object("description", &value_type::description);
};

template<> struct glz::meta<user_data_twitter> {
	using value_type			= user_data_twitter;
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::id_str, "name", &value_type::name, "screen_name", &value_type::screen_name, "location",
		&value_type::location, "description", &value_type::description, "entities", &value_type::entities, "protected", &value_type::user_protected, "followers_count",
		&value_type::followers_count, "friends_count", &value_type::friends_count, "listed_count", &value_type::listed_count, "created_at", &value_type::created_at,
		"favourites_count", &value_type::favourites_count, "geo_enabled", &value_type::geo_enabled, "verified", &value_type::verified, "statuses_count",
		&value_type::statuses_count, "lang", &value_type::lang, "contributors_enabled", &value_type::contributors_enabled, "is_translator", &value_type::is_translator,
		"is_translation_enabled", &value_type::is_translation_enabled, "profile_background_color", &value_type::profile_background_color, "profile_background_image_url",
		&value_type::profile_background_image_url, "profile_background_image_url_https", &value_type::profile_background_image_url_https, "profile_background_tile",
		&value_type::profile_background_tile, "profile_image_url", &value_type::profile_image_url, "profile_image_url_https", &value_type::profile_image_url_https,
		"profile_banner_url", &value_type::profile_banner_url, "profile_link_color", &value_type::profile_link_color, "profile_sidebar_border_color",
		&value_type::profile_sidebar_border_color, "profile_sidebar_fill_color", &value_type::profile_sidebar_fill_color, "profile_text_color", &value_type::profile_text_color,
		"profile_use_background_image", &value_type::profile_use_background_image, "default_profile", &value_type::default_profile, "default_profile_image",
		&value_type::default_profile_image, "following", &value_type::following, "follow_request_sent", &value_type::follow_request_sent, "notifications",
		&value_type::notifications);
};

template<> struct glz::meta<status> {
	using value_type			= status;
	static constexpr auto value = object("metadata", &value_type::metadata, "created_at", &value_type::created_at, "id", &value_type::id, "id_str", &value_type::id_str, "text",
		&value_type::text, "source", &value_type::source, "truncated", &value_type::truncated, "user", &value_type::user, "retweet_count", &value_type::retweet_count,
		"favorite_count", &value_type::favorite_count, "entities", &value_type::entities, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "lang",
		&value_type::lang, "possibly_sensitive", &value_type::possibly_sensitive);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("statuses", &value_type::statuses, "metadata", &value_type::metadata);
};

#endif
#if !defined(ASAN)
namespace glz {

	template<> struct meta<available_tag> {
		using value_type = available_tag;
		static constexpr auto value =
			object("name", &value_type::name, "moderated", &value_type::moderated, "id", &value_type::id);
	};

	template<> struct meta<default_reaction_emoji_data> {
		using value_type			= default_reaction_emoji_data;
		static constexpr auto value = object();
	};

	template<> struct meta<icon_emoji_data> {
		using value_type			= icon_emoji_data;
		static constexpr auto value = object("name", &value_type::name);
	};

	template<> struct meta<permission_overwrite> {
		using value_type			= permission_overwrite;
		static constexpr auto value = object("type", &value_type::type, "id", &value_type::id, "deny", &value_type::deny, "allow", &value_type::allow);
	};

	template<> struct meta<channel_data> {
		using value_type			= channel_data;
		static constexpr auto value = object("version", &value_type::version, "type", &value_type::type, "position", &value_type::position, "permission_overwrites",
			&value_type::permission_overwrites, "name", &value_type::name, "id", &value_type::id, "flags", &value_type::flags, "rate_limit_per_user",
			&value_type::rate_limit_per_user, "parent_id", &value_type::parent_id, "icon_emoji", &value_type::icon_emoji, "user_limit", &value_type::user_limit, "bitrate",
			&value_type::bitrate, "last_pin_timestamp", &value_type::last_pin_timestamp, "channel_template", &value_type::channel_template, "default_reaction_emoji",
			&value_type::default_reaction_emoji, "available_tags", &value_type::available_tags, "nsfw", &value_type::nsfw);
	};

	template<> struct meta<emoji_element> {
		using value_type			= emoji_element;
		static constexpr auto value = object("version", &value_type::version, "roles", &value_type::roles, "require_colons", &value_type::require_colons, "name", &value_type::name,
			"managed", &value_type::managed, "id", &value_type::id, "available", &value_type::available, "animated", &value_type::animated);
	};

	template<> struct meta<avatar_decoration_data_data> {
		using value_type			= avatar_decoration_data_data;
		static constexpr auto value = object("sku_id", &value_type::sku_id, "asset", &value_type::asset);
	};

	template<> struct meta<member_user> {
		using value_type			= member_user;
		static constexpr auto value = object("username", &value_type::username, "public_flags", &value_type::public_flags, "id", &value_type::id, "discriminator",
			&value_type::discriminator, "bot", &value_type::bot, "avatar_decoration_data", &value_type::avatar_decoration_data);
	};

	template<> struct meta<guild_member_data> {
		using value_type			= guild_member_data;
		static constexpr auto value = object("user", &value_type::user, "roles", &value_type::roles, "pending", &value_type::pending, "mute", &value_type::mute, "joined_at",
			&value_type::joined_at, "flags", &value_type::flags, "deaf", &value_type::deaf);
	};

	template<> struct meta<activity_emoji> {
		using value_type			= activity_emoji;
		static constexpr auto value = object("name", &value_type::name, "id", &value_type::id, "animated", &value_type::animated);
	};

	template<> struct meta<activity> {
		using value_type			= activity;
		static constexpr auto value = object("type", &value_type::type, "state", &value_type::state, "name", &value_type::name, "id", &value_type::id, "emoji", &value_type::emoji,
			"created_at", &value_type::created_at, "url", &value_type::url);
	};

	template<> struct meta<client_status_data> {
		using value_type			= client_status_data;
		static constexpr auto value = object("web", &value_type::web, "desktop", &value_type::desktop, "mobile", &value_type::mobile);
	};

	template<> struct meta<presence_user> {
		using value_type			= presence_user;
		static constexpr auto value = object("id", &value_type::id);
	};

	template<> struct meta<presence> {
		using value_type			= presence;
		static constexpr auto value = object("user", &value_type::user, "client_status", &value_type::client_status, "activities", &value_type::activities);
	};

	template<> struct meta<tags_data> {
		using value_type			= tags_data;
		static constexpr auto value = object("bot_id", &value_type::bot_id);
	};

	template<> struct meta<role_data> {
		using value_type			= role_data;
		static constexpr auto value = object("version", &value_type::version, "tags", &value_type::tags, "position", &value_type::position, "permissions", &value_type::permissions,
			"name", &value_type::name, "mentionable", &value_type::mentionable, "managed", &value_type::managed, "id", &value_type::id, "hoist", &value_type::hoist, "flags",
			&value_type::flags, "color", &value_type::color);
	};

	template<> struct meta<soundboard_sound> {
		using value_type			= soundboard_sound;
		static constexpr auto value = object("volume", &value_type::volume, "user_id", &value_type::user_id, "sound_id", &value_type::sound_id, "name", &value_type::name,
			"guild_id", &value_type::guild_id, "available", &value_type::available);
	};

	template<> struct meta<sticker> {
		using value_type = sticker;
		static constexpr auto value = object("version", &value_type::version, "type", &value_type::type, "tags", &value_type::tags, "name", &value_type::name, "id",
			&value_type::id, "guild_id", &value_type::guild_id, "format_type", &value_type::format_type, "available", &value_type::available);
	};

	template<> struct meta<guild_data> {
		using value_type			= guild_data;
		static constexpr auto value = object("lazy", &value_type::lazy, "system_channel_flags", &value_type::system_channel_flags, "members", &value_type::members, "joined_at",
			&value_type::joined_at, "premium_subscription_count", &value_type::premium_subscription_count, "explicit_content_filter", &value_type::explicit_content_filter,
			"guild_scheduled_events", &value_type::guild_scheduled_events, "mfa_level", &value_type::mfa_level, "id", &value_type::id, "unavailable", &value_type::unavailable,
			"voice_states", &value_type::voice_states, "nsfw", &value_type::nsfw, "verification_level", &value_type::verification_level, "name", &value_type::name,
			"preferred_locale", &value_type::preferred_locale, "nsfw_level", &value_type::nsfw_level, "latest_onboarding_question_id", &value_type::latest_onboarding_question_id,
			"default_message_notifications", &value_type::default_message_notifications, "presences", &value_type::presences, "threads", &value_type::threads,
			"max_stage_video_channel_users", &value_type::max_stage_video_channel_users, "premium_progress_bar_enabled", &value_type::premium_progress_bar_enabled, "afk_timeout",
			&value_type::afk_timeout, "region", &value_type::region, "large", &value_type::large, "max_members", &value_type::max_members, "stickers", &value_type::stickers,
			"features", &value_type::features, "public_updates_channel_id", &value_type::public_updates_channel_id, "rules_channel_id", &value_type::rules_channel_id,
			"max_video_channel_users", &value_type::max_video_channel_users, "roles", &value_type::roles, "soundboard_sounds", &value_type::soundboard_sounds, "channels",
			&value_type::channels, "system_channel_id", &value_type::system_channel_id, "member_count", &value_type::member_count, "embedded_activities",
			&value_type::embedded_activities, "premium_tier", &value_type::premium_tier, "application_command_counts", &value_type::application_command_counts, "stage_instances",
			&value_type::stage_instances, "version", &value_type::version, "owner_id", &value_type::owner_id, "emojis", &value_type::emojis);
	};

	template<> struct meta<discord_message> {
		using value_type			= discord_message;
		static constexpr auto value = object("t", &value_type::t, "s", &value_type::s, "op", &value_type::op, "d", &value_type::d);
	};

}// namespace glz

#endif

template<> struct jsonifier::core<available_tag> {
	using value_type				 = available_tag;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::moderated, &value_type::id>();
};

template<> struct jsonifier::core<default_reaction_emoji_data> {
	using value_type				 = default_reaction_emoji_data;
	static constexpr auto parseValue = createValue<>();
};

template<> struct jsonifier::core<icon_emoji_data> {
	using value_type				 = icon_emoji_data;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::id>();
};

template<> struct jsonifier::core<permission_overwrite> {
	using value_type				 = permission_overwrite;
	static constexpr auto parseValue = createValue<&value_type::type, &value_type::id, &value_type::deny, &value_type::allow>();
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue =
		createValue<&value_type::version, &value_type::type, &value_type::position, &value_type::permission_overwrites, &value_type::name, &value_type::id,
			&value_type::flags, &value_type::rate_limit_per_user, &value_type::parent_id, &value_type::icon_emoji, &value_type::user_limit, &value_type::bitrate,  
			&value_type::last_pin_timestamp, &value_type::channel_template, &value_type::default_reaction_emoji, &value_type::available_tags, &value_type::nsfw>();
};

template<> struct jsonifier::core<emoji_element> {
	using value_type				 = emoji_element;
	static constexpr auto parseValue = createValue<&value_type::version, &value_type::roles, &value_type::require_colons, &value_type::name, &value_type::managed,
		&value_type::id, &value_type::available, &value_type::animated>();
};

template<> struct jsonifier::core<avatar_decoration_data_data> {
	using value_type				 = avatar_decoration_data_data;
	static constexpr auto parseValue = createValue<&value_type::sku_id, &value_type::asset>();
};

// Repeat the above pattern for the remaining structures...

template<> struct jsonifier::core<member_user> {
	using value_type				 = member_user;
	static constexpr auto parseValue = createValue<&value_type::username, &value_type::public_flags, &value_type::id, &value_type::discriminator, &value_type::bot,
		&value_type::avatar_decoration_data>();
};

template<> struct jsonifier::core<guild_member_data> {
	using value_type				 = guild_member_data;
	static constexpr auto parseValue = createValue<&value_type::user, &value_type::roles, &value_type::pending, &value_type::mute, &value_type::joined_at,
		&value_type::flags, &value_type::deaf, &value_type::avatar>();
};

template<> struct jsonifier::core<activity_emoji> {
	using value_type				 = activity_emoji;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::id, &value_type::animated>();
};

template<> struct jsonifier::core<activity> {
	using value_type = activity;
	static constexpr auto parseValue =
		createValue<&value_type::type, &value_type::state, &value_type::name, &value_type::id, &value_type::emoji, &value_type::created_at, &value_type::url>();
};

template<> struct jsonifier::core<client_status_data> {
	using value_type				 = client_status_data;
	static constexpr auto parseValue = createValue<&value_type::web, &value_type::desktop, &value_type::mobile>();
};

template<> struct jsonifier::core<presence_user> {
	using value_type				 = presence_user;
	static constexpr auto parseValue = createValue<&value_type::id>();
};

template<> struct jsonifier::core<presence> {
	using value_type = presence;
	static constexpr auto parseValue = createValue<&value_type::user, &value_type::client_status, &value_type::activities>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<&value_type::bot_id>();
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue<&value_type::version, &value_type::tags, &value_type::position, &value_type::permissions, &value_type::name,
		&value_type::mentionable, &value_type::managed, &value_type::id, &value_type::hoist, &value_type::flags, &value_type::color>();
};

template<> struct jsonifier::core<soundboard_sound> {
	using value_type				 = soundboard_sound;
	static constexpr auto parseValue =
		createValue<&value_type::volume, &value_type::user_id, &value_type::sound_id, &value_type::name, &value_type::guild_id, &value_type::available>();
};

template<> struct jsonifier::core<sticker> {
	using value_type				 = sticker;
	static constexpr auto parseValue = createValue<&value_type::version, &value_type::type, &value_type::tags, &value_type::name, &value_type::id, &value_type::guild_id,
		&value_type::format_type, &value_type::available>();
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue<&value_type::lazy, &value_type::system_channel_flags, &value_type::members, &value_type::joined_at,
		&value_type::premium_subscription_count, &value_type::explicit_content_filter, &value_type::guild_scheduled_events, &value_type::mfa_level, &value_type::id,
		&value_type::unavailable, &value_type::voice_states, &value_type::nsfw, &value_type::verification_level, &value_type::name, &value_type::preferred_locale,
		&value_type::nsfw_level, &value_type::latest_onboarding_question_id, &value_type::default_message_notifications, &value_type::presences, &value_type::threads,
		&value_type::max_stage_video_channel_users, &value_type::premium_progress_bar_enabled, &value_type::afk_timeout, &value_type::region, &value_type::large,
		&value_type::max_members, &value_type::stickers, &value_type::features, &value_type::public_updates_channel_id, &value_type::rules_channel_id,
		&value_type::max_video_channel_users, &value_type::roles, &value_type::soundboard_sounds, &value_type::channels, &value_type::system_channel_id,
		&value_type::member_count, &value_type::embedded_activities, &value_type::premium_tier, &value_type::application_command_counts, &value_type::stage_instances, &value_type::version,
		&value_type::owner_id, &value_type::emojis>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type				 = discord_message;
	static constexpr auto parseValue = createValue<&value_type::t, &value_type::s, &value_type::op, &value_type::d>();
};

struct test_struct {
	std::vector<std::string> testStrings{};
	jsonifier::vector<bool> testBools{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
};

struct json_data {
	jsonifier::string theData{};
	std::vector<int32_t> arraySizes{};
};

template<typename OTy> struct Test {
	std::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename OTy> struct TestGenerator {
	std::vector<OTy> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937 gen{ randomEngine() };

	static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

	template<typename value_type> value_type randomizeNumber(value_type mean, value_type stdDeviation) {
		if (stdDeviation >= mean) {
			mean = stdDeviation + 1;
		} else if (stdDeviation == 0) {
			stdDeviation = 1;
		}
		std::normal_distribution<> normalDistributionTwo{ static_cast<double>(mean), static_cast<double>(stdDeviation) };
		auto theResult = normalDistributionTwo(randomEngine);
		if (theResult < 0) {
			theResult = -theResult;
		}
		return theResult;
	}

	JSONIFIER_INLINE static json_data generateJsonData() {
		std::string buffer{};
		TestGenerator generator{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson(generator, buffer);
		json_data returnData{};
		returnData.theData = buffer;
		return returnData;
	}

	std::string generateString() {
		auto length{ randomizeNumber(5.0f, 3.0f) };
		static int32_t charsetSize = charset.size();
		std::mt19937 generator(std::random_device{}());
		std::uniform_int_distribution<int32_t> distribution(0, charsetSize - 1);
		std::string result{};
		for (int32_t x = 0; x < length; ++x) {
			result += charset[distribution(generator)];
		}
		return result;
	}

	double generateDouble() {
		auto newValue = randomizeNumber(double{}, std::numeric_limits<double>::max() / 50000000);
		return generateBool() ? newValue : -newValue;
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumber(50.0f, 50.0f) >= 50.0f);
	};

	uint64_t generateUint() {
		return randomizeNumber(std::numeric_limits<uint64_t>::max() / 2, std::numeric_limits<uint64_t>::max() / 2);
	};

	int64_t generateInt() {
		auto newValue = randomizeNumber(int64_t{}, std::numeric_limits<int64_t>::max());
		return generateBool() ? newValue : -newValue;
	};

	TestGenerator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumber(20, 10);
			auto arraySize02 = randomizeNumber(30, 10);
			auto arraySize03 = randomizeNumber(20, 10);
			v.resize(arraySize01);
			for (uint64_t x = 0; x < arraySize01; ++x) {
				auto arraySize01 = randomizeNumber(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					auto newString = generateString();
					v[x].testStrings.emplace_back(newString);
				}
				arraySize01 = randomizeNumber(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testUints.emplace_back(generateUint());
				}
				arraySize01 = randomizeNumber(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testInts.emplace_back(generateInt());
				}
				arraySize01 = randomizeNumber(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testBools.emplace_back(generateBool());
				}
				arraySize01 = randomizeNumber(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testDoubles.emplace_back(generateDouble());
				}
			}
		};

		fill(a);
		fill(b);
		fill(c);
		fill(d);
		fill(e);
		fill(f);
		fill(g);
		fill(h);
		fill(i);
		fill(j);
		fill(k);
		fill(l);
		fill(m);
		fill(n);
		fill(o);
		fill(p);
		fill(q);
		fill(r);
		fill(s);
		fill(t);
		fill(u);
		fill(v);
		fill(w);
		fill(x);
		fill(y);
		fill(z);
	}
};

GLZ_META(test_struct, testBools, testInts, testUints, testDoubles, testStrings);
GLZ_META(Test<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
GLZ_META(TestGenerator<test_struct>, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z);
template<typename OTy> struct AbcTest {
	std::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using OTy = test_struct;
	static constexpr auto parseValue =
		createValue("testBools", &OTy::testBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles", &OTy::testDoubles, "testStrings", &OTy::testStrings);
};

template<> struct jsonifier::core<Test<test_struct>> {
	using OTy						 = Test<test_struct>;
	static constexpr auto parseValue = createValue("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};


template<> struct jsonifier::core<TestGenerator<test_struct>> {
	using OTy						 = TestGenerator<test_struct>;
	static constexpr auto parseValue = createValue("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i,
		"j", &OTy::j, "k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v",
		&OTy::v, "w", &OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using OTy						 = AbcTest<test_struct>;
	static constexpr auto parseValue = createValue("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r", &OTy::r,
		"q", &OTy::q, "p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e",
		&OTy::e, "d", &OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

GLZ_META(AbcTest<test_struct>, z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a);

#if defined(NDEBUG)
constexpr uint64_t iterations = 400;
#else
constexpr uint64_t iterations = 1;
#endif

struct test_result {
	std::string libraryName{};
	std::string resultType{};
	uint64_t resultSpeed{};
	std::string color{};
};

struct test_results {
	std::vector<test_result> results{};
	std::string markdownResults{};
	std::string testName{};
};

template<> struct jsonifier::core<test_result> {
	using value_type = test_result;
	static constexpr auto parseValue =
		createValue("libraryName", &value_type::libraryName, "resultSpeed", &value_type::resultSpeed, "resultType", &value_type::resultType, "color", &value_type::color);
};

template<> struct jsonifier::core<test_results> {
	using value_type				 = test_results;
	static constexpr auto parseValue = createValue("results", &value_type::results, "testName", &value_type::testName);
};

struct results {
	std::string name{};
	std::string test{};
	std::string url{};
	uint64_t iterations{};
	std::string wColor{};
	std::string rColor{};

	std::optional<uint64_t> json_write_byte_length{};
	std::optional<uint64_t> json_read_byte_length{};
	std::optional<double> json_read{};
	std::optional<double> json_write{};

	bool operator<(const results& other) const {
		bool resultsNew{};
		if (json_read && other.json_read) {
			resultsNew = *this->json_read < *other.json_read;
		} else if (json_write && other.json_write) {
			resultsNew = *this->json_write < *other.json_write;
		}
		return resultsNew;
	}

	void print() const {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";

		if (json_read_byte_length && json_read) {
			auto mbReadCount	 = static_cast<double>(*json_read_byte_length) / 1e+6l;
			auto readSecondCount = *json_read / 1e+9l;
			std::cout << "Read Length: " << *json_read_byte_length << std::endl;
			std::cout << "Read: " << mbReadCount / readSecondCount << " MB/s\n";
		}
		if (json_write_byte_length && json_write) {
			auto mbWrittenCount	  = static_cast<double>(*json_write_byte_length) / 1e+6l;
			auto writeSecondCount = *json_write / 1e+9l;
			std::cout << "Write Length: " << *json_write_byte_length << std::endl;
			std::cout << "Write: " << mbWrittenCount / writeSecondCount << " MB/s\n";
		}

		std::cout << "\n---" << std::endl;
	}

	test_result getWriteResults() const {
		test_result result{};
		if (json_write_byte_length && json_write) {
			result.libraryName	  = name;
			auto mbWrittenCount	  = static_cast<double>(*json_write_byte_length) / 1e+6l;
			auto writeSecondCount = *json_write / 1e+9l;
			result.resultSpeed	  = mbWrittenCount / writeSecondCount;
			result.resultType	  = "Write";
			result.color		  = wColor;
		}
		return result;
	}

	test_result getReadResults() const {
		test_result result{};
		if (json_read_byte_length && json_read) {
			result.libraryName	 = name;
			auto mbWrittenCount	 = static_cast<double>(*json_read_byte_length) / 1e+6l;
			auto readSecondCount = *json_read / 1e+9l;
			result.resultSpeed	 = mbWrittenCount / readSecondCount;
			result.resultType	 = "Read";
			result.color		 = rColor;
		}
		return result;
	}

	std::string jsonStats() {
		std::string write{};
		std::string read{};
		std::string finalstring{};
		if (json_read && json_read_byte_length) {
			double mbReadCount	   = static_cast<double>(*json_read_byte_length) / 1e+6l;
			double readSecondCount = *json_read / 1e+9l;
			std::stringstream stream01{};
			stream01 << static_cast<double>(mbReadCount / readSecondCount);
			read = stream01.str();
		} else {
			read = "N/A";
		}
		if (json_write && json_write_byte_length) {
			double mbWrittenCount	= static_cast<double>(*json_write_byte_length) / 1e+6l;
			double writeSecondCount = *json_write / 1e+9l;
			std::stringstream stream01{};
			stream01 << static_cast<double>(mbWrittenCount / writeSecondCount);
			write = stream01.str();
		} else {
			write = "N/A";
		}

		finalstring = std::string{ "| [" + name + "](" + url + ") | " + read + " | " + write + " |" };
		return finalstring;
	}
};

class FileLoader {
  public:
	FileLoader(const jsonifier::string& filePathNew) {
		filePath	   = filePathNew;
		auto theStream = std::ofstream{ filePath.data(), std::ios::binary | std::ios::out | std::ios::in };
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(const jsonifier::string& fileToSave) {
		auto theStream = std::ofstream{ filePath.data(), std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc };
		theStream << "";
		theStream.write(fileToSave.data(), fileToSave.size());
		theStream.close();
	}

	operator jsonifier::string() {
		return jsonifier::string{ fileContents };
	}

  protected:
	std::string fileContents{};
	std::string filePath{};
};

template<typename Function> double benchmark(Function function, int64_t iterationCount) {
	std::chrono::duration<double, std::nano> currentLowestTime{ std::numeric_limits<double>::max() };
	for (int64_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::high_resolution_clock::now();
		function();
		auto endTime = std::chrono::high_resolution_clock::now();
		auto newTime = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(endTime - startTime);
		if (newTime < currentLowestTime) {
			currentLowestTime = newTime;
		}
	}
	if (currentLowestTime.count() == std::numeric_limits<double>::max()) {
		currentLowestTime = std::chrono::duration<double, std::nano>{ 0 };
	}
	return currentLowestTime.count();
}

auto jsonifierSimdSingleTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - simd", "Single Test", "https://github.com/realtimechris/Jsonifier", 1 };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};
	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 0 }>(uint64Test, buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read				= result;
	r.wColor				= "steelblue";
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		1);
	for (auto& value: uint64Test.a) {
		for (auto& value02: value.testStrings) {
			//std::cout << "VALUE: " << value02 << std::endl;
		}
	}
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSimdTwitterTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - simd", "Twitter Test", "https://github.com/realtimechris/Jsonifier", iterations };
	twitter_message uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 0 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "steelblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSimdTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - simd", "Multi Test", "https://github.com/realtimechris/Jsonifier", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 0 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "steelblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSimdAbcTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - simd", "Abc Test", "https://github.com/realtimechris/Jsonifier", iterations };
	AbcTest<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 0 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();

	r.json_write = result;
	r.wColor	 = "steelblue";
	r.rColor	 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSimdDiscordTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };
	auto newSize = buffer.size();

	results r{ "jsonifier - simd", "Discord Test", "https://github.com/realtimechris/Jsonifier", iterations };
	discord_message discordDataTest{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 0 }>(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();

	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "steelblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierMinifyTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };

	results r{ "jsonifier", "Minify Test", "https://github.com/realtimechris/Jsonifier", iterations };

	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.minify(buffer);
		},
		iterations);

	r.json_write = result;

	r.wColor				 = "steelblue";
	r.rColor				 = "teal";
	r.json_write_byte_length = buffer.size();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSerialSingleTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - serial", "Single Test", "https://github.com/realtimechris/Jsonifier", 1 };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};
	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 10000000000 }>(uint64Test, buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read				= result;
	r.wColor				= "mediumslateblue";
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		1);
	for (auto& value: uint64Test.a) {
		for (auto& value02: value.testStrings) {
			//std::cout << "VALUE: " << value02 << std::endl;
		}
	}
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierValidationTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Validation Test", "https://github.com/realtimechris/Jsonifier", iterations };
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.validate(buffer);
		},
		1);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read				= result;
	r.json_read_byte_length = buffer.size();

	r.wColor = "mediumslateblue";
	r.rColor = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSerialTwitterTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - serial", "Twitter Test", "https://github.com/realtimechris/Jsonifier", iterations };
	twitter_message uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 10000000000 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "mediumslateblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSerialTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - serial", "Multi Test", "https://github.com/realtimechris/Jsonifier", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 10000000000 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "mediumslateblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSerialAbcTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier - serial", "Abc Test", "https://github.com/realtimechris/Jsonifier", iterations };
	AbcTest<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 10000000000 }>(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(uint64Test, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();

	r.json_write = result;
	r.wColor	 = "mediumslateblue";
	r.rColor	 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierSerialDiscordTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };
	auto newSize = buffer.size();

	results r{ "jsonifier - serial", "Discord Test", "https://github.com/realtimechris/Jsonifier", iterations };
	discord_message discordDataTest{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<jsonifier_internal::parser_options{ .simdCutoff = 10000000000 }>(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();

	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(discordDataTest, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "mediumslateblue";
	r.rColor				 = "teal";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierPrettifyTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };

	results r{ "jsonifier", "Prettify Test", "https://github.com/realtimechris/Jsonifier", iterations };

	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.prettify(buffer);
		},
		iterations);

	r.json_write = result;

	r.json_write_byte_length = buffer.size();
	r.wColor				 = "mediumslateblue";
	r.rColor				 = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}
#if !defined(ASAN)
auto glazeSingleTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Single Test", "https://github.com/stephenberry/glaze", 1 };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = false }>(uint64Test, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		1);

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		1);
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glazeTwitterTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Twitter Test", "https://github.com/stephenberry/glaze", iterations };
	twitter_message uint64Test{};

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = false }>(uint64Test, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);

	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glazeTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Multi Test", "https://github.com/stephenberry/glaze", iterations };
	Test<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = false }>(uint64Test, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);

	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glazeAbcTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Abc Test", "https://github.com/stephenberry/glaze", iterations };
	AbcTest<test_struct> uint64Test{};

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = false }>(uint64Test, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read = result;

	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(uint64Test, buffer);
		},
		iterations);
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glazeDiscordTest(const jsonifier::string& discordData, bool doWePrint = true) {
	std::string buffer{ discordData };

	results r{ "glaze", "Discord Test", "https://github.com/stephenberry/glaze", iterations };
	discord_message discordDataTest{};

	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .error_on_unknown_keys = false }>(discordDataTest, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read				= result;
	r.json_read_byte_length = buffer.size();

	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(discordDataTest, buffer);
		},
		iterations);
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}
	return r;
}

auto glazePrettifyTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };

	results r{ "glaze", "Prettify Test", "https://github.com/stephenberry/glaze", iterations };

	auto result = benchmark(
		[&]() {
			glz::prettify(buffer);
		},
		iterations);

	r.json_write = result;

	r.json_write_byte_length = buffer.size();
	r.wColor				 = "skyblue";
	r.rColor				 = "dodgerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

	#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool readInOrder(Test<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

auto simdjsonSingleTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	Test<test_struct> uint64Test{};

	r.json_read_byte_length = buffer.size();
	double result{};
	result = benchmark(
		[&]() {
			try {
				parser.readInOrder(uint64Test, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		1);

	r.json_read = result;
	buffer.clear();
	r.rColor = "cadetblue";
	r.wColor = "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjsonTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Multi Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};

	r.json_read_byte_length = buffer.size();
	double result{};
	result = benchmark(
		[&]() {
			try {
				parser.readInOrder(uint64Test, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read = result;
	r.rColor	= "cadetblue";
	r.wColor	= "cornflowerblue";
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool readOutOfOrder(AbcTest<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

template<typename value_type> value_type getValue(simdjson::ondemand::value value) {
	value_type result;
	if (!value.get(result)) {
		return result;
	}
	return value_type{};
}

template<typename value_type> value_type getValue(simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		return getValue<value_type>(field);
	}
	return value_type{};
}

template<> std::string getValue<std::string>(simdjson::ondemand::value value) {
	std::string_view result;
	if (!value.get(result)) {
		return static_cast<std::string>(result);
	}
	return "";
}

template<typename value_type> void getValues(std::vector<value_type>& returnValue, simdjson::ondemand::value value) {
	auto oldSize = returnValue.size();
	auto iter	 = value.begin();
	for (uint64_t x = 0; x < oldSize && iter != value.end(); ++x, ++iter) {
		returnValue[x] = getValue<value_type>(iter.operator*().value());
	}
	for (; iter != value.end(); ++iter) {
		returnValue.emplace_back(getValue<value_type>(iter.operator*().value()));
	}
}

template<typename value_type> void getValues(std::vector<value_type>& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		getValues<value_type>(returnValue, field);
	}
}

template<typename value_type> void getValues02(jsonifier::vector<value_type>& returnValue, simdjson::ondemand::value value) {
	auto oldSize = returnValue.size();
	auto iter	 = value.begin();
	for (uint64_t x = 0; x < oldSize && iter != value.end(); ++x, ++iter) {
		returnValue[x] = getValue<value_type>(iter.operator*().value());
	}
	for (; iter != value.end(); ++iter) {
		returnValue.emplace_back(getValue<value_type>(iter.operator*().value()));
	}
}

template<typename value_type> void getValues02(jsonifier::vector<value_type>& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		getValues02<value_type>(returnValue, field);
	}
}

template<> void getValues<std::string>(std::vector<std::string>& returnValues, simdjson::ondemand::value value) {
	auto oldSize = returnValues.size();
	auto iter	 = value.begin();
	for (uint64_t x = 0; x < oldSize && iter != value.end(); ++x, ++iter) {
		returnValues[x] = getValue<std::string>(iter.operator*().value());
	}
	for (; iter != value.end(); ++iter) {
		returnValues.emplace_back(getValue<std::string>(iter.operator*().value()));
	}
}

template<> search_metadata_data getValue<search_metadata_data>(simdjson::ondemand::value jsonData) {
	search_metadata_data metadata;
	metadata.completed_in = getValue<double>(jsonData, "completed_in");
	metadata.max_id		  = getValue<int64_t>(jsonData, "max_id");
	metadata.max_id_str	  = getValue<std::string>(jsonData, "max_id_str");
	metadata.next_results = getValue<std::string>(jsonData, "next_results");
	metadata.query		  = getValue<std::string>(jsonData, "query");
	metadata.refresh_url  = getValue<std::string>(jsonData, "refresh_url");
	metadata.count		  = getValue<int64_t>(jsonData, "count");
	metadata.since_id	  = getValue<int64_t>(jsonData, "since_id");
	metadata.since_id_str = getValue<std::string>(jsonData, "since_id_str");

	return metadata;
}

template<> hashtag getValue<hashtag>(simdjson::ondemand::value jsonData) {
	hashtag tag;

	tag.text = getValue<std::string>(jsonData, "text");
	getValues<int64_t>(tag.indices, jsonData, "indices");

	return tag;
}

template<> large_data getValue<large_data>(simdjson::ondemand::value jsonData) {
	large_data size;

	size.w		= getValue<uint64_t>(jsonData, "w");
	size.h		= getValue<uint64_t>(jsonData, "h");
	size.resize = getValue<std::string>(jsonData, "resize");

	return size;
}

template<> sizes_data getValue<sizes_data>(simdjson::ondemand::value jsonData) {
	sizes_data imageSizes;
	imageSizes.medium	 = getValue<large_data>(jsonData, "medium");
	imageSizes.small	 = getValue<large_data>(jsonData, "small");
	imageSizes.thumb	 = getValue<large_data>(jsonData, "thumb");
	imageSizes.large = getValue<large_data>(jsonData, "large");
	return imageSizes;
}

template<> avatar_decoration_data_data getValue<avatar_decoration_data_data>(simdjson::ondemand::value jsonData) {
	avatar_decoration_data_data imageSizes; /*
	imageSizes.medium = getValue<large_data>(jsonData, "medium");
	imageSizes.small  = getValue<large_data>(jsonData, "small");
	imageSizes.thumb  = getValue<large_data>(jsonData, "thumb");
	imageSizes.large  = getValue<large_data>(jsonData, "large");*/
	return imageSizes;
}

template<> media_data getValue<media_data>(simdjson::ondemand::value jsonData) {
	media_data mediaItem;
	mediaItem.id	 = getValue<double>(jsonData, "id");
	mediaItem.id_str = getValue<std::string>(jsonData, "id_str");
	getValues<int64_t>(mediaItem.indices, jsonData, "indices");
	mediaItem.media_url			   = getValue<std::string>(jsonData, "media_url");
	mediaItem.media_url_https	   = getValue<std::string>(jsonData, "media_url_https");
	mediaItem.display_url		   = getValue<std::string>(jsonData, "display_url");
	mediaItem.expanded_url		   = getValue<std::string>(jsonData, "expanded_url");
	mediaItem.type				   = getValue<std::string>(jsonData, "type");
	mediaItem.sizes				   = getValue<sizes_data>(jsonData, "sizes");
	mediaItem.source_status_id	   = getValue<double>(jsonData, "source_status_id");
	mediaItem.source_status_id_str = getValue<std::string>(jsonData, "source_status_id_str");
	return mediaItem;
}

template<> url_data getValue<url_data>(simdjson::ondemand::value jsonData) {
	url_data urlData;
	urlData.expanded_url = getValue<std::string>(jsonData, "expanded_url");
	urlData.display_url	 = getValue<std::string>(jsonData, "display_url");
	getValues<int64_t>(urlData.indices, jsonData, "indices");
	return urlData;
}

template<> user_mention getValue<user_mention>(simdjson::ondemand::value jsonData) {
	user_mention mention;
	mention.screen_name = getValue<std::string>(jsonData, "screen_name");
	mention.name		= getValue<std::string>(jsonData, "name");
	mention.id			= getValue<int64_t>(jsonData, "id");
	mention.id_str		= getValue<std::string>(jsonData, "id_str");
	getValues<int64_t>(mention.indices, jsonData, "indices");
	return mention;
}

template<> status_entities getValue<status_entities>(simdjson::ondemand::value jsonData) {
	status_entities entities;
	getValues<hashtag>(entities.hashtags, jsonData, "hashtags");
	getValues<url_data>(entities.urls, jsonData, "urls");
	getValues<user_mention>(entities.user_mentions, jsonData, "user_mentions");
	getValues<std::string>(entities.symbols, jsonData, "symbols");
	return entities;
}

template<> metadata_data getValue<metadata_data>(simdjson::ondemand::value jsonData) {
	metadata_data meta;
	meta.result_type	   = getValue<std::string>(jsonData, "result_type");
	meta.iso_language_code = getValue<std::string>(jsonData, "iso_language_code");
	return meta;
}

template<> description_data getValue<description_data>(simdjson::ondemand::value jsonData) {
	description_data desc;

	getValues<url_data>(desc.urls, jsonData, "urls");

	return desc;
}

template<> user_entities getValue<user_entities>(simdjson::ondemand::value jsonData) {
	user_entities userEnt;

	userEnt.description = getValue<description_data>(jsonData, "description");

	return userEnt;
}

template<> user_data_twitter getValue<user_data_twitter>(simdjson::ondemand::value jsonData) {
	user_data_twitter userData;
	userData.id									= getValue<int64_t>(jsonData, "id");
	userData.id_str								= getValue<std::string>(jsonData, "id_str");
	userData.name								= getValue<std::string>(jsonData, "name");
	userData.screen_name						= getValue<std::string>(jsonData, "screen_name");
	userData.location							= getValue<std::string>(jsonData, "location");
	userData.description						= getValue<std::string>(jsonData, "description");
	userData.entities							= getValue<user_entities>(jsonData, "entities");
	userData.user_protected						= getValue<bool>(jsonData, "protected");
	userData.followers_count					= getValue<int64_t>(jsonData, "followers_count");
	userData.friends_count						= getValue<int64_t>(jsonData, "friends_count");
	userData.listed_count						= getValue<int64_t>(jsonData, "listed_count");
	userData.created_at							= getValue<std::string>(jsonData, "created_at");
	userData.favourites_count					= getValue<int64_t>(jsonData, "favourites_count");
	userData.geo_enabled						= getValue<bool>(jsonData, "geo_enabled");
	userData.verified							= getValue<bool>(jsonData, "verified");
	userData.statuses_count						= getValue<int64_t>(jsonData, "statuses_count");
	userData.lang								= getValue<std::string>(jsonData, "lang");
	userData.contributors_enabled				= getValue<bool>(jsonData, "contributors_enabled");
	userData.is_translator						= getValue<bool>(jsonData, "is_translator");
	userData.is_translation_enabled				= getValue<bool>(jsonData, "is_translation_enabled");
	userData.profile_background_color			= getValue<std::string>(jsonData, "profile_background_color");
	userData.profile_background_image_url		= getValue<std::string>(jsonData, "profile_background_image_url");
	userData.profile_background_image_url_https = getValue<std::string>(jsonData, "profile_background_image_url_https");
	userData.profile_background_tile			= getValue<bool>(jsonData, "profile_background_tile");
	userData.profile_image_url					= getValue<std::string>(jsonData, "profile_image_url");
	userData.profile_image_url_https			= getValue<std::string>(jsonData, "profile_image_url_https");
	userData.profile_banner_url					= getValue<std::string>(jsonData, "profile_banner_url");
	userData.profile_link_color					= getValue<std::string>(jsonData, "profile_link_color");
	userData.profile_sidebar_border_color		= getValue<std::string>(jsonData, "profile_sidebar_border_color");
	userData.profile_sidebar_fill_color			= getValue<std::string>(jsonData, "profile_sidebar_fill_color");
	userData.profile_text_color					= getValue<std::string>(jsonData, "profile_text_color");
	userData.profile_use_background_image		= getValue<bool>(jsonData, "profile_use_background_image");
	userData.default_profile					= getValue<bool>(jsonData, "default_profile");
	userData.default_profile_image				= getValue<bool>(jsonData, "default_profile_image");
	userData.following							= getValue<bool>(jsonData, "following");
	userData.follow_request_sent				= getValue<bool>(jsonData, "follow_request_sent");
	userData.notifications						= getValue<bool>(jsonData, "notifications");
	return userData;
}

template<> status getValue<status>(simdjson::ondemand::value jsonData) {
	status statusData;
	statusData.metadata			  = getValue<metadata_data>(jsonData, "metadata");
	statusData.created_at		  = getValue<std::string>(jsonData, "created_at");
	statusData.id				  = getValue<double>(jsonData, "id");
	statusData.id_str			  = getValue<std::string>(jsonData, "id_str");
	statusData.text				  = getValue<std::string>(jsonData, "text");
	statusData.source			  = getValue<std::string>(jsonData, "source");
	statusData.truncated		  = getValue<bool>(jsonData, "truncated");
	statusData.user			  = getValue<user_data_twitter>(jsonData, "user");
	statusData.retweet_count	  = getValue<int64_t>(jsonData, "retweet_count");
	statusData.favorite_count	  = getValue<int64_t>(jsonData, "favorite_count");
	statusData.entities			  = getValue<status_entities>(jsonData, "entities");
	statusData.favorited		  = getValue<bool>(jsonData, "favorited");
	statusData.retweeted		  = getValue<bool>(jsonData, "retweeted");
	statusData.lang				  = getValue<std::string>(jsonData, "lang");
	statusData.possibly_sensitive = getValue<bool>(jsonData, "possibly_sensitive");
	return statusData;
}

template<> twitter_message getValue<twitter_message>(simdjson::ondemand::value jsonData) {
	twitter_message message;
	getValues<status>(message.statuses, jsonData, "statuses");
	message.metadata = getValue<search_metadata_data>(jsonData, "metadata");
	return message;
}

template<> member_user getValue<member_user>(simdjson::ondemand::value jsonData) {
	member_user userData;
	userData.avatar_decoration_data = getValue<avatar_decoration_data_data>(jsonData, "avatar_decoration_data");
	userData.discriminator	   = getValue<std::string>(jsonData, "discriminator");
	userData.global_name	   = getValue<std::string>(jsonData, "global_name");
	userData.username		   = getValue<std::string>(jsonData, "username");
	userData.public_flags	   = getValue<uint64_t>(jsonData, "public_flags");
	userData.avatar			   = getValue<std::string>(jsonData, "avatar");
	userData.id				   = getValue<std::string>(jsonData, "id");
	userData.bot			   = getValue<bool>(jsonData, "bot");
	return userData;
}

template<> channel_data getValue<channel_data>(simdjson::ondemand::value jsonData) {
	channel_data channelData;

	channelData.last_pin_timestamp				   = getValue<std::string>(jsonData, "last_pin_timestamp");
	channelData.rate_limit_per_user				   = getValue<uint64_t>(jsonData, "rate_limit_per_user");
	channelData.last_message_id					   = getValue<std::string>(jsonData, "last_message_id");
	channelData.rtc_region						   = getValue<std::string>(jsonData, "rtc_region");
	channelData.parent_id						   = getValue<std::string>(jsonData, "parent_id");
	channelData.user_limit						   = getValue<uint64_t>(jsonData, "user_limit");
	channelData.topic							   = getValue<std::string>(jsonData, "topic");
	channelData.position						   = getValue<uint64_t>(jsonData, "position");
	channelData.bitrate							   = getValue<uint64_t>(jsonData, "bitrate");
	channelData.name							   = getValue<std::string>(jsonData, "name");
	channelData.id								   = getValue<std::string>(jsonData, "id");
	channelData.flags							   = getValue<uint64_t>(jsonData, "flags");
	channelData.type							   = getValue<uint64_t>(jsonData, "type");
	channelData.nsfw							   = getValue<bool>(jsonData, "nsfw");

	return channelData;
}

template<> role_data getValue<role_data>(simdjson::ondemand::value jsonData) {
	role_data roleData;
	roleData.unicode_emoji = getValue<std::string>(jsonData, "unicode_emoji");
	roleData.permissions   = getValue<std::string>(jsonData, "permissions");
	roleData.position	   = getValue<uint64_t>(jsonData, "position");
	roleData.name		   = getValue<std::string>(jsonData, "name");
	roleData.icon		   = getValue<std::string>(jsonData, "icon");
	roleData.mentionable   = getValue<bool>(jsonData, "mentionable");
	roleData.color		   = getValue<uint64_t>(jsonData, "color");
	roleData.id			   = getValue<std::string>(jsonData, "id");
	roleData.flags		   = getValue<uint64_t>(jsonData, "flags");
	roleData.managed	   = getValue<bool>(jsonData, "managed");
	roleData.hoist		   = getValue<bool>(jsonData, "hoist");
	return roleData;
}

template<> guild_member_data getValue<guild_member_data>(simdjson::ondemand::value jsonData) {
	guild_member_data memberData;
	memberData.communication_disabled_until = getValue<std::string>(jsonData, "communication_disabled_until");
	getValues<std::string>(memberData.roles, jsonData, "roles");
	memberData.premium_since = getValue<std::string>(jsonData, "premium_since");
	memberData.joined_at	 = getValue<std::string>(jsonData, "joined_at");
	memberData.avatar		 = getValue<std::string>(jsonData, "avatar");
	memberData.nick			 = getValue<std::string>(jsonData, "nick");
	memberData.user			 = getValue<member_user>(jsonData, "user");
	memberData.flags		 = getValue<uint64_t>(jsonData, "flags");
	memberData.pending		 = getValue<bool>(jsonData, "pending");
	memberData.deaf			 = getValue<bool>(jsonData, "deaf");
	memberData.mute			 = getValue<bool>(jsonData, "mute");
	return memberData;
}

template<> guild_data getValue<guild_data>(simdjson::ondemand::value jsonData) {
	guild_data guildData;
	getValues<guild_member_data>(guildData.members, jsonData, "members");
	getValues<channel_data>(guildData.channels, jsonData, "channels");
	getValues<std::string>(guildData.features, jsonData, "features");
	getValues<role_data>(guildData.roles, jsonData, "roles");
	guildData.default_message_notifications = getValue<uint64_t>(jsonData, "default_message_notifications");
	guildData.max_stage_video_channel_users = getValue<uint64_t>(jsonData, "max_stage_video_channel_users");
	guildData.public_updates_channel_id		= getValue<std::string>(jsonData, "public_updates_channel_id");
	guildData.premium_subscription_count	= getValue<uint64_t>(jsonData, "premium_subscription_count");
	guildData.safety_alerts_channel_id		= getValue<std::string>(jsonData, "safety_alerts_channel_id");
	guildData.premium_progress_bar_enabled	= getValue<bool>(jsonData, "premium_progress_bar_enabled");
	guildData.explicit_content_filter		= getValue<uint64_t>(jsonData, "explicit_content_filter");
	guildData.max_video_channel_users		= getValue<uint64_t>(jsonData, "max_video_channel_users");
	guildData.system_channel_id				= getValue<std::string>(jsonData, "system_channel_id");
	guildData.preferred_locale				= getValue<std::string>(jsonData, "preferred_locale");
	guildData.discovery_splash				= getValue<std::string>(jsonData, "discovery_splash");
	guildData.system_channel_flags			= getValue<uint64_t>(jsonData, "system_channel_flags");
	guildData.rules_channel_id				= getValue<std::string>(jsonData, "rules_channel_id");
	guildData.verification_level			= getValue<uint64_t>(jsonData, "verification_level");
	guildData.application_id				= getValue<std::string>(jsonData, "application_id");
	guildData.vanity_url_code				= getValue<std::string>(jsonData, "vanity_url_code");
	guildData.afk_channel_id				= getValue<std::string>(jsonData, "afk_channel_id");
	guildData.description					= getValue<std::string>(jsonData, "description");
	guildData.member_count					= getValue<uint64_t>(jsonData, "member_count");
	guildData.joined_at						= getValue<std::string>(jsonData, "joined_at");
	guildData.premium_tier					= getValue<uint64_t>(jsonData, "premium_tier");
	guildData.owner_id						= getValue<std::string>(jsonData, "owner_id");
	guildData.max_members					= getValue<uint64_t>(jsonData, "max_members");
	guildData.afk_timeout					= getValue<uint64_t>(jsonData, "afk_timeout");
	guildData.splash						= getValue<std::string>(jsonData, "splash");
	guildData.banner						= getValue<std::string>(jsonData, "banner");
	guildData.nsfw_level					= getValue<uint64_t>(jsonData, "nsfw_level");
	guildData.mfa_level						= getValue<uint64_t>(jsonData, "mfa_level");
	guildData.name							= getValue<std::string>(jsonData, "name");
	guildData.icon							= getValue<std::string>(jsonData, "icon");
	guildData.unavailable					= getValue<bool>(jsonData, "unavailable");
	guildData.id							= getValue<std::string>(jsonData, "id");
	return guildData;
}


template<> discord_message getValue<discord_message>(simdjson::ondemand::value jsonData) {
	discord_message message;

	message.t = getValue<std::string>(jsonData, "t");

	message.s = getValue<uint64_t>(jsonData, "s");

	message.op = getValue<uint64_t>(jsonData, "op");

	message.d = getValue<guild_data>(jsonData, "d");

	return message;
}

	#define SIMD_PULL(x) \
		{ \
			ondemand::array newX = doc[#x].get_array().value(); \
			for (ondemand::value value: newX) { \
				test_struct newStruct{}; \
				getValues<int64_t>(newStruct.testInts, value, "testInts"); \
				getValues<double>(newStruct.testDoubles, value, "testDoubles"); \
				getValues<std::string>(newStruct.testStrings, value, "testStrings"); \
				getValues02<bool>(newStruct.testBools, value, "testBools"); \
				getValues<uint64_t>(newStruct.testUints, value, "testUints"); \
				obj.x.emplace_back(std::move(newStruct)); \
			} \
		}

bool on_demand::readInOrder(Test<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_PULL(a);
	SIMD_PULL(b);
	SIMD_PULL(c);
	SIMD_PULL(d);
	SIMD_PULL(e);
	SIMD_PULL(f);
	SIMD_PULL(g);
	SIMD_PULL(h);
	SIMD_PULL(i);
	SIMD_PULL(j);
	SIMD_PULL(k);
	SIMD_PULL(l);
	SIMD_PULL(m);
	SIMD_PULL(n);
	SIMD_PULL(o);
	SIMD_PULL(p);
	SIMD_PULL(q);
	SIMD_PULL(r);
	SIMD_PULL(s);
	SIMD_PULL(t);
	SIMD_PULL(u);
	SIMD_PULL(v);
	SIMD_PULL(w);
	SIMD_PULL(x);
	SIMD_PULL(y);
	SIMD_PULL(z);
	return false;
}

bool on_demand_abc::readOutOfOrder(AbcTest<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_PULL(z);
	SIMD_PULL(y);
	SIMD_PULL(x);
	SIMD_PULL(w);
	SIMD_PULL(v);
	SIMD_PULL(u);
	SIMD_PULL(t);
	SIMD_PULL(s);
	SIMD_PULL(r);
	SIMD_PULL(q);
	SIMD_PULL(p);
	SIMD_PULL(o);
	SIMD_PULL(n);
	SIMD_PULL(m);
	SIMD_PULL(l);
	SIMD_PULL(k);
	SIMD_PULL(j);
	SIMD_PULL(i);
	SIMD_PULL(h);
	SIMD_PULL(g);
	SIMD_PULL(f);
	SIMD_PULL(e);
	SIMD_PULL(d);
	SIMD_PULL(c);
	SIMD_PULL(b);
	SIMD_PULL(a);
	return false;
}

auto simdjsonMinifyTest(const jsonifier::string& discordData, bool doWePrint = true) {
	std::string buffer{ discordData };

	results r{ "simdjson", "Minify Test", "https://github.com/simdjson/simdjson", iterations };
	dom::parser parser{};
	auto result = benchmark(
		[&]() {
			auto doc	  = parser.parse(buffer);
			auto newValue = simdjson::minify(doc);
		},
		iterations);

	r.json_write = result;

	r.json_write_byte_length = buffer.size();
	r.rColor				 = "cadetblue";
	r.wColor				 = "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjsonTwitterTest(const jsonifier::string& discordData, bool doWePrint = true) {
	std::string buffer{ discordData };

	results r{ "simdjson", "Twitter Test", "https://github.com/simdjson/simdjson", iterations };
	twitter_message message{};
	simdjson::ondemand::parser parser{};
	auto result = benchmark(
		[&]() {
			auto doc = parser.iterate(buffer);
			message	 = getValue<twitter_message>(doc.value());
		},
		iterations);

	r.json_read = result;

	r.json_read_byte_length = buffer.size();
	r.rColor				= "cadetblue";
	r.wColor				= "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjsonDiscordTest(const jsonifier::string& discordData, bool doWePrint = true) {
	std::string buffer{ discordData };

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Discord Test", "https://github.com/simdjson/simdjson", iterations };

	simdjson::ondemand::parser parser;

	discord_message discord_messageData;

	auto result = benchmark(
		[&]() {
			try {
				auto doc	= parser.iterate(buffer);
				discord_messageData = getValue<discord_message>(doc.value());
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
	r.rColor				= "cadetblue";
	r.wColor				= "cornflowerblue";
	if (doWePrint) {
		r.print();
	}
	return r;
}

auto simdjsonAbcTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };
	double result{};

	result = benchmark(
		[&]() {
			try {
				parser.readOutOfOrder(obj, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
	r.rColor				= "cadetblue";
	r.wColor				= "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}
#endif
std::string table_header = R"(
| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |)";

template<typename function_type> std::string unitTest(function_type&& function, std::string testName) {
	bool result{ function() };
	if (result) {
		return testName + " - Success.\n";
	} else {
		return testName + " - Failed.\n";
	}
}

struct doubleTest {
	std::vector<double> doubles{ 0.0454545, 0.0, 22424.3434234234 };
};

template<> struct jsonifier::core<doubleTest> {
	using value_type				 = doubleTest;
	static constexpr auto parseValue = createValue("doubles", &value_type::doubles);
};

test_results multiTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Multi Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeTest(jsonData, false);
	}
	resultsNew.emplace_back(glazeTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSerialTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSerialTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSimdTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSimdTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

test_results twitterTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Twitter Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonTwitterTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonTwitterTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeTwitterTest(jsonData, false);
	}
	resultsNew.emplace_back(glazeTwitterTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSerialTwitterTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSerialTwitterTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSimdTwitterTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSimdTwitterTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

test_results abcTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Abc Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonAbcTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonAbcTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeAbcTest(jsonData, false);
	}
	resultsNew.emplace_back(glazeAbcTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSerialAbcTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSerialAbcTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSimdAbcTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSimdAbcTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

test_results discordTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Discord Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonDiscordTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonDiscordTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeDiscordTest(jsonData, false);
	}
	resultsNew.emplace_back(glazeDiscordTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSerialDiscordTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSerialDiscordTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSimdDiscordTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSimdDiscordTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		table += resultsNew[x].jsonStats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

test_results singleTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Single Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonSingleTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonSingleTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeSingleTest(jsonData, false);
	}
	resultsNew.emplace_back(glazeSingleTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSerialSingleTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSerialSingleTest(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierSimdSingleTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSimdSingleTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
};

test_results minifyTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Minify Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonMinifyTest(jsonData, false);
	}
	resultsNew.emplace_back(simdjsonMinifyTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierMinifyTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierMinifyTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
};

test_results prettifyTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Prettify Test";
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		glazePrettifyTest(jsonData, false);
	}
	resultsNew.emplace_back(glazePrettifyTest(jsonData));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierPrettifyTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierPrettifyTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
};

test_results validationTests(const jsonifier::string& jsonData) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = "Validate Test";

	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierValidationTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierValidationTest(jsonData));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::less<results>());
	for (uint64_t x = 0; x < n; ++x) {
		table += resultsNew[x].jsonStats();
		if (resultsNew[x].getReadResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getReadResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getReadResults());
		}
		if (resultsNew[x].getWriteResults().resultSpeed != 9223372036854775808ull && resultsNew[x].getWriteResults().resultSpeed != 0) {
			jsonResults.results.emplace_back(resultsNew[x].getWriteResults());
		}
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
};

namespace fs = std::filesystem;

jsonifier::string findFileRecursively(const std::string& filename) {
	fs::path currentPath = fs::current_path();
	uint32_t currentLevelAbove{};
	while (!currentPath.empty()) {
		fs::path filePath = currentPath / filename;

		if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
			return jsonifier::string{ filePath.string() };
		}
		currentPath = currentPath.parent_path();
		++currentLevelAbove;
		if (currentLevelAbove >= 5) {
			break;
		}
	}

	std::cerr << "File not found: " << filename << std::endl;
	return "";
}

static const std::string section001{ R"(
> )" +
	jsonifier::toString(iterations) + R"( iterations on a 6 core (Intel i7 8700k))" };

static constexpr jsonifier::string_view section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: ()" };

static constexpr jsonifier::string_view section01{
	R"()

### Single Iteration Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test_Results.png?raw=true" 
alt="SingleCoreAPI WebSite" width="400"/></p>

)"
};

static constexpr jsonifier::string_view section02{
	R"(

### Multi Iteration Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test_Results.png?raw=true" 
alt="MinifyCoreAPI WebSite" width="400"/></p>

)"
};

static const std::string section03{
	R"(

### ABC Test (Out of Sequence Performance) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test_Results.png?raw=true" 
alt="AbcCoreAPI WebSite" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
)"
};

static constexpr jsonifier::string_view section04{ R"(

### Discord Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test_Results.png?raw=true" 
alt="DiscordCoreAPI WebSite" width="400"/></p>

)" };

static constexpr jsonifier::string_view section05{ R"(

### Twitter Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/twitter.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test_Results.png?raw=true" 
alt="TwitterCoreAPI WebSite" width="400"/></p>

)" };

static constexpr jsonifier::string_view section06{
	R"(

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="MinifyCoreAPI WebSite" width="400"/></p>

)"
};

static constexpr jsonifier::string_view section07{
	R"(

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="PrettifyCoreAPI WebSite" width="400"/></p>

)"
};

static constexpr jsonifier::string_view section08{
	R"(

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="ValidateCoreAPI WebSite" width="400"/></p>

)"
};

#include "ConformanceTests.hpp"


struct transcoding {
	jsonifier::string preset{};
	jsonifier::string url{};
};

struct second_download_url {
	jsonifier::string url{};
};

struct mediaNew {
	std::vector<transcoding> transcodings{};
};

struct raw_sound_cloud_song {
	jsonifier::string trackAuthorization{};
	jsonifier::string description{};
	jsonifier::string artworkUrl{};
	jsonifier::string viewUrl{};
	jsonifier::string title{};
	uint64_t duration{};
	media_data media{};
};

struct sound_cloud_search_results {
	jsonifier::vector<raw_sound_cloud_song> collection{};
};

struct data_class {
	jsonifier::string trackAuthorization{};
	jsonifier::string description{};
	jsonifier::string artworkUrl{};
	jsonifier::string avatarUrl{};
	jsonifier::string viewUrl{};
	jsonifier::string title{};
	int64_t duration{};
	media_data media{};
};

struct discord_message_element {
	jsonifier::raw_json_data data{};
	jsonifier::string hydratable{};
};

struct discord_message_data {
	std::vector<discord_message_element> data{};
};


namespace jsonifier {

	template<> struct core<data_class> {
		using value_type = data_class;
		static constexpr auto parseValue =
			createValue("artwork_url", &value_type::artworkUrl, "description", &value_type::description, "duration", &value_type::duration, "media", &value_type::media, "title",
				&value_type::title, "track_authorization", &value_type::trackAuthorization, "avatar_url", &value_type::avatarUrl, "permalink_url", &value_type::viewUrl);
	};

	template<> struct core<discord_message_element> {
		using value_type				 = discord_message_element;
		static constexpr auto parseValue = createValue<&value_type::data, &value_type::hydratable>();
	};

	template<> struct core<discord_message_data> {
		using value_type				 = discord_message_data;
		static constexpr auto parseValue = createValue(&value_type::data);
	};

	template<> struct core<mediaNew> {
		using value_type				 = mediaNew;
		static constexpr auto parseValue = createValue("transcodings", &value_type::transcodings);
	};

	template<> struct core<second_download_url> {
		using value_type				 = second_download_url;
		static constexpr auto parseValue = createValue("url", &value_type::url);
	};

	template<> struct core<transcoding> {
		using value_type				 = transcoding;
		static constexpr auto parseValue = createValue("preset", &value_type::preset, "url", &value_type::url);
	};

	template<> struct core<raw_sound_cloud_song> {
		using value_type				 = raw_sound_cloud_song;
		static constexpr auto parseValue = createValue("artwork_url", &value_type::artworkUrl, "description", &value_type::description, "duration", &value_type::duration, "media",
			&value_type::media, "title", &value_type::title, "track_authorization", &value_type::trackAuthorization, "permalink_url", &value_type::viewUrl);
	};

	template<> struct core<sound_cloud_search_results> {
		using value_type				 = sound_cloud_search_results;
		static constexpr auto parseValue = createValue("collection", &value_type::collection);
	};
}

class testClass {
  public:
	int32_t currentValue{};
	int32_t currentValue02{};
};

template<typename value_type_new> struct get_ptr {
	using value_type = value_type_new;
	value_type newValue{};
	constexpr operator value_type*() {
		return &newValue;
	}
};

template<typename tuple_t, std::size_t... Is> constexpr auto getValuesFromTuple(tuple_t tuple, std::index_sequence<Is...>) {
	constexpr tuple_t newTuple{};
	auto getValuesLambda = [&](auto index) {
		//using member_type = decltype(std::get<index>(newTuple));
		//static constexpr std::tuple<std::remove_pointer_t<jsonifier::concepts::unwrap_t<member_type>>> newerTuple{};
		//static constexpr auto newPtr = &std::get<0>(newerTuple);
		return jsonifier::string_view{};
		//jsonifier_internal::getName<newPtr>();
	};
	
	return std::array<jsonifier::string_view, sizeof...(Is)>{ getValuesLambda(std::integral_constant<uint64_t, Is>{})... };
}
#include <iostream>
#include <type_traits>

template<typename... value_types> struct collect_types {};

template<typename parent_type, typename member_type, typename... value_types> struct collect_types<member_type parent_type::*, value_types...> {
	using type = member_type parent_type::*;
	using types = collect_types<value_types...>;
};

template<typename parent_type, typename member_type> struct collect_types<member_type parent_type::*> {
	using type	= member_type parent_type::*;
};

template<auto memberPtr> constexpr void printMemberName() {
	std::cout << "CURRENT NAME: " << jsonifier_internal::getName<memberPtr>() << std::endl;
}

template<typename... arg_types, size_t... indices> constexpr void testFunction(std::tuple<arg_types...> parents, std::index_sequence<indices...>) {
	// Use a fold expression to call the helper function for each member in the pack
	(printMemberName<std::get<indices>(parents)>(), ...);
}

template<const auto... p> void testPrint() {
	//std::cout << "TEST PRINT 01: " << __FUNCSIG__ << std::endl;
}

class MyClass {
  public:
	int member;
	int anotherMember;
	// Add more members as needed
};

template<auto ptr,typename arg_types> constexpr void testFunction(arg_types&& parents) {
	static_assert(std::conjunction_v<std::is_member_pointer<std::decay_t<arg_types>>>, "testFunction only accepts pointers to members");
	//std::cout << "CURRENT SIGNATURE: " << __FUNCSIG__ << std::endl;
	constexpr auto parentsTuple = std::tuple<typename collect_types<arg_types>::type>();
	using new_type = arg_types;
	constexpr new_type newPtr{};
	testPrint<newPtr>();
	constexpr auto indices = std::index_sequence_for<arg_types>{};
	//testFunction(parentsTuple, indices);
}

template<auto ptr> struct pointer_holder {
	//constexpr pointer_holder(auto ptr){};
};

//template<auto ptr> pointer_holder(auto ptrNew) -> pointer_holder<ptrNew>;

decltype(auto) testFunction02(auto ptr) {
	std::cout << typeid(ptr).name() << std::endl;
};

//template<auto... ptr> decltype(auto) testFunction03(pointer_holder<ptr>... arg) {
//testFunction02(pointer_holder<ptr>{ ptr }...);
//};

static constexpr uint64_t power(uint64_t base, uint64_t exponent) {
	//constexpr newValue = base;
	uint64_t result = 1;
	while (exponent > 0) {
		if (exponent & 1) {
			result *= base;
		}
		base *= base;
		exponent >>= 1;
	}
	return result;
}

template<uint64_t index> JSONIFIER_INLINE static uint64_t prefixXorImpl(uint64_t prevInString) {
	prevInString ^= prevInString << index;
	return prevInString;
}

template<size_t... indices> JSONIFIER_INLINE static uint64_t prefixXor(uint64_t prevInString, std::index_sequence<indices...>) {
	((prevInString = prefixXorImpl<power(2, indices)>(prevInString)), ...);
	return prevInString;
}

int main() {

	//prefixXor(02, std::make_index_sequence<8>{});
	//pointer_holder<&MyClass::member> newValue{ &MyClass::member };
	//testFunction02(&MyClass::member);

	try{
		jsonifier::jsonifier_core parser{};
		//static constexpr auto parseValue = createPtrTuple(&testClass::currentValue, &testClass::currentValue02); 
		//auto memberNames				 = getMemberNames<std::tuple_size_v<decltype(parseValue)>>(parseValue);
		//for (auto& value: memberNames) {
			//std::cout << "CURRENT VALUE: " << value << std::endl;
		//}
		
		json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
#if defined(_WIN32)
		FileLoader fileLoader01{ "../../ReadMe.md" };
		FileLoader fileLoader02{ "../../Json/JsonData.json" };
		FileLoader fileLoader03{ "../../Json/Results.json" };
		fileLoader02.saveFile(parser.prettify(jsonData.theData));
#else
		FileLoader fileLoader01{ "../ReadMe.md" };
		FileLoader fileLoader02{ "../Json/JsonData.json" };
		FileLoader fileLoader03{ "../Json/Results.json" };
		fileLoader02.saveFile(parser.prettify(jsonData.theData));
#endif
		jsonifier::string rawData{
			R"([{"hydratable":"anonymousId","data":"924122-291678-529945-398194"},{"hydratable":"features","data":{"features":["mobi_webauth_oauth_mode",
						"cd_repost_to_artists","v2_use_onetrust_tcfv2_us_ca","mobi_enable_onetrust_tcfv2","mobi_tracking_send_session_id","mobi_use_onetrust_eu1",
						"mobi_use_onetrust_gb"]}}])"
		};
		discord_message_data resultsNew{};
		parser.parseJson(resultsNew, rawData);
		jsonifier::string avatarUrl{};

		for (auto& value: resultsNew.data) {
			if (value.data.getType() == jsonifier::json_type::Object) {
				auto newObject = value.data.operator jsonifier::raw_json_data::object_type();
				for (auto& [key, value]: newObject) {
					std::cout << "Key: " << key << ", Value: " << value.rawJson() << std::endl;
				}
				if (newObject["features"].getType() == jsonifier::json_type::Array) {
					auto newArray = newObject["features"].operator jsonifier::raw_json_data::array_type();
					for (auto& value: newArray) {
						std::cout << "Current Value: " << value.operator jsonifier::string() << std::endl;
					}
				}
			}
		}
		jsonifier::string newString0202{};
		newString0202.reserve(0);
		jsonData.theData = parser.prettify(jsonData.theData);
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t result = std::time(nullptr);
		resultTwo		   = *localtime(&result);
		conformanceTests();
		std::vector<test_results> benchmark_data{};
		FileLoader discordData{ findFileRecursively("../Json/DiscordData.json") };
		FileLoader discordMinifiedData{ findFileRecursively("../Json/DiscordData-Minified.json") };
		auto newData = parser.minify(discordData.operator jsonifier::string());
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		discordData.saveFile(parser.prettify(newData));
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		discordMinifiedData.saveFile(parser.minify(discordData.operator jsonifier::string()));
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		auto singleTestResults = singleTests(jsonData.theData);
		benchmark_data.emplace_back(singleTestResults);
		auto multiTestResults = multiTests(jsonData.theData);
		benchmark_data.emplace_back(multiTestResults);
		auto abcTestResults = abcTests(jsonData.theData);
		benchmark_data.emplace_back(abcTestResults);
		auto discordTestResults = discordTests(discordData);
		benchmark_data.emplace_back(discordTestResults);
		auto twitterTestResults = twitterTests(FileLoader{ findFileRecursively("../Json/twitter.json") });
		benchmark_data.emplace_back(twitterTestResults);
		auto minifyTestResults = minifyTests(discordData);
		benchmark_data.emplace_back(minifyTestResults);
		auto prettifyTestResults = prettifyTests(discordMinifiedData.operator jsonifier::string());
		benchmark_data.emplace_back(prettifyTestResults);
		auto validationTestResults = validationTests(discordData);
		benchmark_data.emplace_back(validationTestResults);
		jsonifier::string resultsStringJson{};
		parser.serializeJson<true>(benchmark_data, resultsStringJson);
		fileLoader03.saveFile(resultsStringJson);
		jsonifier::string newerString = section00 + newTimeString + section01 + singleTestResults.markdownResults + section02 + multiTestResults.markdownResults + section001 +
			section03 + abcTestResults.markdownResults + section001 + section04 + discordTestResults.markdownResults + section001 + section05 + twitterTestResults.markdownResults +
			section001 + section06 + minifyTestResults.markdownResults + section001 + section07 + prettifyTestResults.markdownResults + section001 + section08 +
			validationTestResults.markdownResults + section001;
		fileLoader01.saveFile(newerString);
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}