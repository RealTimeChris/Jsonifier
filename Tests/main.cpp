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

struct status_data {
	std::optional<std::string> in_reply_to_status_id_str{};
	std::optional<std::string> in_reply_to_user_id_str{};
	std::optional<std::string> in_reply_to_screen_name{};
	std::optional<double> in_reply_to_status_id{};
	std::optional<int64_t> in_reply_to_user_id{};
	std::optional<bool> possibly_sensitive{};
	std::nullptr_t contributors{ nullptr };
	std::nullptr_t coordinates{ nullptr };
	std::nullptr_t place{ nullptr };
	std::nullptr_t geo{ nullptr };
	status_entities entities{};
	int64_t favorite_count{};
	metadata_data metadata{};
	std::string created_at{};
	int64_t retweet_count{};
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
	std::vector<status_data> statuses{};
};

struct activity_instances_data {};

struct available_tag {
	std::nullptr_t emoji_name{ nullptr };
	std::nullptr_t emoji_id{ nullptr };
	std::string name{};
	bool moderated{};
	double id{};
};

struct default_reaction_emoji_data {
	std::optional<std::string> emoji_name{};
	std::nullptr_t emoji_id{};
};

struct icon_emoji_data {
	std::nullptr_t id{ nullptr };
	std::string name{};
};

struct permission_overwrite {
	std::string allow{};
	std::string deny{};
	int64_t type{};
	double id{};
};

struct channel_data {
	std::optional<default_reaction_emoji_data> default_reaction_emoji{};
	std::optional<std::vector<available_tag>> available_tags{};
	std::vector<permission_overwrite> permission_overwrites{};
	std::optional<std::string> last_pin_timestamp{};
	std::optional<std::string> channel_template{};
	std::optional<int64_t> default_forum_layout{};
	std::optional<int64_t> rate_limit_per_user{};
	std::optional<icon_emoji_data> icon_emoji{};
	std::nullptr_t voice_background_display{};
	std::optional<double> last_message_id{};
	std::nullptr_t default_sort_order{};
	std::optional<int64_t> user_limit{};
	std::optional<std::string> topic{};
	std::optional<double> parent_id{};
	std::optional<int64_t> bitrate{};
	std::nullptr_t theme_color{};
	std::nullptr_t rtc_region{};
	std::optional<bool> nsfw{};
	std::nullptr_t status{};
	std::string name{};
	int64_t position{};
	int64_t version{};
	int64_t flags{};
	int64_t type{};
	double id{};
};

struct emoji_element {
	std::vector<std::nullptr_t> roles{};
	bool require_colons{};
	std::string name{};
	int64_t version{};
	bool available{};
	bool animated{};
	bool managed{};
	double id{};
};

struct avatar_decoration_data_data {
	std::string asset{};
	double sku_id{};
};

struct user_data {
	std::optional<avatar_decoration_data_data> avatar_decoration_data{};
	std::optional<std::string> display_name{};
	std::optional<std::string> global_name{};
	std::optional<std::string> avatar{};
	std::nullptr_t clan{ nullptr };
	std::string discriminator{};
	std::string username{};
	int64_t public_flags{};
	double id{};
	bool bot{};
};

struct member {
	std::nullptr_t communication_disabled_until{ nullptr };
	std::nullptr_t premium_since{ nullptr };
	std::optional<std::string> nick{};
	std::nullptr_t avatar{ nullptr };
	std::vector<double> roles{};
	std::string joined_at{};
	user_data user{};
	int64_t flags{};
	bool pending{};
	bool deaf{};
	bool mute{};
};

struct assets_data {
	std::optional<std::string> large_image{};
	std::optional<std::string> small_image{};
	std::optional<std::string> large_text{};
};

struct activity_emoji {
	std::string name{};
	bool animated{};
	double id{};
};

struct party_data {
	std::string id{};
};

struct timestamps_data {
	std::optional<int64_t> end{};
	int64_t start{};
};

struct activity {
	std::optional<timestamps_data> timestamps{};
	std::optional<std::string> session_id{};
	std::optional<double> application_id{};
	std::optional<std::string> platform{};
	std::optional<activity_emoji> emoji{};
	std::optional<std::string> details{};
	std::optional<std::string> sync_id{};
	std::optional<assets_data> assets{};
	std::optional<std::string> state{};
	std::optional<party_data> party{};
	std::optional<int64_t> flags{};
	int64_t created_at{};
	std::string name{};
	std::string id{};
	int64_t type{};
};

struct client_status_data {
	std::optional<std::string> desktop{};
	std::optional<std::string> mobile{};
	std::optional<std::string> web{};
};

struct presence_user {
	double id{};
};

struct presence {
	std::nullptr_t broadcast{ nullptr };
	std::vector<activity> activities{};
	client_status_data client_status{};
	presence_user user{};
	status_data status{};
};

struct tags_data {
	std::nullptr_t premium_subscriber{ nullptr };
	std::optional<double> bot_id{};
};

struct role {
	std::nullptr_t unicode_emoji{ nullptr };
	std::nullptr_t icon{ nullptr };
	std::string permissions{};
	bool mentionable{};
	std::string name{};
	int64_t position{};
	int64_t version{};
	tags_data tags{};
	int64_t color{};
	int64_t flags{};
	bool managed{};
	bool hoist{};
	double id{};
};

struct soundboard_sound {
	std::optional<std::string> emoji_name{};
	std::optional<double> emoji_id{};
	std::string name{};
	double guild_id{};
	double sound_id{};
	bool available{};
	double user_id{};
	double volume{};
};

struct sticker {
	std::string description{};
	int64_t format_type{};
	std::string asset{};
	std::string name{};
	std::string tags{};
	double guild_id{};
	int64_t version{};
	bool available{};
	int64_t type{};
	double id{};
};

