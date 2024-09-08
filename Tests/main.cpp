#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX2 | JSONIFIER_POPCNT)
#endif
#include "UnicodeEmoji.hpp"
#include <BnchSwt/BenchmarkSuite.hpp>
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

constexpr jsonifier_internal::string_literal jsonifierLibraryName{ "jsonifier" };
constexpr jsonifier_internal::string_literal simdjsonLibraryName{ "simdjson" };
constexpr jsonifier_internal::string_literal glazeLibraryName{ "glaze" };
constexpr jsonifier_internal::string_literal jsonifierCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/realtimechris/jsonifier/commit/" } + JSONIFIER_COMMIT };
constexpr jsonifier_internal::string_literal glazeCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/stephenberry/glaze/commit/" } + GLAZE_COMMIT };
constexpr jsonifier_internal::string_literal simdjsonCommitUrl{ jsonifier_internal::string_literal{ "https://github.com/simdjson/simdjson/commit/" } + SIMDJSON_COMMIT };

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
	std::string profile_sidebar_border_color{};
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
	bool protectedVal{};
	std::string name{};
	bool geo_enabled{};
	std::string lang{};
	bool following{};
	bool verified{};
	int64_t id{};
};

struct retweeted_status_data {
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

struct status_data {
	std::optional<retweeted_status_data> retweeted_status{};
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

struct icon_emoji_data {
	std::optional<std::string> name{};
	std::nullptr_t id{ nullptr };
};

struct permission_overwrite {
	std::string allow{};
	std::string deny{};
	std::string id{};
	int64_t type{};
};

struct channel_data {
	std::vector<permission_overwrite> permission_overwrites{};
	std::optional<std::string> last_message_id{};
	int64_t default_thread_rate_limit_per_user{};
	std::vector<std::nullptr_t> applied_tags{};
	std::vector<std::nullptr_t> recipients{};
	int64_t default_auto_archive_duration{};
	std::nullptr_t status{ nullptr };
	std::string last_pin_timestamp{};
	std::nullptr_t topic{ nullptr };
	int64_t rate_limit_per_user{};
	icon_emoji_data icon_emoji{};
	int64_t total_message_sent{};
	int64_t video_quality_mode{};
	std::string application_id{};
	std::string permissions{};
	int64_t message_count{};
	std::string parent_id{};
	int64_t member_count{};
	std::string owner_id{};
	std::string guild_id{};
	int64_t user_limit{};
	int64_t position{};
	std::string name{};
	std::string icon{};
	int64_t version{};
	int64_t bitrate{};
	std::string id{};
	int64_t flags{};
	int64_t type{};
	bool managed{};
	bool nsfw{};
};

struct user_data {
	std::nullptr_t avatar_decoration_data{ nullptr };
	std::optional<std::string> display_name{};
	std::optional<std::string> global_name{};
	std::optional<std::string> avatar{};
	std::nullptr_t banner{ nullptr };
	std::nullptr_t locale{ nullptr };
	std::string discriminator{};
	std::string user_name{};
	int64_t accent_color{};
	int64_t premium_type{};
	int64_t public_flags{};
	std::string email{};
	bool mfa_enabled{};
	std::string id{};
	int64_t flags{};
	bool verified{};
	bool system{};
	bool bot{};
};

struct member_data {
	std::nullptr_t communication_disabled_until{ nullptr };
	std::nullptr_t premium_since{ nullptr };
	std::optional<std::string> nick{};
	std::nullptr_t avatar{ nullptr };
	std::vector<std::string> roles{};
	std::string permissions{};
	std::string joined_at{};
	std::string guild_id{};
	user_data user{};
	int64_t flags{};
	bool pending{};
	bool deaf{};
	bool mute{};
};

struct tags_data {
	std::nullptr_t premium_subscriber{ nullptr };
	std::optional<std::string> bot_id{};
};

struct role_data {
	std::nullptr_t unicode_emoji{ nullptr };
	std::nullptr_t icon{ nullptr };
	std::string permissions{};
	int64_t position{};
	std::string name{};
	bool mentionable{};
	int64_t version{};
	std::string id{};
	tags_data tags{};
	int64_t color{};
	int64_t flags{};
	bool managed{};
	bool hoist{};
};

struct guild_data {
	std::nullptr_t latest_on_boarding_question_id{ nullptr };
	std::vector<std::nullptr_t> guild_scheduled_events{};
	std::nullptr_t safety_alerts_channel_id{ nullptr };
	std::nullptr_t inventory_settings{ nullptr };
	std::vector<std::nullptr_t> voice_states{};
	std::nullptr_t discovery_splash{ nullptr };
	std::nullptr_t vanity_url_code{ nullptr };
	std::nullptr_t application_id{ nullptr };
	std::nullptr_t afk_channel_id{ nullptr };
	int64_t default_message_notifications{};
	int64_t max_stage_video_channel_users{};
	std::string public_updates_channel_id{};
	std::nullptr_t description{ nullptr };
	std::vector<std::nullptr_t> threads{};
	std::vector<channel_data> channels{};
	int64_t premium_subscription_count{};
	int64_t approximate_presence_count{};
	std::vector<std::string> features{};
	std::vector<std::string> stickers{};
	bool premium_progress_bar_enabled{};
	std::vector<member_data> members{};
	std::nullptr_t hub_type{ nullptr };
	int64_t approximate_member_count{};
	int64_t explicit_content_filter{};
	int64_t max_video_channel_users{};
	std::nullptr_t splash{ nullptr };
	std::nullptr_t banner{ nullptr };
	std::string system_channel_id{};
	std::string widget_channel_id{};
	std::string preferred_locale{};
	int64_t system_channel_flags{};
	std::string rules_channel_id{};
	std::vector<role_data> roles{};
	int64_t verification_level{};
	std::string permissions{};
	int64_t max_presences{};
	std::string discovery{};
	std::string joined_at{};
	int64_t member_count{};
	int64_t premium_tier{};
	std::string owner_id{};
	int64_t max_members{};
	int64_t afk_timeout{};
	bool widget_enabled{};
	std::string region{};
	int64_t nsfw_level{};
	int64_t mfa_level{};
	std::string name{};
	std::string icon{};
	bool unavailable{};
	std::string id{};
	int64_t flags{};
	bool large{};
	bool owner{};
	bool nsfw{};
	bool lazy{};
};

struct discord_message {
	std::string t{};
	guild_data d{};
	int64_t op{};
	int64_t s{};
};

struct names {};

struct event {
	std::optional<std::string> description{};
	std::vector<int64_t> subTopicIds{};
	std::optional<std::string> logo{};
	std::vector<int64_t> topicIds{};
	std::nullptr_t subjectCode{};
	std::nullptr_t subtitle{};
	std::string name{};
	int64_t id{};
};

struct price {
	int64_t audienceSubCategoryId{};
	int64_t seatCategoryId{};
	int64_t amount{};
};

struct area {
	std::vector<std::nullptr_t> blockIds{};
	int64_t areaId{};
};

struct seat_category {
	std::vector<area> areas{};
	int64_t seatCategoryId{};
};

struct performance {
	std::vector<seat_category> seatCategories{};
	std::optional<std::string> logo{};
	std::nullptr_t seatMapImage{};
	std::vector<price> prices{};
	std::string venueCode{};
	std::nullptr_t name{};
	uint64_t eventId{};
	int64_t start{};
	uint64_t id{};
};

struct venue_names {
	std::string PLEYEL_PLEYEL{};
};

struct citm_catalog_message {
	std::map<std::string, std::string> audienceSubCategoryNames{};
	std::map<std::string, std::vector<uint64_t>> topicSubTopics{};
	std::map<std::string, std::string> seatCategoryNames{};
	std::map<std::string, std::string> subTopicNames{};
	std::map<std::string, std::string> areaNames{};
	std::map<std::string, std::string> topicNames{};
	std::vector<performance> performances{};
	std::map<std::string, event> events{};
	venue_names venueNames{};
	names subjectNames{};
	names blockNames{};
};

template<> struct jsonifier::core<names> {
	using value_type				 = names;
	static constexpr auto parseValue = createValue();
};

template<> struct jsonifier::core<event> {
	using value_type				 = event;
	static constexpr auto parseValue = createValue<&value_type::description, &value_type::subTopicIds, &value_type::logo, &value_type::topicIds, &value_type::subjectCode,
		&value_type::subtitle, &value_type::name, &value_type::id>();
};

template<> struct jsonifier::core<price> {
	using value_type				 = price;
	static constexpr auto parseValue = createValue<&value_type::audienceSubCategoryId, &value_type::seatCategoryId, &value_type::amount>();
};

template<> struct jsonifier::core<area> {
	using value_type				 = area;
	static constexpr auto parseValue = createValue<&value_type::blockIds, &value_type::areaId>();
};

template<> struct jsonifier::core<seat_category> {
	using value_type				 = seat_category;
	static constexpr auto parseValue = createValue<&value_type::areas, &value_type::seatCategoryId>();
};

template<> struct jsonifier::core<performance> {
	using value_type				 = performance;
	static constexpr auto parseValue = createValue<&value_type::seatCategories, &value_type::logo, &value_type::seatMapImage, &value_type::prices, &value_type::venueCode,
		&value_type::name, &value_type::eventId, &value_type::start, &value_type::id>();
};

template<> struct jsonifier::core<venue_names> {
	using value_type				 = venue_names;
	static constexpr auto parseValue = createValue<&value_type::PLEYEL_PLEYEL>();
};

template<> struct jsonifier::core<citm_catalog_message> {
	using value_type = citm_catalog_message;
	static constexpr auto parseValue =
		createValue<&value_type::audienceSubCategoryNames, &value_type::topicSubTopics, &value_type::seatCategoryNames, &value_type::subTopicNames, &value_type::areaNames,
			&value_type::topicNames, &value_type::performances, &value_type::events, &value_type::venueNames, &value_type::subjectNames, &value_type::blockNames>();
};

#if !defined(ASAN_ENABLED)

template<> struct glz::meta<names> {
	using value_type			= names;
	static constexpr auto value = object();
};

template<> struct glz::meta<event> {
	using value_type			= event;
	static constexpr auto value = object("description", &value_type::description, "subTopicIds", &value_type::subTopicIds, "logo", &value_type::logo, "topicIds",
		&value_type::topicIds, "subjectCode", &value_type::subjectCode, "subtitle", &value_type::subtitle, "name", &value_type::name, "id", &value_type::id);
};

template<> struct glz::meta<price> {
	using value_type			= price;
	static constexpr auto value = object("audienceSubCategoryId", &value_type::audienceSubCategoryId, "seatCategoryId", &value_type::seatCategoryId, "amount", &value_type::amount);
};

template<> struct glz::meta<area> {
	using value_type			= area;
	static constexpr auto value = object("blockIds", &value_type::blockIds, "areaId", &value_type::areaId);
};

template<> struct glz::meta<seat_category> {
	using value_type			= seat_category;
	static constexpr auto value = object("areas", &value_type::areas, "seatCategoryId", &value_type::seatCategoryId);
};

template<> struct glz::meta<performance> {
	using value_type			= performance;
	static constexpr auto value = object("seatCategories", &value_type::seatCategories, "logo", &value_type::logo, "seatMapImage", &value_type::seatMapImage, "prices",
		&value_type::prices, "venueCode", &value_type::venueCode, "name", &value_type::name, "eventId", &value_type::eventId, "start", &value_type::start, "id", &value_type::id);
};

template<> struct glz::meta<venue_names> {
	using value_type			= venue_names;
	static constexpr auto value = object("PLEYEL_PLEYEL", &value_type::PLEYEL_PLEYEL);
};

template<> struct glz::meta<citm_catalog_message> {
	using value_type			= citm_catalog_message;
	static constexpr auto value = object("audienceSubCategoryNames", &value_type::audienceSubCategoryNames, "topicSubTopics", &value_type::topicSubTopics, "seatCategoryNames",
		&value_type::seatCategoryNames, "subTopicNames", &value_type::subTopicNames, "areaNames", &value_type::areaNames, "topicNames", &value_type::topicNames, "performances",
		&value_type::performances, "events", &value_type::events, "venueNames", &value_type::venueNames, "subjectNames", &value_type::subjectNames, "blockNames",
		&value_type::blockNames);
};
#endif

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

#if !defined(ASAN_ENABLED)

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
	static constexpr auto parseValue = createValue("profile_background_image_url_https", &value_type::profile_background_image_url_https, "profile_banner_url",
		&value_type::profile_banner_url, "profile_background_image_url", &value_type::profile_background_image_url, "profile_sidebar_border_color",
		&value_type::profile_sidebar_border_color, "profile_sidebar_fill_color", &value_type::profile_sidebar_fill_color, "time_zone", &value_type::time_zone,
		"profile_background_color", &value_type::profile_background_color, "profile_image_url_https", &value_type::profile_image_url_https, "utc_offset", &value_type::utc_offset,
		"profile_use_background_image", &value_type::profile_use_background_image, "url", &value_type::url, "profile_text_color", &value_type::profile_text_color,
		"profile_link_color", &value_type::profile_link_color, "profile_image_url", &value_type::profile_image_url, "profile_background_tile", &value_type::profile_background_tile,
		"is_translation_enabled", &value_type::is_translation_enabled, "default_profile_image", &value_type::default_profile_image, "contributors_enabled",
		&value_type::contributors_enabled, "follow_request_sent", &value_type::follow_request_sent, "favourites_count", &value_type::favourites_count, "description",
		&value_type::description, "screen_name", &value_type::screen_name, "followers_count", &value_type::followers_count, "statuses_count", &value_type::statuses_count,
		"created_at", &value_type::created_at, "entities", &value_type::entities, "friends_count", &value_type::friends_count, "default_profile", &value_type::default_profile,
		"listed_count", &value_type::listed_count, "location", &value_type::location, "user_protected", &value_type::user_protected, "is_translator", &value_type::is_translator,
		"id_str", &value_type::id_str, "notifications", &value_type::notifications, "string", &value_type::string, "name", &value_type::name, "geo_enabled",
		&value_type::geo_enabled, "lang", &value_type::lang, "protected", &value_type::protectedVal, "following", &value_type::following, "verified", &value_type::verified, "id",
		&value_type::id);
};

template<> struct jsonifier::core<retweeted_status_data> {
	using value_type				 = retweeted_status_data;
	static constexpr auto parseValue = createValue<&value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id_str, &value_type::in_reply_to_screen_name,
		&value_type::in_reply_to_status_id, &value_type::in_reply_to_user_id, &value_type::possibly_sensitive, &value_type::contributors, &value_type::coordinates,
		&value_type::place, &value_type::geo, &value_type::entities, &value_type::favorite_count, &value_type::metadata, &value_type::created_at, &value_type::retweet_count,
		&value_type::source, &value_type::id_str, &value_type::user, &value_type::lang, &value_type::text, &value_type::truncated, &value_type::favorited, &value_type::retweeted,
		&value_type::id>();
};

template<> struct jsonifier::core<status_data> {
	using value_type				 = status_data;
	static constexpr auto parseValue = createValue<&value_type::in_reply_to_status_id_str, &value_type::in_reply_to_user_id_str, &value_type::in_reply_to_screen_name,
		&value_type::in_reply_to_status_id, &value_type::in_reply_to_user_id, &value_type::possibly_sensitive, &value_type::contributors, &value_type::coordinates,
		&value_type::retweeted_status, &value_type::place, &value_type::geo, &value_type::entities, &value_type::favorite_count, &value_type::metadata, &value_type::created_at,
		&value_type::retweet_count, &value_type::source, &value_type::id_str, &value_type::user, &value_type::lang, &value_type::text, &value_type::truncated,
		&value_type::favorited, &value_type::retweeted, &value_type::id>();
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue<&value_type::search_metadata, &value_type::statuses>();
};

#if !defined(ASAN_ENABLED)

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
	using value_type				 = twitter_user;
	static constexpr auto value = object("profile_background_image_url_https", &value_type::profile_background_image_url_https, "profile_banner_url",
		&value_type::profile_banner_url, "profile_background_image_url", &value_type::profile_background_image_url, "profile_sidebar_border_color",
		&value_type::profile_sidebar_border_color, "profile_sidebar_fill_color", &value_type::profile_sidebar_fill_color, "time_zone", &value_type::time_zone,
		"profile_background_color", &value_type::profile_background_color, "profile_image_url_https", &value_type::profile_image_url_https, "utc_offset", &value_type::utc_offset,
		"profile_use_background_image", &value_type::profile_use_background_image, "url", &value_type::url, "profile_text_color", &value_type::profile_text_color,
		"profile_link_color", &value_type::profile_link_color, "profile_image_url", &value_type::profile_image_url, "profile_background_tile", &value_type::profile_background_tile,
		"is_translation_enabled", &value_type::is_translation_enabled, "default_profile_image", &value_type::default_profile_image, "contributors_enabled",
		&value_type::contributors_enabled, "follow_request_sent", &value_type::follow_request_sent, "favourites_count", &value_type::favourites_count, "description",
		&value_type::description, "screen_name", &value_type::screen_name, "followers_count", &value_type::followers_count, "statuses_count", &value_type::statuses_count,
		"created_at", &value_type::created_at, "entities", &value_type::entities, "friends_count", &value_type::friends_count, "default_profile", &value_type::default_profile,
		"listed_count", &value_type::listed_count, "location", &value_type::location, "user_protected", &value_type::user_protected, "is_translator", &value_type::is_translator,
		"id_str", &value_type::id_str, "notifications", &value_type::notifications, "string", &value_type::string, "name", &value_type::name, "geo_enabled",
		&value_type::geo_enabled, "lang", &value_type::lang, "protected", &value_type::protectedVal, "following", &value_type::following, "verified", &value_type::verified, "id",
		&value_type::id);
};

