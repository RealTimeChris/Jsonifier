#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif
#include "UnicodeEmoji.hpp"
#include <jsonifier/Index.hpp>
#include "glaze/glaze.hpp"
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

struct geometry_data {
	std::vector<std::vector<std::vector<double>>> coordinates{};
	std::string type{};
};

struct properties_data {
	std::string name{};
};

struct feature {
	properties_data properties{};
	geometry_data geometry{};
	std::string type{};
};

struct canada_message {
	std::vector<feature> features{};
	std::string type{};
};

struct search_metadata_data {
	std::string since_id_str{};
	std::string next_results{};
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
	large_data medium{};
	large_data small{};
	large_data thumb{};
	large_data large{};
};

struct media_data {
	std::optional<std::string> source_status_id_str{};
	std::optional<double> source_status_id{};
	std::vector<int64_t> indices{};
	std::string media_url_https{};
	std::string expanded_url{};
	std::string display_url{};
	std::string media_url{};
	std::string id_str{};
	std::string type{};
	sizes_data sizes{};
	std::string url{};
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
	std::optional<std::vector<media_data>> media{};
	std::vector<user_mention> user_mentions{};
	std::vector<std::nullptr_t> symbols{};
	std::vector<hashtag> hashtags{};
	std::vector<url_data> urls{};
};

struct metadata_data {
	std::string iso_language_code{};
	std::string result_type{};
};

struct description_data {
	std::vector<url_data> urls{};
};

struct user_entities {
	std::optional<description_data> url{};
	description_data description{};
};

struct twitter_user {
	std::string profile_background_image_url_https{};
	std::optional<std::string> profile_banner_url{};
	std::string profile_background_image_url{};
	std::string profile_sidebar_fill_color{};
	std::optional<std::string> time_zone{};
	std::string profile_background_color{};
	std::string profile_image_url_https{};
	std::optional<int64_t> utc_offset{};
	bool profile_use_background_image{};
	std::optional<std::string> url{};
	std::string profile_text_color{};
	std::string profile_link_color{};
	std::string profile_image_url{};
	bool profile_background_tile{};
	bool is_translation_enabled{};
	bool default_profile_image{};
	bool contributors_enabled{};
	bool follow_request_sent{};
	int64_t favourites_count{};
	std::string description{};
	std::string screen_name{};
	int64_t followers_count{};
	int64_t statuses_count{};
	std::string created_at{};
	user_entities entities{};
	int64_t friends_count{};
	bool default_profile{};
	int64_t listed_count{};
	std::string location{};
	bool user_protected{};
	bool is_translator{};
	std::string id_str{};
	bool notifications{};
	std::string string{};
	std::string name{};
	bool geo_enabled{};
	std::string lang{};
	bool following{};
	bool verified{};
	int64_t id{};
};

struct status {
	std::optional<std::string> in_reply_to_status_id_str{};
	std::optional<std::string> in_reply_to_user_id_str{};
	std::optional<std::string> in_reply_to_screen_name{};
	std::optional<double> in_reply_to_status_id{};
	std::optional<int64_t> in_reply_to_user_id{};
	std::optional<bool> possibly_sensitive{};
	std::nullptr_t contributors{};
	std::nullptr_t coordinates{};
	status_entities entities{};
	int64_t favorite_count{};
	metadata_data metadata{};
	std::string created_at{};
	int64_t retweet_count{};
	std::nullptr_t place{};
	std::nullptr_t geo{};
	std::string source{};
	std::string id_str{};
	twitter_user user{};
	std::string lang{};
	std::string text{};
	bool truncated{};
	bool favorited{};
	bool retweeted{};
	double id{};
};

struct twitter_message {
	search_metadata_data search_metadata{};
	std::vector<status> statuses{};
};

struct avatar_decoration_data_data {
	std::string sku_id{};
	std::string asset{};
};

class user_data {
  public:
	std::optional<avatar_decoration_data_data> avatar_decoration_data{};
	std::optional<std::string> display_name{};
	std::optional<std::string> global_name{};
	std::optional<std::string> banner{};
	std::optional<std::string> avatar{};
	std::optional<std::string> locale{};
	std::string discriminator{};
	std::string user_name{};
	uint64_t accent_color{};
	uint64_t premium_type{};
	uint64_t public_flags{};
	std::string email{};
	bool mfa_enabled{};
	std::string id{};
	uint64_t flags{};
	bool verified{};
	bool system{};
	bool bot{};
};

class guild_scheduled_event_data {
  public:
	std::string scheduled_start_time{};
	std::string scheduled_end_time{};
	std::string description{};
	uint64_t entity_metadata{};
	std::string creator_id{};
	std::string channel_id{};
	uint64_t privacy_level{};
	std::string entity_id{};
	std::string guild_id{};
	uint64_t entity_type{};
	uint64_t user_count{};
	user_data creator{};
	std::string name{};
	uint64_t status{};
	std::string id{};
};

struct tags_data {
	std::optional<std::string> premium_subscriber{};
	std::optional<std::string> bot_id{};
};

class role_data {
  public:
	std::optional<std::string> unicode_emoji{};
	std::optional<std::string> icon{};
	std::optional<tags_data> tags{};
	std::string permissions{};
	uint64_t position{};
	std::string name{};
	bool mentionable{};
	uint64_t version{};
	uint64_t color{};
	std::string id{};
	uint64_t flags{};
	bool managed{};
	bool hoist{};
};

struct icon_emoji_data {
	std::optional<std::string> name{};
	std::optional<std::string> id{};
};

struct permission_overwrites_data {
	std::string allow{};
	std::string deny{};
	std::string id{};
	uint64_t type{};
};

class channel_data {
  public:
	std::vector<permission_overwrites_data> permission_overwrites{};
	std::optional<std::string> last_message_id{};
	uint64_t default_thread_rate_limit_per_user{};
	std::vector<std::string> applied_tags{};
	std::optional<std::string> rtc_region{};
	uint64_t default_auto_archive_duration{};
	std::optional<std::string> status{};
	std::vector<user_data> recipients{};
	std::optional<std::string> topic{};
	std::string last_pin_timestamp{};
	uint64_t total_message_sent{};
	uint64_t rate_limit_per_user{};
	uint64_t video_quality_mode{};
	std::string application_id{};
	std::string permissions{};
	uint64_t message_count{};
	uint64_t member_count{};
	std::string parent_id{};
	icon_emoji_data icon_emoji{};
	std::string owner_id{};
	std::string guild_id{};
	uint64_t user_limit{};
	uint64_t position{};
	uint64_t version{};
	uint64_t bitrate{};
	std::string name{};
	std::string icon{};
	std::string id{};
	uint64_t flags{};
	uint64_t type{};
	bool managed{};
	bool nsfw{};
};

class guild_member_data {
  public:
	std::optional<std::string> communication_disabled_until{};
	std::optional<std::string> premium_since{};
	std::optional<std::string> avatar{};
	std::optional<std::string> nick{};
	std::vector<std::string> roles{};
	std::string permissions{};
	std::string joined_at{};
	std::string guild_id{};
	user_data user{};
	uint64_t flags{};
	bool pending{};
	bool deaf{};
	bool mute{};
};

class guild_data {
  public:
	std::vector<guild_scheduled_event_data> guild_scheduled_events{};
	std::optional<std::string> latest_on_boarding_question_id{};
	std::optional<std::string> safety_alerts_channel_id{};
	std::optional<std::string> inventory_settings{};
	std::optional<std::string> discovery_splash{};
	std::optional<std::string> application_id{};
	std::optional<std::string> vanity_url_code{};
	std::optional<std::string> description{};
	std::vector<guild_member_data> members{};
	uint64_t default_message_notifications{};
	std::vector<std::string> voice_states{};
	std::vector<channel_data> channels{};
	uint64_t max_stage_video_channel_users{};
	std::string public_updates_channel_id{};
	std::optional<std::string> hub_type{};
	uint64_t premium_subscription_count{};
	std::vector<std::string> features{};
	uint64_t approximate_presence_count{};
	std::vector<std::string> stickers{};
	std::optional<std::string> splash{};
	std::vector<std::string> threads{};
	uint64_t approximate_member_count{};
	bool premium_progress_bar_enabled{};
	uint64_t explicit_content_filter{};
	uint64_t max_video_channel_users{};
	std::vector<role_data> roles{};
	std::string system_channel_id{};
	std::string widget_channel_id{};
	std::string preferred_locale{};
	uint64_t system_channel_flags{};
	std::string rules_channel_id{};
	uint64_t verification_level{};
	std::string afk_channel_id{};
	std::string permissions{};
	uint64_t max_presences{};
	std::string discovery{};
	uint64_t member_count{};
	std::string joined_at{};
	uint64_t premium_tier{};
	std::string owner_id{};
	uint64_t max_members{};
	uint64_t afk_timeout{};
	std::string banner{};
	bool widget_enabled{};
	std::string region{};
	uint64_t nsfw_level{};
	uint64_t mfa_level{};
	std::string name{};
	std::string icon{};
	bool unavailable{};
	std::string id{};
	uint64_t flags{};
	bool large{};
	bool owner{};
	bool nsfw{};
	bool lazy{};
};

struct discord_message {
	std::string t{};
	guild_data d{};
	uint64_t op{};
	uint64_t s{};
};

template<> struct jsonifier::core<geometry_data> {
	using value_type				 = geometry_data;
	static constexpr auto parseValue = createValue<&value_type::coordinates, &value_type::type>();
};

template<> struct jsonifier::core<properties_data> {
	using value_type				 = properties_data;
	static constexpr auto parseValue = createValue<&value_type::name>();
};

template<> struct jsonifier::core<feature> {
	using value_type				 = feature;
	static constexpr auto parseValue = createValue<&value_type::properties, &value_type::geometry, &value_type::type>();
};

template<> struct jsonifier::core<canada_message> {
	using value_type				 = canada_message;
	static constexpr auto parseValue = createValue<&value_type::features, &value_type::type>();
};

#if !defined(ASAN)

template<> struct glz::meta<geometry_data> {
	using value_type			= geometry_data;
	static constexpr auto value = object("coordinates", &value_type::coordinates, "type", &value_type::type);
};

template<> struct glz::meta<properties_data> {
	using value_type			= properties_data;
	static constexpr auto value = object("name", &value_type::name);
};

template<> struct glz::meta<feature> {
	using value_type			= feature;
	static constexpr auto value = object("properties", &value_type::properties, "geometry", &value_type::geometry, "type", &value_type::type);
};

template<> struct glz::meta<canada_message> {
	using value_type			= canada_message;
	static constexpr auto value = object("features", &value_type::features, "type", &value_type::type);
};

#endif