struct guild_data {
	std::nullptr_t communication_disabled_until{ nullptr };
	std::vector<std::nullptr_t> guild_scheduled_events{};
	activity_instances_data application_command_counts{};
	std::nullptr_t safety_alerts_channel_id{ nullptr };
	std::vector<std::nullptr_t> embedded_activities{};
	std::vector<std::nullptr_t> stage_instances{};
	std::nullptr_t inventory_settings{ nullptr };
	std::vector<std::nullptr_t> voice_states{};
	std::nullptr_t vanity_url_code{ nullptr };
	std::nullptr_t incidents_data{ nullptr };
	std::nullptr_t application_id{ nullptr };
	std::nullptr_t afk_channel_id{ nullptr };
	int64_t default_message_notifications{};
	int64_t max_stage_video_channel_users{};
	double latest_onboarding_question_id{};
	std::nullptr_t home_header{ nullptr };
	std::nullptr_t description{ nullptr };
	std::vector<std::nullptr_t> threads{};
	std::vector<channel_data> channels{};
	int64_t premium_subscription_count{};
	std::vector<emoji_element> emojis{};
	bool premium_progress_bar_enabled{};
	std::vector<std::string> features{};
	std::nullptr_t hub_type{ nullptr };
	double public_updates_channel_id{};
	int64_t explicit_content_filter{};
	int64_t max_video_channel_users{};
	std::nullptr_t banner{ nullptr };
	std::nullptr_t splash{ nullptr };
	std::vector<sticker> stickers{};
	int64_t system_channel_flags{};
	std::string preferred_locale{};
	std::nullptr_t clan{ nullptr };
	presence activity_instances{};
	std::vector<member> members{};
	int64_t verification_level{};
	double system_channel_id{};
	std::vector<role> roles{};
	int64_t premium_tier{};
	int64_t member_count{};
	int64_t max_members{};
	int64_t afk_timeout{};
	std::string region{};
	int64_t nsfw_level{};
	int64_t created_at{};
	int64_t mfa_level{};
	std::string name{};
	std::string icon{};
	bool unavailable{};
	double owner_id{};
	int64_t version{};
	bool large{};
	bool nsfw{};
	bool lazy{};
	double id{};
};