template<> struct glz::meta<retweeted_status_data> {
	using value_type			= retweeted_status_data;
	static constexpr auto value = object("in_reply_to_status_id_str", &value_type::in_reply_to_status_id_str, "in_reply_to_user_id_str", &value_type::in_reply_to_user_id_str,
		"in_reply_to_screen_name", &value_type::in_reply_to_screen_name, "in_reply_to_status_id", &value_type::in_reply_to_status_id, "in_reply_to_user_id",
		&value_type::in_reply_to_user_id, "possibly_sensitive", &value_type::possibly_sensitive, "contributors", &value_type::contributors, "coordinates", &value_type::coordinates,
		"place", &value_type::place, "geo", &value_type::geo, "entities", &value_type::entities, "favorite_count", &value_type::favorite_count, "metadata", &value_type::metadata,
		"created_at", &value_type::created_at, "retweet_count", &value_type::retweet_count, "source", &value_type::source, "id_str", &value_type::id_str, "user", &value_type::user,
		"lang", &value_type::lang, "text", &value_type::text, "truncated", &value_type::truncated, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "id",
		&value_type::id);
};

template<> struct glz::meta<status_data> {
	using value_type			= status_data;
	static constexpr auto value = object("in_reply_to_status_id_str", &value_type::in_reply_to_status_id_str, "in_reply_to_user_id_str", &value_type::in_reply_to_user_id_str,
		"in_reply_to_screen_name", &value_type::in_reply_to_screen_name, "in_reply_to_status_id", &value_type::in_reply_to_status_id, "in_reply_to_user_id",
		&value_type::in_reply_to_user_id, "possibly_sensitive", &value_type::possibly_sensitive, "contributors", &value_type::contributors, "coordinates", &value_type::coordinates,
		"place", &value_type::place, "geo", &value_type::geo, "entities", &value_type::entities, "favorite_count", &value_type::favorite_count, "metadata", &value_type::metadata,
		"created_at", &value_type::created_at, "retweeted_status", &value_type::retweeted_status, "retweet_count", &value_type::retweet_count, "source", &value_type::source,
		"id_str", &value_type::id_str, "user", &value_type::user, "lang", &value_type::lang, "text", &value_type::text, "truncated", &value_type::truncated, "favorited",
		&value_type::favorited, "retweeted", &value_type::retweeted, "id", &value_type::id);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("search_metadata", &value_type::search_metadata, "statuses", &value_type::statuses);
};

#endif

template<> struct jsonifier::core<icon_emoji_data> {
	using value_type				 = icon_emoji_data;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::id>();
};

template<> struct jsonifier::core<permission_overwrite> {
	using value_type				 = permission_overwrite;
	static constexpr auto parseValue = createValue<&value_type::allow, &value_type::deny, &value_type::id, &value_type::type>();
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue = createValue<&value_type::permission_overwrites, &value_type::last_message_id, &value_type::default_thread_rate_limit_per_user,
		&value_type::applied_tags, &value_type::recipients, &value_type::default_auto_archive_duration, &value_type::status, &value_type::last_pin_timestamp, &value_type::topic,
		&value_type::rate_limit_per_user, &value_type::icon_emoji, &value_type::total_message_sent, &value_type::video_quality_mode, &value_type::application_id,
		&value_type::permissions, &value_type::message_count, &value_type::parent_id, &value_type::member_count, &value_type::owner_id, &value_type::guild_id,
		&value_type::user_limit, &value_type::position, &value_type::name, &value_type::icon, &value_type::version, &value_type::bitrate, &value_type::id, &value_type::flags,
		&value_type::type, &value_type::managed, &value_type::nsfw>();
};

template<> struct jsonifier::core<user_data> {
	using value_type = user_data;
	static constexpr auto parseValue =
		createValue<&value_type::avatar_decoration_data, &value_type::display_name, &value_type::global_name, &value_type::avatar, &value_type::banner, &value_type::locale,
			&value_type::discriminator, &value_type::user_name, &value_type::accent_color, &value_type::premium_type, &value_type::public_flags, &value_type::email,
			&value_type::mfa_enabled, &value_type::id, &value_type::flags, &value_type::verified, &value_type::system, &value_type::bot>();
};

template<> struct jsonifier::core<member_data> {
	using value_type = member_data;
	static constexpr auto parseValue =
		createValue<&value_type::communication_disabled_until, &value_type::premium_since, &value_type::nick, &value_type::avatar, &value_type::roles, &value_type::permissions,
			&value_type::joined_at, &value_type::guild_id, &value_type::user, &value_type::flags, &value_type::pending, &value_type::deaf, &value_type::mute>();
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue<&value_type::premium_subscriber, &value_type::bot_id>();
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue<&value_type::unicode_emoji, &value_type::icon, &value_type::permissions, &value_type::position, &value_type::name,
		&value_type::mentionable, &value_type::version, &value_type::id, &value_type::tags, &value_type::color, &value_type::flags, &value_type::managed, &value_type::hoist>();
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue<&value_type::latest_on_boarding_question_id, &value_type::guild_scheduled_events, &value_type::safety_alerts_channel_id,
		&value_type::inventory_settings, &value_type::voice_states, &value_type::discovery_splash, &value_type::vanity_url_code, &value_type::application_id,
		&value_type::afk_channel_id, &value_type::default_message_notifications, &value_type::max_stage_video_channel_users, &value_type::public_updates_channel_id,
		&value_type::description, &value_type::threads, &value_type::channels, &value_type::premium_subscription_count, &value_type::approximate_presence_count,
		&value_type::features, &value_type::stickers, &value_type::premium_progress_bar_enabled, &value_type::members, &value_type::hub_type, &value_type::approximate_member_count,
		&value_type::explicit_content_filter, &value_type::max_video_channel_users, &value_type::splash, &value_type::banner, &value_type::system_channel_id,
		&value_type::widget_channel_id, &value_type::preferred_locale, &value_type::system_channel_flags, &value_type::rules_channel_id, &value_type::roles,
		&value_type::verification_level, &value_type::permissions, &value_type::max_presences, &value_type::discovery, &value_type::joined_at, &value_type::member_count,
		&value_type::premium_tier, &value_type::owner_id, &value_type::max_members, &value_type::afk_timeout, &value_type::widget_enabled, &value_type::region,
		&value_type::nsfw_level, &value_type::mfa_level, &value_type::name, &value_type::icon, &value_type::unavailable, &value_type::id, &value_type::flags, &value_type::large,
		&value_type::owner, &value_type::nsfw, &value_type::lazy>();
};