template<> struct jsonifier::core<search_metadata_data> {
	using value_type				 = search_metadata_data;
	static constexpr auto parseValue = createValue<&value_type::since_id_str, &value_type::next_results, &value_type::refresh_url, &value_type::max_id_str,
		&value_type::completed_in, &value_type::query, &value_type::since_id, &value_type::count, &value_type::max_id>();
};

template<> struct jsonifier::core<hashtag> {
	using value_type				 = hashtag;
	static constexpr auto parseValue = createValue<&value_type::indices, &value_type::text>();
};

template<> struct jsonifier::core<large_data> {
	using value_type				 = large_data;
	static constexpr auto parseValue = createValue<&value_type::resize, &value_type::w, &value_type::h>();
};

template<> struct jsonifier::core<sizes_data> {
	using value_type				 = sizes_data;
	static constexpr auto parseValue = createValue<&value_type::medium, &value_type::small, &value_type::thumb, &value_type::large>();
};

template<> struct jsonifier::core<media_data> {
	using value_type = media_data;
	static constexpr auto parseValue =
		createValue<&value_type::source_status_id_str, &value_type::source_status_id, &value_type::indices, &value_type::media_url_https, &value_type::expanded_url,
			&value_type::display_url, &value_type::media_url, &value_type::id_str, &value_type::type, &value_type::sizes, &value_type::url, &value_type::id>();
};

template<> struct jsonifier::core<url_data> {
	using value_type				 = url_data;
	static constexpr auto parseValue = createValue<&value_type::indices, &value_type::expanded_url, &value_type::display_url, &value_type::url>();
};

template<> struct jsonifier::core<user_mention> {
	using value_type				 = user_mention;
	static constexpr auto parseValue = createValue<&value_type::indices, &value_type::screen_name, &value_type::id_str, &value_type::name, &value_type::id>();
};

template<> struct jsonifier::core<status_entities> {
	using value_type				 = status_entities;
	static constexpr auto parseValue = createValue<&value_type::media, &value_type::user_mentions, &value_type::symbols, &value_type::hashtags, &value_type::urls>();
};

template<> struct jsonifier::core<metadata_data> {
	using value_type				 = metadata_data;
	static constexpr auto parseValue = createValue<&value_type::iso_language_code, &value_type::result_type>();
};

template<> struct jsonifier::core<description_data> {
	using value_type				 = description_data;
	static constexpr auto parseValue = createValue<&value_type::urls>();
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue<&value_type::url, &value_type::description>();
};

template<> struct jsonifier::core<twitter_user> {
	using value_type				 = twitter_user;
	static constexpr auto parseValue = createValue<&value_type::profile_background_image_url_https, &value_type::profile_banner_url, &value_type::profile_background_image_url,
		&value_type::profile_sidebar_fill_color, &value_type::time_zone, &value_type::profile_background_color, &value_type::profile_image_url_https, &value_type::utc_offset,
		&value_type::profile_use_background_image, &value_type::url, &value_type::profile_text_color, &value_type::profile_link_color, &value_type::profile_image_url,
		&value_type::profile_background_tile, &value_type::is_translation_enabled, &value_type::default_profile_image, &value_type::contributors_enabled,
		&value_type::follow_request_sent, &value_type::favourites_count, &value_type::description, &value_type::screen_name, &value_type::followers_count,
		&value_type::statuses_count, &value_type::created_at, &value_type::entities, &value_type::friends_count, &value_type::default_profile, &value_type::listed_count,
		&value_type::location, &value_type::user_protected, &value_type::is_translator, &value_type::id_str, &value_type::notifications, &value_type::string, &value_type::name,
		&value_type::geo_enabled, &value_type::lang, &value_type::following, &value_type::verified, &value_type::id>();
};

template<> struct jsonifier::core<status> {
	using value_type				 = status;
	static constexpr auto parseValue = createValue<&value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id_str, &value_type::in_reply_to_screen_name,
		&value_type::in_reply_to_status_id, &value_type::in_reply_to_user_id, &value_type::possibly_sensitive, &value_type::contributors, &value_type::coordinates,
		&value_type::entities, &value_type::favorite_count, &value_type::metadata, &value_type::created_at, &value_type::retweet_count, &value_type::place, &value_type::geo,
		&value_type::source, &value_type::id_str, &value_type::user, &value_type::lang, &value_type::text, &value_type::truncated, &value_type::favorited, &value_type::retweeted,
		&value_type::id>();
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue<&value_type::search_metadata, &value_type::statuses>();
};


#if !defined(ASAN)

template<> struct glz::meta<search_metadata_data> {
	using value_type = search_metadata_data;
	static constexpr auto value =
		object("next_results", &value_type::next_results, "since_id_str", &value_type::since_id_str, "refresh_url", &value_type::refresh_url, "max_id_str", &value_type::max_id_str,
			"completed_in", &value_type::completed_in, "query", &value_type::query, "since_id", &value_type::since_id, "count", &value_type::count, "max_id", &value_type::max_id);
};

template<> struct glz::meta<media_data> {
	using value_type			= media_data;
	static constexpr auto value = object("source_status_id_str", &value_type::source_status_id_str, "source_status_id", &value_type::source_status_id, "indices",
		&value_type::indices, "media_url_https", &value_type::media_url_https, "expanded_url", &value_type::expanded_url, "display_url", &value_type::display_url, "media_url",
		&value_type::media_url, "id_str", &value_type::id_str, "type", &value_type::type, "sizes", &value_type::sizes, "url", &value_type::url, "id", &value_type::id);
};

template<> struct glz::meta<status_entities> {
	using value_type			= status_entities;
	static constexpr auto value = object("media", &value_type::media, "user_mentions", &value_type::user_mentions, "symbols", &value_type::symbols, "hashtags",
		&value_type::hashtags, "urls", &value_type::urls);
};

template<> struct glz::meta<metadata_data> {
	using value_type			= metadata_data;
	static constexpr auto value = object("iso_language_code", &value_type::iso_language_code, "result_type", &value_type::result_type);
};

template<> struct glz::meta<description_data> {
	using value_type			= description_data;
	static constexpr auto value = object("urls", &value_type::urls);
};

template<> struct glz::meta<user_entities> {
	using value_type			= user_entities;
	static constexpr auto value = object("url", &value_type::url, "description", &value_type::description);
};

template<> struct glz::meta<twitter_user> {
	using value_type			= twitter_user;
	static constexpr auto value = object("profile_banner_url", &value_type::profile_banner_url, "profile_background_image_url_https",
		&value_type::profile_background_image_url_https, "profile_background_image_url", &value_type::profile_background_image_url, "std::string", &value_type::string, "time_zone",
		&value_type::time_zone, "profile_sidebar_fill_color", &value_type::profile_sidebar_fill_color, "profile_background_color", &value_type::profile_background_color,
		"utc_offset", &value_type::utc_offset, "profile_image_url_https", &value_type::profile_image_url_https, "profile_use_background_image",
		&value_type::profile_use_background_image, "url", &value_type::url, "profile_link_color", &value_type::profile_link_color, "profile_text_color",
		&value_type::profile_text_color, "profile_image_url", &value_type::profile_image_url, "profile_background_tile", &value_type::profile_background_tile,
		"is_translation_enabled", &value_type::is_translation_enabled, "contributors_enabled", &value_type::contributors_enabled, "default_profile_image",
		&value_type::default_profile_image, "favourites_count", &value_type::favourites_count, "description", &value_type::description, "screen_name", &value_type::screen_name,
		"followers_count", &value_type::followers_count, "entities", &value_type::entities, "follow_request_sent", &value_type::follow_request_sent, "created_at",
		&value_type::created_at, "statuses_count", &value_type::statuses_count, "location", &value_type::location, "friends_count", &value_type::friends_count, "listed_count",
		&value_type::listed_count, "default_profile", &value_type::default_profile, "user_protected", &value_type::user_protected, "notifications", &value_type::notifications,
		"is_translator", &value_type::is_translator, "id_str", &value_type::id_str, "name", &value_type::name, "lang", &value_type::lang, "geo_enabled", &value_type::geo_enabled,
		"following", &value_type::following, "verified", &value_type::verified, "id", &value_type::id, "string", &value_type::string);
};

template<> struct glz::meta<status> {
	using value_type			= status;
	static constexpr auto value = object("in_reply_to_status_id_str", &value_type::in_reply_to_status_id_str, "in_reply_to_screen_name", &value_type::in_reply_to_screen_name,
		"in_reply_to_user_id_str", &value_type::in_reply_to_user_id_str, "in_reply_to_status_id", &value_type::in_reply_to_status_id, "contributors", &value_type::contributors,
		"in_reply_to_user_id", &value_type::in_reply_to_user_id, "coordinates", &value_type::coordinates, "possibly_sensitive", &value_type::possibly_sensitive, "place",
		&value_type::place, "geo", &value_type::geo, "entities", &value_type::entities, "user", &value_type::user, "metadata", &value_type::metadata, "created_at",
		&value_type::created_at, "favorite_count", &value_type::favorite_count, "retweet_count", &value_type::retweet_count, "source", &value_type::source, "id_str",
		&value_type::id_str, "lang", &value_type::lang, "text", &value_type::text, "truncated", &value_type::truncated, "favorited", &value_type::favorited, "retweeted",
		&value_type::retweeted, "id", &value_type::id);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("search_metadata", &value_type::search_metadata, "statuses", &value_type::statuses);
};

#endif
template<> struct jsonifier::core<avatar_decoration_data_data> {
	using value_type				 = avatar_decoration_data_data;
	static constexpr auto parseValue = createValue<&value_type::sku_id, &value_type::asset>();
};

template<> struct jsonifier::core<user_data> {
	using value_type = user_data;
	static constexpr auto parseValue =
		createValue<&value_type::avatar_decoration_data, &value_type::display_name, &value_type::global_name, &value_type::banner, &value_type::avatar, &value_type::locale,
			&value_type::discriminator, &value_type::user_name, &value_type::accent_color, &value_type::premium_type, &value_type::public_flags, &value_type::email,
			&value_type::mfa_enabled, &value_type::id, &value_type::flags, &value_type::verified, &value_type::system, &value_type::bot>();
};

template<> struct jsonifier::core<guild_scheduled_event_data> {
	using value_type				 = guild_scheduled_event_data;
	static constexpr auto parseValue = createValue<&value_type::scheduled_start_time, &value_type::scheduled_end_time, &value_type::description, &value_type::entity_metadata,
		&value_type::creator_id, &value_type::channel_id, &value_type::privacy_level, &value_type::entity_id, &value_type::guild_id, &value_type::entity_type,
		&value_type::user_count, &value_type::creator, &value_type::name, &value_type::status, &value_type::id>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<&value_type::premium_subscriber, &value_type::bot_id>();
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue<&value_type::unicode_emoji, &value_type::icon, &value_type::tags, &value_type::permissions, &value_type::position,
		&value_type::name, &value_type::mentionable, &value_type::version, &value_type::color, &value_type::id, &value_type::flags, &value_type::managed, &value_type::hoist>();
};