struct discord_message {
	std::string t{};
	guild_data d{};
	int64_t op{};
	int64_t s{};
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

template<> struct jsonifier::core<status_data> {
	using value_type				 = status_data;
	static constexpr auto parseValue = createValue<&value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id_str, &value_type::in_reply_to_screen_name,
		&value_type::in_reply_to_status_id, &value_type::in_reply_to_user_id, &value_type::possibly_sensitive, &value_type::contributors, &value_type::coordinates,
		&value_type::place, &value_type::geo, &value_type::entities, &value_type::favorite_count, &value_type::metadata, &value_type::created_at, &value_type::retweet_count,
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
		object("since_id_str", &value_type::since_id_str, "next_results", &value_type::next_results, "refresh_url", &value_type::refresh_url, "max_id_str", &value_type::max_id_str,
			"completed_in", &value_type::completed_in, "query", &value_type::query, "since_id", &value_type::since_id, "count", &value_type::count, "max_id", &value_type::max_id);
};

template<> struct glz::meta<hashtag> {
	using value_type			= hashtag;
	static constexpr auto value = object("indices", &value_type::indices, "text", &value_type::text);
};

template<> struct glz::meta<large_data> {
	using value_type			= large_data;
	static constexpr auto value = object("resize", &value_type::resize, "w", &value_type::w, "h", &value_type::h);
};

template<> struct glz::meta<sizes_data> {
	using value_type			= sizes_data;
	static constexpr auto value = object("medium", &value_type::medium, "small", &value_type::small, "thumb", &value_type::thumb, "large", &value_type::large);
};

template<> struct glz::meta<media_data> {
	using value_type			= media_data;
	static constexpr auto value = object("source_status_id_str", &value_type::source_status_id_str, "source_status_id", &value_type::source_status_id, "indices",
		&value_type::indices, "media_url_https", &value_type::media_url_https, "expanded_url", &value_type::expanded_url, "display_url", &value_type::display_url, "media_url",
		&value_type::media_url, "id_str", &value_type::id_str, "type", &value_type::type, "sizes", &value_type::sizes, "url", &value_type::url, "id", &value_type::id);
};

template<> struct glz::meta<url_data> {
	using value_type = url_data;
	static constexpr auto value =
		object("indices", &value_type::indices, "expanded_url", &value_type::expanded_url, "display_url", &value_type::display_url, "url", &value_type::url);
};

template<> struct glz::meta<user_mention> {
	using value_type = user_mention;
	static constexpr auto value =
		object("indices", &value_type::indices, "screen_name", &value_type::screen_name, "id_str", &value_type::id_str, "name", &value_type::name, "id", &value_type::id);
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
	static constexpr auto value = object("profile_background_image_url_https", &value_type::profile_background_image_url_https, "profile_banner_url",
		&value_type::profile_banner_url, "profile_background_image_url", &value_type::profile_background_image_url, "profile_sidebar_fill_color",
		&value_type::profile_sidebar_fill_color, "time_zone", &value_type::time_zone, "profile_background_color", &value_type::profile_background_color, "profile_image_url_https",
		&value_type::profile_image_url_https, "utc_offset", &value_type::utc_offset, "profile_use_background_image", &value_type::profile_use_background_image, "url",
		&value_type::url, "profile_text_color", &value_type::profile_text_color, "profile_link_color", &value_type::profile_link_color, "profile_image_url",
		&value_type::profile_image_url, "profile_background_tile", &value_type::profile_background_tile, "is_translation_enabled", &value_type::is_translation_enabled,
		"default_profile_image", &value_type::default_profile_image, "contributors_enabled", &value_type::contributors_enabled, "follow_request_sent",
		&value_type::follow_request_sent, "favourites_count", &value_type::favourites_count, "description", &value_type::description, "screen_name", &value_type::screen_name,
		"followers_count", &value_type::followers_count, "statuses_count", &value_type::statuses_count, "created_at", &value_type::created_at, "entities", &value_type::entities,
		"friends_count", &value_type::friends_count, "default_profile", &value_type::default_profile, "listed_count", &value_type::listed_count, "location", &value_type::location,
		"user_protected", &value_type::user_protected, "is_translator", &value_type::is_translator, "id_str", &value_type::id_str, "notifications", &value_type::notifications,
		"string", &value_type::string, "name", &value_type::name, "geo_enabled", &value_type::geo_enabled, "lang", &value_type::lang, "following", &value_type::following,
		"verified", &value_type::verified, "id", &value_type::id);
};

template<> struct glz::meta<status_data> {
	using value_type			= status_data;
	static constexpr auto value = object("in_reply_to_status_id_str", &value_type::in_reply_to_status_id_str, "in_reply_to_user_id_str", &value_type::in_reply_to_user_id_str,
		"in_reply_to_screen_name", &value_type::in_reply_to_screen_name, "in_reply_to_status_id", &value_type::in_reply_to_status_id, "in_reply_to_user_id",
		&value_type::in_reply_to_user_id, "possibly_sensitive", &value_type::possibly_sensitive, "contributors", &value_type::contributors, "coordinates", &value_type::coordinates,
		"place", &value_type::place, "geo", &value_type::geo, "entities", &value_type::entities, "favorite_count", &value_type::favorite_count, "metadata", &value_type::metadata,
		"created_at", &value_type::created_at, "retweet_count", &value_type::retweet_count, "source", &value_type::source, "id_str", &value_type::id_str, "user", &value_type::user,
		"lang", &value_type::lang, "text", &value_type::text, "truncated", &value_type::truncated, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "id",
		&value_type::id);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("search_metadata", &value_type::search_metadata, "statuses", &value_type::statuses);
};

#endif

template<> struct jsonifier::core<activity_instances_data> {
	using value_type				 = activity_instances_data;
	static constexpr auto parseValue = createValue<>();
};

template<> struct jsonifier::core<available_tag> {
	using value_type				 = available_tag;
	static constexpr auto parseValue = createValue<&value_type::emoji_name, &value_type::emoji_id, &value_type::name, &value_type::moderated, &value_type::id>();
};

template<> struct jsonifier::core<default_reaction_emoji_data> {
	using value_type				 = default_reaction_emoji_data;
	static constexpr auto parseValue = createValue<&value_type::emoji_name, &value_type::emoji_id>();
};

template<> struct jsonifier::core<icon_emoji_data> {
	using value_type				 = icon_emoji_data;
	static constexpr auto parseValue = createValue<&value_type::id, &value_type::name>();
};

template<> struct jsonifier::core<permission_overwrite> {
	using value_type				 = permission_overwrite;
	static constexpr auto parseValue = createValue<&value_type::allow, &value_type::deny, &value_type::type, &value_type::id>();
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue = createValue<&value_type::default_reaction_emoji, &value_type::available_tags, &value_type::permission_overwrites,
		&value_type::last_pin_timestamp, &value_type::channel_template, &value_type::default_forum_layout, &value_type::rate_limit_per_user, &value_type::icon_emoji,
		&value_type::voice_background_display, &value_type::last_message_id, &value_type::default_sort_order, &value_type::user_limit, &value_type::topic, &value_type::parent_id,
		&value_type::bitrate, &value_type::theme_color, &value_type::rtc_region, &value_type::nsfw, &value_type::status, &value_type::name, &value_type::position,
		&value_type::version, &value_type::flags, &value_type::type, &value_type::id>();
};

template<> struct jsonifier::core<emoji_element> {
	using value_type				 = emoji_element;
	static constexpr auto parseValue = createValue<&value_type::roles, &value_type::require_colons, &value_type::name, &value_type::version, &value_type::available,
		&value_type::animated, &value_type::managed, &value_type::id>();
};

template<> struct jsonifier::core<avatar_decoration_data_data> {
	using value_type				 = avatar_decoration_data_data;
	static constexpr auto parseValue = createValue<&value_type::asset, &value_type::sku_id>();
};

template<> struct jsonifier::core<user_data> {
	using value_type				 = user_data;
	static constexpr auto parseValue = createValue<&value_type::avatar_decoration_data, &value_type::display_name, &value_type::global_name, &value_type::avatar, &value_type::clan,
		&value_type::discriminator, &value_type::username, &value_type::public_flags, &value_type::id, &value_type::bot>();
};

template<> struct jsonifier::core<member> {
	using value_type				 = member;
	static constexpr auto parseValue = createValue<&value_type::communication_disabled_until, &value_type::premium_since, &value_type::nick, &value_type::avatar,
		&value_type::roles, &value_type::joined_at, &value_type::user, &value_type::flags, &value_type::pending, &value_type::deaf, &value_type::mute>();
};

template<> struct jsonifier::core<assets_data> {
	using value_type				 = assets_data;
	static constexpr auto parseValue = createValue<&value_type::large_image, &value_type::small_image, &value_type::large_text>();
};

template<> struct jsonifier::core<activity_emoji> {
	using value_type				 = activity_emoji;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::animated, &value_type::id>();
};

template<> struct jsonifier::core<party_data> {
	using value_type				 = party_data;
	static constexpr auto parseValue = createValue<&value_type::id>();
};

template<> struct jsonifier::core<timestamps_data> {
	using value_type				 = timestamps_data;
	static constexpr auto parseValue = createValue<&value_type::end, &value_type::start>();
};

template<> struct jsonifier::core<activity> {
	using value_type				 = activity;
	static constexpr auto parseValue = createValue<&value_type::timestamps, &value_type::session_id, &value_type::application_id, &value_type::platform, &value_type::emoji,
		&value_type::details, &value_type::sync_id, &value_type::assets, &value_type::state, &value_type::party, &value_type::flags, &value_type::created_at, &value_type::name,
		&value_type::id, &value_type::type>();
};

template<> struct jsonifier::core<client_status_data> {
	using value_type				 = client_status_data;
	static constexpr auto parseValue = createValue<&value_type::desktop, &value_type::mobile, &value_type::web>();
};

template<> struct jsonifier::core<presence_user> {
	using value_type				 = presence_user;
	static constexpr auto parseValue = createValue<&value_type::id>();
};

template<> struct jsonifier::core<presence> {
	using value_type				 = presence;
	static constexpr auto parseValue = createValue<&value_type::broadcast, &value_type::activities, &value_type::client_status, &value_type::user, &value_type::status>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<&value_type::premium_subscriber, &value_type::bot_id>();
};

template<> struct jsonifier::core<role> {
	using value_type				 = role;
	static constexpr auto parseValue = createValue<&value_type::unicode_emoji, &value_type::icon, &value_type::permissions, &value_type::mentionable, &value_type::name,
		&value_type::position, &value_type::version, &value_type::tags, &value_type::color, &value_type::flags, &value_type::managed, &value_type::hoist, &value_type::id>();
};

template<> struct jsonifier::core<soundboard_sound> {
	using value_type				 = soundboard_sound;
	static constexpr auto parseValue = createValue<&value_type::emoji_name, &value_type::emoji_id, &value_type::name, &value_type::guild_id, &value_type::sound_id,
		&value_type::available, &value_type::user_id, &value_type::volume>();
};

template<> struct jsonifier::core<sticker> {
	using value_type				 = sticker;
	static constexpr auto parseValue = createValue<&value_type::description, &value_type::format_type, &value_type::asset, &value_type::name, &value_type::tags,
		&value_type::guild_id, &value_type::version, &value_type::available, &value_type::type, &value_type::id>();
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue<&value_type::communication_disabled_until, &value_type::guild_scheduled_events, &value_type::application_command_counts,
		&value_type::safety_alerts_channel_id, &value_type::embedded_activities, &value_type::stage_instances, &value_type::inventory_settings, &value_type::voice_states,
		&value_type::vanity_url_code, &value_type::incidents_data, &value_type::application_id, &value_type::afk_channel_id, &value_type::default_message_notifications,
		&value_type::max_stage_video_channel_users, &value_type::latest_onboarding_question_id, &value_type::home_header, &value_type::description, &value_type::threads,
		&value_type::channels, &value_type::premium_subscription_count, &value_type::emojis, &value_type::premium_progress_bar_enabled, &value_type::features,
		&value_type::hub_type, &value_type::public_updates_channel_id, &value_type::explicit_content_filter, &value_type::max_video_channel_users, &value_type::banner,
		&value_type::splash, &value_type::stickers, &value_type::system_channel_flags, &value_type::preferred_locale, &value_type::clan, &value_type::activity_instances,
		&value_type::members, &value_type::verification_level, &value_type::system_channel_id, &value_type::roles, &value_type::premium_tier, &value_type::member_count,
		&value_type::max_members, &value_type::afk_timeout, &value_type::region, &value_type::nsfw_level, &value_type::created_at, &value_type::mfa_level, &value_type::name,
		&value_type::icon, &value_type::unavailable, &value_type::owner_id, &value_type::version, &value_type::large, &value_type::nsfw, &value_type::lazy, &value_type::id>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type				 = discord_message;
	static constexpr auto parseValue = createValue<&value_type::t, &value_type::d, &value_type::op, &value_type::s>();
};

#if !defined(ASAN)

template<> struct glz::meta<activity_instances_data> {
	using value_type			= activity_instances_data;
	static constexpr auto value = object();
};

template<> struct glz::meta<available_tag> {
	using value_type = available_tag;
	static constexpr auto value =
		object("emoji_name", &value_type::emoji_name, "emoji_id", &value_type::emoji_id, "name", &value_type::name, "moderated", &value_type::moderated, "id", &value_type::id);
};

template<> struct glz::meta<default_reaction_emoji_data> {
	using value_type			= default_reaction_emoji_data;
	static constexpr auto value = object("emoji_name", &value_type::emoji_name, "emoji_id", &value_type::emoji_id);
};

template<> struct glz::meta<icon_emoji_data> {
	using value_type			= icon_emoji_data;
	static constexpr auto value = object("id", &value_type::id, "name", &value_type::name);
};

template<> struct glz::meta<permission_overwrite> {
	using value_type			= permission_overwrite;
	static constexpr auto value = object("allow", &value_type::allow, "deny", &value_type::deny, "type", &value_type::type, "id", &value_type::id);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object("default_reaction_emoji", &value_type::default_reaction_emoji, "available_tags", &value_type::available_tags, "permission_overwrites",
		&value_type::permission_overwrites, "last_pin_timestamp", &value_type::last_pin_timestamp, "channel_template", &value_type::channel_template, "default_forum_layout",
		&value_type::default_forum_layout, "rate_limit_per_user", &value_type::rate_limit_per_user, "icon_emoji", &value_type::icon_emoji, "voice_background_display",
		&value_type::voice_background_display, "last_message_id", &value_type::last_message_id, "default_sort_order", &value_type::default_sort_order, "user_limit",
		&value_type::user_limit, "topic", &value_type::topic, "parent_id", &value_type::parent_id, "bitrate", &value_type::bitrate, "theme_color", &value_type::theme_color,
		"rtc_region", &value_type::rtc_region, "nsfw", &value_type::nsfw, "status", &value_type::status, "name", &value_type::name, "position", &value_type::position, "version",
		&value_type::version, "flags", &value_type::flags, "type", &value_type::type, "id", &value_type::id);
};

template<> struct glz::meta<emoji_element> {
	using value_type			= emoji_element;
	static constexpr auto value = object("roles", &value_type::roles, "require_colons", &value_type::require_colons, "name", &value_type::name, "version", &value_type::version,
		"available", &value_type::available, "animated", &value_type::animated, "managed", &value_type::managed, "id", &value_type::id);
};

template<> struct glz::meta<avatar_decoration_data_data> {
	using value_type			= avatar_decoration_data_data;
	static constexpr auto value = object("asset", &value_type::asset, "sku_id", &value_type::sku_id);
};

template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object("avatar_decoration_data", &value_type::avatar_decoration_data, "display_name", &value_type::display_name, "global_name",
		&value_type::global_name, "avatar", &value_type::avatar, "clan", &value_type::clan, "discriminator", &value_type::discriminator, "username", &value_type::username,
		"public_flags", &value_type::public_flags, "id", &value_type::id, "bot", &value_type::bot);
};