template<> struct jsonifier::core<discord_message> {
	using value_type				 = discord_message;
	static constexpr auto parseValue = createValue<&value_type::t, &value_type::d, &value_type::op, &value_type::s>();
};

#if !defined(ASAN_ENABLED)

template<> struct glz::meta<icon_emoji_data> {
	using value_type			= icon_emoji_data;
	static constexpr auto value = object("name", &value_type::name, "id", &value_type::id);
};

template<> struct glz::meta<permission_overwrite> {
	using value_type			= permission_overwrite;
	static constexpr auto value = object("allow", &value_type::allow, "deny", &value_type::deny, "id", &value_type::id, "type", &value_type::type);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object("permission_overwrites", &value_type::permission_overwrites, "last_message_id", &value_type::last_message_id,
		"default_thread_rate_limit_per_user", &value_type::default_thread_rate_limit_per_user, "applied_tags", &value_type::applied_tags, "recipients", &value_type::recipients,
		"default_auto_archive_duration", &value_type::default_auto_archive_duration, "status", &value_type::status, "last_pin_timestamp", &value_type::last_pin_timestamp, "topic",
		&value_type::topic, "rate_limit_per_user", &value_type::rate_limit_per_user, "icon_emoji", &value_type::icon_emoji, "total_message_sent", &value_type::total_message_sent,
		"video_quality_mode", &value_type::video_quality_mode, "application_id", &value_type::application_id, "permissions", &value_type::permissions, "message_count",
		&value_type::message_count, "parent_id", &value_type::parent_id, "member_count", &value_type::member_count, "owner_id", &value_type::owner_id, "guild_id",
		&value_type::guild_id, "user_limit", &value_type::user_limit, "position", &value_type::position, "name", &value_type::name, "icon", &value_type::icon, "version",
		&value_type::version, "bitrate", &value_type::bitrate, "id", &value_type::id, "flags", &value_type::flags, "type", &value_type::type, "managed", &value_type::managed,
		"nsfw", &value_type::nsfw);
};

template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object("avatar_decoration_data", &value_type::avatar_decoration_data, "display_name", &value_type::display_name, "global_name",
		&value_type::global_name, "avatar", &value_type::avatar, "banner", &value_type::banner, "locale", &value_type::locale, "discriminator", &value_type::discriminator,
		"user_name", &value_type::user_name, "accent_color", &value_type::accent_color, "premium_type", &value_type::premium_type, "public_flags", &value_type::public_flags,
		"email", &value_type::email, "mfa_enabled", &value_type::mfa_enabled, "id", &value_type::id, "flags", &value_type::flags, "verified", &value_type::verified, "system",
		&value_type::system, "bot", &value_type::bot);
};

template<> struct glz::meta<member_data> {
	using value_type			= member_data;
	static constexpr auto value = object("communication_disabled_until", &value_type::communication_disabled_until, "premium_since", &value_type::premium_since, "nick",
		&value_type::nick, "avatar", &value_type::avatar, "roles", &value_type::roles, "permissions", &value_type::permissions, "joined_at", &value_type::joined_at, "guild_id",
		&value_type::guild_id, "user", &value_type::user, "flags", &value_type::flags, "pending", &value_type::pending, "deaf", &value_type::deaf, "mute", &value_type::mute);
};

template<> struct glz::meta<tags_data> {
	using value_type			= tags_data;
	static constexpr auto value = object("premium_subscriber", &value_type::premium_subscriber, "bot_id", &value_type::bot_id);
};

template<> struct glz::meta<role_data> {
	using value_type			= role_data;
	static constexpr auto value = object("unicode_emoji", &value_type::unicode_emoji, "icon", &value_type::icon, "permissions", &value_type::permissions, "position",
		&value_type::position, "name", &value_type::name, "mentionable", &value_type::mentionable, "version", &value_type::version, "id", &value_type::id, "tags",
		&value_type::tags, "color", &value_type::color, "flags", &value_type::flags, "managed", &value_type::managed, "hoist", &value_type::hoist);
};