template<> struct jsonifier::core<icon_emoji_data> {
	using value_type				 = icon_emoji_data;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::id>();
};

template<> struct jsonifier::core<permission_overwrites_data> {
	using value_type				 = permission_overwrites_data;
	static constexpr auto parseValue = createValue<&value_type::allow, &value_type::deny, &value_type::id, &value_type::type>();
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue = createValue<&value_type::permission_overwrites, &value_type::last_message_id, &value_type::default_thread_rate_limit_per_user,
		&value_type::applied_tags, &value_type::rtc_region, &value_type::default_auto_archive_duration, &value_type::status, &value_type::recipients, &value_type::topic,
		&value_type::last_pin_timestamp, &value_type::total_message_sent, &value_type::rate_limit_per_user, &value_type::video_quality_mode, &value_type::application_id,
		&value_type::permissions, &value_type::message_count, &value_type::member_count, &value_type::parent_id, &value_type::icon_emoji, &value_type::owner_id,
		&value_type::guild_id, &value_type::user_limit, &value_type::position, &value_type::version, &value_type::bitrate, &value_type::name, &value_type::icon, &value_type::id,
		&value_type::flags, &value_type::type, &value_type::managed, &value_type::nsfw>();
};

template<> struct jsonifier::core<guild_member_data> {
	using value_type = guild_member_data;
	static constexpr auto parseValue =
		createValue<&value_type::communication_disabled_until, &value_type::premium_since, &value_type::avatar, &value_type::nick, &value_type::roles, &value_type::permissions,
			&value_type::joined_at, &value_type::guild_id, &value_type::user, &value_type::flags, &value_type::pending, &value_type::deaf, &value_type::mute>();
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue<&value_type::guild_scheduled_events, &value_type::latest_on_boarding_question_id, &value_type::safety_alerts_channel_id,
		&value_type::inventory_settings, &value_type::discovery_splash, &value_type::application_id, &value_type::vanity_url_code, &value_type::description, &value_type::members,
		&value_type::default_message_notifications, &value_type::voice_states, &value_type::channels, &value_type::max_stage_video_channel_users,
		&value_type::public_updates_channel_id, &value_type::hub_type, &value_type::premium_subscription_count, &value_type::features, &value_type::approximate_presence_count,
		&value_type::stickers, &value_type::splash, &value_type::threads, &value_type::approximate_member_count, &value_type::premium_progress_bar_enabled,
		&value_type::explicit_content_filter, &value_type::max_video_channel_users, &value_type::roles, &value_type::system_channel_id, &value_type::widget_channel_id,
		&value_type::preferred_locale, &value_type::system_channel_flags, &value_type::rules_channel_id, &value_type::verification_level, &value_type::afk_channel_id,
		&value_type::permissions, &value_type::max_presences, &value_type::discovery, &value_type::member_count, &value_type::joined_at, &value_type::premium_tier,
		&value_type::owner_id, &value_type::max_members, &value_type::afk_timeout, &value_type::banner, &value_type::widget_enabled, &value_type::region, &value_type::nsfw_level,
		&value_type::mfa_level, &value_type::name, &value_type::icon, &value_type::unavailable, &value_type::id, &value_type::flags, &value_type::large, &value_type::owner,
		&value_type::nsfw, &value_type::lazy>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type				 = discord_message;
	static constexpr auto parseValue = createValue<&value_type::t, &value_type::d, &value_type::op, &value_type::s>();
};

#if !defined(ASAN)

template<> struct glz::meta<avatar_decoration_data_data> {
	using value_type			= avatar_decoration_data_data;
	static constexpr auto value = object("sku_id", &value_type::sku_id, "asset", &value_type::asset);
};

template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object("avatar_decoration_data", &value_type::avatar_decoration_data, "display_name", &value_type::display_name, "global_name",
		&value_type::global_name, "banner", &value_type::banner, "avatar", &value_type::avatar, "locale", &value_type::locale, "discriminator", &value_type::discriminator,
		"user_name", &value_type::user_name, "accent_color", &value_type::accent_color, "premium_type", &value_type::premium_type, "public_flags", &value_type::public_flags,
		"email", &value_type::email, "mfa_enabled", &value_type::mfa_enabled, "id", &value_type::id, "flags", &value_type::flags, "verified", &value_type::verified, "system",
		&value_type::system, "bot", &value_type::bot);
};

template<> struct glz::meta<guild_scheduled_event_data> {
	using value_type			= guild_scheduled_event_data;
	static constexpr auto value = object("scheduled_start_time", &value_type::scheduled_start_time, "scheduled_end_time", &value_type::scheduled_end_time, "description",
		&value_type::description, "entity_metadata", &value_type::entity_metadata, "creator_id", &value_type::creator_id, "channel_id", &value_type::channel_id, "privacy_level",
		&value_type::privacy_level, "entity_id", &value_type::entity_id, "guild_id", &value_type::guild_id, "entity_type", &value_type::entity_type, "user_count",
		&value_type::user_count, "creator", &value_type::creator, "name", &value_type::name, "status", &value_type::status, "id", &value_type::id);
};

template<> struct glz::meta<tags_data> {
	using value_type			= tags_data;
	static constexpr auto value = object("premium_subscriber", &value_type::premium_subscriber, "bot_id", &value_type::bot_id);
};

template<> struct glz::meta<role_data> {
	using value_type			= role_data;
	static constexpr auto value = object("unicode_emoji", &value_type::unicode_emoji, "icon", &value_type::icon, "tags", &value_type::tags, "permissions", &value_type::permissions,
		"position", &value_type::position, "name", &value_type::name, "mentionable", &value_type::mentionable, "version", &value_type::version, "color", &value_type::color, "id",
		&value_type::id, "flags", &value_type::flags, "managed", &value_type::managed, "hoist", &value_type::hoist);
};

template<> struct glz::meta<icon_emoji_data> {
	using value_type			= icon_emoji_data;
	static constexpr auto value = object("name", &value_type::name, "id", &value_type::id);
};

template<> struct glz::meta<permission_overwrites_data> {
	using value_type			= permission_overwrites_data;
	static constexpr auto value = object("allow", &value_type::allow, "deny", &value_type::deny, "id", &value_type::id, "type", &value_type::type);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object("permission_overwrites", &value_type::permission_overwrites, "last_message_id", &value_type::last_message_id,
		"default_thread_rate_limit_per_user", &value_type::default_thread_rate_limit_per_user, "applied_tags", &value_type::applied_tags, "rtc_region", &value_type::rtc_region,
		"default_auto_archive_duration", &value_type::default_auto_archive_duration, "status", &value_type::status, "recipients", &value_type::recipients, "topic",
		&value_type::topic, "last_pin_timestamp", &value_type::last_pin_timestamp, "total_message_sent", &value_type::total_message_sent, "rate_limit_per_user",
		&value_type::rate_limit_per_user, "video_quality_mode", &value_type::video_quality_mode, "application_id", &value_type::application_id, "permissions",
		&value_type::permissions, "message_count", &value_type::message_count, "member_count", &value_type::member_count, "parent_id", &value_type::parent_id, "icon_emoji",
		&value_type::icon_emoji, "owner_id", &value_type::owner_id, "guild_id", &value_type::guild_id, "user_limit", &value_type::user_limit, "position", &value_type::position,
		"version", &value_type::version, "bitrate", &value_type::bitrate, "name", &value_type::name, "icon", &value_type::icon, "id", &value_type::id, "flags", &value_type::flags,
		"type", &value_type::type, "managed", &value_type::managed, "nsfw", &value_type::nsfw);
};

template<> struct glz::meta<guild_member_data> {
	using value_type			= guild_member_data;
	static constexpr auto value = object("communication_disabled_until", &value_type::communication_disabled_until, "premium_since", &value_type::premium_since, "avatar",
		&value_type::avatar, "nick", &value_type::nick, "roles", &value_type::roles, "permissions", &value_type::permissions, "joined_at", &value_type::joined_at, "guild_id",
		&value_type::guild_id, "user", &value_type::user, "flags", &value_type::flags, "pending", &value_type::pending, "deaf", &value_type::deaf, "mute", &value_type::mute);
};

template<> struct glz::meta<guild_data> {
	using value_type			= guild_data;
	static constexpr auto value = object("guild_scheduled_events", &value_type::guild_scheduled_events, "latest_on_boarding_question_id",
		&value_type::latest_on_boarding_question_id, "safety_alerts_channel_id", &value_type::safety_alerts_channel_id, "inventory_settings", &value_type::inventory_settings,
		"discovery_splash", &value_type::discovery_splash, "application_id", &value_type::application_id, "vanity_url_code", &value_type::vanity_url_code, "description",
		&value_type::description, "members", &value_type::members, "default_message_notifications", &value_type::default_message_notifications, "voice_states",
		&value_type::voice_states, "channels", &value_type::channels, "max_stage_video_channel_users", &value_type::max_stage_video_channel_users, "public_updates_channel_id",
		&value_type::public_updates_channel_id, "hub_type", &value_type::hub_type, "premium_subscription_count", &value_type::premium_subscription_count, "features",
		&value_type::features, "approximate_presence_count", &value_type::approximate_presence_count, "stickers", &value_type::stickers, "splash", &value_type::splash, "threads",
		&value_type::threads, "approximate_member_count", &value_type::approximate_member_count, "premium_progress_bar_enabled", &value_type::premium_progress_bar_enabled,
		"explicit_content_filter", &value_type::explicit_content_filter, "max_video_channel_users", &value_type::max_video_channel_users, "roles", &value_type::roles,
		"system_channel_id", &value_type::system_channel_id, "widget_channel_id", &value_type::widget_channel_id, "preferred_locale", &value_type::preferred_locale,
		"system_channel_flags", &value_type::system_channel_flags, "rules_channel_id", &value_type::rules_channel_id, "verification_level", &value_type::verification_level,
		"afk_channel_id", &value_type::afk_channel_id, "permissions", &value_type::permissions, "max_presences", &value_type::max_presences, "discovery", &value_type::discovery,
		"member_count", &value_type::member_count, "joined_at", &value_type::joined_at, "premium_tier", &value_type::premium_tier, "owner_id", &value_type::owner_id, "max_members",
		&value_type::max_members, "afk_timeout", &value_type::afk_timeout, "banner", &value_type::banner, "widget_enabled", &value_type::widget_enabled, "region",
		&value_type::region, "nsfw_level", &value_type::nsfw_level, "mfa_level", &value_type::mfa_level, "name", &value_type::name, "icon", &value_type::icon, "unavailable",
		&value_type::unavailable, "id", &value_type::id, "flags", &value_type::flags, "large", &value_type::large, "owner", &value_type::owner, "nsfw", &value_type::nsfw, "lazy",
		&value_type::lazy);
};