template<> struct glz::meta<member> {
	using value_type			= member;
	static constexpr auto value = object("communication_disabled_until", &value_type::communication_disabled_until, "premium_since", &value_type::premium_since, "nick",
		&value_type::nick, "avatar", &value_type::avatar, "roles", &value_type::roles, "joined_at", &value_type::joined_at, "user", &value_type::user, "flags", &value_type::flags,
		"pending", &value_type::pending, "deaf", &value_type::deaf, "mute", &value_type::mute);
};

template<> struct glz::meta<assets_data> {
	using value_type			= assets_data;
	static constexpr auto value = object("large_image", &value_type::large_image, "small_image", &value_type::small_image, "large_text", &value_type::large_text);
};

template<> struct glz::meta<activity_emoji> {
	using value_type			= activity_emoji;
	static constexpr auto value = object("name", &value_type::name, "animated", &value_type::animated, "id", &value_type::id);
};

template<> struct glz::meta<party_data> {
	using value_type			= party_data;
	static constexpr auto value = object("id", &value_type::id);
};

template<> struct glz::meta<timestamps_data> {
	using value_type			= timestamps_data;
	static constexpr auto value = object("end", &value_type::end, "start", &value_type::start);
};

template<> struct glz::meta<activity> {
	using value_type = activity;
	static constexpr auto value =
		object("timestamps", &value_type::timestamps, "session_id", &value_type::session_id, "application_id", &value_type::application_id, "platform", &value_type::platform,
			"emoji", &value_type::emoji, "details", &value_type::details, "sync_id", &value_type::sync_id, "assets", &value_type::assets, "state", &value_type::state, "party",
			&value_type::party, "flags", &value_type::flags, "created_at", &value_type::created_at, "name", &value_type::name, "id", &value_type::id, "type", &value_type::type);
};

template<> struct glz::meta<client_status_data> {
	using value_type			= client_status_data;
	static constexpr auto value = object("desktop", &value_type::desktop, "mobile", &value_type::mobile, "web", &value_type::web);
};

template<> struct glz::meta<presence_user> {
	using value_type			= presence_user;
	static constexpr auto value = object("id", &value_type::id);
};

template<> struct glz::meta<presence> {
	using value_type			= presence;
	static constexpr auto value = object("broadcast", &value_type::broadcast, "activities", &value_type::activities, "client_status", &value_type::client_status, "user",
		&value_type::user, "status", &value_type::status);
};

template<> struct glz::meta<tags_data> {
	using value_type			= tags_data;
	static constexpr auto value = object("premium_subscriber", &value_type::premium_subscriber, "bot_id", &value_type::bot_id);
};

template<> struct glz::meta<role> {
	using value_type			= role;
	static constexpr auto value = object("unicode_emoji", &value_type::unicode_emoji, "icon", &value_type::icon, "permissions", &value_type::permissions, "mentionable",
		&value_type::mentionable, "name", &value_type::name, "position", &value_type::position, "version", &value_type::version, "tags", &value_type::tags, "color",
		&value_type::color, "flags", &value_type::flags, "managed", &value_type::managed, "hoist", &value_type::hoist, "id", &value_type::id);
};