template<> struct glz::meta<guild_data> {
	using value_type			= guild_data;
	static constexpr auto value = object("latest_on_boarding_question_id", &value_type::latest_on_boarding_question_id, "guild_scheduled_events",
		&value_type::guild_scheduled_events, "safety_alerts_channel_id", &value_type::safety_alerts_channel_id, "inventory_settings", &value_type::inventory_settings,
		"voice_states", &value_type::voice_states, "discovery_splash", &value_type::discovery_splash, "vanity_url_code", &value_type::vanity_url_code, "application_id",
		&value_type::application_id, "afk_channel_id", &value_type::afk_channel_id, "default_message_notifications", &value_type::default_message_notifications,
		"max_stage_video_channel_users", &value_type::max_stage_video_channel_users, "public_updates_channel_id", &value_type::public_updates_channel_id, "description",
		&value_type::description, "threads", &value_type::threads, "channels", &value_type::channels, "premium_subscription_count", &value_type::premium_subscription_count,
		"approximate_presence_count", &value_type::approximate_presence_count, "features", &value_type::features, "stickers", &value_type::stickers, "premium_progress_bar_enabled",
		&value_type::premium_progress_bar_enabled, "members", &value_type::members, "hub_type", &value_type::hub_type, "approximate_member_count",
		&value_type::approximate_member_count, "explicit_content_filter", &value_type::explicit_content_filter, "max_video_channel_users", &value_type::max_video_channel_users,
		"splash", &value_type::splash, "banner", &value_type::banner, "system_channel_id", &value_type::system_channel_id, "widget_channel_id", &value_type::widget_channel_id,
		"preferred_locale", &value_type::preferred_locale, "system_channel_flags", &value_type::system_channel_flags, "rules_channel_id", &value_type::rules_channel_id, "roles",
		&value_type::roles, "verification_level", &value_type::verification_level, "permissions", &value_type::permissions, "max_presences", &value_type::max_presences,
		"discovery", &value_type::discovery, "joined_at", &value_type::joined_at, "member_count", &value_type::member_count, "premium_tier", &value_type::premium_tier, "owner_id",
		&value_type::owner_id, "max_members", &value_type::max_members, "afk_timeout", &value_type::afk_timeout, "widget_enabled", &value_type::widget_enabled, "region",
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

template<typename value_type> struct test {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct test_generator {
	std::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

	std::random_device randomEngine{};
	std::mt19937_64 gen{ randomEngine() };

	static constexpr std::string_view charset{ "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~\"\\\r\b\f\t\n" };

	template<typename value_type01, typename value_type02> value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_real_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	template<jsonifier::concepts::integer_t value_type01, jsonifier::concepts::integer_t value_type02> value_type01 randomizeNumberUniform(value_type01 start, value_type02 end) {
		std::uniform_int_distribution<value_type01> dis{ start, static_cast<value_type01>(end) };
		return dis(gen);
	}

	void insertUnicodeInJSON(std::string& jsonString) {
		auto newStringView = unicode_emoji::unicodeEmoji[randomizeNumberUniform(0ull, std::size(unicode_emoji::unicodeEmoji) - 1)];
		jsonString += static_cast<jsonifier::string>(newStringView);
	}

	std::string generateString() {
		auto length{ randomizeNumberUniform(32, 64) };
		constexpr size_t charsetSize = charset.size();
		auto unicodeCount			 = randomizeNumberUniform(1, length / 8);
		std::string result{};
		for (int32_t x = 0; x < length; ++x) {
			if (x % unicodeCount == 0) [[unlikely]] {
				insertUnicodeInJSON(result);
			}
			result += charset[randomizeNumberUniform(0ull, charsetSize - 1)];
		}
		return result;
	}

	double generateDouble() {
		return randomizeNumberUniform(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
	};

	bool generateBool() {
		return static_cast<bool>(randomizeNumberUniform(0, 100) >= 50);
	};

	size_t generateUint() {
		return randomizeNumberUniform(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max());
	};

	int64_t generateInt() {
		return randomizeNumberUniform(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
	};

	test_generator() {
		auto fill = [&](auto& v) {
			auto arraySize01 = randomizeNumberUniform(5ull, 25ull);
			v.resize(arraySize01);
			for (size_t x = 0; x < arraySize01; ++x) {
				auto arraySize02 = randomizeNumberUniform(5ull, 35ull);
				auto arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newString = generateString();
					v[x].testStrings.emplace_back(newString);
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testUints.emplace_back(generateUint());
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					v[x].testInts.emplace_back(generateInt());
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
					auto newBool = generateBool();
					v[x].testBools.emplace_back(newBool);
				}
				arraySize03 = randomizeNumberUniform(0ull, arraySize02);
				for (size_t y = 0; y < arraySize03; ++y) {
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
	double iterationCount{};
	std::string color{};
	bool operator>(const test_element_final& other) const noexcept {
		return resultSpeed > other.resultSpeed;
	}
};

struct test_element_pair {
	test_element_final writeData{};
	test_element_final readData{};
	bool operator>(const test_element_pair& other) const noexcept {
		return writeData > other.writeData && readData > other.readData;
	}
};

struct test_elements_final {
	std::vector<test_element_final> results{};
	std::string testName{};
};

using test_results_final = std::vector<test_elements_final>;

template<> struct jsonifier::core<test_element_final> {
	using value_type				 = test_element_final;
	static constexpr auto parseValue = createValue<&value_type::libraryName, &value_type::resultType, &value_type::resultSpeed, &value_type::iterationCount, &value_type::color>();
};

template<> struct jsonifier::core<test_elements_final> {
	using value_type				 = test_elements_final;
	static constexpr auto parseValue = createValue<&value_type::results, &value_type::testName>();
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
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};

template<> struct jsonifier::core<test_generator<test_struct>> {
	using value_type				 = test_generator<test_struct>;
	static constexpr auto parseValue = createValue<&value_type::a, &value_type::b, &value_type::c, &value_type::d, &value_type::e, &value_type::f, &value_type::g, &value_type::h,
		&value_type::i, &value_type::j, &value_type::k, &value_type::l, &value_type::m, &value_type::n, &value_type::o, &value_type::p, &value_type::q, &value_type::r,
		&value_type::s, &value_type::t, &value_type::u, &value_type::v, &value_type::w, &value_type::x, &value_type::y, &value_type::z>();
};

template<> struct jsonifier::core<abc_test<test_struct>> {
	using value_type				 = abc_test<test_struct>;
	static constexpr auto parseValue = createValue<&value_type::z, &value_type::y, &value_type::x, &value_type::w, &value_type::v, &value_type::u, &value_type::t, &value_type::s,
		&value_type::r, &value_type::q, &value_type::p, &value_type::o, &value_type::n, &value_type::m, &value_type::l, &value_type::k, &value_type::j, &value_type::i,
		&value_type::h, &value_type::g, &value_type::f, &value_type::e, &value_type::d, &value_type::c, &value_type::b, &value_type::a>();
};

#if !defined(ASAN_ENABLED)

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

#endif

#if defined(NDEBUG)
constexpr size_t totalIterationCountCap{ 500 };
#else
constexpr size_t totalIterationCountCap{ 101 };
#endif

enum class result_type { read = 0, write = 1 };

template<result_type type> std::string enumToString() {
	return type == result_type::read ? "Read" : "Write";
}

template<result_type type> struct result {
	std::optional<size_t> byteLength{};
	std::optional<double> jsonSpeed{};
	std::optional<double> jsonTime{};
	std::optional<double> iterationCount{};
	std::optional<double> cv{};
	std::string color{};

	result& operator=(result&&) noexcept	  = default;
	result(result&&) noexcept				  = default;
	result& operator=(const result&) noexcept = default;
	result(const result&) noexcept			  = default;

	result() noexcept = default;

	result(const std::string& colorNew, size_t byteLengthNew, const bnch_swt::benchmark_result_final& results) {
		iterationCount.emplace(results.iterationCount);
		byteLength.emplace(byteLengthNew);
		const auto cpuFrequency = bnch_swt::getCpuFrequency();
		jsonTime.emplace(results.median);
		cv.emplace(results.cv * 100.0f);
		auto mbWrittenCount	  = static_cast<double>(byteLength.value()) / 1e+6l;
		auto writeSecondCount = jsonTime.value() / 1e+9l;
		jsonSpeed.emplace(mbWrittenCount / writeSecondCount);
		color = colorNew;
	}

	operator bool() const noexcept {
		return jsonSpeed.has_value();
	}

	bool operator>(const result& other) const noexcept {
		if (jsonSpeed.has_value() && other.jsonSpeed.has_value()) {
			return jsonSpeed.value() > other.jsonSpeed.value();
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
	jsonifier::string name{};
	jsonifier::string test{};
	jsonifier::string url{};
	size_t iterations{};

	bool operator>(const results_data& other) const noexcept {
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

	results_data(const jsonifier::string& nameNew, const jsonifier::string& testNew, const jsonifier::string& urlNew, size_t iterationsNew) {
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

	void print() const noexcept {
		std::cout << jsonifier::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";
		if (readResult.byteLength.has_value() && readResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::read>() + " Speed (MB/S): " << std::setprecision(6) << readResult.jsonSpeed.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Length (Bytes): " << readResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Runtime (ns): " << std::setprecision(6) << readResult.jsonTime.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Iteration Count: " << std::setprecision(4) << readResult.iterationCount.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Coefficient of Variance (%): " << std::setprecision(4) << readResult.cv.value() << std::endl;
		}
		if (writeResult.byteLength.has_value() && writeResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::write>() + " Speed (MB/S): " << std::setprecision(6) << writeResult.jsonSpeed.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Length (Bytes): " << writeResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Runtime (ns): " << std::setprecision(6) << writeResult.jsonTime.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Iteration Count: " << std::setprecision(4) << writeResult.iterationCount.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Coefficient of Variance (%): " << std::setprecision(4) << writeResult.cv.value() << std::endl;
		}
		std::cout << "\n---" << std::endl;
	}

	std::string jsonStats() const noexcept {
		std::string writeLength{};
		std::string writeTime{};
		std::string writeIterationCount{};
		std::string writeCv{};
		std::string write{};
		std::string readLength{};
		std::string readTime{};
		std::string readIterationCount{};
		std::string readCv{};
		std::string read{};
		std::string finalString{ "| [" + name + "](" + url + ") | " };
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << std::setprecision(6) << readResult.jsonSpeed.value();
			read = stream01.str();
			std::stringstream stream02{};
			stream02 << readResult.byteLength.value();
			readLength = stream02.str();
			std::stringstream stream03{};
			stream03 << std::setprecision(6) << readResult.jsonTime.value();
			readTime = stream03.str();
			std::stringstream stream04{};
			stream04 << std::setprecision(4) << readResult.iterationCount.value();
			readIterationCount = stream04.str();
			finalString += read + " | " + readLength + " | " + readTime + " | " + readIterationCount + " | ";
		} else {
			readLength		   = "N/A";
			readTime		   = "N/A";
			read			   = "N/A";
			readIterationCount = "N/A";
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << std::setprecision(6) << writeResult.jsonSpeed.value();
			write = stream01.str();
			std::stringstream stream02{};
			stream02 << writeResult.byteLength.value();
			writeLength = stream02.str();
			std::stringstream stream03{};
			stream03 << std::setprecision(6) << writeResult.jsonTime.value();
			writeTime = stream03.str();
			std::stringstream stream04{};
			stream04 << std::setprecision(4) << writeResult.iterationCount.value();
			writeIterationCount = stream04.str();
			finalString += write + " | " + writeLength + " | " + writeTime + " | " + writeIterationCount + " | ";
		} else {
			writeLength			= "N/A";
			writeTime			= "N/A";
			write				= "N/A";
			writeIterationCount = "N/A";
		}
		return finalString;
	}
};
struct test_results {
	std::vector<results_data> results{};
	std::string markdownResults{};
	std::string testName{};
};

template<result_type type> struct jsonifier::core<result<type>> {
	using value_type				 = result<type>;
	static constexpr auto parseValue = createValue<&value_type::byteLength, &value_type::jsonSpeed, &value_type::jsonTime, &value_type::color>();
};

template<> struct jsonifier::core<results_data> {
	using value_type				 = results_data;
	static constexpr auto parseValue = createValue<&value_type::name, &value_type::readResult, &value_type::writeResult, &value_type::test, &value_type::url>();
};

template<> struct jsonifier::core<test_results> {
	using value_type				 = test_results;
	static constexpr auto parseValue = createValue<&value_type::results, &value_type::testName>();
};

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

const jsonifier::string basePath{ JSON_PATH };

template<json_library lib, test_type type, typename test_data_type, bool minified, size_t iterations, jsonifier_internal::string_literal testName> struct json_test_helper {};

#include "simdjson.h"

template<typename value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	value.get(valueNew);
}

template<jsonifier::concepts::bool_t value_type> void getValue(value_type&& valueNew, simdjson::ondemand::value value) {
	getValue<bool>(valueNew, value);
}

template<> void getValue(std::nullptr_t&, simdjson::ondemand::value) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	auto oldSize = valueNew.size();
	if (auto resultCode = value.get(result); !resultCode) {
		auto iter = result.begin();
		for (size_t x = 0; x < oldSize && iter != result.end(); ++x, ++iter) {
			getValue(valueNew[x], iter.operator*().value());
		}
		for (; iter != result.end(); ++iter) {
			getValue(valueNew.emplace_back(), iter.operator*().value());
		}
	}
}

template<> void getValue<std::string>(std::string& valueNew, simdjson::ondemand::value value) {
	std::string_view result;
	if (auto resultCode = value.get(result); !resultCode) {
		valueNew = static_cast<jsonifier::string>(result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& valueNew, simdjson::ondemand::value valueNewer) {
	simdjson::ondemand::value result;
	if (auto resultCode = valueNewer.get(result); !resultCode) {
		getValue(valueNew.emplace(), result);
	}
}

template<jsonifier::concepts::optional_t value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		if (result.type() != simdjson::ondemand::json_type::null) {
			getValue(returnValue.emplace(), result);
		}
	}
}

template<typename value_type> void getValue(value_type& returnValue, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		getValue(returnValue, result);
	}
}

template<> void getValue(std::nullptr_t&, simdjson::ondemand::value, const std::string&) {
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::array result;
	if (auto resultCode = value[key].get(result); !resultCode) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (size_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
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
	if (auto resultCode = value[key].get(result); !resultCode) {
		auto oldSize = returnValues.size();
		auto iter	 = result.begin();
		for (size_t x = 0; iter != result.end() && x < oldSize; ++x, ++iter) {
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

template<> void getValue(names& returnValue, simdjson::ondemand::value jsonData) {
}

template<> void getValue(event& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.subTopicIds, jsonData, "subTopicIds");
	getValue(returnValue.logo, jsonData, "logo");
	getValue(returnValue.topicIds, jsonData, "topicIds");
	getValue(returnValue.subjectCode, jsonData, "subjectCode");
	getValue(returnValue.subtitle, jsonData, "subtitle");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(price& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.audienceSubCategoryId, jsonData, "audienceSubCategoryId");
	getValue(returnValue.seatCategoryId, jsonData, "seatCategoryId");
	getValue(returnValue.amount, jsonData, "amount");
}

template<> void getValue(area& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.blockIds, jsonData, "blockIds");
	getValue(returnValue.areaId, jsonData, "areaId");
}

template<> void getValue(seat_category& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.areas, jsonData, "areas");
	getValue(returnValue.seatCategoryId, jsonData, "seatCategoryId");
}

template<> void getValue(performance& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.seatCategories, jsonData, "seatCategories");
	getValue(returnValue.logo, jsonData, "logo");
	getValue(returnValue.seatMapImage, jsonData, "seatMapImage");
	getValue(returnValue.prices, jsonData, "prices");
	getValue(returnValue.venueCode, jsonData, "venueCode");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.eventId, jsonData, "eventId");
	getValue(returnValue.start, jsonData, "start");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(venue_names& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.PLEYEL_PLEYEL, jsonData, "PLEYEL_PLEYEL");
}

template<> void getValue(citm_catalog_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.audienceSubCategoryNames, jsonData, "audienceSubCategoryNames");
	getValue(returnValue.topicSubTopics, jsonData, "topicSubTopics");
	getValue(returnValue.seatCategoryNames, jsonData, "seatCategoryNames");
	getValue(returnValue.subTopicNames, jsonData, "subTopicNames");
	getValue(returnValue.areaNames, jsonData, "areaNames");
	getValue(returnValue.topicNames, jsonData, "topicNames");
	getValue(returnValue.performances, jsonData, "performances");
	getValue(returnValue.events, jsonData, "events");
	getValue(returnValue.venueNames, jsonData, "venueNames");
	getValue(returnValue.subjectNames, jsonData, "subjectNames");
	getValue(returnValue.blockNames, jsonData, "blockNames");
}

template<> void getValue(test_struct& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.testBools, jsonData, "testBools");
	getValue(returnValue.testInts, jsonData, "testInts");
	getValue(returnValue.testUints, jsonData, "testUints");
	getValue(returnValue.testStrings, jsonData, "testStrings");
	getValue(returnValue.testDoubles, jsonData, "testDoubles");
}

template<> void getValue(test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.a, jsonData, "a");
	getValue(returnValue.b, jsonData, "b");
	getValue(returnValue.c, jsonData, "c");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.e, jsonData, "e");
	getValue(returnValue.f, jsonData, "f");
	getValue(returnValue.g, jsonData, "g");
	getValue(returnValue.h, jsonData, "h");
	getValue(returnValue.i, jsonData, "i");
	getValue(returnValue.j, jsonData, "j");
	getValue(returnValue.k, jsonData, "k");
	getValue(returnValue.l, jsonData, "l");
	getValue(returnValue.m, jsonData, "m");
	getValue(returnValue.n, jsonData, "n");
	getValue(returnValue.o, jsonData, "o");
	getValue(returnValue.p, jsonData, "p");
	getValue(returnValue.q, jsonData, "q");
	getValue(returnValue.r, jsonData, "r");
	getValue(returnValue.s, jsonData, "s");
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.u, jsonData, "u");
	getValue(returnValue.v, jsonData, "v");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.x, jsonData, "x");
	getValue(returnValue.y, jsonData, "y");
	getValue(returnValue.z, jsonData, "z");
}

template<> void getValue(geometry_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.coordinates, jsonData, "coordinates");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(properties_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.name, jsonData, "name");
}