template<> struct glz::meta<discord_message> {
	using value_type			= discord_message;
	static constexpr auto value = object("t", &value_type::t, "d", &value_type::d, "op", &value_type::op, "s", &value_type::s);
};

#endif

struct test_struct {
	std::vector<std::string> testStrings{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
	std::vector<bool> testBools{};
};

namespace fs = std::filesystem;

class file_loader {
  public:
	file_loader(const std::string& filePathNew) {
		filePath = filePathNew;
		std::string directory{ filePathNew.substr(0, filePathNew.find_last_of("/")) };
		if (!fs::exists(directory)) {
			std::filesystem::create_directories(directory);
		}

		if (!fs::exists(filePath)) {
			std::ofstream createFile(filePath.data());
			createFile.close();
		}

		std::ifstream theStream(filePath.data(), std::ios::binary | std::ios::in);
		std::stringstream inputStream{};
		inputStream << theStream.rdbuf();
		fileContents = inputStream.str();
		theStream.close();
	}

	void saveFile(const std::string& fileToSave) {
		std::ofstream theStream(filePath.data(), std::ios::binary | std::ios::out | std::ios::trunc);
		theStream.write(fileToSave.data(), fileToSave.size());
		theStream.close();
	}

	operator std::string() {
		return std::string{ fileContents };
	}

  protected:
	std::string fileContents{};
	std::string filePath{};
};

template<typename value_type> struct Test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937_64 gen{ randomEngine() };

	static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

	template<typename value_type_new> value_type_new randomizeNumberNormal(value_type_new mean, value_type_new stdDeviation) {
		std::normal_distribution<> normalDistributionTwo{ static_cast<double>(mean), static_cast<double>(stdDeviation) };
		auto theResult = normalDistributionTwo(randomEngine);
		if (theResult < 0) {
			theResult = -theResult;
		}
		return theResult;
	}

	template<typename value_type_new> value_type_new randomizeNumberUniform(value_type_new range) {
		std::uniform_int_distribution<uint64_t> dis(0, range);
		return dis(randomEngine);
	}