template<> struct glz::meta<soundboard_sound> {
	using value_type			= soundboard_sound;
	static constexpr auto value = object("emoji_name", &value_type::emoji_name, "emoji_id", &value_type::emoji_id, "name", &value_type::name, "guild_id", &value_type::guild_id,
		"sound_id", &value_type::sound_id, "available", &value_type::available, "user_id", &value_type::user_id, "volume", &value_type::volume);
};

template<> struct glz::meta<sticker> {
	using value_type = sticker;
	static constexpr auto value =
		object("description", &value_type::description, "format_type", &value_type::format_type, "asset", &value_type::asset, "name", &value_type::name, "tags", &value_type::tags,
			"guild_id", &value_type::guild_id, "version", &value_type::version, "available", &value_type::available, "type", &value_type::type, "id", &value_type::id);
};

template<> struct glz::meta<guild_data> {
	using value_type			= guild_data;
	static constexpr auto value = object("communication_disabled_until", &value_type::communication_disabled_until, "guild_scheduled_events", &value_type::guild_scheduled_events,
		"application_command_counts", &value_type::application_command_counts, "safety_alerts_channel_id", &value_type::safety_alerts_channel_id, "embedded_activities",
		&value_type::embedded_activities, "stage_instances", &value_type::stage_instances, "inventory_settings", &value_type::inventory_settings, "voice_states",
		&value_type::voice_states, "vanity_url_code", &value_type::vanity_url_code, "incidents_data", &value_type::incidents_data, "application_id", &value_type::application_id,
		"afk_channel_id", &value_type::afk_channel_id, "default_message_notifications", &value_type::default_message_notifications, "max_stage_video_channel_users",
		&value_type::max_stage_video_channel_users, "latest_onboarding_question_id", &value_type::latest_onboarding_question_id, "home_header", &value_type::home_header,
		"description", &value_type::description, "threads", &value_type::threads, "channels", &value_type::channels, "premium_subscription_count",
		&value_type::premium_subscription_count, "emojis", &value_type::emojis, "premium_progress_bar_enabled", &value_type::premium_progress_bar_enabled, "features",
		&value_type::features, "hub_type", &value_type::hub_type, "public_updates_channel_id", &value_type::public_updates_channel_id, "explicit_content_filter",
		&value_type::explicit_content_filter, "max_video_channel_users", &value_type::max_video_channel_users, "banner", &value_type::banner, "splash", &value_type::splash,
		"stickers", &value_type::stickers, "system_channel_flags", &value_type::system_channel_flags, "preferred_locale", &value_type::preferred_locale, "clan", &value_type::clan,
		"activity_instances", &value_type::activity_instances, "members", &value_type::members, "verification_level", &value_type::verification_level, "system_channel_id",
		&value_type::system_channel_id, "roles", &value_type::roles, "premium_tier", &value_type::premium_tier, "member_count", &value_type::member_count, "max_members",
		&value_type::max_members, "afk_timeout", &value_type::afk_timeout, "region", &value_type::region, "nsfw_level", &value_type::nsfw_level, "created_at",
		&value_type::created_at, "mfa_level", &value_type::mfa_level, "name", &value_type::name, "icon", &value_type::icon, "unavailable", &value_type::unavailable, "owner_id",
		&value_type::owner_id, "version", &value_type::version, "large", &value_type::large, "nsfw", &value_type::nsfw, "lazy", &value_type::lazy, "id", &value_type::id);
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

template<typename value_type> struct test {
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

template<typename value_type> struct abc_test {
	std::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue<&value_type::testStrings, &value_type::testUints, &value_type::testDoubles, &value_type::testInts, &value_type::testBools>();
};

template<> struct jsonifier::core<test<test_struct>> {
	using value_type				 = test<test_struct>;
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

template<> struct jsonifier::core<abc_test<test_struct>> {
	using value_type				 = abc_test<test_struct>;
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

template<> struct glz::meta<test<test_struct>> {
	using value_type			= test<test_struct>;
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

template<> struct glz::meta<abc_test<test_struct>> {
	using value_type			= abc_test<test_struct>;
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
			readTime   = "N/A";
			read	   = "N/A";
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
			write		= "N/A";
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
	parse_and_serialize = 0,
	minify				= 1,
	prettify			= 2,
	validate			= 3,
};

enum class json_library {
	jsonifier = 0,
	glaze	  = 1,
	simdjson  = 2,
};

static std::string basePath{ JSON_PATH };

template<json_library lib, test_type type, typename test_data_type, bool minified, uint64_t iterations> struct json_test_helper;

template<typename test_data_type, bool minified, uint64_t iterations>
struct json_test_helper<json_library::jsonifier, test_type::parse_and_serialize, test_data_type, minified, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

#if !defined(ASAN)
template<typename test_data_type, bool minified, uint64_t iterations>
struct json_test_helper<json_library::glaze, test_type::parse_and_serialize, test_data_type, minified, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

	#include "simdjson.h"

template<typename value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	value.get(valueNew);
}

template<jsonifier::concepts::bool_t value_type> void getValue(value_type&& valueNew, simdjson::ondemand::value value) {
	getValue<bool>(valueNew, value);
}

template<> void getValue(std::nullptr_t& valueNew, simdjson::ondemand::value value) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	auto oldSize = valueNew.size();
	if (auto resultCode = value.get(result); !resultCode) {
		auto iter = result.begin();
		for (uint64_t x = 0; x < oldSize && iter != result.end(); ++x, ++iter) {
			getValue(valueNew[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(valueNew.emplace_back(), iter.operator*().value());
		}
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
}

template<> void getValue<std::string>(std::string& valueNew, simdjson::ondemand::value value) {
	std::string_view result;
	if (auto resultCode = value.get(result); !resultCode) {
		valueNew = static_cast<std::string>(result);
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value valueNewer) {
	simdjson::ondemand::value result;
	if (auto resultCode = valueNewer.get(result); !resultCode) {
		getValue(valueNew.emplace(), result);
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		if (result.type() != simdjson::ondemand::json_type::null) {
			getValue(returnValue.emplace(), result);
		}
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
}

template<typename value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		getValue(returnValue, result);
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
}

template<> void getValue(std::nullptr_t& returnValue, simdjson::ondemand::value value, const std::string& key) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::array result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (uint64_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
			getValue(returnValues[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(returnValues.emplace_back(), iter.operator*().value());
		}
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
	}
	return;
}

template<jsonifier::concepts::map_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::object result;
	if (auto resultCode = value[key].get(result); !resultCode) {
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
	} else {
		std::stringstream newStream{};
		newStream << resultCode;
		throw std::runtime_error{ "Sorry, but failed to parse due to: " + newStream.str() };
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

template<> void getValue(search_metadata_data& returnValue, simdjson::ondemand::value jsonDataNew) {
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

template<> void getValue(hashtag& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.text, jsonDataNew, "text");
}

template<> void getValue(large_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.resize, jsonDataNew, "resize");
	getValue(returnValue.w, jsonDataNew, "w");
	getValue(returnValue.h, jsonDataNew, "h");
}

template<> void getValue(sizes_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.medium, jsonDataNew, "medium");
	getValue(returnValue.small, jsonDataNew, "small");
	getValue(returnValue.thumb, jsonDataNew, "thumb");
	getValue(returnValue.large, jsonDataNew, "large");
}

template<> void getValue(media_data& returnValue, simdjson::ondemand::value jsonDataNew) {
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

template<> void getValue(url_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.expanded_url, jsonDataNew, "expanded_url");
	getValue(returnValue.display_url, jsonDataNew, "display_url");
	getValue(returnValue.url, jsonDataNew, "url");
}

template<> void getValue(user_mention& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.indices, jsonDataNew, "indices");
	getValue(returnValue.screen_name, jsonDataNew, "screen_name");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(status_entities& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.media, jsonDataNew, "media");
	getValue(returnValue.user_mentions, jsonDataNew, "user_mentions");
	getValue(returnValue.symbols, jsonDataNew, "symbols");
	getValue(returnValue.hashtags, jsonDataNew, "hashtags");
	getValue(returnValue.urls, jsonDataNew, "urls");
}

template<> void getValue(metadata_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.iso_language_code, jsonDataNew, "iso_language_code");
	getValue(returnValue.result_type, jsonDataNew, "result_type");
}

template<> void getValue(description_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.urls, jsonDataNew, "urls");
}

template<> void getValue(user_entities& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.url, jsonDataNew, "url");
	getValue(returnValue.description, jsonDataNew, "description");
}

template<> void getValue(twitter_user& returnValue, simdjson::ondemand::value jsonDataNew) {
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

template<> void getValue(status_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.in_reply_to_status_id_str, jsonDataNew, "in_reply_to_status_id_str");
	getValue(returnValue.in_reply_to_user_id_str, jsonDataNew, "in_reply_to_user_id_str");
	getValue(returnValue.in_reply_to_screen_name, jsonDataNew, "in_reply_to_screen_name");
	getValue(returnValue.in_reply_to_status_id, jsonDataNew, "in_reply_to_status_id");
	getValue(returnValue.in_reply_to_user_id, jsonDataNew, "in_reply_to_user_id");
	getValue(returnValue.possibly_sensitive, jsonDataNew, "possibly_sensitive");
	getValue(returnValue.contributors, jsonDataNew, "contributors");
	getValue(returnValue.coordinates, jsonDataNew, "coordinates");
	getValue(returnValue.place, jsonDataNew, "place");
	getValue(returnValue.geo, jsonDataNew, "geo");
	getValue(returnValue.entities, jsonDataNew, "entities");
	getValue(returnValue.favorite_count, jsonDataNew, "favorite_count");
	getValue(returnValue.metadata, jsonDataNew, "metadata");
	getValue(returnValue.created_at, jsonDataNew, "created_at");
	getValue(returnValue.retweet_count, jsonDataNew, "retweet_count");
	getValue(returnValue.source, jsonDataNew, "source");
	getValue(returnValue.id_str, jsonDataNew, "id_str");
	getValue(returnValue.user, jsonDataNew, "user");
	getValue(returnValue.lang, jsonDataNew, "lang");
	getValue(returnValue.text, jsonDataNew, "text");
	getValue(returnValue.truncated, jsonDataNew, "truncated");
	getValue(returnValue.favorited, jsonDataNew, "favorited");
	getValue(returnValue.retweeted, jsonDataNew, "retweeted");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(twitter_message& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.search_metadata, jsonDataNew, "search_metadata");
	getValue(returnValue.statuses, jsonDataNew, "statuses");
}

template<> void getValue(activity_instances_data& returnValue, simdjson::ondemand::value jsonDataNew) {
}

template<> void getValue(available_tag& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.emoji_name, jsonDataNew, "emoji_name");
	getValue(returnValue.emoji_id, jsonDataNew, "emoji_id");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.moderated, jsonDataNew, "moderated");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(default_reaction_emoji_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.emoji_name, jsonDataNew, "emoji_name");
	getValue(returnValue.emoji_id, jsonDataNew, "emoji_id");
}