template<> void getValue(feature& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.properties, jsonData, "properties");
	getValue(returnValue.geometry, jsonData, "geometry");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(canada_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.features, jsonData, "features");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(search_metadata_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.since_id_str, jsonData, "since_id_str");
	getValue(returnValue.next_results, jsonData, "next_results");
	getValue(returnValue.refresh_url, jsonData, "refresh_url");
	getValue(returnValue.max_id_str, jsonData, "max_id_str");
	getValue(returnValue.completed_in, jsonData, "completed_in");
	getValue(returnValue.query, jsonData, "query");
	getValue(returnValue.since_id, jsonData, "since_id");
	getValue(returnValue.count, jsonData, "count");
	getValue(returnValue.max_id, jsonData, "max_id");
}

template<> void getValue(hashtag& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.text, jsonData, "text");
}

template<> void getValue(large_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.resize, jsonData, "resize");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.h, jsonData, "h");
}

template<> void getValue(sizes_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.medium, jsonData, "medium");
	getValue(returnValue.small, jsonData, "small");
	getValue(returnValue.thumb, jsonData, "thumb");
	getValue(returnValue.large, jsonData, "large");
}

template<> void getValue(media_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.source_status_id_str, jsonData, "source_status_id_str");
	getValue(returnValue.source_status_id, jsonData, "source_status_id");
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.media_url_https, jsonData, "media_url_https");
	getValue(returnValue.expanded_url, jsonData, "expanded_url");
	getValue(returnValue.display_url, jsonData, "display_url");
	getValue(returnValue.media_url, jsonData, "media_url");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.type, jsonData, "type");
	getValue(returnValue.sizes, jsonData, "sizes");
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(url_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.expanded_url, jsonData, "expanded_url");
	getValue(returnValue.display_url, jsonData, "display_url");
	getValue(returnValue.url, jsonData, "url");
}

template<> void getValue(user_mention& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.indices, jsonData, "indices");
	getValue(returnValue.screen_name, jsonData, "screen_name");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(status_entities& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.media, jsonData, "media");
	getValue(returnValue.user_mentions, jsonData, "user_mentions");
	getValue(returnValue.symbols, jsonData, "symbols");
	getValue(returnValue.hashtags, jsonData, "hashtags");
	getValue(returnValue.urls, jsonData, "urls");
}

template<> void getValue(metadata_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.iso_language_code, jsonData, "iso_language_code");
	getValue(returnValue.result_type, jsonData, "result_type");
}

template<> void getValue(description_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.urls, jsonData, "urls");
}

template<> void getValue(user_entities& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.description, jsonData, "description");
}

template<> void getValue(twitter_user& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.profile_background_image_url_https, jsonData, "profile_background_image_url_https");
	getValue(returnValue.profile_banner_url, jsonData, "profile_banner_url");
	getValue(returnValue.profile_background_image_url, jsonData, "profile_background_image_url");
	getValue(returnValue.profile_sidebar_fill_color, jsonData, "profile_sidebar_fill_color");
	getValue(returnValue.time_zone, jsonData, "time_zone");
	getValue(returnValue.profile_background_color, jsonData, "profile_background_color");
	getValue(returnValue.profile_image_url_https, jsonData, "profile_image_url_https");
	getValue(returnValue.utc_offset, jsonData, "utc_offset");
	getValue(returnValue.profile_use_background_image, jsonData, "profile_use_background_image");
	getValue(returnValue.url, jsonData, "url");
	getValue(returnValue.profile_text_color, jsonData, "profile_text_color");
	getValue(returnValue.profile_link_color, jsonData, "profile_link_color");
	getValue(returnValue.profile_image_url, jsonData, "profile_image_url");
	getValue(returnValue.profile_background_tile, jsonData, "profile_background_tile");
	getValue(returnValue.is_translation_enabled, jsonData, "is_translation_enabled");
	getValue(returnValue.default_profile_image, jsonData, "default_profile_image");
	getValue(returnValue.contributors_enabled, jsonData, "contributors_enabled");
	getValue(returnValue.follow_request_sent, jsonData, "follow_request_sent");
	getValue(returnValue.favourites_count, jsonData, "favourites_count");
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.screen_name, jsonData, "screen_name");
	getValue(returnValue.followers_count, jsonData, "followers_count");
	getValue(returnValue.statuses_count, jsonData, "statuses_count");
	getValue(returnValue.created_at, jsonData, "created_at");
	getValue(returnValue.entities, jsonData, "entities");
	getValue(returnValue.friends_count, jsonData, "friends_count");
	getValue(returnValue.default_profile, jsonData, "default_profile");
	getValue(returnValue.listed_count, jsonData, "listed_count");
	getValue(returnValue.location, jsonData, "location");
	getValue(returnValue.user_protected, jsonData, "user_protected");
	getValue(returnValue.is_translator, jsonData, "is_translator");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.notifications, jsonData, "notifications");
	getValue(returnValue.string, jsonData, "string");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.geo_enabled, jsonData, "geo_enabled");
	getValue(returnValue.lang, jsonData, "lang");
	getValue(returnValue.following, jsonData, "following");
	getValue(returnValue.verified, jsonData, "verified");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(retweeted_status_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.in_reply_to_status_id_str, jsonData, "in_reply_to_status_id_str");
	getValue(returnValue.in_reply_to_user_id_str, jsonData, "in_reply_to_user_id_str");
	getValue(returnValue.in_reply_to_screen_name, jsonData, "in_reply_to_screen_name");
	getValue(returnValue.in_reply_to_status_id, jsonData, "in_reply_to_status_id");
	getValue(returnValue.in_reply_to_user_id, jsonData, "in_reply_to_user_id");
	getValue(returnValue.possibly_sensitive, jsonData, "possibly_sensitive");
	getValue(returnValue.contributors, jsonData, "contributors");
	getValue(returnValue.coordinates, jsonData, "coordinates");
	getValue(returnValue.place, jsonData, "place");
	getValue(returnValue.geo, jsonData, "geo");
	getValue(returnValue.entities, jsonData, "entities");
	getValue(returnValue.favorite_count, jsonData, "favorite_count");
	getValue(returnValue.metadata, jsonData, "metadata");
	getValue(returnValue.created_at, jsonData, "created_at");
	getValue(returnValue.retweet_count, jsonData, "retweet_count");
	getValue(returnValue.source, jsonData, "source");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.user, jsonData, "user");
	getValue(returnValue.lang, jsonData, "lang");
	getValue(returnValue.text, jsonData, "text");
	getValue(returnValue.truncated, jsonData, "truncated");
	getValue(returnValue.favorited, jsonData, "favorited");
	getValue(returnValue.retweeted, jsonData, "retweeted");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(status_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.in_reply_to_status_id_str, jsonData, "in_reply_to_status_id_str");
	getValue(returnValue.in_reply_to_user_id_str, jsonData, "in_reply_to_user_id_str");
	getValue(returnValue.in_reply_to_screen_name, jsonData, "in_reply_to_screen_name");
	getValue(returnValue.in_reply_to_status_id, jsonData, "in_reply_to_status_id");
	getValue(returnValue.in_reply_to_user_id, jsonData, "in_reply_to_user_id");
	getValue(returnValue.possibly_sensitive, jsonData, "possibly_sensitive");
	getValue(returnValue.contributors, jsonData, "contributors");
	getValue(returnValue.coordinates, jsonData, "coordinates");
	getValue(returnValue.place, jsonData, "place");
	getValue(returnValue.geo, jsonData, "geo");
	getValue(returnValue.entities, jsonData, "entities");
	getValue(returnValue.favorite_count, jsonData, "favorite_count");
	getValue(returnValue.metadata, jsonData, "metadata");
	getValue(returnValue.created_at, jsonData, "created_at");
	getValue(returnValue.retweet_count, jsonData, "retweet_count");
	getValue(returnValue.source, jsonData, "source");
	getValue(returnValue.id_str, jsonData, "id_str");
	getValue(returnValue.user, jsonData, "user");
	getValue(returnValue.lang, jsonData, "lang");
	getValue(returnValue.text, jsonData, "text");
	getValue(returnValue.truncated, jsonData, "truncated");
	getValue(returnValue.favorited, jsonData, "favorited");
	getValue(returnValue.retweeted, jsonData, "retweeted");
	getValue(returnValue.retweeted_status, jsonData, "retweeted_status");
	getValue(returnValue.id, jsonData, "id");
}

template<> void getValue(twitter_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.search_metadata, jsonData, "search_metadata");
	getValue(returnValue.statuses, jsonData, "statuses");
}

template<> void getValue(icon_emoji_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.name, jsonData, "name");
	// Since nullptr_t is not typically deserialized, handling it as necessary.
}

template<> void getValue(permission_overwrite& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.allow, jsonData, "allow");
	getValue(returnValue.deny, jsonData, "deny");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.type, jsonData, "type");
}

template<> void getValue(channel_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.permission_overwrites, jsonData, "permission_overwrites");
	getValue(returnValue.last_message_id, jsonData, "last_message_id");
	getValue(returnValue.default_thread_rate_limit_per_user, jsonData, "default_thread_rate_limit_per_user");
	getValue(returnValue.applied_tags, jsonData, "applied_tags");
	getValue(returnValue.recipients, jsonData, "recipients");
	getValue(returnValue.default_auto_archive_duration, jsonData, "default_auto_archive_duration");
	getValue(returnValue.status, jsonData, "status");
	getValue(returnValue.last_pin_timestamp, jsonData, "last_pin_timestamp");
	getValue(returnValue.topic, jsonData, "topic");
	getValue(returnValue.rate_limit_per_user, jsonData, "rate_limit_per_user");
	getValue(returnValue.icon_emoji, jsonData, "icon_emoji");
	getValue(returnValue.total_message_sent, jsonData, "total_message_sent");
	getValue(returnValue.video_quality_mode, jsonData, "video_quality_mode");
	getValue(returnValue.application_id, jsonData, "application_id");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.message_count, jsonData, "message_count");
	getValue(returnValue.parent_id, jsonData, "parent_id");
	getValue(returnValue.member_count, jsonData, "member_count");
	getValue(returnValue.owner_id, jsonData, "owner_id");
	getValue(returnValue.guild_id, jsonData, "guild_id");
	getValue(returnValue.user_limit, jsonData, "user_limit");
	getValue(returnValue.position, jsonData, "position");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.version, jsonData, "version");
	getValue(returnValue.bitrate, jsonData, "bitrate");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.type, jsonData, "type");
	getValue(returnValue.managed, jsonData, "managed");
	getValue(returnValue.nsfw, jsonData, "nsfw");
}

template<> void getValue(user_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.avatar_decoration_data, jsonData, "avatar_decoration_data");
	getValue(returnValue.display_name, jsonData, "display_name");
	getValue(returnValue.global_name, jsonData, "global_name");
	getValue(returnValue.avatar, jsonData, "avatar");
	getValue(returnValue.banner, jsonData, "banner");
	getValue(returnValue.locale, jsonData, "locale");
	getValue(returnValue.discriminator, jsonData, "discriminator");
	getValue(returnValue.user_name, jsonData, "user_name");
	getValue(returnValue.accent_color, jsonData, "accent_color");
	getValue(returnValue.premium_type, jsonData, "premium_type");
	getValue(returnValue.public_flags, jsonData, "public_flags");
	getValue(returnValue.email, jsonData, "email");
	getValue(returnValue.mfa_enabled, jsonData, "mfa_enabled");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.verified, jsonData, "verified");
	getValue(returnValue.system, jsonData, "system");
	getValue(returnValue.bot, jsonData, "bot");
}