	void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[randomizeNumberUniform(std::size(unicode_emoji::unicodeEmoji) - 1)];
		jsonString += static_cast<std::string>(newStringView);
	}

	std::string generateString() {
		auto length{ randomizeNumberNormal(64.0f, 16.0f) };
		static constexpr int32_t charsetSize = charset.size();
		auto unicodeCount					 = randomizeNumberUniform(length / 8);
		std::string result{};
		for (int32_t x = 0; x < length; ++x) {
			if (x == static_cast<int32_t>(length / unicodeCount)) {
				insertUnicodeInJSON(result);
			}
			result += charset[randomizeNumberUniform(charsetSize - 1)];
		}
		return result;
	}

	double generateDouble() {
		auto newValue = randomizeNumberNormal(double{}, std::numeric_limits<double>::max() / 50000000);
		return generateBool() ? newValue : -newValue;
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumberNormal(50.0f, 50.0f) >= 50.0f);
	};

	uint64_t generateUint() {
		return randomizeNumberNormal(std::numeric_limits<uint64_t>::max() / 2, std::numeric_limits<uint64_t>::max() / 2);
	};

	int64_t generateInt() {
		auto newValue = randomizeNumberNormal(int64_t{}, std::numeric_limits<int64_t>::max());
		return generateBool() ? newValue : -newValue;
	};

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberNormal(35, 10);
			auto arraySize02 = randomizeNumberNormal(15, 10);
			auto arraySize03 = randomizeNumberNormal(5, 1);
			v.resize(arraySize01);
			for (uint64_t x = 0; x < arraySize01; ++x) {
				auto arraySize01 = randomizeNumberNormal(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					auto newString = generateString();
					v[x].testStrings.emplace_back(newString);
				}
				arraySize01 = randomizeNumberNormal(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testUints.emplace_back(generateUint());
				}
				arraySize01 = randomizeNumberNormal(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					v[x].testInts.emplace_back(generateInt());
				}
				arraySize01 = randomizeNumberNormal(arraySize02, arraySize03);
				for (uint64_t y = 0; y < arraySize01; ++y) {
					auto newBool = generateBool();
					v[x].testBools.emplace_back(newBool);
				}
				arraySize01 = randomizeNumberNormal(arraySize02, arraySize03);
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

struct test_element_final {
	std::string libraryName{};
	std::string resultType{};
	double resultSpeed{};
	std::string color{};
	bool operator>(const test_element_final& other) const {
		return resultSpeed > other.resultSpeed;
	}
};

struct test_element_pair {
	test_element_final writeData{};
	test_element_final readData{};
	bool operator>(const test_element_pair& other) const {
		return writeData > other.writeData && readData > other.readData;
	}
};

struct test_elements_final {
	std::vector<test_element_final> results{};
	std::string testName{};
};

using test_results_final = std::vector<test_elements_final>;

template<> struct jsonifier::core<test_element_final> {
	using value_type = test_element_final;
	static constexpr auto parseValue =
		createValue("libraryName", &value_type::libraryName, "resultType", &value_type::resultType, "resultSpeed", &value_type::resultSpeed, "color", &value_type::color);
};

template<> struct jsonifier::core<test_elements_final> {
	using value_type				 = test_elements_final;
	static constexpr auto parseValue = createValue("results", &value_type::results, "testName", &value_type::testName);
};

template<typename value_type> struct AbcTest {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<&value_type::testStrings, &value_type::testUints, &value_type::testDoubles, &value_type::testInts, &value_type::testBools>();
};

template<> struct jsonifier::core<Test<test_struct>> {
	using value_type				 = Test<test_struct>;
	static constexpr auto parseValue = createValue("a", &value_type::a, "b", &value_type::b, "c", &value_type::c, "d", &value_type::d, "e", &value_type::e, "f", &value_type::f,
		"g", &value_type::g, "h", &value_type::h, "i", &value_type::i, "j", &value_type::j, "k", &value_type::k, "l", &value_type::l, "m", &value_type::m, "n", &value_type::n, "o",
		&value_type::o, "p", &value_type::p, "q", &value_type::q, "r", &value_type::r, "s", &value_type::s, "t", &value_type::t, "u", &value_type::u, "v", &value_type::v, "w",
		&value_type::w, "x", &value_type::x, "y", &value_type::y, "z", &value_type::z);
};


template<> struct jsonifier::core<test_generator<test_struct>> {
	using value_type				 = test_generator<test_struct>;
	static constexpr auto parseValue = createValue("a", &value_type::a, "b", &value_type::b, "c", &value_type::c, "d", &value_type::d, "e", &value_type::e, "f", &value_type::f,
		"g", &value_type::g, "h", &value_type::h, "i", &value_type::i, "j", &value_type::j, "k", &value_type::k, "l", &value_type::l, "m", &value_type::m, "n", &value_type::n, "o",
		&value_type::o, "p", &value_type::p, "q", &value_type::q, "r", &value_type::r, "s", &value_type::s, "t", &value_type::t, "u", &value_type::u, "v", &value_type::v, "w",
		&value_type::w, "x", &value_type::x, "y", &value_type::y, "z", &value_type::z);
};

template<> struct jsonifier::core<AbcTest<test_struct>> {
	using value_type				 = AbcTest<test_struct>;
	static constexpr auto parseValue = createValue("z", &value_type::z, "y", &value_type::y, "x", &value_type::x, "w", &value_type::w, "v", &value_type::v, "u", &value_type::u,
		"t", &value_type::t, "s", &value_type::s, "r", &value_type::r, "q", &value_type::q, "p", &value_type::p, "o", &value_type::o, "n", &value_type::n, "m", &value_type::m, "l",
		&value_type::l, "k", &value_type::k, "j", &value_type::j, "i", &value_type::i, "h", &value_type::h, "g", &value_type::g, "f", &value_type::f, "e", &value_type::e, "d",
		&value_type::d, "c", &value_type::c, "b", &value_type::b, "a", &value_type::a);
};

template<> struct glz::meta<test_struct> {
	using value_type			= test_struct;
	static constexpr auto value = object("testStrings", &value_type::testStrings, "testUints", &value_type::testUints, "testDoubles", &value_type::testDoubles, "testInts",
		&value_type::testInts, "testBools", &value_type::testBools);
};

template<> struct glz::meta<Test<test_struct>> {
	using value_type			= Test<test_struct>;
	static constexpr auto value = object("a", &value_type::a, "b", &value_type::b, "c", &value_type::c, "d", &value_type::d, "e", &value_type::e, "f", &value_type::f, "g",
		&value_type::g, "h", &value_type::h, "i", &value_type::i, "j", &value_type::j, "k", &value_type::k, "l", &value_type::l, "m", &value_type::m, "n", &value_type::n, "o",
		&value_type::o, "p", &value_type::p, "q", &value_type::q, "r", &value_type::r, "s", &value_type::s, "t", &value_type::t, "u", &value_type::u, "v", &value_type::v, "w",
		&value_type::w, "x", &value_type::x, "y", &value_type::y, "z", &value_type::z);
};


template<> struct glz::meta<test_generator<test_struct>> {
	using value_type			= test_generator<test_struct>;
	static constexpr auto value = object("a", &value_type::a, "b", &value_type::b, "c", &value_type::c, "d", &value_type::d, "e", &value_type::e, "f", &value_type::f, "g",
		&value_type::g, "h", &value_type::h, "i", &value_type::i, "j", &value_type::j, "k", &value_type::k, "l", &value_type::l, "m", &value_type::m, "n", &value_type::n, "o",
		&value_type::o, "p", &value_type::p, "q", &value_type::q, "r", &value_type::r, "s", &value_type::s, "t", &value_type::t, "u", &value_type::u, "v", &value_type::v, "w",
		&value_type::w, "x", &value_type::x, "y", &value_type::y, "z", &value_type::z);
};

template<> struct glz::meta<AbcTest<test_struct>> {
	using value_type			= AbcTest<test_struct>;
	static constexpr auto value = object("z", &value_type::z, "y", &value_type::y, "x", &value_type::x, "w", &value_type::w, "v", &value_type::v, "u", &value_type::u, "t",
		&value_type::t, "s", &value_type::s, "r", &value_type::r, "q", &value_type::q, "p", &value_type::p, "o", &value_type::o, "n", &value_type::n, "m", &value_type::m, "l",
		&value_type::l, "k", &value_type::k, "j", &value_type::j, "i", &value_type::i, "h", &value_type::h, "g", &value_type::g, "f", &value_type::f, "e", &value_type::e, "d",
		&value_type::d, "c", &value_type::c, "b", &value_type::b, "a", &value_type::a);
};

#if defined(NDEBUG)
constexpr uint64_t iterationsVal = 100;
#else
constexpr uint64_t iterationsVal = 1;
#endif

enum class result_type { read = 0, write = 1 };

template<result_type type> std::string enumToString() {
	return type == result_type::read ? "Read" : "Write";
}

template<result_type type> struct result {
	std::optional<uint64_t> byteLength{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::string color{};

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result() noexcept = default;

	result(const std::string& colorNew, uint64_t byteLengthNew, double jsonTimeNew) {
		byteLength.emplace(byteLengthNew);
		jsonTime.emplace(jsonTimeNew);
		auto mbWrittenCount	  = static_cast<double>(byteLength.value()) / 1e+6l;
		auto writeSecondCount = jsonTime.value() / 1e+9l;
		jsonSpeed.emplace(mbWrittenCount / writeSecondCount);
		color = colorNew;
	}

	operator bool() const {
		return jsonSpeed.has_value();
	}

	bool operator>(const result& other) const {
		if (jsonSpeed.has_value() && other.jsonSpeed.has_value()) {
			return this->jsonSpeed.value() > other.jsonSpeed.value();
		} else if (!jsonSpeed.has_value()) {
			return false;
		} else if (!other.jsonSpeed.has_value()) {
			return true;
		}
		return false;
	}
};

struct results_data {
	std::unordered_set<std::string> jsonifierExcludedKeys{};
	result<result_type::write> writeResult{};
	result<result_type::read> readResult{};
	uint64_t iterations{};
	std::string name{};
	std::string test{};
	std::string url{};

	bool operator>(const results_data& other) const {
		if (readResult && other.readResult) {
			return readResult > other.readResult;
		} else if (writeResult && other.writeResult) {
			return writeResult > other.writeResult;
		} else {
			return false;
		}
	}

	results_data& operator=(results_data&&) noexcept	  = default;
	results_data(results_data&&) noexcept				  = default;
	results_data& operator=(const results_data&) noexcept = default;
	results_data(const results_data&) noexcept			  = default;

	results_data() noexcept = default;

	results_data(const std::string& nameNew, const std::string& testNew, const std::string& urlNew, uint64_t iterationsNew) {
		iterations = iterationsNew;
		name	   = nameNew;
		test	   = testNew;
		url		   = urlNew;
	}

	void checkForMissingKeys() {
		if (!writeResult.jsonSpeed.has_value()) {
			jsonifierExcludedKeys.emplace("writeResult");
		} else if (!readResult.jsonSpeed.has_value()) {
			jsonifierExcludedKeys.emplace("readResult");
		}
	}

	void print() const {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";
		if (readResult.byteLength.has_value() && readResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::read>() + " Speed: " << readResult.jsonSpeed.value() << " MB/s\n";
			std::cout << enumToString<result_type::read>() + " Length: " << readResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Runtime: " << readResult.jsonTime.value() << std::endl;
		}
		if (writeResult.byteLength.has_value() && writeResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::write>() + " Speed: " << writeResult.jsonSpeed.value() << " MB/s\n";
			std::cout << enumToString<result_type::write>() + " Length: " << writeResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Runtime: " << writeResult.jsonTime.value() << std::endl;
		}
		std::cout << "\n---" << std::endl;
	}

	std::string jsonStats() const {
		std::string write{};
		std::string read{};
		std::string writeTime{};
		std::string readTime{};
		std::string writeLength{};
		std::string readLength{};
		std::string finalstring{};
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << readResult.jsonSpeed.value();
			read = stream01.str();
			std::stringstream stream02{};
			stream02 << readResult.byteLength.value();
			readLength = stream02.str();
			std::stringstream stream03{};
			stream03 << readResult.jsonTime.value();
			readTime = stream03.str();
		} else {
			readLength = "N/A";
			readTime = "N/A";
			read = "N/A";
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << writeResult.jsonSpeed.value();
			write = stream01.str();
			std::stringstream stream02{};
			stream02 << writeResult.byteLength.value();
			writeLength = stream02.str();
			std::stringstream stream03{};
			stream03 << writeResult.jsonTime.value();
			writeTime = stream03.str();
		} else {
			writeLength = "N/A";
			writeTime	= "N/A";
			write = "N/A";
		}

		finalstring =
			std::string{ "| [" + name + "](" + url + ") | " + read + " | " + readLength + " | " + readTime + " | " + write + " | " + writeLength + " | " + writeTime + " |" };
		return finalstring;
	}
};

struct test_results {
	std::vector<results_data> results{};
	std::string markdownResults{};
	std::string testName{};
};

template<result_type type> struct jsonifier::core<result<type>> {
	using value_type = result<type>;
	static constexpr auto parseValue =
		createValue("byteLength", &value_type::byteLength, "jsonSpeed", &value_type::jsonSpeed, "jsonTime", &value_type::jsonTime, "color", &value_type::color);
};

template<> struct jsonifier::core<results_data> {
	using value_type = results_data;
	static constexpr auto parseValue =
		createValue("name", &value_type::name, "readResult", &value_type::readResult, "writeResult", &value_type::writeResult, "test", &value_type::test, "url", &value_type::url);
};

template<> struct jsonifier::core<test_results> {
	using value_type				 = test_results;
	static constexpr auto parseValue = createValue("results", &value_type::results, "testName", &value_type::testName);
};

void doNotOptimizeAway(std::function<void(void)>&& x) {
	x();
	static auto ttid = std::this_thread::get_id();
	if (ttid == std::thread::id()) {
		const auto* p = &x;
		putchar(*reinterpret_cast<const char*>(p));
		std::abort();
	}
}

template<typename function_type> double benchmark(function_type&& function, int64_t iterationCount) {
	std::vector<double> iterations{};
	iterations.reserve(iterationCount);
	function_type newFunction{ std::forward<function_type>(function) };
	for (int64_t x = 0; x < iterationCount; ++x) {
		auto startTime = std::chrono::high_resolution_clock::now();
		doNotOptimizeAway(newFunction);
		auto endTime = std::chrono::high_resolution_clock::now();
		auto newTime = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(endTime - startTime);
		iterations.emplace_back(newTime.count());
	}
	std::sort(iterations.begin(), iterations.end());
	if (iterations.size() % 2 == 0) {
		auto mid = iterations.size() / 2;
		return (iterations[mid - 1] + iterations[mid]) / 2.0;
	} else {
		return iterations[iterations.size() / 2];
	}
}

enum class test_type {
	parse_and_serialize		   = 0,
	minify					   = 1,
	prettify				   = 2,
	validate				   = 3,
};

static std::string basePath{ JSON_PATH };

template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto jsonifierTest(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	results_data r{ "jsonifier", testName, "https://github.com/realtimechris/jsonifier", iterations };
	test_data_type testData{};
	jsonifier::jsonifier_core parser{};
	auto readResult = benchmark(
		[&]() {
			auto result = parser.parseJson<jsonifier::parse_options{ .minified = minified }>(testData, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	auto readSize = buffer.size();
	buffer.clear();
	auto writeResult = benchmark(
		[&]() {
			auto result = parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, buffer);
		},
		iterations);

	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	auto writtenSize = buffer.size();
	r.readResult	 = result<result_type::read>{ "teal", readSize, readResult };
	r.writeResult	 = result<result_type::write>{ "steelblue", writtenSize, writeResult };
	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::prettify, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "jsonifier", "Prettify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };
	jsonifier::jsonifier_core parser{};

	auto writeResult = benchmark(
		[&]() {
			newerBuffer = parser.prettifyJson(buffer);
		},
		iterationsVal);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size(), writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::minify, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };

	results_data r{ "jsonifier", "Minify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };

	jsonifier::jsonifier_core parser{};
	std::string newerBuffer{};
	auto writeResult = benchmark(
		[&]() {
			newerBuffer = parser.minifyJson(buffer);
		},
		iterationsVal);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(newerBuffer);

	r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size(), writeResult };

	if (doWePrint) {
		r.print();
	}
	return r;
}

template<> auto jsonifierTest<test_type::validate, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };

	results_data r{ "jsonifier", "Validation Test", "https://github.com/realtimechris/jsonifier", iterationsVal };
	jsonifier::jsonifier_core parser{};

	auto readResult = benchmark(
		[&]() {
			auto result = parser.validateJson(buffer);
		},
		iterationsVal);

	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.readResult = result<result_type::read>{ "teal", buffer.size(), readResult };

	if (doWePrint) {
		r.print();
	}


	return r;
}