template<> void getValue(icon_emoji_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.name, jsonDataNew, "name");
}

template<> void getValue(permission_overwrite& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.allow, jsonDataNew, "allow");
	getValue(returnValue.deny, jsonDataNew, "deny");
	getValue(returnValue.type, jsonDataNew, "type");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(channel_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.default_reaction_emoji, jsonDataNew, "default_reaction_emoji");
	getValue(returnValue.available_tags, jsonDataNew, "available_tags");
	getValue(returnValue.permission_overwrites, jsonDataNew, "permission_overwrites");
	getValue(returnValue.last_pin_timestamp, jsonDataNew, "last_pin_timestamp");
	getValue(returnValue.channel_template, jsonDataNew, "channel_template");
	getValue(returnValue.default_forum_layout, jsonDataNew, "default_forum_layout");
	getValue(returnValue.rate_limit_per_user, jsonDataNew, "rate_limit_per_user");
	getValue(returnValue.icon_emoji, jsonDataNew, "icon_emoji");
	getValue(returnValue.voice_background_display, jsonDataNew, "voice_background_display");
	getValue(returnValue.last_message_id, jsonDataNew, "last_message_id");
	getValue(returnValue.default_sort_order, jsonDataNew, "default_sort_order");
	getValue(returnValue.user_limit, jsonDataNew, "user_limit");
	getValue(returnValue.topic, jsonDataNew, "topic");
	getValue(returnValue.parent_id, jsonDataNew, "parent_id");
	getValue(returnValue.bitrate, jsonDataNew, "bitrate");
	getValue(returnValue.theme_color, jsonDataNew, "theme_color");
	getValue(returnValue.rtc_region, jsonDataNew, "rtc_region");
	getValue(returnValue.nsfw, jsonDataNew, "nsfw");
	getValue(returnValue.status, jsonDataNew, "status");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.position, jsonDataNew, "position");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.type, jsonDataNew, "type");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(emoji_element& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.roles, jsonDataNew, "roles");
	getValue(returnValue.require_colons, jsonDataNew, "require_colons");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.available, jsonDataNew, "available");
	getValue(returnValue.animated, jsonDataNew, "animated");
	getValue(returnValue.managed, jsonDataNew, "managed");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(avatar_decoration_data_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.asset, jsonDataNew, "asset");
	getValue(returnValue.sku_id, jsonDataNew, "sku_id");
}