template<> void getValue(member_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.communication_disabled_until, jsonData, "communication_disabled_until");
	getValue(returnValue.premium_since, jsonData, "premium_since");
	getValue(returnValue.nick, jsonData, "nick");
	getValue(returnValue.avatar, jsonData, "avatar");
	getValue(returnValue.roles, jsonData, "roles");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.joined_at, jsonData, "joined_at");
	getValue(returnValue.guild_id, jsonData, "guild_id");
	getValue(returnValue.user, jsonData, "user");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.pending, jsonData, "pending");
	getValue(returnValue.deaf, jsonData, "deaf");
	getValue(returnValue.mute, jsonData, "mute");
}

template<> void getValue(tags_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.premium_subscriber, jsonData, "premium_subscriber");
	getValue(returnValue.bot_id, jsonData, "bot_id");
}

template<> void getValue(role_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.unicode_emoji, jsonData, "unicode_emoji");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.position, jsonData, "position");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.mentionable, jsonData, "mentionable");
	getValue(returnValue.version, jsonData, "version");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.tags, jsonData, "tags");
	getValue(returnValue.color, jsonData, "color");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.managed, jsonData, "managed");
	getValue(returnValue.hoist, jsonData, "hoist");
}

template<> void getValue(guild_data& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.latest_on_boarding_question_id, jsonData, "latest_on_boarding_question_id");
	getValue(returnValue.guild_scheduled_events, jsonData, "guild_scheduled_events");
	getValue(returnValue.safety_alerts_channel_id, jsonData, "safety_alerts_channel_id");
	getValue(returnValue.inventory_settings, jsonData, "inventory_settings");
	getValue(returnValue.voice_states, jsonData, "voice_states");
	getValue(returnValue.discovery_splash, jsonData, "discovery_splash");
	getValue(returnValue.vanity_url_code, jsonData, "vanity_url_code");
	getValue(returnValue.application_id, jsonData, "application_id");
	getValue(returnValue.afk_channel_id, jsonData, "afk_channel_id");
	getValue(returnValue.default_message_notifications, jsonData, "default_message_notifications");
	getValue(returnValue.max_stage_video_channel_users, jsonData, "max_stage_video_channel_users");
	getValue(returnValue.public_updates_channel_id, jsonData, "public_updates_channel_id");
	getValue(returnValue.description, jsonData, "description");
	getValue(returnValue.threads, jsonData, "threads");
	getValue(returnValue.channels, jsonData, "channels");
	getValue(returnValue.premium_subscription_count, jsonData, "premium_subscription_count");
	getValue(returnValue.approximate_presence_count, jsonData, "approximate_presence_count");
	getValue(returnValue.features, jsonData, "features");
	getValue(returnValue.stickers, jsonData, "stickers");
	getValue(returnValue.premium_progress_bar_enabled, jsonData, "premium_progress_bar_enabled");
	getValue(returnValue.members, jsonData, "members");
	getValue(returnValue.hub_type, jsonData, "hub_type");
	getValue(returnValue.approximate_member_count, jsonData, "approximate_member_count");
	getValue(returnValue.explicit_content_filter, jsonData, "explicit_content_filter");
	getValue(returnValue.max_video_channel_users, jsonData, "max_video_channel_users");
	getValue(returnValue.splash, jsonData, "splash");
	getValue(returnValue.banner, jsonData, "banner");
	getValue(returnValue.system_channel_id, jsonData, "system_channel_id");
	getValue(returnValue.widget_channel_id, jsonData, "widget_channel_id");
	getValue(returnValue.preferred_locale, jsonData, "preferred_locale");
	getValue(returnValue.system_channel_flags, jsonData, "system_channel_flags");
	getValue(returnValue.rules_channel_id, jsonData, "rules_channel_id");
	getValue(returnValue.roles, jsonData, "roles");
	getValue(returnValue.verification_level, jsonData, "verification_level");
	getValue(returnValue.permissions, jsonData, "permissions");
	getValue(returnValue.max_presences, jsonData, "max_presences");
	getValue(returnValue.discovery, jsonData, "discovery");
	getValue(returnValue.joined_at, jsonData, "joined_at");
	getValue(returnValue.member_count, jsonData, "member_count");
	getValue(returnValue.premium_tier, jsonData, "premium_tier");
	getValue(returnValue.owner_id, jsonData, "owner_id");
	getValue(returnValue.max_members, jsonData, "max_members");
	getValue(returnValue.afk_timeout, jsonData, "afk_timeout");
	getValue(returnValue.widget_enabled, jsonData, "widget_enabled");
	getValue(returnValue.region, jsonData, "region");
	getValue(returnValue.nsfw_level, jsonData, "nsfw_level");
	getValue(returnValue.mfa_level, jsonData, "mfa_level");
	getValue(returnValue.name, jsonData, "name");
	getValue(returnValue.icon, jsonData, "icon");
	getValue(returnValue.unavailable, jsonData, "unavailable");
	getValue(returnValue.id, jsonData, "id");
	getValue(returnValue.flags, jsonData, "flags");
	getValue(returnValue.large, jsonData, "large");
	getValue(returnValue.owner, jsonData, "owner");
	getValue(returnValue.nsfw, jsonData, "nsfw");
	getValue(returnValue.lazy, jsonData, "lazy");
}

template<> void getValue(discord_message& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.op, jsonData, "op");
	getValue(returnValue.s, jsonData, "s");
}

template<> void getValue(abc_test<test_struct>& returnValue, simdjson::ondemand::value jsonData) {
	getValue(returnValue.a, jsonData, "a");
	getValue(returnValue.b, jsonData, "b");
	getValue(returnValue.c, jsonData, "c");
	getValue(returnValue.d, jsonData, "d");
	getValue(returnValue.e, jsonData, "e");
	getValue(returnValue.f, jsonData, "f");
	getValue(returnValue.g, jsonData, "g");
	getValue(returnValue.h, jsonData, "h");
	getValue(returnValue.i, jsonData, "i");
	getValue(returnValue.j, jsonData, "j");
	getValue(returnValue.k, jsonData, "k");
	getValue(returnValue.l, jsonData, "l");
	getValue(returnValue.m, jsonData, "m");
	getValue(returnValue.n, jsonData, "n");
	getValue(returnValue.o, jsonData, "o");
	getValue(returnValue.p, jsonData, "p");
	getValue(returnValue.q, jsonData, "q");
	getValue(returnValue.r, jsonData, "r");
	getValue(returnValue.s, jsonData, "s");
	getValue(returnValue.t, jsonData, "t");
	getValue(returnValue.u, jsonData, "u");
	getValue(returnValue.v, jsonData, "v");
	getValue(returnValue.w, jsonData, "w");
	getValue(returnValue.x, jsonData, "x");
	getValue(returnValue.y, jsonData, "y");
	getValue(returnValue.z, jsonData, "z");
}

std::string table_header = R"(
| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ----------- | ------------------- | -------------- | -------------------- | ------------ | -------------------- | --------------- | --------------------- |   )";

std::string read_table_header = R"(
| Library | Read (MB/s) | Read Length (Bytes) | Read Time (ns) | Read Iteration Count |
| ------- | ----------- | ------------------- | -------------- | -------------------- |   )";

std::string write_table_header = R"(
| Library | Write (MB/s) | Write Length (Bytes) | Write Time (ns) | Write Iteration Count |
| ------- | ------------ | -------------------- | --------------- | --------------------- |   )";

#if defined(__x86_64__) || defined(_M_AMD64)
static inline void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
inline static uint64_t xgetbv();
#endif

std::string getCPUInfo() {
	char brand[49] = { 0 };
	uint32_t regs[12]{};
#if defined(__x86_64__) || defined(_M_AMD64)
	regs[0] = 0x80000000;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	if (regs[0] < 0x80000004) {
		return {};
	}
	regs[0] = 0x80000002;
	cpuid(regs, regs + 1, regs + 2, regs + 3);
	regs[4] = 0x80000003;
	cpuid(regs + 4, regs + 5, regs + 6, regs + 7);
	regs[8] = 0x80000004;
	cpuid(regs + 8, regs + 9, regs + 10, regs + 11);

	memcpy(brand, regs, sizeof(regs));
#endif
	return { brand, sizeof(regs) };
}

#if defined(__x86_64__) || defined(_M_AMD64)

static inline void cpuid(uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
	#if defined(_MSC_VER)
	int32_t cpuInfo[4];
	__cpuidex(cpuInfo, *eax, *ecx);
	*eax = cpuInfo[0];
	*ebx = cpuInfo[1];
	*ecx = cpuInfo[2];
	*edx = cpuInfo[3];
	#elif defined(HAVE_GCC_GET_CPUID) && defined(USE_GCC_GET_CPUID)
	uint32_t level = *eax;
	__get_cpuid(level, eax, ebx, ecx, edx);
	#else
	uint32_t a = *eax, b, c = *ecx, d;
	asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(a), "c"(c));
	*eax = a;
	*ebx = b;
	*ecx = c;
	*edx = d;
	#endif
}
#endif

static const std::string section001{ R"(
 > At least )" +
	jsonifier::toString(30) + R"( iterations on a ()" + getCPUInfo() + R"(), until coefficient of variance is at or below 1%.
)" };

constexpr auto newString02{ jsonifier_internal::string_literal{ R"(#### Using the following commits:
----
| Jsonifier: [)" } +
	jsonifier_internal::string_literal{ JSONIFIER_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/RealTimeChris/Jsonifier/commit/)" } +
	jsonifier_internal::string_literal{ JSONIFIER_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } + jsonifier_internal::string_literal{ R"(| Glaze: [)" } +
	jsonifier_internal::string_literal{ GLAZE_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/stephenberry/glaze/commit/)" } +
	jsonifier_internal::string_literal{ GLAZE_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } + jsonifier_internal::string_literal{ R"(| Simdjson: [)" } +
	jsonifier_internal::string_literal{ SIMDJSON_COMMIT } + jsonifier_internal::string_literal{ R"(](https://github.com/simdjson/simdjson/commit/)" } +
	jsonifier_internal::string_literal{ SIMDJSON_COMMIT } + jsonifier_internal::string_literal{ ")  \n" } };

constexpr jsonifier::string_view section002{ newString02.view() };

constexpr jsonifier::string_view section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++19 compiler)

Latest Results: ()" };