#if !defined(ASAN)
template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto glazeTest(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	results_data r{ "glaze", testName, "https://github.com/stephenberry/glaze", iterations };
	test_data_type testData{};
	auto readResult = benchmark(
		[&]() {
			auto result = glz::read<glz::opts{ .skip_null_members = false, .minified = minified, .force_conformance = true }>(testData, buffer);
			if (result) {
				std::cout << "Glaze Error: " << glz::format_error(result, buffer) << std::endl;
			}
		},
		iterations);
	auto readSize = buffer.size();
	buffer.clear();
	auto writeResult = benchmark(
		[&]() {
			glz::write<glz::opts{ .skip_null_members = false, .prettify = !minified, .minified = minified, .force_conformance = true }>(testData, buffer);
		},
		iterations);

	auto writtenSize = buffer.size();
	r.readResult	 = result<result_type::read>{ "dodgerblue", readSize, readResult };
	r.writeResult	 = result<result_type::write>{ "skyblue", writtenSize, writeResult };
	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto glazeTest<test_type::prettify, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "glaze", "Prettify Test", "https://github.com/stephenberry/glaze", iterationsVal };

	auto writeResult = benchmark(
		[&]() {
			newerBuffer = glz::prettify_json(buffer);
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size(), writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto glazeTest<test_type::minify, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "glaze", "Minify Test", "https://github.com/stephenberry/glaze", iterationsVal };

	auto writeResult = benchmark(
		[&]() {
			newerBuffer = glz::minify_json(buffer);
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size(), writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

	#include "simdjson.h"

template<typename value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	value.get(valueNew);
}

template<jsonifier::concepts::bool_t value_type> void getValue(value_type&& valueNew, simdjson::ondemand::value value) {
	getValue<bool>(valueNew, value);
}

void getValue(std::nullptr_t& valueNew, simdjson::ondemand::value value) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	auto oldSize = valueNew.size();
	value_type returnValues{};
	if (!value.get(result)) {
		auto iter = result.begin();
		for (uint64_t x = 0; x < oldSize && iter != result.end(); ++x, ++iter) {
			getValue(returnValues[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			typename value_type::value_type returnValue{};
			getValue(returnValue, iter.operator*().value());
			returnValues.emplace_back(std::move(returnValue));
		}
	}
}

template<> void getValue<std::string>(std::string& valueNew, simdjson::ondemand::value value) {
	std::string_view result;
	if (!value.get(result)) {
		valueNew = static_cast<std::string>(result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value valueNewer) {
	simdjson::ondemand::value result;
	if (!valueNewer.get(result)) {
		getValue(valueNew.emplace(), result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (!value[key].get(result)) {
		getValue(returnValue.emplace(), result);
	}
}

template<typename value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (!value[key].get(result)) {
		getValue(returnValue, result);
	}
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::array result;
	if (!value[key].get(result)) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (uint64_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
			getValue(returnValues[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(returnValues.emplace_back(), iter.operator*().value());
		}
	}
	return;
}

template<jsonifier::concepts::map_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::object result;
	if (!value[key].get(result)) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (uint64_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
			typename value_type::mapped_type returnValue{};
			getValue(returnValue, iter.operator*().value());
			returnValues[static_cast<typename value_type::key_type>(iter.operator*().key().raw())] = std::move(returnValue);
		}
		for (; iter != result.end(); ++iter) {
			typename value_type::mapped_type returnValue{};
			getValue(returnValue, iter.operator*().value());
			returnValues[static_cast<typename value_type::key_type>(iter.operator*().key().raw())] = std::move(returnValue);
		}
	}
	return;
}

template<> void getValue<geometry_data>(geometry_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.coordinates, jsonDataNew, "coordinates");
	getValue(returnValue.type, jsonDataNew, "type");
}

template<> void getValue<properties_data>(properties_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.name, jsonDataNew, "name");
}

template<> void getValue<feature>(feature& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.properties, jsonDataNew, "properties");
	getValue(returnValue.geometry, jsonDataNew, "geometry");
	getValue(returnValue.type, jsonDataNew, "type");
}

template<> void getValue<canada_message>(canada_message& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.features, jsonDataNew, "features");
	getValue(returnValue.type, jsonDataNew, "type");
}

template<> void getValue<search_metadata_data>(search_metadata_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.since_id_str, jsonDataNew, "since_id_str");
	getValue(returnValue.next_results, jsonDataNew, "next_results");
	getValue(returnValue.refresh_url, jsonDataNew, "refresh_url");
	getValue(returnValue.max_id_str, jsonDataNew, "max_id_str");
	getValue(returnValue.completed_in, jsonDataNew, "completed_in");
	getValue(returnValue.query, jsonDataNew, "query");
	getValue(returnValue.since_id, jsonDataNew, "since_id");
	getValue(returnValue.count, jsonDataNew, "count");
	getValue(returnValue.max_id, jsonDataNew, "max_id");
}

template<> void getValue<hashtag>(hashtag& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.text, jsonDataNew, "text");
}

template<> void getValue<large_data>(large_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.resize, jsonDataNew, "resize");
	getValue(returnValue.w, jsonDataNew, "w");
	getValue(returnValue.h, jsonDataNew, "h");
}

template<> void getValue<sizes_data>(sizes_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.medium, jsonDataNew, "medium");
	getValue(returnValue.small, jsonDataNew, "small");
	getValue(returnValue.thumb, jsonDataNew, "thumb");
	getValue(returnValue.large, jsonDataNew, "large");
}

template<> void getValue<media_data>(media_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.source_status_id_str, jsonDataNew, "source_status_id_str");
	getValue(returnValue.source_status_id, jsonDataNew, "source_status_id");
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.media_url_https, jsonDataNew, "media_url_https");
	getValue(returnValue.expanded_url, jsonDataNew, "expanded_url");
	getValue(returnValue.display_url, jsonDataNew, "display_url");
	getValue(returnValue.media_url, jsonDataNew, "media_url");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.type, jsonDataNew, "type");
	getValue(returnValue.sizes, jsonDataNew, "sizes");
	getValue(returnValue.url, jsonDataNew, "url");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue<url_data>(url_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.expanded_url, jsonDataNew, "expanded_url");
	getValue(returnValue.display_url, jsonDataNew, "display_url");
	getValue(returnValue.url, jsonDataNew, "url");
}

template<> void getValue<user_mention>(user_mention& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.screen_name, jsonDataNew, "screen_name");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue<status_entities>(status_entities& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.media, jsonDataNew, "media");
	getValue(returnValue.user_mentions, jsonDataNew, "user_mentions");
	getValue(returnValue.symbols, jsonDataNew, "symbols");
	getValue(returnValue.hashtags, jsonDataNew, "hashtags");
	getValue(returnValue.urls, jsonDataNew, "urls");
}

template<> void getValue<metadata_data>(metadata_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.iso_language_code, jsonDataNew, "iso_language_code");
	getValue(returnValue.result_type, jsonDataNew, "result_type");
}

template<> void getValue<description_data>(description_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.urls, jsonDataNew, "urls");
}

template<> void getValue<user_entities>(user_entities& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.url, jsonDataNew, "url");
	getValue(returnValue.description, jsonDataNew, "description");
}

template<> void getValue<twitter_user>(twitter_user& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.profile_background_image_url_https, jsonDataNew, "profile_background_image_url_https");
	getValue(returnValue.profile_banner_url, jsonDataNew, "profile_banner_url");
	getValue(returnValue.profile_background_image_url, jsonDataNew, "profile_background_image_url");
	getValue(returnValue.profile_sidebar_fill_color, jsonDataNew, "profile_sidebar_fill_color");
	getValue(returnValue.time_zone, jsonDataNew, "time_zone");
	getValue(returnValue.profile_background_color, jsonDataNew, "profile_background_color");
	getValue(returnValue.profile_image_url_https, jsonDataNew, "profile_image_url_https");
	getValue(returnValue.utc_offset, jsonDataNew, "utc_offset");
	getValue(returnValue.profile_use_background_image, jsonDataNew, "profile_use_background_image");
	getValue(returnValue.url, jsonDataNew, "url");
	getValue(returnValue.profile_text_color, jsonDataNew, "profile_text_color");
	getValue(returnValue.profile_link_color, jsonDataNew, "profile_link_color");
	getValue(returnValue.profile_image_url, jsonDataNew, "profile_image_url");
	getValue(returnValue.profile_background_tile, jsonDataNew, "profile_background_tile");
	getValue(returnValue.is_translation_enabled, jsonDataNew, "is_translation_enabled");
	getValue(returnValue.default_profile_image, jsonDataNew, "default_profile_image");
	getValue(returnValue.contributors_enabled, jsonDataNew, "contributors_enabled");
	getValue(returnValue.follow_request_sent, jsonDataNew, "follow_request_sent");
	getValue(returnValue.favourites_count, jsonDataNew, "favourites_count");
	getValue(returnValue.description, jsonDataNew, "description");
	getValue(returnValue.screen_name, jsonDataNew, "screen_name");
	getValue(returnValue.followers_count, jsonDataNew, "followers_count");
	getValue(returnValue.statuses_count, jsonDataNew, "statuses_count");
	getValue(returnValue.created_at, jsonDataNew, "created_at");
	getValue(returnValue.entities, jsonDataNew, "entities");
	getValue(returnValue.friends_count, jsonDataNew, "friends_count");
	getValue(returnValue.default_profile, jsonDataNew, "default_profile");
	getValue(returnValue.listed_count, jsonDataNew, "listed_count");
	getValue(returnValue.location, jsonDataNew, "location");
	getValue(returnValue.user_protected, jsonDataNew, "user_protected");
	getValue(returnValue.is_translator, jsonDataNew, "is_translator");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.notifications, jsonDataNew, "notifications");
	getValue(returnValue.string, jsonDataNew, "string");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.geo_enabled, jsonDataNew, "geo_enabled");
	getValue(returnValue.lang, jsonDataNew, "lang");
	getValue(returnValue.following, jsonDataNew, "following");
	getValue(returnValue.verified, jsonDataNew, "verified");
	getValue(returnValue.id, jsonDataNew, "id");
}

void getValue(status& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.in_reply_to_status_id_str, jsonDataNew, "in_reply_to_status_id_str");
	getValue(returnValue.in_reply_to_user_id_str, jsonDataNew, "in_reply_to_user_id_str");
	getValue(returnValue.in_reply_to_screen_name, jsonDataNew, "in_reply_to_screen_name");
	getValue(returnValue.in_reply_to_status_id, jsonDataNew, "in_reply_to_status_id");
	getValue(returnValue.in_reply_to_user_id, jsonDataNew, "in_reply_to_user_id");
	getValue(returnValue.possibly_sensitive, jsonDataNew, "possibly_sensitive");
	getValue(returnValue.entities, jsonDataNew, "entities");
	getValue(returnValue.favorite_count, jsonDataNew, "favorite_count");
	getValue(returnValue.retweet_count, jsonDataNew, "retweet_count");
	getValue(returnValue.metadata, jsonDataNew, "metadata");
	getValue(returnValue.created_at, jsonDataNew, "created_at");
	getValue(returnValue.source, jsonDataNew, "source");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.lang, jsonDataNew, "lang");
	getValue(returnValue.text, jsonDataNew, "text");
	getValue(returnValue.truncated, jsonDataNew, "truncated");
	getValue(returnValue.favorited, jsonDataNew, "favorited");
	getValue(returnValue.retweeted, jsonDataNew, "retweeted");
	getValue(returnValue.user, jsonDataNew, "user");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue<twitter_message>(twitter_message& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.search_metadata, jsonDataNew, "search_metadata");
	getValue(returnValue.statuses, jsonDataNew, "statuses");
}

template<> void getValue<avatar_decoration_data_data>(avatar_decoration_data_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.sku_id, jsonDataNew, "sku_id");
	getValue(returnValue.asset, jsonDataNew, "asset");
}

template<> void getValue<user_data>(user_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.avatar_decoration_data, jsonDataNew, "avatar_decoration_data");
	getValue(returnValue.display_name, jsonDataNew, "display_name");
	getValue(returnValue.global_name, jsonDataNew, "global_name");
	getValue(returnValue.banner, jsonDataNew, "banner");
	getValue(returnValue.avatar, jsonDataNew, "avatar");
	getValue(returnValue.locale, jsonDataNew, "locale");
	getValue(returnValue.discriminator, jsonDataNew, "discriminator");
	getValue(returnValue.user_name, jsonDataNew, "user_name");
	getValue(returnValue.accent_color, jsonDataNew, "accent_color");
	getValue(returnValue.premium_type, jsonDataNew, "premium_type");
	getValue(returnValue.public_flags, jsonDataNew, "public_flags");
	getValue(returnValue.email, jsonDataNew, "email");
	getValue(returnValue.mfa_enabled, jsonDataNew, "mfa_enabled");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.verified, jsonDataNew, "verified");
	getValue(returnValue.system, jsonDataNew, "system");
	getValue(returnValue.bot, jsonDataNew, "bot");
}