template<> void getValue(user_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.avatar_decoration_data, jsonDataNew, "avatar_decoration_data");
	getValue(returnValue.display_name, jsonDataNew, "display_name");
	getValue(returnValue.global_name, jsonDataNew, "global_name");
	getValue(returnValue.avatar, jsonDataNew, "avatar");
	getValue(returnValue.clan, jsonDataNew, "clan");
	getValue(returnValue.discriminator, jsonDataNew, "discriminator");
	getValue(returnValue.username, jsonDataNew, "username");
	getValue(returnValue.public_flags, jsonDataNew, "public_flags");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.bot, jsonDataNew, "bot");
}

template<> void getValue(member& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.communication_disabled_until, jsonDataNew, "communication_disabled_until");
	getValue(returnValue.premium_since, jsonDataNew, "premium_since");
	getValue(returnValue.nick, jsonDataNew, "nick");
	getValue(returnValue.avatar, jsonDataNew, "avatar");
	getValue(returnValue.roles, jsonDataNew, "roles");
	getValue(returnValue.joined_at, jsonDataNew, "joined_at");
	getValue(returnValue.user, jsonDataNew, "user");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.pending, jsonDataNew, "pending");
	getValue(returnValue.deaf, jsonDataNew, "deaf");
	getValue(returnValue.mute, jsonDataNew, "mute");
}

template<> void getValue(assets_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.large_image, jsonDataNew, "large_image");
	getValue(returnValue.small_image, jsonDataNew, "small_image");
	getValue(returnValue.large_text, jsonDataNew, "large_text");
}

template<> void getValue(activity_emoji& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.animated, jsonDataNew, "animated");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(party_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(timestamps_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.end, jsonDataNew, "end");
	getValue(returnValue.start, jsonDataNew, "start");
}

template<> void getValue(activity& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.timestamps, jsonDataNew, "timestamps");
	getValue(returnValue.session_id, jsonDataNew, "session_id");
	getValue(returnValue.application_id, jsonDataNew, "application_id");
	getValue(returnValue.platform, jsonDataNew, "platform");
	getValue(returnValue.emoji, jsonDataNew, "emoji");
	getValue(returnValue.details, jsonDataNew, "details");
	getValue(returnValue.sync_id, jsonDataNew, "sync_id");
	getValue(returnValue.assets, jsonDataNew, "assets");
	getValue(returnValue.state, jsonDataNew, "state");
	getValue(returnValue.party, jsonDataNew, "party");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.created_at, jsonDataNew, "created_at");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.id, jsonDataNew, "id");
	getValue(returnValue.type, jsonDataNew, "type");
}

template<> void getValue(client_status_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.desktop, jsonDataNew, "desktop");
	getValue(returnValue.mobile, jsonDataNew, "mobile");
	getValue(returnValue.web, jsonDataNew, "web");
}

template<> void getValue(presence_user& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(presence& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.broadcast, jsonDataNew, "broadcast");
	getValue(returnValue.activities, jsonDataNew, "activities");
	getValue(returnValue.client_status, jsonDataNew, "client_status");
	getValue(returnValue.user, jsonDataNew, "user");
	getValue(returnValue.status, jsonDataNew, "status");
}

template<> void getValue(tags_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.premium_subscriber, jsonDataNew, "premium_subscriber");
	getValue(returnValue.bot_id, jsonDataNew, "bot_id");
}

template<> void getValue(role& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.unicode_emoji, jsonDataNew, "unicode_emoji");
	getValue(returnValue.icon, jsonDataNew, "icon");
	getValue(returnValue.permissions, jsonDataNew, "permissions");
	getValue(returnValue.mentionable, jsonDataNew, "mentionable");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.position, jsonDataNew, "position");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.tags, jsonDataNew, "tags");
	getValue(returnValue.color, jsonDataNew, "color");
	getValue(returnValue.flags, jsonDataNew, "flags");
	getValue(returnValue.managed, jsonDataNew, "managed");
	getValue(returnValue.hoist, jsonDataNew, "hoist");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(soundboard_sound& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.emoji_name, jsonDataNew, "emoji_name");
	getValue(returnValue.emoji_id, jsonDataNew, "emoji_id");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.guild_id, jsonDataNew, "guild_id");
	getValue(returnValue.sound_id, jsonDataNew, "sound_id");
	getValue(returnValue.available, jsonDataNew, "available");
	getValue(returnValue.user_id, jsonDataNew, "user_id");
	getValue(returnValue.volume, jsonDataNew, "volume");
}