constexpr jsonifier::string_view section01{
	R"(

### Json Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section02{
	R"(

### Json Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Json%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section03{
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

constexpr jsonifier::string_view section04{ R"(

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section05{ R"(

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section06{ R"(

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section07{ R"(

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section08{ R"(

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section09{ R"(

### CitmCatalog Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section10{ R"(

### CitmCatalog Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CitmCatalogData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/CitmCatalog%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section11{ R"(

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section12{ R"(

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

constexpr jsonifier::string_view section13{
	R"(

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section14{
	R"(

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

constexpr jsonifier::string_view section15{
	R"(

### Validation Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Validate%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

std::string getCurrentWorkingDirectory() {
	try {
		return std::filesystem::current_path().string();
	} catch (const std::filesystem::filesystem_error& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return "";
	}
}

void executePythonScript(const jsonifier::string& scriptPath, const jsonifier::string& argument01, const jsonifier::string& argument02) {
#if defined(JSONIFIER_WIN)
	constexpr std::string_view pythonName{ "python " };
#else
	constexpr std::string_view pythonName{ "python3 " };
#endif
	jsonifier::string command = static_cast<jsonifier::string>(pythonName) + scriptPath + " " + argument01 + " " + argument02;
	int32_t result			  = system(command.data());
	if (result != 0) {
		std::cout << "Error: Failed to execute Python script. Command exited with code " << result << std::endl;
	}
}

template<test_type type, typename test_data_type, bool minified, jsonifier_internal::string_literal testName> struct json_tests_helper;

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::prettify, std::string, false, testName> {
	JSONIFIER_ALWAYS_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)
		std::string glazeBuffer{ jsonData };
		std::string glazeNewerBuffer{};
		auto glazeWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"steelblue">([&]() {
			glz::prettify_json(glazeBuffer, glazeNewerBuffer);
			bnch_swt::doNotOptimizeAway(glazeNewerBuffer);
		});

		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeWriteResult.iterationCount };

		bnch_swt::file_loader glazeFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-glaze.json" };
		glazeFileLoader.saveFile(glazeNewerBuffer);
		glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeNewerBuffer.size(), glazeWriteResult };
		glazeResults.print();

		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };

		jsonifier::jsonifier_core parser{};
		std::string jsonifierNewerBuffer{};
		auto jsonifierWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.prettifyJson(jsonifierBuffer, jsonifierNewerBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierNewerBuffer);
		});

		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierWriteResult.iterationCount };

		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		bnch_swt::file_loader jsonifierFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-jsonifier.json" };
		jsonifierFileLoader.saveFile(jsonifierNewerBuffer);

		jsonifierResults.writeResult = result<result_type::write>{ "steelblue", jsonifierNewerBuffer.size(), jsonifierWriteResult };

		jsonifierResults.print();

		resultsNew.emplace_back(jsonifierResults);

		std::string table{};
		const auto n = resultsNew.size();
		table += write_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
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

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::minify, std::string, false, testName> {
	JSONIFIER_ALWAYS_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)

		std::string simdjsonBuffer{ jsonData };

		simdjson::dom::parser simdjsonParser{};
		std::string simdjsonNewerBuffer{};

		auto simdjsonWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"cornflowerblue">([&]() {
			try {
				simdjsonNewerBuffer = simdjson::minify(simdjsonParser.parse(simdjsonBuffer));
				bnch_swt::doNotOptimizeAway(simdjsonNewerBuffer);
			} catch (std::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
			return;
		});

		results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonWriteResult.iterationCount };

		bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-simdjson.json" };
		simdjsonFileLoader.saveFile(simdjsonNewerBuffer);
		simdjsonResults.writeResult = result<result_type::write>{ "cornflowerblue", simdjsonNewerBuffer.size(), simdjsonWriteResult };

		simdjsonResults.print();

		resultsNew.emplace_back(simdjsonResults);
		std::string glazeBuffer{ jsonData };


		std::string glazeNewerBuffer{};
		auto glazeWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Write", glazeLibraryName,
			"steelblue">([&]() {
			glazeNewerBuffer = glz::minify_json(glazeBuffer);
			bnch_swt::doNotOptimizeAway(glazeNewerBuffer);
		});
		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeWriteResult.iterationCount };

		bnch_swt::file_loader glazeFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-glaze.json" };
		glazeFileLoader.saveFile(glazeNewerBuffer);
		glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeNewerBuffer.size(), glazeWriteResult };
		glazeResults.print();
		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };
		std::string jsonifierNewerBuffer{};
		jsonifier::jsonifier_core parser{};
		auto jsonifierWriteResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.minifyJson(jsonifierBuffer, jsonifierNewerBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierNewerBuffer);
		});
		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierWriteResult.iterationCount };
		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}
		bnch_swt::file_loader jsonifierFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-jsonifier.json" };
		jsonifierFileLoader.saveFile(jsonifierNewerBuffer);

		jsonifierResults.writeResult = result<result_type::write>{ "steelblue", jsonifierNewerBuffer.size(), jsonifierWriteResult };
		jsonifierResults.print();
		resultsNew.emplace_back(jsonifierResults);
		std::string table{};
		const auto n = resultsNew.size();
		table += write_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
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