template<> void getValue<guild_scheduled_event_data>(guild_scheduled_event_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.scheduled_start_time, jsonDataNew, "scheduled_start_time");
	getValue(returnValue.scheduled_end_time, jsonDataNew, "scheduled_end_time");
	getValue(returnValue.description, jsonDataNew, "description");
	getValue(returnValue.entity_metadata, jsonDataNew, "entity_metadata");
	getValue(returnValue.creator_id, jsonDataNew, "creator_id");
	getValue(returnValue.channel_id, jsonDataNew, "channel_id");
	getValue(returnValue.privacy_level, jsonDataNew, "privacy_level");
	getValue(returnValue.entity_id, jsonDataNew, "entity_id");
	getValue(returnValue.guild_id, jsonDataNew, "guild_id");
	getValue(returnValue.entity_type, jsonDataNew, "entity_type");
	getValue(returnValue.user_count, jsonDataNew, "user_count");
	getValue(returnValue.creator, jsonDataNew, "creator");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.status, jsonDataNew, "status");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue<tags_data>(tags_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.premium_subscriber, jsonDataNew, "premium_subscriber");
	getValue(returnValue.bot_id, jsonDataNew, "bot_id");
}

template<> void getValue<role_data>(role_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.unicode_emoji, jsonDataNew, "unicode_emoji");
	getValue(returnValue.icon, jsonDataNew, "icon");
	getValue(returnValue.tags, jsonDataNew, "tags");
	getValue(returnValue.permissions, jsonDataNew, "permissions");
	getValue(returnValue.position, jsonDataNew, "position");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.mentionable, jsonDataNew, "mentionable");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.color, jsonDataNew, "color");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.managed, jsonDataNew, "managed");
	getValue(returnValue.hoist, jsonDataNew, "hoist");
}

template<> void getValue<icon_emoji_data>(icon_emoji_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue<permission_overwrites_data>(permission_overwrites_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.allow, jsonDataNew, "allow");
	getValue(returnValue.deny, jsonDataNew, "deny");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.type, jsonDataNew, "type");
}

template<> void getValue<guild_member_data>(guild_member_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.communication_disabled_until, jsonDataNew, "communication_disabled_until");
	getValue(returnValue.premium_since, jsonDataNew, "premium_since");
	getValue(returnValue.avatar, jsonDataNew, "avatar");
	getValue(returnValue.nick, jsonDataNew, "nick");
	getValue(returnValue.roles, jsonDataNew, "roles");
	getValue(returnValue.permissions, jsonDataNew, "permissions");
	getValue(returnValue.joined_at, jsonDataNew, "joined_at");
	getValue(returnValue.guild_id, jsonDataNew, "guild_id");
	getValue(returnValue.user, jsonDataNew, "user");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.pending, jsonDataNew, "pending");
	getValue(returnValue.deaf, jsonDataNew, "deaf");
	getValue(returnValue.mute, jsonDataNew, "mute");
}

template<> void getValue<channel_data>(channel_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.permission_overwrites, jsonDataNew, "permission_overwrites");
	getValue(returnValue.last_message_id, jsonDataNew, "last_message_id");
	getValue(returnValue.default_thread_rate_limit_per_user, jsonDataNew, "default_thread_rate_limit_per_user");
	getValue(returnValue.applied_tags, jsonDataNew, "applied_tags");
	getValue(returnValue.rtc_region, jsonDataNew, "rtc_region");
	getValue(returnValue.default_auto_archive_duration, jsonDataNew, "default_auto_archive_duration");
	getValue(returnValue.status, jsonDataNew, "status");
	getValue(returnValue.recipients, jsonDataNew, "recipients");
	getValue(returnValue.topic, jsonDataNew, "topic");
	getValue(returnValue.last_pin_timestamp, jsonDataNew, "last_pin_timestamp");
	getValue(returnValue.total_message_sent, jsonDataNew, "total_message_sent");
	getValue(returnValue.rate_limit_per_user, jsonDataNew, "rate_limit_per_user");
	getValue(returnValue.video_quality_mode, jsonDataNew, "video_quality_mode");
	getValue(returnValue.application_id, jsonDataNew, "application_id");
	getValue(returnValue.permissions, jsonDataNew, "permissions");
	getValue(returnValue.message_count, jsonDataNew, "message_count");
	getValue(returnValue.member_count, jsonDataNew, "member_count");
	getValue(returnValue.parent_id, jsonDataNew, "parent_id");
	getValue(returnValue.icon_emoji, jsonDataNew, "icon_emoji");
	getValue(returnValue.owner_id, jsonDataNew, "owner_id");
	getValue(returnValue.guild_id, jsonDataNew, "guild_id");
	getValue(returnValue.user_limit, jsonDataNew, "user_limit");
	getValue(returnValue.position, jsonDataNew, "position");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.bitrate, jsonDataNew, "bitrate");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.icon, jsonDataNew, "icon");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.type, jsonDataNew, "type");
	getValue(returnValue.managed, jsonDataNew, "managed");
	getValue(returnValue.nsfw, jsonDataNew, "nsfw");
}

template<> void getValue<guild_data>(guild_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.guild_scheduled_events, jsonDataNew, "guild_scheduled_events");
	getValue(returnValue.latest_on_boarding_question_id, jsonDataNew, "latest_on_boarding_question_id");
	getValue(returnValue.safety_alerts_channel_id, jsonDataNew, "safety_alerts_channel_id");
	getValue(returnValue.inventory_settings, jsonDataNew, "inventory_settings");
	getValue(returnValue.discovery_splash, jsonDataNew, "discovery_splash");
	getValue(returnValue.application_id, jsonDataNew, "application_id");
	getValue(returnValue.vanity_url_code, jsonDataNew, "vanity_url_code");
	getValue(returnValue.description, jsonDataNew, "description");
	getValue(returnValue.members, jsonDataNew, "members");
	getValue(returnValue.default_message_notifications, jsonDataNew, "default_message_notifications");
	getValue(returnValue.voice_states, jsonDataNew, "voice_states");
	getValue(returnValue.channels, jsonDataNew, "channels");
	getValue(returnValue.max_stage_video_channel_users, jsonDataNew, "max_stage_video_channel_users");
	getValue(returnValue.public_updates_channel_id, jsonDataNew, "public_updates_channel_id");
	getValue(returnValue.hub_type, jsonDataNew, "hub_type");
	getValue(returnValue.premium_subscription_count, jsonDataNew, "premium_subscription_count");
	getValue(returnValue.features, jsonDataNew, "features");
	getValue(returnValue.approximate_presence_count, jsonDataNew, "approximate_presence_count");
	getValue(returnValue.stickers, jsonDataNew, "stickers");
	getValue(returnValue.splash, jsonDataNew, "splash");
	getValue(returnValue.threads, jsonDataNew, "threads");
	getValue(returnValue.approximate_member_count, jsonDataNew, "approximate_member_count");
	getValue(returnValue.premium_progress_bar_enabled, jsonDataNew, "premium_progress_bar_enabled");
	getValue(returnValue.explicit_content_filter, jsonDataNew, "explicit_content_filter");
	getValue(returnValue.max_video_channel_users, jsonDataNew, "max_video_channel_users");
	getValue(returnValue.roles, jsonDataNew, "roles");
	getValue(returnValue.system_channel_id, jsonDataNew, "system_channel_id");
	getValue(returnValue.widget_channel_id, jsonDataNew, "widget_channel_id");
	getValue(returnValue.preferred_locale, jsonDataNew, "preferred_locale");
	getValue(returnValue.system_channel_flags, jsonDataNew, "system_channel_flags");
	getValue(returnValue.rules_channel_id, jsonDataNew, "rules_channel_id");
	getValue(returnValue.verification_level, jsonDataNew, "verification_level");
	getValue(returnValue.afk_channel_id, jsonDataNew, "afk_channel_id");
	getValue(returnValue.permissions, jsonDataNew, "permissions");
	getValue(returnValue.max_presences, jsonDataNew, "max_presences");
	getValue(returnValue.discovery, jsonDataNew, "discovery");
	getValue(returnValue.member_count, jsonDataNew, "member_count");
	getValue(returnValue.joined_at, jsonDataNew, "joined_at");
	getValue(returnValue.premium_tier, jsonDataNew, "premium_tier");
	getValue(returnValue.owner_id, jsonDataNew, "owner_id");
	getValue(returnValue.max_members, jsonDataNew, "max_members");
	getValue(returnValue.afk_timeout, jsonDataNew, "afk_timeout");
	getValue(returnValue.banner, jsonDataNew, "banner");
	getValue(returnValue.widget_enabled, jsonDataNew, "widget_enabled");
	getValue(returnValue.region, jsonDataNew, "region");
	getValue(returnValue.nsfw_level, jsonDataNew, "nsfw_level");
	getValue(returnValue.mfa_level, jsonDataNew, "mfa_level");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.icon, jsonDataNew, "icon");
	getValue(returnValue.unavailable, jsonDataNew, "unavailable");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.large, jsonDataNew, "large");
	getValue(returnValue.owner, jsonDataNew, "owner");
	getValue(returnValue.nsfw, jsonDataNew, "nsfw");
	getValue(returnValue.lazy, jsonDataNew, "lazy");
}