template<> void getValue(sticker& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.description, jsonDataNew, "description");
	getValue(returnValue.format_type, jsonDataNew, "format_type");
	getValue(returnValue.asset, jsonDataNew, "asset");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.tags, jsonDataNew, "tags");
	getValue(returnValue.guild_id, jsonDataNew, "guild_id");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.available, jsonDataNew, "available");
	getValue(returnValue.type, jsonDataNew, "type");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(guild_data& returnValue, simdjson::ondemand::value jsonDataNew) {
	getValue(returnValue.communication_disabled_until, jsonDataNew, "communication_disabled_until");
	getValue(returnValue.guild_scheduled_events, jsonDataNew, "guild_scheduled_events");
	getValue(returnValue.application_command_counts, jsonDataNew, "application_command_counts");
	getValue(returnValue.safety_alerts_channel_id, jsonDataNew, "safety_alerts_channel_id");
	getValue(returnValue.embedded_activities, jsonDataNew, "embedded_activities");
	getValue(returnValue.stage_instances, jsonDataNew, "stage_instances");
	getValue(returnValue.inventory_settings, jsonDataNew, "inventory_settings");
	getValue(returnValue.voice_states, jsonDataNew, "voice_states");
	getValue(returnValue.vanity_url_code, jsonDataNew, "vanity_url_code");
	getValue(returnValue.incidents_data, jsonDataNew, "incidents_data");
	getValue(returnValue.application_id, jsonDataNew, "application_id");
	getValue(returnValue.afk_channel_id, jsonDataNew, "afk_channel_id");
	getValue(returnValue.default_message_notifications, jsonDataNew, "default_message_notifications");
	getValue(returnValue.max_stage_video_channel_users, jsonDataNew, "max_stage_video_channel_users");
	getValue(returnValue.latest_onboarding_question_id, jsonDataNew, "latest_onboarding_question_id");
	getValue(returnValue.home_header, jsonDataNew, "home_header");
	getValue(returnValue.description, jsonDataNew, "description");
	getValue(returnValue.threads, jsonDataNew, "threads");
	getValue(returnValue.channels, jsonDataNew, "channels");
	getValue(returnValue.premium_subscription_count, jsonDataNew, "premium_subscription_count");
	getValue(returnValue.emojis, jsonDataNew, "emojis");
	getValue(returnValue.premium_progress_bar_enabled, jsonDataNew, "premium_progress_bar_enabled");
	getValue(returnValue.features, jsonDataNew, "features");
	getValue(returnValue.hub_type, jsonDataNew, "hub_type");
	getValue(returnValue.public_updates_channel_id, jsonDataNew, "public_updates_channel_id");
	getValue(returnValue.explicit_content_filter, jsonDataNew, "explicit_content_filter");
	getValue(returnValue.max_video_channel_users, jsonDataNew, "max_video_channel_users");
	getValue(returnValue.banner, jsonDataNew, "banner");
	getValue(returnValue.splash, jsonDataNew, "splash");
	getValue(returnValue.stickers, jsonDataNew, "stickers");
	getValue(returnValue.system_channel_flags, jsonDataNew, "system_channel_flags");
	getValue(returnValue.preferred_locale, jsonDataNew, "preferred_locale");
	getValue(returnValue.clan, jsonDataNew, "clan");
	getValue(returnValue.activity_instances, jsonDataNew, "activity_instances");
	getValue(returnValue.members, jsonDataNew, "members");
	getValue(returnValue.verification_level, jsonDataNew, "verification_level");
	getValue(returnValue.system_channel_id, jsonDataNew, "system_channel_id");
	getValue(returnValue.roles, jsonDataNew, "roles");
	getValue(returnValue.premium_tier, jsonDataNew, "premium_tier");
	getValue(returnValue.member_count, jsonDataNew, "member_count");
	getValue(returnValue.max_members, jsonDataNew, "max_members");
	getValue(returnValue.afk_timeout, jsonDataNew, "afk_timeout");
	getValue(returnValue.region, jsonDataNew, "region");
	getValue(returnValue.nsfw_level, jsonDataNew, "nsfw_level");
	getValue(returnValue.created_at, jsonDataNew, "created_at");
	getValue(returnValue.mfa_level, jsonDataNew, "mfa_level");
	getValue(returnValue.name, jsonDataNew, "name");
	getValue(returnValue.icon, jsonDataNew, "icon");
	getValue(returnValue.unavailable, jsonDataNew, "unavailable");
	getValue(returnValue.owner_id, jsonDataNew, "owner_id");
	getValue(returnValue.version, jsonDataNew, "version");
	getValue(returnValue.large, jsonDataNew, "large");
	getValue(returnValue.nsfw, jsonDataNew, "nsfw");
	getValue(returnValue.lazy, jsonDataNew, "lazy");
	getValue(returnValue.id, jsonDataNew, "id");
}

template<> void getValue(discord_message& returnValue, simdjson::ondemand::value jsonDataNew) {
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


template<> void getValue<test<test_struct>>(test<test_struct>& obj, simdjson::ondemand::value doc) {
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

template<> void getValue<abc_test<test_struct>>(abc_test<test_struct>& obj, simdjson::ondemand::value doc) {
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

template<typename test_data_type, bool minified, uint64_t iterations>
struct json_test_helper<json_library::simdjson, test_type::parse_and_serialize, test_data_type, minified, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};

template<uint64_t iterations> struct json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations> {
	static auto run(const std::string& newBuffer, const std::string& testName, bool doWePrint = true) {
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
};
#endif

std::string table_header = R"(
| Library | Read (MB/s) | ReadLength (Bytes) | ReadTime (ns) | Write (MB/s) | WriteLength (Bytes) | WriteTime (ns)
| ------------------------------------------------- | ---------- | ----------- | ---------- | ----------- | ---------- | ---------- |)";

template<test_type type, typename test_data_type, bool minified, uint64_t iterations> struct json_tests_helper;

template<test_type type, typename test_data_type, bool minified, uint64_t iterations> struct json_tests_helper {
	static test_results run(const std::string& jsonDataNew, const std::string& testName) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = testName;
#if !defined(ASAN)
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::simdjson, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, true));
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::glaze, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::glaze, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, true));
#endif
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::jsonifier, type, test_data_type, minified, iterations>::run(jsonDataNew, jsonResults.testName, true));

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
};

template<uint64_t iterations> struct json_tests_helper<test_type::prettify, std::string, false, iterations> {
	static test_results run(const std::string& jsonDataNew, const std::string& testName) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = testName;
#if !defined(ASAN)
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::glaze, test_type::prettify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));
#endif
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::jsonifier, test_type::prettify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));

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
};

template<uint64_t iterations> struct json_tests_helper<test_type::minify, std::string, false, iterations> {
	static test_results run(const std::string& jsonDataNew, const std::string& testName) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = testName;
#if !defined(ASAN)
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::simdjson, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::glaze, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));
#endif
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::jsonifier, test_type::minify, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));

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
};

template<uint64_t iterations> struct json_tests_helper<test_type::validate, std::string, false, iterations> {
	static test_results run(const std::string& jsonDataNew, const std::string& testName) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = testName;
		for (uint32_t y = 0; y < 2; ++y) {
			json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, false);
		}
		resultsNew.emplace_back(json_test_helper<json_library::jsonifier, test_type::validate, std::string, false, iterations>::run(jsonDataNew, jsonResults.testName, true));

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
};

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
		auto testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, false, iterationsVal>::run(jsonDataNew, "Json Test (Prettified)");
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, test<test_struct>, true, iterationsVal>::run(jsonMinifiedData, "Json Test (Minified)");
		newerString += section02;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<test_struct>, false, iterationsVal>::run(jsonDataNew, "Abc Test (Prettified)");
		newerString += section03;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, abc_test<test_struct>, true, iterationsVal>::run(jsonMinifiedData, "Abc Test (Minified)");
		newerString += section04;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, false, iterationsVal>::run(discordData, "Discord Test (Prettified)");
		newerString += section05;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, discord_message, true, iterationsVal>::run(discordMinifiedData, "Discord Test (Minified)");
		newerString += section06;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, false, iterationsVal>::run(canadaData, "Canada Test (Prettified)");
		newerString += section07;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, canada_message, true, iterationsVal>::run(canadaMinifiedData, "Canada Test (Minified)");
		newerString += section08;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, false, iterationsVal>::run(twitterData, "Twitter Test (Prettified)");
		newerString += section09;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::parse_and_serialize, twitter_message, true, iterationsVal>::run(twitterMinifiedData, "Twitter Test (Minified)");
		newerString += section10;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::minify, std::string, false, iterationsVal>::run(discordData, "Minify Test");
		newerString += section11;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::prettify, std::string, false, iterationsVal>::run(jsonMinifiedData, "Prettify Test");
		newerString += section12;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::validate, std::string, false, iterationsVal>::run(discordData, "Validate Test");
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