template<jsonifier_internal::string_literal testName> struct json_tests_helper<test_type::validate, std::string, false, testName> {
	JSONIFIER_ALWAYS_INLINE static test_results run(const std::string& jsonData) {
		jsonifier::vector<results_data> resultsNew{};
		test_results jsonResults{};
		jsonResults.testName = static_cast<jsonifier::string>(testName);
#if !defined(ASAN_ENABLED)

		std::string glazeBuffer{ jsonData };
		auto glazeReadResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read", glazeLibraryName,
			"skyblue">([&]() {
			auto result = glz::validate_json(glazeBuffer);
			bnch_swt::doNotOptimizeAway(result);
		});

		results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>(testName), static_cast<jsonifier::string>(glazeCommitUrl),
			glazeReadResult.iterationCount };

		bnch_swt::file_loader glazeFileLoader{ basePath + "/" + static_cast<jsonifier::string>(testName) + "-glaze.json" };
		glazeFileLoader.saveFile(glazeBuffer);
		glazeResults.readResult = result<result_type::read>{ "skyblue", glazeBuffer.size(), glazeReadResult };
		glazeResults.print();
		resultsNew.emplace_back(glazeResults);
#endif
		std::string jsonifierBuffer{ jsonData };
		jsonifier::jsonifier_core parser{};
		auto jsonifierReadResult = bnch_swt::benchmark_stage<"Json-Performance",
			bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::runBenchmark<testName + "-Read",
			jsonifierLibraryName, "steelblue">([&]() {
			parser.validateJson(jsonifierBuffer);
			bnch_swt::doNotOptimizeAway(jsonifierBuffer);
		});

		results_data jsonifierResults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>(testName),
			static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };

		for (auto& value: parser.getErrors()) {
			std::cout << "Jsonifier Error: " << value << std::endl;
		}

		jsonifierResults.readResult = result<result_type::read>{ "teal", jsonifierBuffer.size(), jsonifierReadResult };

		jsonifierResults.print();

		resultsNew.emplace_back(jsonifierResults);

		std::string table{};
		const auto n = resultsNew.size();
		table += read_table_header + "\n";
		std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
		for (size_t x = 0; x < n; ++x) {
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

#include "ConformanceTests.hpp"
#include "FailTests.hpp"

int32_t main() {
	try {
		std::bitset<2048> bits{};
		bits.operator[](2034);
		conformanceTests();
		failTests();
		test_generator<test_struct> testJsonData{};
		std::string jsonData{};
		jsonifier::jsonifier_core parser{};
		bnch_swt::file_loader fileLoader01{ README_PATH };
		bnch_swt::file_loader fileLoader02{ basePath + "/JsonData-Prettified.json" };
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonData);
		fileLoader02.saveFile(jsonData);
		bnch_swt::file_loader fileLoader03{ basePath + "/JsonData-Minified.json" };
		std::string jsonMinifiedData{ parser.minifyJson(jsonData) };
		fileLoader03.saveFile(jsonMinifiedData);
		bnch_swt::file_loader fileLoader04{ basePath + "/Results.json" };
		bnch_swt::file_loader fileLoader05{ basePath + "/DiscordData-Prettified.json" };
		std::string discordData{ fileLoader05.operator jsonifier::string&() };
		discord_message discordMessage{};
		//std::cout << "WERE HERE THIS IS IT 0101: " << jsonMinifiedData << std::endl;
		parser.parseJson(discordMessage, discordData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0101: " << discordData << std::endl;
		fileLoader05.saveFile(discordData);
		bnch_swt::file_loader fileLoader06{ basePath + "/DiscordData-Minified.json" };
		std::string discordMinifiedData{ fileLoader06.operator jsonifier::string&() };
		discordMinifiedData = parser.minifyJson(discordData);
		fileLoader06.saveFile(discordMinifiedData);
		bnch_swt::file_loader fileLoader07{ basePath + "/CanadaData-Prettified.json" };
		std::string canadaData{ fileLoader07.operator jsonifier::string&() };
		canada_message canadaMessage{};
		parser.parseJson(canadaMessage, canadaData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0202: " << canadaData << std::endl;
		fileLoader07.saveFile(canadaData);
		bnch_swt::file_loader fileLoader08{ basePath + "/CanadaData-Minified.json" };
		std::string canadaMinifiedData{ fileLoader08.operator jsonifier::string&() };
		canadaMinifiedData = parser.minifyJson(canadaData);
		fileLoader08.saveFile(canadaMinifiedData);
		bnch_swt::file_loader fileLoader09{ basePath + "/TwitterData-Prettified.json" };
		std::string twitterData{ fileLoader09.operator jsonifier::string&() };
		twitter_message twitterMessage{};
		parser.parseJson(twitterMessage, twitterData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0303: " << twitterData << std::endl;
		fileLoader09.saveFile(twitterData);
		bnch_swt::file_loader fileLoader10{ basePath + "/TwitterData-Minified.json" };
		std::string twitterMinifiedData{ fileLoader10.operator jsonifier::string&() };
		twitterMinifiedData = parser.minifyJson(twitterData);
		fileLoader10.saveFile(twitterMinifiedData);
		bnch_swt::file_loader fileLoader11{ basePath + "/CitmCatalogData-Prettified.json" };
		std::string citmCatalogData{ fileLoader11.operator jsonifier::string&() };
		citm_catalog_message citmCatalogMessage{};
		parser.parseJson(citmCatalogMessage, citmCatalogData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		//std::cout << "WERE HERE THIS IS IT 0303: " << citmCatalogData << std::endl;
		fileLoader11.saveFile(citmCatalogData);
		bnch_swt::file_loader fileLoader12{ basePath + "/CitmCatalogData-Minified.json" };
		std::string citmCatalogMinifiedData{ fileLoader12.operator jsonifier::string&() };
		citmCatalogMinifiedData = parser.minifyJson(citmCatalogData);
		fileLoader12.saveFile(citmCatalogMinifiedData);
		for (auto& value: parser.getErrors()) {
			std::cout << "PARSER ERROR: " << value << std::endl;
		}
		std::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t timeResult = std::time(nullptr);
		resultTwo			   = *localtime(&timeResult);
		std::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		jsonifier::string newerString{ section00 + newTimeString + ")\n" + section002 + section001 + section01 };

		std::string jsonifierBuffer{ jsonData };
		std::string simdjsonBuffer{ jsonData };
		std::string glazeBuffer{ jsonData };
#if !defined(ASAN_ENABLED)
		simdjson::ondemand::parser simdjsonParser{};
#endif
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Json Test (Prettified)");
#if !defined(ASAN_ENABLED)
			test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult = bnch_swt::benchmark_stage<jsonifier_internal::string_literal{ "Json-Performance" },
				bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
				runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
					try {
						getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
						bnch_swt::doNotOptimizeAway(simdjsonTestData);
					} catch (std::exception& error) {
						std::cout << "Simdjson Error: " << error.what() << std::endl;
					}
				});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();

			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Prettified)") + "-glaze.json" };
			glazeFileLoader.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Json Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoader.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Json Test (Minified)");

#if !defined(ASAN_ENABLED)
			test<test_struct> simdjsonTestData{};
			simdjsonBuffer = jsonMinifiedData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonMinifiedData;

			test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderJson{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Minified)") + "-glaze.json" };
			glazeFileLoaderJson.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonMinifiedData;
			test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Json Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Json Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderJson{ basePath + "/" + static_cast<jsonifier::string>("Json Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderJson.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section02);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = jsonData;
			jsonResults.testName = static_cast<jsonifier::string>("Abc Test (Prettified)");
#if !defined(ASAN_ENABLED)
			abc_test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonData;

			abc_test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderAbc{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Prettified)") + "-glaze.json" };
			glazeFileLoaderAbc.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonData;
			abc_test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Abc Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderAbc{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoaderAbc.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section03);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = jsonMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Abc Test (Minified)");
#if !defined(ASAN_ENABLED)
			abc_test<test_struct> simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = jsonMinifiedData;

			abc_test<test_struct> glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderAbc{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Minified)") + "-glaze.json" };
			glazeFileLoaderAbc.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = jsonMinifiedData;
			abc_test<test_struct> jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Abc Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Abc-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Abc Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Abcifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderAbc{ basePath + "/" + static_cast<jsonifier::string>("Abc Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderAbc.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section04);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			simdjsonBuffer		 = discordData;
			jsonResults.testName = static_cast<jsonifier::string>("Discord Test (Prettified)");
#if !defined(ASAN_ENABLED)
			discord_message simdjsonTestData{};
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = discordData;

			discord_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderDiscord{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Prettified)") + "-glaze.json" };
			glazeFileLoaderDiscord.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = discordData;
			discord_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Discord Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderDiscord{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoaderDiscord.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section05);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Discord Test (Minified)");
#if !defined(ASAN_ENABLED)
			discord_message simdjsonTestData{};
			simdjsonBuffer = discordMinifiedData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = discordMinifiedData;

			discord_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderDiscord{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Minified)") + "-glaze.json" };
			glazeFileLoaderDiscord.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = discordMinifiedData;
			discord_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Discord Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Discord-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Discord Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Discordifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderDiscord{ basePath + "/" + static_cast<jsonifier::string>("Discord Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderDiscord.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section06);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
			jsonResults.testName = static_cast<jsonifier::string>("Canada Test (Prettified)");
#if !defined(ASAN_ENABLED)
			canada_message simdjsonTestData{};
			resultsNew.clear();
			simdjsonBuffer = canadaData;
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = canadaData;

			canada_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderCanada{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Prettified)") + "-glaze.json" };
			glazeFileLoaderCanada.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = canadaData;
			canada_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Canada Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderCanada{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoaderCanada.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section07);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			canada_message simdjsonTestData{};
			simdjsonBuffer		 = canadaMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Canada Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = canadaMinifiedData;

			canada_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderCanada{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Minified)") + "-glaze.json" };
			glazeFileLoaderCanada.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = canadaMinifiedData;
			canada_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Canada Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Canada Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderCanada{ basePath + "/" + static_cast<jsonifier::string>("Canada Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderCanada.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section08);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			citm_catalog_message simdjsonTestData{};
			simdjsonBuffer		 = citmCatalogData;
			jsonResults.testName = static_cast<jsonifier::string>("CitmCatalog Test (Prettified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = citmCatalogData;

			citm_catalog_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderCitmCatalog{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Prettified)") + "-glaze.json" };
			glazeFileLoaderCitmCatalog.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = citmCatalogData;
			citm_catalog_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderCitmCatalog{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoaderCitmCatalog.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section09);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			citm_catalog_message simdjsonTestData{};
			simdjsonBuffer		 = citmCatalogMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("CitmCatalog Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = citmCatalogMinifiedData;

			citm_catalog_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderCitmCatalog{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Minified)") + "-glaze.json" };
			glazeFileLoaderCitmCatalog.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = citmCatalogMinifiedData;
			citm_catalog_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("CitmCatalog Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "CitmCatalog Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderCitmCatalog{ basePath + "/" + static_cast<jsonifier::string>("CitmCatalog Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderCitmCatalog.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section10);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			twitter_message simdjsonTestData{};
			simdjsonBuffer		 = twitterData;
			jsonResults.testName = static_cast<jsonifier::string>("Twitter Test (Prettified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Prettified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = twitterData;

			twitter_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = false }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = true, .minified = false }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderTwitter{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Prettified)") + "-glaze.json" };
			glazeFileLoaderTwitter.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = twitterData;
			twitter_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = false }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Twitter Test (Prettified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Prettified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderTwitter{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Prettified)") + "-jsonifier.json" };
			jsonifierFileLoaderTwitter.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section11);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		{
			jsonifier::vector<results_data> resultsNew{};
			test_results jsonResults{};
#if !defined(ASAN_ENABLED)
			twitter_message simdjsonTestData{};
			simdjsonBuffer		 = twitterMinifiedData;
			jsonResults.testName = static_cast<jsonifier::string>("Twitter Test (Minified)");
			auto simdjsonReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", simdjsonLibraryName, "cadetblue">([&]() {
						try {
							getValue(simdjsonTestData, simdjsonParser.iterate(simdjsonBuffer).value());
							bnch_swt::doNotOptimizeAway(simdjsonTestData);
						} catch (std::exception& error) {
							std::cout << "Simdjson Error: " << error.what() << std::endl;
						}
					});
			parser.serializeJson(simdjsonTestData, simdjsonBuffer);
			auto simdjsonReadSize = simdjsonBuffer.size();
			results_data simdjsonResults{ static_cast<jsonifier::string>(simdjsonLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(simdjsonCommitUrl), simdjsonReadResult.iterationCount };

			simdjsonResults.readResult = result<result_type::read>{ "cadetblue", simdjsonReadSize, simdjsonReadResult };
			bnch_swt::file_loader simdjsonFileLoader{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Minified)") + "-simdjson.json" };
			simdjsonFileLoader.saveFile(simdjsonBuffer);
			simdjsonResults.print();
			resultsNew.emplace_back(simdjsonResults);

			glazeBuffer = twitterMinifiedData;

			twitter_message glazeTestData{};
			auto glazeReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", glazeLibraryName, "dodgerblue">([&]() {
						if (auto error = glz::read<glz::opts{ .skip_null_members = false, .minified = true }>(glazeTestData, glazeBuffer); error) {
							std::cout << "Glaze Error: " << glz::format_error(error, glazeBuffer) << std::endl;
						}
						auto* newPtr = &glazeTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			results_data glazeResults{ static_cast<jsonifier::string>(glazeLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(glazeCommitUrl), glazeReadResult.iterationCount };
			std::string glazeNewerBuffer{};
			auto glazeWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Write", glazeLibraryName, "steelblue">([&]() {
						bnch_swt::doNotOptimizeAway(glz::write<glz::opts{ .skip_null_members = false, .prettify = false, .minified = true }>(glazeTestData, glazeNewerBuffer));
						auto* newPtr = &glazeNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			auto glazeReadSize		 = glazeNewerBuffer.size();
			auto glazeWrittenSize	 = glazeNewerBuffer.size();
			glazeResults.readResult	 = result<result_type::read>{ "dodgerblue", glazeReadSize, glazeReadResult };
			glazeResults.writeResult = result<result_type::write>{ "skyblue", glazeWrittenSize, glazeWriteResult };
			bnch_swt::file_loader glazeFileLoaderTwitter{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Minified)") + "-glaze.json" };
			glazeFileLoaderTwitter.saveFile(glazeBuffer);
			glazeResults.print();
			resultsNew.emplace_back(glazeResults);
#endif
			jsonifierBuffer = twitterMinifiedData;
			twitter_message jsonifierTestData{};
			auto jsonifierReadResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Read", jsonifierLibraryName, "teal">([&]() {
						parser.parseJson<jsonifier::parse_options{ .minified = true }>(jsonifierTestData, jsonifierBuffer);
						auto* newPtr = &jsonifierTestData;
						bnch_swt::doNotOptimizeAway(newPtr);
					});
			results_data jsonifierResesults{ static_cast<jsonifier::string>(jsonifierLibraryName), static_cast<jsonifier::string>("Twitter Test (Minified)"),
				static_cast<jsonifier::string>(jsonifierCommitUrl), jsonifierReadResult.iterationCount };
			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}
			std::string jsonifierNewerBuffer{};
			auto jsonifierWriteResult =
				bnch_swt::benchmark_stage<"Json-Performance", bnch_swt::bench_options{ .type = bnch_swt::result_type::time, .totalIterationCountCap = totalIterationCountCap }>::
					runBenchmark<jsonifier_internal::string_literal{ "Twitter Test (Minified)" } + "-Write", jsonifierLibraryName, "steelblue">([&]() {
						parser.serializeJson<jsonifier::serialize_options{ .prettify = false }>(jsonifierTestData, jsonifierNewerBuffer);
						auto* newPtr = &jsonifierNewerBuffer;
						bnch_swt::doNotOptimizeAway(newPtr);
					});

			for (auto& value: parser.getErrors()) {
				std::cout << "Jsonifier Error: " << value << std::endl;
			}

			auto jsonifierReadSize		   = jsonifierNewerBuffer.size();
			auto jsonifierWrittenSize	   = jsonifierNewerBuffer.size();
			jsonifierResesults.readResult  = result<result_type::read>{ "teal", jsonifierReadSize, jsonifierReadResult };
			jsonifierResesults.writeResult = result<result_type::write>{ "steelblue", jsonifierWrittenSize, jsonifierWriteResult };
			bnch_swt::file_loader jsonifierFileLoaderTwitter{ basePath + "/" + static_cast<jsonifier::string>("Twitter Test (Minified)") + "-jsonifier.json" };
			jsonifierFileLoaderTwitter.saveFile(jsonifierBuffer);
			jsonifierResesults.print();
			resultsNew.emplace_back(jsonifierResesults);

			std::string table{};
			const auto n = resultsNew.size();
			table += table_header + "\n";
			std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results_data>());
			for (size_t x = 0; x < n; ++x) {
				jsonResults.results.emplace_back(resultsNew[x]);
				table += resultsNew[x].jsonStats();
				if (x != n - 1) {
					table += "\n";
				}
			}
			jsonResults.markdownResults = table;
			auto testResults			= jsonResults;
			newerString += static_cast<jsonifier::string>(section12);
			newerString += testResults.markdownResults;
			benchmark_data.emplace_back(testResults);
		}
		auto testResults = json_tests_helper<test_type::minify, std::string, false, "Minify Test">::run(discordData);
		newerString += static_cast<jsonifier::string>(section13);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::prettify, std::string, false, "Prettify Test">::run(jsonMinifiedData);
		newerString += static_cast<jsonifier::string>(section14);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = json_tests_helper<test_type::validate, std::string, false, "Validate Test">::run(discordData);
		newerString += static_cast<jsonifier::string>(section15);
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		bnch_swt::benchmark_stage<"Json-Performance">::printResults();
		std::string resultsStringJson{};
		test_results_final resultsData{};
		for (auto& value: benchmark_data) {
			test_elements_final testElement{};
			testElement.testName = value.testName;
			for (auto& valueNew: value.results) {
				test_element_final resultFinal{};
				if (valueNew.readResult.jsonSpeed.has_value()) {
					resultFinal.libraryName	   = valueNew.name;
					resultFinal.color		   = valueNew.readResult.color;
					resultFinal.resultSpeed	   = valueNew.readResult.jsonSpeed.value();
					resultFinal.iterationCount = valueNew.readResult.iterationCount.value();
					resultFinal.resultType	   = "Read";
					testElement.results.emplace_back(resultFinal);
				}
				if (valueNew.writeResult.jsonSpeed.has_value()) {
					resultFinal.libraryName	   = valueNew.name;
					resultFinal.color		   = valueNew.writeResult.color;
					resultFinal.resultSpeed	   = valueNew.writeResult.jsonSpeed.value();
					resultFinal.iterationCount = valueNew.writeResult.iterationCount.value();
					resultFinal.resultType	   = "Write";
					testElement.results.emplace_back(resultFinal);
				}
			}
			resultsData.emplace_back(testElement);
		}
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(resultsData, resultsStringJson);
		std::cout << "Json Data: " << resultsStringJson << std::endl;
		fileLoader04.saveFile(resultsStringJson);
		fileLoader01.saveFile(newerString);
		std::cout << "Md Data: " << newerString << std::endl;
		executePythonScript(static_cast<std::string>(BASE_PATH) + "/GenerateGraphs.py", basePath + "/Results.json", static_cast<std::string>(GRAPHS_PATH));
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}