template<> void getValue<discord_message>(discord_message& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.t, jsonDataNew, "t");
	getValue(returnValue.d, jsonDataNew, "d");
	getValue(returnValue.op, jsonDataNew, "op");
	getValue(returnValue.s, jsonDataNew, "s");
}

	#define SIMD_PULL(x) \
		{ \
			simdjson::ondemand::array newX = doc[#x].get_array().value(); \
			for (simdjson::ondemand::value value: newX) { \
				test_struct newStruct{}; \
				getValue(newStruct.testStrings, value, "testStrings"); \
				getValue(newStruct.testUints, value, "testUints"); \
				getValue(newStruct.testDoubles, value, "testDoubles"); \
				getValue(newStruct.testInts, value, "testInts"); \
				getValue(newStruct.testBools, value, "testBools"); \
				obj.x.emplace_back(std::move(newStruct)); \
			} \
		}

	#define SIMD_PULL_ABC(x) \
		{ \
			simdjson::ondemand::array newX = doc[#x].get_array().value(); \
			for (simdjson::ondemand::value value: newX) { \
				test_struct newStruct{}; \
				getValue(newStruct.testBools, value, "testBools"); \
				getValue(newStruct.testInts, value, "testInts"); \
				getValue(newStruct.testDoubles, value, "testDoubles"); \
				getValue(newStruct.testUints, value, "testUints"); \
				getValue(newStruct.testStrings, value, "testStrings"); \
				obj.x.emplace_back(std::move(newStruct)); \
			} \
		}


template<> void getValue<Test<test_struct>>(Test<test_struct>& obj, simdjson::ondemand::value doc) {
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
}

template<> void getValue<AbcTest<test_struct>>(AbcTest<test_struct>& obj, simdjson::ondemand::value doc) {
	SIMD_PULL_ABC(z);
	SIMD_PULL_ABC(y);
	SIMD_PULL_ABC(x);
	SIMD_PULL_ABC(w);
	SIMD_PULL_ABC(v);
	SIMD_PULL_ABC(u);
	SIMD_PULL_ABC(t);
	SIMD_PULL_ABC(s);
	SIMD_PULL_ABC(r);
	SIMD_PULL_ABC(q);
	SIMD_PULL_ABC(p);
	SIMD_PULL_ABC(o);
	SIMD_PULL_ABC(n);
	SIMD_PULL_ABC(m);
	SIMD_PULL_ABC(l);
	SIMD_PULL_ABC(k);
	SIMD_PULL_ABC(j);
	SIMD_PULL_ABC(i);
	SIMD_PULL_ABC(h);
	SIMD_PULL_ABC(g);
	SIMD_PULL_ABC(f);
	SIMD_PULL_ABC(e);
	SIMD_PULL_ABC(d);
	SIMD_PULL_ABC(c);
	SIMD_PULL_ABC(b);
	SIMD_PULL_ABC(a);
}

template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto simdjsonTest(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	results_data r{ "simdjson", testName, "https://github.com/simdjson/simdjson", iterations };
	test_data_type testData{};

	simdjson::ondemand::parser parser{};
	auto readSize	= buffer.size();
	auto readResult = benchmark(
		[&]() {
			try {
				auto doc = parser.iterate(buffer);
				getValue(testData, doc.value());
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.readResult = result<result_type::read>{ "cadetblue", readSize, readResult };
	file_loader fileLoader{ basePath + "/" + testName + "-simdjson.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto simdjsonTest<test_type::minify, std::string, false, iterationsVal>(const std::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "simdjson", "Minify Test", "https://github.com/simdjson/simdjson", iterationsVal };

	simdjson::dom::parser parser{};

	auto writeResult = benchmark(
		[&]() {
			try {
				auto doc	= parser.parse(buffer);
				newerBuffer = simdjson::minify(doc);
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-simdjson.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "cornflowerblue", newerBuffer.size(), writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}
#endif

std::string table_header = R"(
| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |)";

template<test_type type, typename test_data_type, bool minified, uint64_t iterations> test_results jsonTests(const std::string& jsonDataNew, const std::string& testName) {
	jsonifier::vector<results_data> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t y = 0; y < 2; ++y) {
		simdjsonTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(simdjsonTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, true));
	for (uint32_t y = 0; y < 2; ++y) {
		glazeTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t y = 0; y < 2; ++y) {
		jsonifierTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<type, test_data_type, minified, iterations>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
	for (uint64_t x = 0; x < n; ++x) {
		jsonResults.results.emplace_back(resultsNew[x]);
		table += resultsNew[x].jsonStats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

template<> test_results jsonTests<test_type::prettify, std::string, false, iterationsVal>(const std::string& jsonDataNew, const std::string& testName) {
	jsonifier::vector<results_data> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t y = 0; y < 2; ++y) {
		glazeTest<test_type::prettify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<test_type::prettify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t y = 0; y < 2; ++y) {
		jsonifierTest<test_type::prettify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::prettify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
	for (uint64_t x = 0; x < n; ++x) {
		jsonResults.results.emplace_back(resultsNew[x]);
		table += resultsNew[x].jsonStats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

template<> test_results jsonTests<test_type::minify, std::string, false, iterationsVal>(const std::string& jsonDataNew, const std::string& testName) {
	jsonifier::vector<results_data> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t y = 0; y < 2; ++y) {
		simdjsonTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(simdjsonTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));
	for (uint32_t y = 0; y < 2; ++y) {
		glazeTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t y = 0; y < 2; ++y) {
		jsonifierTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::minify, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
	for (uint64_t x = 0; x < n; ++x) {
		jsonResults.results.emplace_back(resultsNew[x]);
		table += resultsNew[x].jsonStats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

template<> test_results jsonTests<test_type::validate, std::string, false, iterationsVal>(const std::string& jsonDataNew, const std::string& testName) {
	jsonifier::vector<results_data> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
	for (uint32_t y = 0; y < 2; ++y) {
		jsonifierTest<test_type::validate, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::validate, std::string, false, iterationsVal>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
	for (uint64_t x = 0; x < n; ++x) {
		jsonResults.results.emplace_back(resultsNew[x]);
		table += resultsNew[x].jsonStats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	jsonResults.markdownResults = table;
	return jsonResults;
}

static const std::string section001{ R"(> )" + jsonifier::toString(iterationsVal) + R"( iterations on a 6 core (Intel i7 8700k)
)" };

static constexpr std::string_view section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: ()" };

static constexpr std::string_view section01{
	R"(

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section02{
	R"(

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static const std::string_view section03{
	R"(

### ABC Test (Out of Sequence Performance - Prettified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
)"
};

static constexpr std::string_view section04{ R"(

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section05{ R"(

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section06{ R"(

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section07{ R"(

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section08{ R"(

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section09{ R"(

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section10{ R"(

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section11{
	R"(

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section12{
	R"(

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section13{
	R"(

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

#include "ConformanceTests.hpp"


int main() {
	try {
		test_generator<test_struct> testJsonData{};
		std::string jsonDataNew{};
		jsonifier::jsonifier_core parser{};
		file_loader fileLoader01{ basePath + "/../ReadMe.md" };
		file_loader fileLoader02{ basePath + "/JsonData-Prettified.json" };
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonDataNew);
		fileLoader02.saveFile(jsonDataNew);
		file_loader fileLoader03{ basePath + "/JsonData-Minified.json" };
		std::string jsonMinifiedData{ parser.minifyJson(jsonDataNew) };
		fileLoader03.saveFile(jsonMinifiedData);
		file_loader fileLoader04{ basePath + "/Results.json" };
		file_loader fileLoader05{ basePath + "/DiscordData-Prettified.json" };
		std::string discordData{ fileLoader05.operator std::string() };
		discord_message discordMessage{};
		parser.parseJson(discordMessage, discordData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(discordMessage, discordData);
		fileLoader05.saveFile(discordData);
		file_loader fileLoader06{ basePath + "/DiscordData-Minified.json" };
		std::string discordMinifiedData{ fileLoader06.operator std::string() };
		discordMinifiedData = parser.minifyJson(discordData);
		fileLoader06.saveFile(discordMinifiedData);
		file_loader fileLoader07{ basePath + "/CanadaData-Prettified.json" };
		std::string canadaData{ fileLoader07.operator std::string() };
		canada_message canadaMessage{};
		parser.parseJson(canadaMessage, canadaData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(canadaMessage, canadaData);
		fileLoader07.saveFile(canadaData);
		file_loader fileLoader08{ basePath + "/CanadaData-Minified.json" };
		std::string canadaMinifiedData{ fileLoader08.operator std::string() };
		canadaMinifiedData = parser.minifyJson(canadaData);
		fileLoader08.saveFile(canadaMinifiedData);
		file_loader fileLoader09{ basePath + "/TwitterData-Prettified.json" };
		std::string twitterData{ fileLoader09.operator std::string() };
		twitter_message twitterMessage{};
		parser.parseJson(twitterMessage, twitterData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(twitterMessage, twitterData);
		fileLoader09.saveFile(twitterData);
		file_loader fileLoader10{ basePath + "/TwitterData-Minified.json" };
		std::string twitterMinifiedData{ fileLoader10.operator std::string() };
		twitterMinifiedData = parser.minifyJson(twitterData);
		fileLoader10.saveFile(twitterMinifiedData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t result = std::time(nullptr);
		resultTwo		   = *localtime(&result);
		conformanceTests();
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		std::string newerString{ static_cast<std::string>(section00) + newTimeString + static_cast<std::string>(section01) };
		auto testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, false, iterationsVal>(jsonDataNew, "Json Test (Prettified)");
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, true, iterationsVal>(jsonMinifiedData, "Json Test (Minified)");
		newerString += section02;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, AbcTest<test_struct>, false, iterationsVal>(jsonDataNew, "Abc Test (Prettified)");
		newerString += section03;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, AbcTest<test_struct>, true, iterationsVal>(jsonMinifiedData, "Abc Test (Minified)");
		newerString += section04;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, false, iterationsVal>(discordData, "Discord Test (Prettified)");
		newerString += section05;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, true, iterationsVal>(discordMinifiedData, "Discord Test (Minified)");
		newerString += section06;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, canada_message, false, iterationsVal>(canadaData, "Canada Test (Prettified)");
		newerString += section07;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, canada_message, true, iterationsVal>(canadaMinifiedData, "Canada Test (Minified)");
		newerString += section08;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, twitter_message, false, iterationsVal>(twitterData, "Twitter Test (Prettified)");
		newerString += section09;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, twitter_message, true, iterationsVal>(twitterMinifiedData, "Twitter Test (Minified)");
		newerString += section10;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::minify, std::string, false, iterationsVal>(discordData, "Minify Test");
		newerString += section11;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::prettify, std::string, false, iterationsVal>(jsonMinifiedData, "Prettify Test");
		newerString += section12;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::validate, std::string, false, iterationsVal>(discordData, "Validate Test");
		newerString += section13;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		std::string resultsStringJson{};
		test_results_final resultsData{};
		for (auto& value: benchmark_data) {
			test_elements_final testElement{};
			testElement.testName = value.testName;
			for (auto& valueNew: value.results) {
				test_element_final resultFinal{};
				if (valueNew.readResult.jsonSpeed.has_value()) {
					resultFinal.libraryName = valueNew.name;
					resultFinal.color		= valueNew.readResult.color;
					resultFinal.resultSpeed = valueNew.readResult.jsonSpeed.value();
					resultFinal.resultType	= "Read";
					testElement.results.emplace_back(resultFinal);
				}
				if (valueNew.writeResult.jsonSpeed.has_value()) {
					resultFinal.libraryName = valueNew.name;
					resultFinal.color		= valueNew.writeResult.color;
					resultFinal.resultSpeed = valueNew.writeResult.jsonSpeed.value();
					resultFinal.resultType	= "Write";
					testElement.results.emplace_back(resultFinal);
				}
			}
			resultsData.emplace_back(testElement);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(resultsData, resultsStringJson);
		fileLoader04.saveFile(resultsStringJson);
		fileLoader01.saveFile(newerString);
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}