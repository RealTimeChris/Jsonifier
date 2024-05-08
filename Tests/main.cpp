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
	jsonifier::vector<jsonifier::vector<jsonifier::vector<double>>> coordinates{};
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
	jsonifier::vector<feature> features{};
	std::string type{};
};

struct search_metadata {
	std::string nextResults{};
	std::string sinceIdStr{};
	std::string refreshUrl{};
	std::string maxIdStr{};
	double completedIn{};
	std::string query{};
	int64_t sinceId{};
	int64_t count{};
	double maxId{};
};

struct hashtag {
	jsonifier::vector<int64_t> indices{};
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
	std::optional<std::string> sourceStatusIdStr{};
	std::optional<double> sourceStatusId{};
	jsonifier::vector<int64_t> indices{};
	std::string mediaUrlHttps{};
	std::string expandedUrl{};
	std::string displayUrl{};
	std::string mediaUrl{};
	std::string idStr{};
	std::string type{};
	sizes_data sizes{};
	std::string url{};
	double id{};
};

struct url_data {
	jsonifier::vector<int64_t> indices{};
	std::string expandedUrl{};
	std::string displayUrl{};
	std::string url{};
};

struct user_mention {
	jsonifier::vector<int64_t> indices{};
	std::string screenName{};
	std::string idStr{};
	std::string name{};
	int64_t id{};
};

struct status_entities {
	std::optional<jsonifier::vector<media_data>> media{};
	jsonifier::vector<user_mention> userMentions{};
	jsonifier::vector<std::string> symbols{};
	jsonifier::vector<hashtag> hashtags{};
	jsonifier::vector<url_data> urls{};
};

struct metadata_data {
	std::string isoLanguageCode{};
	std::string resultType{};
};

struct description_data {
	jsonifier::vector<url_data> urls{};
};

struct user_entities {
	std::optional<description_data> url{};
	description_data description{};
};

struct user_data_twitter {
	int64_t id{};
	std::string idStr{};
	std::string name{};
	std::string screenName{};
	std::string location{};
	std::string description{};
	std::optional<std::string> url{};
	user_entities entities{};
	bool userProtected{};
	int64_t followersCount{};
	int64_t friendsCount{};
	int64_t listedCount{};
	std::string createdAt{};
	int64_t favouritesCount{};
	std::optional<int64_t> utcOffset{};
	std::optional<std::string> timeZone{};
	bool geoEnabled{};
	bool verified{};
	int64_t statusesCount{};
	std::string lang{};
	bool contributorsEnabled{};
	bool isTranslator{};
	bool isTranslationEnabled{};
	std::string profileBackgroundColor{};
	std::string profileBackgroundImageUrl{};
	std::string profileBackgroundImageUrlHttps{};
	bool profileBackgroundTile{};
	std::string profileImageUrl{};
	std::string profileImageUrlHttps{};
	std::optional<std::string> profileBannerUrl{};
	std::string profileLinkColor{};
	std::string profileSidebarBorderColor{};
	std::string profileSidebarFillColor{};
	std::string profileTextColor{};
	bool profileUseBackgroundImage{};
	bool defaultProfile{};
	bool defaultProfileImage{};
	bool following{};
	bool followRequestSent{};
	bool notifications{};
};

struct status {
	metadata_data metadata{};
	std::string createdAt{};
	double id{};
	std::string idStr{};
	std::string text{};
	std::string source{};
	bool truncated{};
	std::optional<double> inReplyToStatusId{};
	std::optional<std::string> inReplyToStatusIdStr{};
	std::optional<int64_t> inReplyToUserId{};
	std::optional<std::string> inReplyToUserIdStr{};
	std::optional<std::string> inReplyToScreenName{};
	user_data_twitter user{};
	std::optional<std::string> geo{};
	std::optional<std::string> coordinates{};
	std::optional<std::string> place{};
	std::optional<std::string> contributors{};
	int64_t retweetCount{};
	int64_t favoriteCount{};
	status_entities entities{};
	bool favorited{};
	bool retweeted{};
	std::string lang{};
	std::shared_ptr<status> retweetedStatus{};
	std::optional<bool> possiblySensitive{};
};

struct twitter_message {
	search_metadata searchMetadata{};
	jsonifier::vector<status> statuses{};
};

struct avatar_decoration_data {
	std::string skuId{};
	std::string asset{};
};

class user_data {
  public:
	std::optional<avatar_decoration_data> avatarDecorationData{};
	std::optional<std::string> displayName{};
	std::optional<std::string> globalName{};
	std::optional<std::string> banner{};
	std::optional<std::string> avatar{};
	std::optional<std::string> locale{};
	std::string discriminator{};
	std::string userName{};
	uint64_t accentColor{};
	uint64_t premiumType{};
	uint64_t publicFlags{};
	std::string email{};
	bool mfaEnabled{};
	std::string id{};
	uint64_t flags{};
	bool verified{};
	bool system{};
	bool bot{};
};

class guild_scheduled_event_data {
  public:
	std::string scheduledStartTime{};
	std::string scheduledEndTime{};
	std::string description{};
	uint64_t entityMetadata{};
	std::string creatorId{};
	std::string channelId{};
	uint64_t privacyLevel{};
	std::string entityId{};
	std::string guildId{};
	uint64_t entityType{};
	uint64_t userCount{};
	user_data creator{};
	std::string name{};
	uint64_t status{};
	std::string id{};
};

struct tags_data {
	std::optional<std::string> premiumSubscriber{};
	std::optional<std::string> botId{};
};

class role_data {
  public:
	std::optional<std::string> unicodeEmoji{};
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

struct icon_emoji {
	std::optional<std::string> name{};
	std::optional<std::string> id{};
};

struct permission_overwrites {
	std::string allow{};
	std::string deny{};
	std::string id{};
	uint64_t type{};
};

class channel_data {
  public:
	jsonifier::vector<permission_overwrites> permissionOverwrites{};
	std::optional<std::string> lastMessageId{};
	uint64_t defaultThreadRateLimitPerUser{};
	jsonifier::vector<std::string> appliedTags{};
	std::optional<std::string> rtcRegion{};
	uint64_t defaultAutoArchiveDuration{};
	std::optional<std::string> status{};
	jsonifier::vector<user_data> recipients{};
	std::optional<std::string> topic{};
	std::string lastPinTimestamp{};
	uint64_t totalMessageSent{};
	uint64_t rateLimitPerUser{};
	uint64_t videoQualityMode{};
	std::string applicationId{};
	std::string permissions{};
	uint64_t messageCount{};
	uint64_t memberCount{};
	std::string parentId{};
	icon_emoji iconEmoji{};
	std::string ownerId{};
	std::string guildId{};
	uint64_t userLimit{};
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
	std::optional<std::string> communicationDisabledUntil{};
	std::optional<std::string> premiumSince{};
	std::optional<std::string> avatar{};
	std::optional<std::string> nick{};
	jsonifier::vector<std::string> roles{};
	std::string permissions{};
	std::string joinedAt{};
	std::string guildId{};
	user_data user{};
	uint64_t flags{};
	bool pending{};
	bool deaf{};
	bool mute{};
};

class guild_data {
  public:
	jsonifier::vector<guild_scheduled_event_data> guildScheduledEvents{};
	std::optional<std::string> latestOnBoardingQuestionId{};
	std::optional<std::string> safetyAlertsChannelId{};
	std::optional<std::string> inventorySettings{};
	std::optional<std::string> discoverySplash{};
	std::optional<std::string> applicationId{};
	std::optional<std::string> vanityUrlCode{};
	std::optional<std::string> description{};
	jsonifier::vector<guild_member_data> members{};
	uint64_t defaultMessageNotifications{};
	jsonifier::vector<std::string> voiceStates{};
	jsonifier::vector<channel_data> channels{};
	uint64_t maxStageVideoChannelUsers{};
	std::string publicUpdatesChannelId{};
	std::optional<std::string> hubType{};
	uint64_t premiumSubscriptionCount{};
	jsonifier::vector<std::string> features{};
	uint64_t approximatePresenceCount{};
	jsonifier::vector<std::string> stickers{};
	std::optional<std::string> splash{};
	jsonifier::vector<std::string> threads{};
	uint64_t approximateMemberCount{};
	bool premiumProgressBarEnabled{};
	uint64_t explicitContentFilter{};
	uint64_t maxVideoChannelUsers{};
	jsonifier::vector<role_data> roles{};
	std::string systemChannelId{};
	std::string widgetChannelId{};
	std::string preferredLocale{};
	uint64_t systemChannelFlags{};
	std::string rulesChannelId{};
	uint64_t verificationLevel{};
	std::string afkChannelId{};
	std::string permissions{};
	uint64_t maxPresences{};
	std::string discovery{};
	uint64_t memberCount{};
	std::string joinedAt{};
	uint64_t premiumTier{};
	std::string ownerId{};
	uint64_t maxMembers{};
	uint64_t afkTimeout{};
	std::string banner{};
	bool widgetEnabled{};
	std::string region{};
	uint64_t nsfwLevel{};
	uint64_t mfaLevel{};
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
	static constexpr auto parseValue = createValue<&value_type::type, &value_type::coordinates>();
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
	static constexpr auto value = object(&value_type::type, &value_type::coordinates);
};

template<> struct glz::meta<properties_data> {
	using value_type			= properties_data;
	static constexpr auto value = object(&value_type::name);
};

template<> struct glz::meta<feature> {
	using value_type			= feature;
	static constexpr auto value = object(&value_type::properties, &value_type::geometry, &value_type::type);
};

template<> struct glz::meta<canada_message> {
	using value_type			= canada_message;
	static constexpr auto value = object(&value_type::features, &value_type::type);
};
#endif

template<> struct jsonifier::core<search_metadata> {
	using value_type = search_metadata;
	static constexpr auto parseValue =
		createValue("completed_in", &value_type::completedIn, "max_id", &value_type::maxId, "max_id_str", &value_type::maxIdStr, "next_results", &value_type::nextResults, "query",
			&value_type::query, "refresh_url", &value_type::refreshUrl, "count", &value_type::count, "since_id", &value_type::sinceId, "since_id_str", &value_type::sinceIdStr);
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
	static constexpr auto parseValue = createValue("medium", &value_type::medium, "small", &value_type::small, "thumb", &value_type::thumb, "large", &value_type::large);
};

template<> struct jsonifier::core<media_data> {
	using value_type				 = media_data;
	static constexpr auto parseValue = createValue("id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices, "media_url", &value_type::mediaUrl,
		"media_url_https", &value_type::mediaUrlHttps, "url", &value_type::url, "display_url", &value_type::displayUrl, "expanded_url", &value_type::expandedUrl, "type",
		&value_type::type, "sizes", &value_type::sizes, "source_status_id", &value_type::sourceStatusId, "source_status_id_str", &value_type::sourceStatusIdStr);
};

template<> struct jsonifier::core<url_data> {
	using value_type = url_data;
	static constexpr auto parseValue =
		createValue("url", &value_type::url, "expanded_url", &value_type::expandedUrl, "display_url", &value_type::displayUrl, "indices", &value_type::indices);
};

template<> struct jsonifier::core<user_mention> {
	using value_type = user_mention;
	static constexpr auto parseValue =
		createValue("screen_name", &value_type::screenName, "name", &value_type::name, "id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices);
};

template<> struct jsonifier::core<status_entities> {
	using value_type = status_entities;
	static constexpr auto parseValue =
		createValue("hashtags", &value_type::hashtags, "symbols", &value_type::symbols, "urls", &value_type::urls, "user_mentions", &value_type::userMentions);
};

template<> struct jsonifier::core<metadata_data> {
	using value_type				 = metadata_data;
	static constexpr auto parseValue = createValue("result_type", &value_type::resultType, "iso_language_code", &value_type::isoLanguageCode);
};

template<> struct jsonifier::core<description_data> {
	using value_type				 = description_data;
	static constexpr auto parseValue = createValue("urls", &value_type::urls);
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue("description", &value_type::description, "url", &value_type::url);
};

template<> struct jsonifier::core<user_data_twitter> {
	using value_type				 = user_data_twitter;
	static constexpr auto parseValue = createValue("id", &value_type::id, "id_str", &value_type::idStr, "name", &value_type::name, "screen_name", &value_type::screenName,
		"location", &value_type::location, "description", &value_type::description, "url", &value_type::url, "entities", &value_type::entities, "protected",
		&value_type::userProtected, "followers_count", &value_type::followersCount, "friends_count", &value_type::friendsCount, "listed_count", &value_type::listedCount,
		"created_at", &value_type::createdAt, "favourites_count", &value_type::favouritesCount, "utc_offset", &value_type::utcOffset, "time_zone", &value_type::timeZone,
		"geo_enabled", &value_type::geoEnabled, "verified", &value_type::verified, "statuses_count", &value_type::statusesCount, "lang", &value_type::lang, "contributors_enabled",
		&value_type::contributorsEnabled, "is_translator", &value_type::isTranslator, "is_translation_enabled", &value_type::isTranslationEnabled, "profile_background_color",
		&value_type::profileBackgroundColor, "profile_background_image_url", &value_type::profileBackgroundImageUrl, "profile_background_image_url_https",
		&value_type::profileBackgroundImageUrlHttps, "profile_background_tile", &value_type::profileBackgroundTile, "profile_image_url", &value_type::profileImageUrl,
		"profile_image_url_https", &value_type::profileImageUrlHttps, "profile_banner_url", &value_type::profileBannerUrl, "profile_link_color", &value_type::profileLinkColor,
		"profile_sidebar_border_color", &value_type::profileSidebarBorderColor, "profile_sidebar_fill_color", &value_type::profileSidebarFillColor, "profile_text_color",
		&value_type::profileTextColor, "profile_use_background_image", &value_type::profileUseBackgroundImage, "default_profile", &value_type::defaultProfile,
		"default_profile_image", &value_type::defaultProfileImage, "following", &value_type::following, "follow_request_sent", &value_type::followRequestSent, "notifications",
		&value_type::notifications);
};

template<> struct jsonifier::core<status> {
	using value_type				 = status;
	static constexpr auto parseValue = createValue("metadata", &value_type::metadata, "created_at", &value_type::createdAt, "id", &value_type::id, "id_str", &value_type::idStr,
		"text", &value_type::text, "source", &value_type::source, "truncated", &value_type::truncated, "in_reply_to_status_id", &value_type::inReplyToStatusId,
		"in_reply_to_status_id_str", &value_type::inReplyToStatusIdStr, "in_reply_to_user_id", &value_type::inReplyToUserId, "in_reply_to_user_id_str",
		&value_type::inReplyToUserIdStr, "in_reply_to_screen_name", &value_type::inReplyToScreenName, "user", &value_type::user, "geo", &value_type::geo, "coordinates",
		&value_type::coordinates, "place", &value_type::place, "contributors", &value_type::contributors, "retweet_count", &value_type::retweetCount, "favorite_count",
		&value_type::favoriteCount, "entities", &value_type::entities, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "lang", &value_type::lang,
		"possibly_sensitive", &value_type::possiblySensitive);
};

template<> struct jsonifier::core<twitter_message> {
	using value_type				 = twitter_message;
	static constexpr auto parseValue = createValue("statuses", &value_type::statuses, "search_metadata", &value_type::searchMetadata);
};

#if !defined(ASAN)
template<> struct glz::meta<search_metadata> {
	using value_type = search_metadata;
	static constexpr auto value =
		object("completed_in", &value_type::completedIn, "max_id", &value_type::maxId, "max_id_str", &value_type::maxIdStr, "next_results", &value_type::nextResults, "query",
			&value_type::query, "refresh_url", &value_type::refreshUrl, "count", &value_type::count, "since_id", &value_type::sinceId, "since_id_str", &value_type::sinceIdStr);
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
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices, "media_url", &value_type::mediaUrl,
		"media_url_https", &value_type::mediaUrlHttps, "url", &value_type::url, "display_url", &value_type::displayUrl, "expanded_url", &value_type::expandedUrl, "type",
		&value_type::type, "sizes", &value_type::sizes, "source_status_id", &value_type::sourceStatusId, "source_status_id_str", &value_type::sourceStatusIdStr);
};

template<> struct glz::meta<url_data> {
	using value_type = url_data;
	static constexpr auto value =
		object("url", &value_type::url, "expanded_url", &value_type::expandedUrl, "display_url", &value_type::displayUrl, "indices", &value_type::indices);
};

template<> struct glz::meta<user_mention> {
	using value_type = user_mention;
	static constexpr auto value =
		object("screen_name", &value_type::screenName, "name", &value_type::name, "id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices);
};

template<> struct glz::meta<status_entities> {
	using value_type			= status_entities;
	static constexpr auto value = object("hashtags", &value_type::hashtags, "symbols", &value_type::symbols, "urls", &value_type::urls, "user_mentions", &value_type::userMentions);
};

template<> struct glz::meta<metadata_data> {
	using value_type			= metadata_data;
	static constexpr auto value = object("result_type", &value_type::resultType, "iso_language_code", &value_type::isoLanguageCode);
};

template<> struct glz::meta<description_data> {
	using value_type			= description_data;
	static constexpr auto value = object("urls", &value_type::urls);
};

template<> struct glz::meta<user_entities> {
	using value_type			= user_entities;
	static constexpr auto value = object("description", &value_type::description, "url", &value_type::url);
};

template<> struct glz::meta<user_data_twitter> {
	using value_type			= user_data_twitter;
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::idStr, "name", &value_type::name, "screen_name", &value_type::screenName, "location",
		&value_type::location, "description", &value_type::description, "url", &value_type::url, "entities", &value_type::entities, "protected", &value_type::userProtected,
		"followers_count", &value_type::followersCount, "friends_count", &value_type::friendsCount, "listed_count", &value_type::listedCount, "created_at", &value_type::createdAt,
		"favourites_count", &value_type::favouritesCount, "utc_offset", &value_type::utcOffset, "time_zone", &value_type::timeZone, "geo_enabled", &value_type::geoEnabled,
		"verified", &value_type::verified, "statuses_count", &value_type::statusesCount, "lang", &value_type::lang, "contributors_enabled", &value_type::contributorsEnabled,
		"is_translator", &value_type::isTranslator, "is_translation_enabled", &value_type::isTranslationEnabled, "profile_background_color", &value_type::profileBackgroundColor,
		"profile_background_image_url", &value_type::profileBackgroundImageUrl, "profile_background_image_url_https", &value_type::profileBackgroundImageUrlHttps,
		"profile_background_tile", &value_type::profileBackgroundTile, "profile_image_url", &value_type::profileImageUrl, "profile_image_url_https",
		&value_type::profileImageUrlHttps, "profile_banner_url", &value_type::profileBannerUrl, "profile_link_color", &value_type::profileLinkColor, "profile_sidebar_border_color",
		&value_type::profileSidebarBorderColor, "profile_sidebar_fill_color", &value_type::profileSidebarFillColor, "profile_text_color", &value_type::profileTextColor,
		"profile_use_background_image", &value_type::profileUseBackgroundImage, "default_profile", &value_type::defaultProfile, "default_profile_image",
		&value_type::defaultProfileImage, "following", &value_type::following, "follow_request_sent", &value_type::followRequestSent, "notifications", &value_type::notifications);
};

template<> struct glz::meta<status> {
	using value_type			= status;
	static constexpr auto value = object("metadata", &value_type::metadata, "created_at", &value_type::createdAt, "id", &value_type::id, "id_str", &value_type::idStr, "text",
		&value_type::text, "source", &value_type::source, "truncated", &value_type::truncated, "in_reply_to_status_id", &value_type::inReplyToStatusId, "in_reply_to_status_id_str",
		&value_type::inReplyToStatusIdStr, "in_reply_to_user_id", &value_type::inReplyToUserId, "in_reply_to_user_id_str", &value_type::inReplyToUserIdStr,
		"in_reply_to_screen_name", &value_type::inReplyToScreenName, "user", &value_type::user, "geo", &value_type::geo, "coordinates", &value_type::coordinates, "place",
		&value_type::place, "contributors", &value_type::contributors, "retweet_count", &value_type::retweetCount, "favorite_count", &value_type::favoriteCount, "entities",
		&value_type::entities, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "lang", &value_type::lang, "retweeted_status", glz::skip{},
		"possibly_sensitive", &value_type::possiblySensitive);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("statuses", &value_type::statuses, "search_metadata", &value_type::searchMetadata);
};
#endif

template<> struct jsonifier::core<avatar_decoration_data> {
	using value_type				 = avatar_decoration_data;
	static constexpr auto parseValue = createValue("asset", &value_type::asset, "sku_id", &value_type::skuId);
};

template<> struct jsonifier::core<tags_data> {
	using value_type				 = tags_data;
	static constexpr auto parseValue = createValue("bot_id", &value_type::botId, "premium_subscriber", &value_type::premiumSubscriber);
};

template<> struct jsonifier::core<user_data> {
	using value_type = user_data;
	static constexpr auto parseValue =
		createValue("id", &value_type::id, "username", &value_type::userName, "discriminator", &value_type::discriminator, "bot", &value_type::bot, "system", &value_type::system,
			"mfa_enabled", &value_type::mfaEnabled, "accentColor", &value_type::accentColor, "locale", &value_type::locale, "verified", &value_type::verified, "email",
			&value_type::email, "flags", &value_type::flags, "premium_type", &value_type::premiumType, "public_flags", &value_type::publicFlags, "global_name",
			&value_type::globalName, "display_name", &value_type::displayName, "avatar_decoration_data", &value_type::avatarDecorationData, "avatar", &value_type::avatar);
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue("unicode_emoji", &value_type::unicodeEmoji, "icon", &value_type::icon, "tags", &value_type::tags, "permissions",
		&value_type::permissions, "version", &value_type::version, "position", &value_type::position, "name", &value_type::name, "mentionable", &value_type::mentionable, "color",
		&value_type::color, "id", &value_type::id, "flags", &value_type::flags, "managed", &value_type::managed, "hoist", &value_type::hoist);
};

template<> struct jsonifier::core<guild_member_data> {
	using value_type = guild_member_data;
	static constexpr auto parseValue =
		createValue("user", &value_type::user, "roles", &value_type::roles, "joined_at", &value_type::joinedAt, "deaf", &value_type::deaf, "mute", &value_type::mute, "flags",
			&value_type::flags, "pending", &value_type::pending, "permissions", &value_type::permissions, "guild_id", &value_type::guildId, &value_type::guildId, "premium_since",
			&value_type::premiumSince, "avatar", &value_type::avatar, "nick", &value_type::nick, "communication_disabled_until", &value_type::communicationDisabledUntil);
};

template<> struct jsonifier::core<icon_emoji> {
	using value_type				 = icon_emoji;
	static constexpr auto parseValue = createValue("name", &value_type::name, "id", &value_type::id);
};

template<> struct jsonifier::core<permission_overwrites> {
	using value_type				 = permission_overwrites;
	static constexpr auto parseValue = createValue("allow", &value_type::allow, "deny", &value_type::deny, "id", &value_type::id, "type", &value_type::type);
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue = createValue("default_thread_rate_limit_per_user", &value_type::defaultThreadRateLimitPerUser, "applied_tags", &value_type::appliedTags,
		"default_auto_archive_duration", &value_type::defaultAutoArchiveDuration, "recipients", &value_type::recipients, "topic", &value_type::topic, "last_pin_timestamp",
		&value_type::lastPinTimestamp, "total_message_sent", &value_type::totalMessageSent, "rate_limit_per_user", &value_type::rateLimitPerUser, "video_quality_mode",
		&value_type::videoQualityMode, "last_message_id", &value_type::lastMessageId, "application_id", &value_type::applicationId, "permissions", &value_type::permissions,
		"rtc_region", &value_type::rtcRegion, "message_count", &value_type::messageCount, "member_count", &value_type::memberCount, "parent_id", &value_type::parentId,
		"icon_emoji", &value_type::iconEmoji, "owner_id", &value_type::ownerId, "guild_id", &value_type::guildId, "user_limit", &value_type::userLimit, "position",
		&value_type::position, "version", &value_type::version, "bitrate", &value_type::bitrate, "name", &value_type::name, "icon", &value_type::icon, "id", &value_type::id,
		"flags", &value_type::flags, "type", &value_type::type, "managed", &value_type::managed, "nsfw", &value_type::nsfw, "permission_overwrites",
		&value_type::permissionOverwrites, "status", &value_type::status);
};

template<> struct jsonifier::core<guild_scheduled_event_data> {
	using value_type				 = guild_scheduled_event_data;
	static constexpr auto parseValue = createValue("id", &value_type::id, "privacy_level", &value_type::privacyLevel, "entity_metadata", &value_type::entityMetadata, "entity_type",
		&value_type::entityType, "status", &value_type::status, "scheduled_start_time", &value_type::scheduledStartTime, "scheduled_end_time", &value_type::scheduledEndTime,
		"creator_id", &value_type::creatorId, "channel_id", &value_type::channelId, "entity_id", &value_type::entityId, "user_count", &value_type::userCount, "guild_id",
		&value_type::guildId, "creator", &value_type::creator, "name", &value_type::name);
};

template<> struct jsonifier::core<guild_data> {
	using value_type				 = guild_data;
	static constexpr auto parseValue = createValue("default_message_notifications", &value_type::defaultMessageNotifications, "guild_scheduled_events",
		&value_type::guildScheduledEvents, "explicit_content_filter", &value_type::explicitContentFilter, "system_channel_flags", &value_type::systemChannelFlags, "widget_enabled",
		&value_type::widgetEnabled, "unavailable", &value_type::unavailable, "owner", &value_type::owner, "large", &value_type::large, "member_count", &value_type::memberCount,
		"verification_level", &value_type::verificationLevel, "id", &value_type::id, "channels", &value_type::channels, "roles", &value_type::roles, "members",
		&value_type::members, "owner_id", &value_type::ownerId, "permissions", &value_type::permissions, "features", &value_type::features, "max_stage_video_channel_users",
		&value_type::maxStageVideoChannelUsers, "premium_subscription_count", &value_type::premiumSubscriptionCount, "approximate_presence_count",
		&value_type::approximatePresenceCount, "approximate_member_count", &value_type::approximateMemberCount, "premium_progress_bar_enabled",
		&value_type::premiumProgressBarEnabled, "max_video_channel_users", &value_type::maxVideoChannelUsers, "preferred_locale", &value_type::preferredLocale, "system_channel_id",
		&value_type::systemChannelId, "widget_channel_id", &value_type::widgetChannelId, "nsfw_level", &value_type::nsfwLevel, "premium_tier", &value_type::premiumTier,
		"afk_timeout", &value_type::afkTimeout, "max_members", &value_type::maxMembers, "mfa_level", &value_type::mfaLevel, "name", &value_type::name, "icon", &value_type::icon,
		"lazy", &value_type::lazy, "joined_at", &value_type::joinedAt, "discovery_splash", &value_type::discoverySplash, "hub_type", &value_type::hubType,
		"safety_alerts_channel_id", &value_type::safetyAlertsChannelId, "voice_states", &value_type::voiceStates, "nsfw", &value_type::nsfw, "latest_onboarding_question_id",
		&value_type::latestOnBoardingQuestionId, "threads", &value_type::threads, "region", &value_type::region, "description", &value_type::description, "inventory_settings",
		&value_type::inventorySettings, "stickers", &value_type::stickers, "splash", &value_type::splash, "public_updates_channel_id", &value_type::publicUpdatesChannelId,
		"rules_channel_id", &value_type::rulesChannelId, "application_id", &value_type::applicationId, "vanity_url_code", &value_type::vanityUrlCode);
};

template<> struct jsonifier::core<discord_message> {
	using value_type				 = discord_message;
	static constexpr auto parseValue = createValue("t", &value_type::t, "s", &value_type::s, "op", &value_type::op, "d", &value_type::d);
};
#if !defined(ASAN)
template<> struct glz::meta<avatar_decoration_data> {
	using value_type			= avatar_decoration_data;
	static constexpr auto value = object("asset", &value_type::asset, "sku_id", &value_type::skuId);
};

template<> struct glz::meta<tags_data> {
	using value_type			= tags_data;
	static constexpr auto value = object("bot_id", &value_type::botId, "premium_subscriber", &value_type::premiumSubscriber);
};

template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object("id", &value_type::id, "username", &value_type::userName, "discriminator", &value_type::discriminator, "bot", &value_type::bot, "system",
		&value_type::system, "mfa_enabled", &value_type::mfaEnabled, "accentColor", &value_type::accentColor, "locale", &value_type::locale, "verified", &value_type::verified,
		"email", &value_type::email, "flags", &value_type::flags, "premium_type", &value_type::premiumType, "public_flags", &value_type::publicFlags, "global_name",
		&value_type::globalName, "display_name", &value_type::displayName, "avatar_decoration_data", &value_type::avatarDecorationData, "avatar", &value_type::avatar);
};

template<> struct glz::meta<role_data> {
	using value_type			= role_data;
	static constexpr auto value = object("unicode_emoji", &value_type::unicodeEmoji, "icon", &value_type::icon, "tags", &value_type::tags, "permissions", &value_type::permissions,
		"version", &value_type::version, "position", &value_type::position, "name", &value_type::name, "mentionable", &value_type::mentionable, "color", &value_type::color, "id",
		&value_type::id, "flags", &value_type::flags, "managed", &value_type::managed, "hoist", &value_type::hoist);
};

template<> struct glz::meta<guild_member_data> {
	using value_type = guild_member_data;
	static constexpr auto value =
		object("user", &value_type::user, "roles", &value_type::roles, "joined_at", &value_type::joinedAt, "deaf", &value_type::deaf, "mute", &value_type::mute, "flags",
			&value_type::flags, "pending", &value_type::pending, "permissions", &value_type::permissions, "guild_id", &value_type::guildId, &value_type::guildId, "premium_since",
			&value_type::premiumSince, "avatar", &value_type::avatar, "nick", &value_type::nick, "communication_disabled_until", &value_type::communicationDisabledUntil);
};

template<> struct glz::meta<icon_emoji> {
	using value_type			= icon_emoji;
	static constexpr auto value = object("name", &value_type::name, "id", &value_type::id);
};

template<> struct glz::meta<permission_overwrites> {
	using value_type			= permission_overwrites;
	static constexpr auto value = object("allow", &value_type::allow, "deny", &value_type::deny, "id", &value_type::id, "type", &value_type::type);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object("default_thread_rate_limit_per_user", &value_type::defaultThreadRateLimitPerUser, "applied_tags", &value_type::appliedTags,
		"default_auto_archive_duration", &value_type::defaultAutoArchiveDuration, "recipients", &value_type::recipients, "topic", &value_type::topic, "last_pin_timestamp",
		&value_type::lastPinTimestamp, "total_message_sent", &value_type::totalMessageSent, "rate_limit_per_user", &value_type::rateLimitPerUser, "video_quality_mode",
		&value_type::videoQualityMode, "last_message_id", &value_type::lastMessageId, "application_id", &value_type::applicationId, "permissions", &value_type::permissions,
		"rtc_region", &value_type::rtcRegion, "message_count", &value_type::messageCount, "member_count", &value_type::memberCount, "parent_id", &value_type::parentId,
		"icon_emoji", &value_type::iconEmoji, "owner_id", &value_type::ownerId, "guild_id", &value_type::guildId, "user_limit", &value_type::userLimit, "position",
		&value_type::position, "version", &value_type::version, "bitrate", &value_type::bitrate, "name", &value_type::name, "icon", &value_type::icon, "id", &value_type::id,
		"flags", &value_type::flags, "type", &value_type::type, "managed", &value_type::managed, "nsfw", &value_type::nsfw, "permission_overwrites",
		&value_type::permissionOverwrites, "status", &value_type::status);
};

template<> struct glz::meta<guild_scheduled_event_data> {
	using value_type			= guild_scheduled_event_data;
	static constexpr auto value = object("id", &value_type::id, "privacy_level", &value_type::privacyLevel, "entity_metadata", &value_type::entityMetadata, "entity_type",
		&value_type::entityType, "status", &value_type::status, "scheduled_start_time", &value_type::scheduledStartTime, "scheduled_end_time", &value_type::scheduledEndTime,
		"creator_id", &value_type::creatorId, "channel_id", &value_type::channelId, "entity_id", &value_type::entityId, "user_count", &value_type::userCount, "guild_id",
		&value_type::guildId, "creator", &value_type::creator, "name", &value_type::name);
};

template<> struct glz::meta<guild_data> {
	using value_type			= guild_data;
	static constexpr auto value = object("default_message_notifications", &value_type::defaultMessageNotifications, "guild_scheduled_events", &value_type::guildScheduledEvents,
		"explicit_content_filter", &value_type::explicitContentFilter, "system_channel_flags", &value_type::systemChannelFlags, "widget_enabled", &value_type::widgetEnabled,
		"unavailable", &value_type::unavailable, "owner", &value_type::owner, "large", &value_type::large, "member_count", &value_type::memberCount, "verification_level",
		&value_type::verificationLevel, "id", &value_type::id, "channels", &value_type::channels, "roles", &value_type::roles, "members", &value_type::members, "owner_id",
		&value_type::ownerId, "permissions", &value_type::permissions, "features", &value_type::features, "max_stage_video_channel_users", &value_type::maxStageVideoChannelUsers,
		"premium_subscription_count", &value_type::premiumSubscriptionCount, "approximate_presence_count", &value_type::approximatePresenceCount, "approximate_member_count",
		&value_type::approximateMemberCount, "premium_progress_bar_enabled", &value_type::premiumProgressBarEnabled, "max_video_channel_users", &value_type::maxVideoChannelUsers,
		"preferred_locale", &value_type::preferredLocale, "system_channel_id", &value_type::systemChannelId, "widget_channel_id", &value_type::widgetChannelId, "nsfw_level",
		&value_type::nsfwLevel, "premium_tier", &value_type::premiumTier, "afk_timeout", &value_type::afkTimeout, "max_members", &value_type::maxMembers, "mfa_level",
		&value_type::mfaLevel, "name", &value_type::name, "icon", &value_type::icon, "lazy", &value_type::lazy, "joined_at", &value_type::joinedAt, "discovery_splash",
		&value_type::discoverySplash, "hub_type", &value_type::hubType, "safety_alerts_channel_id", &value_type::safetyAlertsChannelId, "voice_states", &value_type::voiceStates,
		"nsfw", &value_type::nsfw, "latest_onboarding_question_id", &value_type::latestOnBoardingQuestionId, "presences", glz::skip{}, "threads", &value_type::threads, "region",
		&value_type::region, "description", &value_type::description, "inventory_settings", &value_type::inventorySettings, "stickers", &value_type::stickers, "splash",
		&value_type::splash, "public_updates_channel_id", &value_type::publicUpdatesChannelId, "rules_channel_id", &value_type::rulesChannelId, "application_id",
		&value_type::applicationId, "vanity_url_code", &value_type::vanityUrlCode);
};

template<> struct glz::meta<discord_message> {
	using value_type			= discord_message;
	static constexpr auto value = object("t", &value_type::t, "s", &value_type::s, "op", &value_type::op, "d", &value_type::d);
};
#endif

struct test_struct {
	jsonifier::vector<bool> jsonifierTestBools{};
	jsonifier::vector<std::string> testStrings{};
	jsonifier::vector<bool> glazeTestBools{};
	jsonifier::vector<uint64_t> testUints{};
	jsonifier::vector<double> testDoubles{};
	jsonifier::vector<int64_t> testInts{};
};

namespace fs = std::filesystem;

class file_loader {
  public:
	file_loader(const jsonifier::string& filePathNew) {
		filePath = filePathNew;
		std::string directory{ filePathNew.substr(0, filePathNew.findLastOf("/")) };
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

	void saveFile(const jsonifier::string& fileToSave) {
		std::ofstream theStream(filePath.data(), std::ios::binary | std::ios::out | std::ios::trunc);
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

template<typename value_type> struct Test {
	jsonifier::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

template<typename value_type> struct test_generator {
	jsonifier::vector<value_type> a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;

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
					v[x].jsonifierTestBools.emplace_back(newBool);
					v[x].glazeTestBools.emplace_back(newBool);
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
	jsonifier::vector<test_element_final> results{};
	std::string testName{};
};

using test_results_final = jsonifier::vector<test_elements_final>;

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
	jsonifier::vector<value_type> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using value_type				 = test_struct;
	static constexpr auto parseValue = createValue("testBools", &value_type::jsonifierTestBools, "testInts", &value_type::testInts, "testUints", &value_type::testUints,
		"testDoubles", &value_type::testDoubles, "testStrings", &value_type::testStrings);
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
	static constexpr auto value = object("testBools", &value_type::glazeTestBools, "testInts", &value_type::testInts, "testUints", &value_type::testUints, "testDoubles",
		&value_type::testDoubles, "testStrings", &value_type::testStrings);
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
constexpr uint64_t iterationsVal = 10;
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
			std::cout << enumToString<result_type::read>() + " Length: " << readResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::read>() + " Speed: " << readResult.jsonSpeed.value() << " MB/s\n";
		}
		if (writeResult.byteLength.has_value() && writeResult.jsonSpeed.has_value()) {
			std::cout << enumToString<result_type::write>() + " Length: " << writeResult.byteLength.value() << std::endl;
			std::cout << enumToString<result_type::write>() + " Speed: " << writeResult.jsonSpeed.value() << " MB/s\n";
		}
		std::cout << "\n---" << std::endl;
	}

	std::string jsonStats() const {
		std::string write{};
		std::string read{};
		std::string finalstring{};
		if (readResult.jsonTime.has_value() && readResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << readResult.jsonSpeed.value();
			read = stream01.str();
		} else {
			read = "N/A";
		}
		if (writeResult.jsonTime.has_value() && writeResult.byteLength.has_value()) {
			std::stringstream stream01{};
			stream01 << writeResult.jsonSpeed.value();
			write = stream01.str();
		} else {
			write = "N/A";
		}

		finalstring = std::string{ "| [" + name + "](" + url + ") | " + read + " | " + write + " |" };
		return finalstring;
	}
};

struct test_results {
	jsonifier::vector<results_data> results{};
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
	jsonifier::vector<double> iterations{};
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
	parse_and_serialize_single = 1,
	minify					   = 2,
	prettify				   = 3,
	validate				   = 4,
};

static constexpr jsonifier::string_view basePath{ JSON_PATH };

template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto jsonifierTest(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	auto iterationsVal = type == test_type::parse_and_serialize_single ? 1 : iterations;

	results_data r{ "jsonifier", testName, "https://github.com/realtimechris/jsonifier", iterationsVal };
	test_data_type testData{};
	jsonifier::jsonifier_core parser{};
	auto readResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				auto result = parser.parseJson<jsonifier::parse_options{ .minified = minified }>(testData, buffer);
			}
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	auto readSize = buffer.size();
	buffer.clear();
	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				auto result = parser.serializeJson<jsonifier::serialize_options{ .prettify = !minified }>(testData, buffer);
			}
		},
		iterations);
	auto writtenSize = buffer.size();
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.readResult  = result<result_type::read>{ "teal", readSize * iterationsVal, readResult };
	r.writeResult = result<result_type::write>{ "steelblue", writtenSize * iterationsVal, writeResult };
	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::prettify, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "jsonifier", "Prettify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };
	jsonifier::jsonifier_core parser{};

	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				newerBuffer = parser.prettifyJson(buffer);
			}
		},
		iterationsVal);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size() * iterationsVal, writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::minify, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };

	results_data r{ "jsonifier", "Minify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };

	jsonifier::jsonifier_core parser{};
	std::string newerBuffer{};
	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				newerBuffer = parser.minifyJson(buffer);
			}
		},
		iterationsVal);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	file_loader fileLoader{ basePath + "/" + testName + "-jsonifier.json" };
	fileLoader.saveFile(newerBuffer);

	r.writeResult = result<result_type::write>{ "steelblue", newerBuffer.size() * iterationsVal, writeResult };

	if (doWePrint) {
		r.print();
	}
	return r;
}

template<> auto jsonifierTest<test_type::validate, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };

	results_data r{ "jsonifier", "Validation Test", "https://github.com/realtimechris/jsonifier", iterationsVal };
	jsonifier::jsonifier_core parser{};

	auto readResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				auto result = parser.validateJson(buffer);
			}
		},
		iterationsVal);

	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.readResult = result<result_type::read>{ "teal", buffer.size() * iterationsVal, readResult };

	if (doWePrint) {
		r.print();
	}


	return r;
}

#if !defined(ASAN)
template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto glazeTest(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	auto iterationsVal = type == test_type::parse_and_serialize_single ? 1 : iterations;

	results_data r{ "glaze", testName, "https://github.com/stephenberry/glaze", iterations };
	test_data_type testData{};
	auto readResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				auto result = glz::read<glz::opts{ .minified = minified }>(testData, buffer);
				if (result) {
					std::cout << "Glaze Error: " << glz::format_error(result, buffer) << std::endl;
				}
			}
		},
		iterations);
	auto readSize = buffer.size();
	buffer.clear();
	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				glz::write<glz::opts{ .prettify = !minified }>(testData, buffer);
			}
		},
		iterations);

	auto writtenSize = buffer.size();
	r.readResult	 = result<result_type::read>{ "dodgerblue", readSize * iterationsVal, readResult };
	r.writeResult	 = result<result_type::write>{ "skyblue", writtenSize * iterationsVal, writeResult };
	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto glazeTest<test_type::prettify, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "glaze", "Prettify Test", "https://github.com/stephenberry/glaze", iterationsVal };

	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				newerBuffer = glz::prettify_json(buffer);
			}
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size() * iterationsVal, writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto glazeTest<test_type::minify, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "glaze", "Minify Test", "https://github.com/stephenberry/glaze", iterationsVal };

	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				newerBuffer = glz::minify_json(buffer);
			}
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-glaze.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "skyblue", newerBuffer.size() * iterationsVal, writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}

	#include "simdjson.h"

using namespace simdjson;

template<typename value_type> value_type getValue(simdjson::ondemand::value value) {
	value_type result;
	if (!value.get(result)) {
		return result;
	}
	return value_type{};
}
 
template<jsonifier::concepts::vector_t value_type> value_type getValue(simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	value_type returnValues{};
	if (!value.get(result)) {
		for (auto iter = result.begin(); iter != result.end(); ++iter) {
			returnValues.emplace_back(getValue<typename value_type::value_type>(iter.operator*().value()));
		}
	}

	return returnValues;
}

template<jsonifier::concepts::optional_t value_type> value_type getValue(simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	value_type returnValues{};
	if (!value.get(result)) {
		for (auto iter = result.begin(); iter != result.end(); ++iter) {
			returnValues.emplace(getValue<typename value_type::value_type>(iter.operator*().value()));
		}
	}

	return returnValues;
}

template<> std::string getValue<std::string>(simdjson::ondemand::value value) {
	std::string_view result;
	if (!value.get(result)) {
		return static_cast<std::string>(result);
	}
	return std ::string{};
}

template<typename value_type> void getValue(value_type& valueNew, simdjson::ondemand::value value) {
	valueNew = getValue<value_type>(value);
}

template<jsonifier::concepts::vector_t value_type> void getValue(value_type& returnValues, simdjson::ondemand::value value) {
	simdjson::ondemand::array result;
	if (!value.get(result)) {
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

template<typename value_type> value_type getValue(simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value result;
	if (!value[key].get(result)) {
		return getValue<value_type>(result);
	}
	return value_type{};
}

template<typename value_type> void getValue(jsonifier::vector<value_type>& returnValues, simdjson::ondemand::value value, const std::string& key) {
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

template<> geometry_data getValue<geometry_data>(simdjson::ondemand::value jsonDataNew) {
	geometry_data metadata;
	metadata.type = getValue<std::string>(jsonDataNew, "type");
	getValue<jsonifier::vector<jsonifier::vector<double>>>(metadata.coordinates, jsonDataNew, "coordinates");
	return metadata;
}

template<> properties_data getValue<properties_data>(simdjson::ondemand::value jsonDataNew) {
	properties_data metadata;
	metadata.name = getValue<std::string>(jsonDataNew, "name");
	return metadata;
}

template<> feature getValue<feature>(simdjson::ondemand::value jsonDataNew) {
	feature metadata;
	metadata.properties = getValue<properties_data>(jsonDataNew, "properties");
	metadata.geometry	= getValue<geometry_data>(jsonDataNew, "geometry");
	metadata.type		= getValue<std::string>(jsonDataNew, "type");
	return metadata;
}

template<> canada_message getValue<canada_message>(simdjson::ondemand::value jsonDataNew) {
	canada_message metadata;
	getValue<feature>(metadata.features, jsonDataNew, "features");
	metadata.type = getValue<std::string>(jsonDataNew, "type");
	return metadata;
}

template<> search_metadata getValue<search_metadata>(simdjson::ondemand::value jsonDataNew) {
	search_metadata metadata;
	metadata.completedIn = getValue<double>(jsonDataNew, "completed_in");
	metadata.maxId		 = getValue<int64_t>(jsonDataNew, "max_id");
	metadata.maxIdStr	 = getValue<std::string>(jsonDataNew, "max_id_str");
	metadata.nextResults = getValue<std::string>(jsonDataNew, "next_results");
	metadata.query		 = getValue<std::string>(jsonDataNew, "query");
	metadata.refreshUrl	 = getValue<std::string>(jsonDataNew, "refresh_url");
	metadata.count		 = getValue<int64_t>(jsonDataNew, "count");
	metadata.sinceId	 = getValue<int64_t>(jsonDataNew, "since_id");
	metadata.sinceIdStr	 = getValue<std::string>(jsonDataNew, "since_id_str");

	return metadata;
}

template<> hashtag getValue<hashtag>(simdjson::ondemand::value jsonDataNew) {
	hashtag tag;

	tag.text = getValue<std::string>(jsonDataNew, "text");
	getValue<int64_t>(tag.indices, jsonDataNew, "indices");

	return tag;
}

template<> large_data getValue<large_data>(simdjson::ondemand::value jsonDataNew) {
	large_data data;

	data.w		= getValue<int64_t>(jsonDataNew, "w");
	data.h		= getValue<int64_t>(jsonDataNew, "h");
	data.resize = getValue<std::string>(jsonDataNew, "resize");

	return data;
}

template<> sizes_data getValue<sizes_data>(simdjson::ondemand::value jsonDataNew) {
	sizes_data data;

	data.medium = getValue<large_data>(jsonDataNew, "medium");
	data.small	= getValue<large_data>(jsonDataNew, "small");
	data.thumb	= getValue<large_data>(jsonDataNew, "thumb");
	data.large	= getValue<large_data>(jsonDataNew, "large");

	return data;
}

template<> media_data getValue<media_data>(simdjson::ondemand::value jsonDataNew) {
	media_data media;

	media.id	= getValue<double>(jsonDataNew, "id");
	media.idStr = getValue<std::string>(jsonDataNew, "id_str");
	getValue<int64_t>(media.indices, jsonDataNew, "indices");
	media.mediaUrl			= getValue<std::string>(jsonDataNew, "media_url");
	media.mediaUrlHttps		= getValue<std::string>(jsonDataNew, "media_url_https");
	media.url				= getValue<std::string>(jsonDataNew, "url");
	media.displayUrl		= getValue<std::string>(jsonDataNew, "display_url");
	media.expandedUrl		= getValue<std::string>(jsonDataNew, "expanded_url");
	media.type				= getValue<std::string>(jsonDataNew, "type");
	media.sizes				= getValue<sizes_data>(jsonDataNew, "sizes");
	media.sourceStatusId	= getValue<double>(jsonDataNew, "source_status_id");
	media.sourceStatusIdStr = getValue<std::string>(jsonDataNew, "source_status_id_str");

	return media;
}

template<> url_data getValue<url_data>(simdjson::ondemand::value jsonDataNew) {
	url_data urlData;

	urlData.url			= getValue<std::string>(jsonDataNew, "url");
	urlData.expandedUrl = getValue<std::string>(jsonDataNew, "expanded_url");
	urlData.displayUrl	= getValue<std::string>(jsonDataNew, "display_url");
	getValue<int64_t>(urlData.indices, jsonDataNew, "indices");

	return urlData;
}

template<> user_mention getValue<user_mention>(simdjson::ondemand::value jsonDataNew) {
	user_mention mention;

	mention.screenName = getValue<std::string>(jsonDataNew, "screen_name");
	mention.name	   = getValue<std::string>(jsonDataNew, "name");
	mention.id		   = getValue<int64_t>(jsonDataNew, "id");
	mention.idStr	   = getValue<std::string>(jsonDataNew, "id_str");
	getValue<int64_t>(mention.indices, jsonDataNew, "indices");

	return mention;
}

template<> status_entities getValue<status_entities>(simdjson::ondemand::value jsonDataNew) {
	status_entities entities;

	getValue<hashtag>(entities.hashtags, jsonDataNew, "hashtags");
	getValue<std::string>(entities.symbols, jsonDataNew, "symbols");
	getValue<url_data>(entities.urls, jsonDataNew, "urls");
	getValue<user_mention>(entities.userMentions, jsonDataNew, "user_mentions");

	return entities;
}

template<> metadata_data getValue<metadata_data>(simdjson::ondemand::value jsonDataNew) {
	metadata_data data;

	data.resultType		 = getValue<std::string>(jsonDataNew, "result_type");
	data.isoLanguageCode = getValue<std::string>(jsonDataNew, "iso_language_code");

	return data;
}

template<> description_data getValue<description_data>(simdjson::ondemand::value jsonDataNew) {
	description_data data;

	getValue<url_data>(data.urls, jsonDataNew, "urls");

	return data;
}

template<> user_entities getValue<user_entities>(simdjson::ondemand::value jsonDataNew) {
	user_entities entities;

	entities.description = getValue<description_data>(jsonDataNew, "description");
	entities.url		 = getValue<description_data>(jsonDataNew, "url");

	return entities;
}

template<> user_data_twitter getValue<user_data_twitter>(simdjson::ondemand::value jsonDataNew) {
	user_data_twitter usr;

	usr.id							   = getValue<int64_t>(jsonDataNew, "id");
	usr.idStr						   = getValue<std::string>(jsonDataNew, "id_str");
	usr.name						   = getValue<std::string>(jsonDataNew, "name");
	usr.screenName					   = getValue<std::string>(jsonDataNew, "screen_name");
	usr.location					   = getValue<std::string>(jsonDataNew, "location");
	usr.description					   = getValue<std::string>(jsonDataNew, "description");
	usr.url							   = getValue<std::string>(jsonDataNew, "url");
	usr.entities					   = getValue<user_entities>(jsonDataNew, "entities");
	usr.userProtected				   = getValue<bool>(jsonDataNew, "protected");
	usr.followersCount				   = getValue<int64_t>(jsonDataNew, "followers_count");
	usr.friendsCount				   = getValue<int64_t>(jsonDataNew, "friends_count");
	usr.listedCount					   = getValue<int64_t>(jsonDataNew, "listed_count");
	usr.createdAt					   = getValue<std::string>(jsonDataNew, "created_at");
	usr.favouritesCount				   = getValue<int64_t>(jsonDataNew, "favourites_count");
	usr.utcOffset					   = getValue<int64_t>(jsonDataNew, "utc_offset");
	usr.timeZone					   = getValue<std::string>(jsonDataNew, "time_zone");
	usr.geoEnabled					   = getValue<bool>(jsonDataNew, "geo_enabled");
	usr.verified					   = getValue<bool>(jsonDataNew, "verified");
	usr.statusesCount				   = getValue<int64_t>(jsonDataNew, "statuses_count");
	usr.lang						   = getValue<std::string>(jsonDataNew, "lang");
	usr.contributorsEnabled			   = getValue<bool>(jsonDataNew, "contributors_enabled");
	usr.isTranslator				   = getValue<bool>(jsonDataNew, "is_translator");
	usr.isTranslationEnabled		   = getValue<bool>(jsonDataNew, "is_translation_enabled");
	usr.profileBackgroundColor		   = getValue<std::string>(jsonDataNew, "profile_background_color");
	usr.profileBackgroundImageUrl	   = getValue<std::string>(jsonDataNew, "profile_background_image_url");
	usr.profileBackgroundImageUrlHttps = getValue<std::string>(jsonDataNew, "profile_background_image_url_https");
	usr.profileBackgroundTile		   = getValue<bool>(jsonDataNew, "profile_background_tile");
	usr.profileImageUrl				   = getValue<std::string>(jsonDataNew, "profile_image_url");
	usr.profileImageUrlHttps		   = getValue<std::string>(jsonDataNew, "profile_image_url_https");
	usr.profileBannerUrl			   = getValue<std::string>(jsonDataNew, "profile_banner_url");
	usr.profileLinkColor			   = getValue<std::string>(jsonDataNew, "profile_link_color");
	usr.profileSidebarBorderColor	   = getValue<std::string>(jsonDataNew, "profile_sidebar_border_color");
	usr.profileSidebarFillColor		   = getValue<std::string>(jsonDataNew, "profile_sidebar_fill_color");
	usr.profileTextColor			   = getValue<std::string>(jsonDataNew, "profile_text_color");
	usr.profileUseBackgroundImage	   = getValue<bool>(jsonDataNew, "profile_use_background_image");
	usr.defaultProfile				   = getValue<bool>(jsonDataNew, "default_profile");
	usr.defaultProfileImage			   = getValue<bool>(jsonDataNew, "default_profile_image");
	usr.following					   = getValue<bool>(jsonDataNew, "following");
	usr.followRequestSent			   = getValue<bool>(jsonDataNew, "follow_request_sent");
	usr.notifications				   = getValue<bool>(jsonDataNew, "notifications");

	return usr;
}

template<> status getValue<status>(simdjson::ondemand::value jsonDataNew) {
	status st;

	st.metadata				= getValue<metadata_data>(jsonDataNew, "metadata");
	st.createdAt			= getValue<std::string>(jsonDataNew, "created_at");
	st.id					= getValue<double>(jsonDataNew, "id");
	st.idStr				= getValue<std::string>(jsonDataNew, "id_str");
	st.text					= getValue<std::string>(jsonDataNew, "text");
	st.source				= getValue<std::string>(jsonDataNew, "source");
	st.truncated			= getValue<bool>(jsonDataNew, "truncated");
	st.inReplyToStatusId	= getValue<double>(jsonDataNew, "in_reply_to_status_id");
	st.inReplyToStatusIdStr = getValue<std::string>(jsonDataNew, "in_reply_to_status_id_str");
	st.inReplyToUserId		= getValue<int64_t>(jsonDataNew, "in_reply_to_user_id");
	st.inReplyToUserIdStr	= getValue<std::string>(jsonDataNew, "in_reply_to_user_id_str");
	st.inReplyToScreenName	= getValue<std::string>(jsonDataNew, "in_reply_to_screen_name");
	st.user					= getValue<user_data_twitter>(jsonDataNew, "user");
	st.geo					= getValue<std::string>(jsonDataNew, "geo");
	st.coordinates			= getValue<std::string>(jsonDataNew, "coordinates");
	st.place				= getValue<std::string>(jsonDataNew, "place");
	st.contributors			= getValue<std::string>(jsonDataNew, "contributors");
	st.retweetCount			= getValue<int64_t>(jsonDataNew, "retweet_count");
	st.favoriteCount		= getValue<int64_t>(jsonDataNew, "favorite_count");
	st.entities				= getValue<status_entities>(jsonDataNew, "entities");
	st.favorited			= getValue<bool>(jsonDataNew, "favorited");
	st.retweeted			= getValue<bool>(jsonDataNew, "retweeted");
	st.lang					= getValue<std::string>(jsonDataNew, "lang");
	st.possiblySensitive	= getValue<bool>(jsonDataNew, "possibly_sensitive");

	st.retweetedStatus = nullptr;// This will be set separately if retweeted_status is present

	return st;
}


template<> twitter_message getValue<twitter_message>(simdjson::ondemand::value jsonDataNew) {
	twitter_message message;

	getValue<status>(message.statuses, jsonDataNew, "statuses");
	message.searchMetadata = getValue<search_metadata>(jsonDataNew, "search_metadata");

	return message;
}

template<> avatar_decoration_data getValue<avatar_decoration_data>(simdjson::ondemand::value jsonDataNew) {
	avatar_decoration_data message;
	message.asset = getValue<std::string>(jsonDataNew, "asset");
	message.skuId = getValue<std::string>(jsonDataNew, "sku_id");

	return message;
}

template<> user_data getValue<user_data>(simdjson::ondemand::value jsonDataNew) {
	user_data userData;

	userData.avatarDecorationData = getValue<avatar_decoration_data>(jsonDataNew, "avatar_decoration_data");
	userData.discriminator		  = getValue<std::string>(jsonDataNew, "discriminator");
	userData.globalName			  = getValue<std::string>(jsonDataNew, "global_name");
	userData.userName			  = getValue<std::string>(jsonDataNew, "user_name");
	userData.accentColor		  = getValue<uint64_t>(jsonDataNew, "accent_color");
	userData.premiumType		  = getValue<uint64_t>(jsonDataNew, "premium_type");
	userData.publicFlags		  = getValue<uint64_t>(jsonDataNew, "public_flags");
	userData.locale				  = getValue<std::string>(jsonDataNew, "locale");
	userData.banner				  = getValue<std::string>(jsonDataNew, "banner");
	userData.avatar				  = getValue<std::string>(jsonDataNew, "avatar");
	userData.email				  = getValue<std::string>(jsonDataNew, "email");
	userData.mfaEnabled			  = getValue<bool>(jsonDataNew, "mfa_enabled");
	userData.id					  = getValue<std::string>(jsonDataNew, "id");
	userData.flags				  = getValue<uint64_t>(jsonDataNew, "flags");
	userData.verified			  = getValue<bool>(jsonDataNew, "verified");
	userData.system				  = getValue<bool>(jsonDataNew, "system");
	userData.bot				  = getValue<bool>(jsonDataNew, "bot");

	return userData;
}

template<> guild_scheduled_event_data getValue<guild_scheduled_event_data>(simdjson::ondemand::value jsonDataNew) {
	guild_scheduled_event_data eventData;

	eventData.scheduledStartTime = getValue<std::string>(jsonDataNew, "scheduled_start_time");
	eventData.scheduledEndTime	 = getValue<std::string>(jsonDataNew, "scheduled_end_time");
	eventData.description		 = getValue<std::string>(jsonDataNew, "description");
	eventData.entityMetadata	 = getValue<uint64_t>(jsonDataNew, "entity_metadata");
	eventData.creatorId			 = getValue<std::string>(jsonDataNew, "creator_id");
	eventData.channelId			 = getValue<std::string>(jsonDataNew, "channel_id");
	eventData.privacyLevel		 = getValue<uint64_t>(jsonDataNew, "privacy_level");
	eventData.entityId			 = getValue<std::string>(jsonDataNew, "entity_id");
	eventData.guildId			 = getValue<std::string>(jsonDataNew, "guild_id");
	eventData.entityType		 = getValue<uint64_t>(jsonDataNew, "entity_type");
	eventData.userCount			 = getValue<uint64_t>(jsonDataNew, "user_count");
	eventData.creator			 = getValue<user_data>(jsonDataNew, "creator");
	eventData.name				 = getValue<std::string>(jsonDataNew, "name");
	eventData.status			 = getValue<uint64_t>(jsonDataNew, "status");
	eventData.id				 = getValue<std::string>(jsonDataNew, "id");

	return eventData;
}

template<> channel_data getValue<channel_data>(simdjson::ondemand::value jsonDataNew) {
	channel_data channelData;

	channelData.defaultThreadRateLimitPerUser = getValue<uint64_t>(jsonDataNew, "default_thread_rate_limit_per_user");

	channelData.defaultAutoArchiveDuration = getValue<uint64_t>(jsonDataNew, "default_auto_archive_duration");

	channelData.lastPinTimestamp = getValue<std::string>(jsonDataNew, "last_pin_timestamp");
	channelData.totalMessageSent = getValue<uint64_t>(jsonDataNew, "total_message_sent");
	channelData.rateLimitPerUser = getValue<uint64_t>(jsonDataNew, "rate_limit_per_user");
	channelData.videoQualityMode = getValue<uint64_t>(jsonDataNew, "video_quality_mode");
	channelData.lastMessageId	 = getValue<std::string>(jsonDataNew, "last_message_id");
	channelData.applicationId	 = getValue<std::string>(jsonDataNew, "application_id");
	channelData.permissions		 = getValue<std::string>(jsonDataNew, "permissions");
	channelData.rtcRegion		 = getValue<std::string>(jsonDataNew, "rtc_region");
	channelData.messageCount	 = getValue<uint64_t>(jsonDataNew, "message_count");
	channelData.memberCount		 = getValue<uint64_t>(jsonDataNew, "member_count");
	channelData.parentId		 = getValue<std::string>(jsonDataNew, "parent_id");
	channelData.ownerId			 = getValue<std::string>(jsonDataNew, "owner_id");
	channelData.guildId			 = getValue<std::string>(jsonDataNew, "guild_id");
	channelData.userLimit		 = getValue<uint64_t>(jsonDataNew, "user_limit");
	channelData.topic			 = getValue<std::string>(jsonDataNew, "topic");
	channelData.position		 = getValue<uint64_t>(jsonDataNew, "position");
	channelData.bitrate			 = getValue<uint64_t>(jsonDataNew, "bitrate");
	channelData.name			 = getValue<std::string>(jsonDataNew, "name");
	channelData.icon			 = getValue<std::string>(jsonDataNew, "icon");
	channelData.id				 = getValue<std::string>(jsonDataNew, "id");
	channelData.flags			 = getValue<uint64_t>(jsonDataNew, "flags");
	channelData.type			 = getValue<uint64_t>(jsonDataNew, "type");
	channelData.managed			 = getValue<bool>(jsonDataNew, "managed");
	channelData.nsfw			 = getValue<bool>(jsonDataNew, "nsfw");

	return channelData;
}

template<> role_data getValue<role_data>(simdjson::ondemand::value jsonDataNew) {
	role_data roleData;

	roleData.unicodeEmoji = getValue<std::string>(jsonDataNew, "unicode_emoji");
	roleData.permissions  = getValue<std::string>(jsonDataNew, "permissions");
	roleData.position	  = getValue<uint64_t>(jsonDataNew, "position");
	roleData.name		  = getValue<std::string>(jsonDataNew, "name");
	roleData.icon		  = getValue<std::string>(jsonDataNew, "icon");
	roleData.mentionable  = getValue<bool>(jsonDataNew, "mentionable");
	roleData.color		  = getValue<uint64_t>(jsonDataNew, "color");
	roleData.id			  = getValue<std::string>(jsonDataNew, "id");
	roleData.flags		  = getValue<uint64_t>(jsonDataNew, "flags");
	roleData.managed	  = getValue<bool>(jsonDataNew, "managed");
	roleData.hoist		  = getValue<bool>(jsonDataNew, "hoist");

	return roleData;
}

template<> guild_member_data getValue<guild_member_data>(simdjson::ondemand::value jsonDataNew) {
	guild_member_data memberData;

	memberData.communicationDisabledUntil = getValue<std::string>(jsonDataNew, "communication_disabled_until");
	getValue<std::string>(memberData.roles, jsonDataNew, "roles");
	memberData.premiumSince = getValue<std::string>(jsonDataNew, "premium_since");
	memberData.permissions	= getValue<std::string>(jsonDataNew, "permissions");
	memberData.joinedAt		= getValue<std::string>(jsonDataNew, "joined_at");
	memberData.guildId		= getValue<std::string>(jsonDataNew, "guild_id");
	memberData.avatar		= getValue<std::string>(jsonDataNew, "avatar");
	memberData.nick			= getValue<std::string>(jsonDataNew, "nick");
	memberData.user			= getValue<user_data>(jsonDataNew, "user");
	memberData.flags		= getValue<uint64_t>(jsonDataNew, "flags");
	memberData.pending		= getValue<bool>(jsonDataNew, "pending");
	memberData.deaf			= getValue<bool>(jsonDataNew, "deaf");
	memberData.mute			= getValue<bool>(jsonDataNew, "mute");

	return memberData;
}

template<> guild_data getValue<guild_data>(simdjson::ondemand::value jsonDataNew) {
	guild_data guildData;

	getValue<guild_scheduled_event_data>(guildData.guildScheduledEvents, jsonDataNew, "guild_scheduled_events");

	getValue<guild_member_data>(guildData.members, jsonDataNew, "members");

	guildData.defaultMessageNotifications = getValue<uint64_t>(jsonDataNew, "default_message_notifications");

	getValue<channel_data>(guildData.channels, jsonDataNew, "channels");

	guildData.maxStageVideoChannelUsers = getValue<uint64_t>(jsonDataNew, "max_stage_video_channel_users");
	guildData.publicUpdatesChannelId	= getValue<std::string>(jsonDataNew, "public_updates_channel_id");
	guildData.premiumSubscriptionCount	= getValue<uint64_t>(jsonDataNew, "premium_subscription_count");

	getValue<std::string>(guildData.features, jsonDataNew, "features");

	guildData.approximatePresenceCount	= getValue<uint64_t>(jsonDataNew, "approximate_presence_count");
	guildData.safetyAlertsChannelId		= getValue<std::string>(jsonDataNew, "safety_alerts_channel_id");
	guildData.approximateMemberCount	= getValue<uint64_t>(jsonDataNew, "approximate_member_count");
	guildData.premiumProgressBarEnabled = getValue<bool>(jsonDataNew, "premium_progress_bar_enabled");
	guildData.explicitContentFilter		= getValue<uint64_t>(jsonDataNew, "explicit_content_filter");
	guildData.maxVideoChannelUsers		= getValue<uint64_t>(jsonDataNew, "max_video_channel_users");

	getValue<role_data>(guildData.roles, jsonDataNew, "roles");

	guildData.systemChannelId	 = getValue<std::string>(jsonDataNew, "system_channel_id");
	guildData.widgetChannelId	 = getValue<std::string>(jsonDataNew, "widget_channel_id");
	guildData.preferredLocale	 = getValue<std::string>(jsonDataNew, "preferred_locale");
	guildData.discoverySplash	 = getValue<std::string>(jsonDataNew, "discovery_splash");
	guildData.systemChannelFlags = getValue<uint64_t>(jsonDataNew, "system_channel_flags");
	guildData.rulesChannelId	 = getValue<std::string>(jsonDataNew, "rules_channel_id");
	guildData.verificationLevel	 = getValue<uint64_t>(jsonDataNew, "verification_level");
	guildData.applicationId		 = getValue<std::string>(jsonDataNew, "application_id");
	guildData.vanityUrlCode		 = getValue<std::string>(jsonDataNew, "vanity_url_code");
	guildData.afkChannelId		 = getValue<std::string>(jsonDataNew, "afk_channel_id");
	guildData.description		 = getValue<std::string>(jsonDataNew, "description");
	guildData.permissions		 = getValue<std::string>(jsonDataNew, "permissions");
	guildData.maxPresences		 = getValue<uint64_t>(jsonDataNew, "max_presences");
	guildData.discovery			 = getValue<std::string>(jsonDataNew, "discovery");
	guildData.memberCount		 = getValue<uint64_t>(jsonDataNew, "member_count");
	guildData.joinedAt			 = getValue<std::string>(jsonDataNew, "joined_at");
	guildData.premiumTier		 = getValue<uint64_t>(jsonDataNew, "premium_tier");
	guildData.ownerId			 = getValue<std::string>(jsonDataNew, "owner_id");
	guildData.maxMembers		 = getValue<uint64_t>(jsonDataNew, "max_members");
	guildData.afkTimeout		 = getValue<uint64_t>(jsonDataNew, "afk_timeout");
	guildData.splash			 = getValue<std::string>(jsonDataNew, "splash");
	guildData.banner			 = getValue<std::string>(jsonDataNew, "banner");
	guildData.widgetEnabled		 = getValue<bool>(jsonDataNew, "widget_enabled");
	guildData.nsfwLevel			 = getValue<uint64_t>(jsonDataNew, "nsfw_level");
	guildData.mfaLevel			 = getValue<uint64_t>(jsonDataNew, "mfa_level");
	guildData.name				 = getValue<std::string>(jsonDataNew, "name");
	guildData.icon				 = getValue<std::string>(jsonDataNew, "icon");
	guildData.unavailable		 = getValue<bool>(jsonDataNew, "unavailable");
	guildData.id				 = getValue<std::string>(jsonDataNew, "id");
	guildData.flags				 = getValue<uint64_t>(jsonDataNew, "flags");
	guildData.large				 = getValue<bool>(jsonDataNew, "large");
	guildData.owner				 = getValue<bool>(jsonDataNew, "owner");

	return guildData;
}

template<> discord_message getValue<discord_message>(simdjson::ondemand::value jsonDataNew) {
	discord_message message;

	message.t = getValue<std::string>(jsonDataNew, "t");

	message.s = getValue<uint64_t>(jsonDataNew, "s");

	message.op = getValue<uint64_t>(jsonDataNew, "op");

	message.d = getValue<guild_data>(jsonDataNew, "d");

	return message;
}

	#define SIMD_PULL(x) \
		{ \
			ondemand::array newX = doc[#x].get_array().value(); \
			for (ondemand::value value: newX) { \
				test_struct newStruct{}; \
				getValue<int64_t>(newStruct.testInts, value, "testInts"); \
				getValue<double>(newStruct.testDoubles, value, "testDoubles"); \
				getValue<std::string>(newStruct.testStrings, value, "testStrings"); \
				getValue<bool>(newStruct.jsonifierTestBools, value, "testBools"); \
				getValue<uint64_t>(newStruct.testUints, value, "testUints"); \
				obj.x.emplace_back(std::move(newStruct)); \
			} \
		}


template<> Test<test_struct> getValue<Test<test_struct>>(simdjson::ondemand::value doc) {
	Test<test_struct> obj{};
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
	return obj;
}

template<> AbcTest<test_struct> getValue<AbcTest<test_struct>>(simdjson::ondemand::value doc) {
	AbcTest<test_struct> obj{};
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
	return obj;
}

template<test_type type = test_type::parse_and_serialize, typename test_data_type, bool minified, uint64_t iterations>
auto simdjsonTest(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ newBuffer };

	auto iterationsVal = type == test_type::parse_and_serialize_single ? 1 : iterations;

	results_data r{ "simdjson", testName, "https://github.com/simdjson/simdjson", iterations };
	test_data_type testData{};

	simdjson::ondemand::parser parser{};
	auto readSize	= buffer.size();
	auto readResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				try {
					auto doc = parser.iterate(buffer);
					testData = getValue<test_data_type>(doc.value());
				} catch (std ::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
				}
			}
		},
		iterations);

	r.readResult = result<result_type::read>{ "cadetblue", readSize * iterationsVal, readResult };
	file_loader fileLoader{ basePath + "/" + testName + "-simdjson.json" };
	fileLoader.saveFile(buffer);
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto simdjsonTest<test_type::minify, std::string, false, iterationsVal>(const jsonifier::string& newBuffer, const std::string& testName, bool doWePrint) {
	std::string buffer{ newBuffer };
	std::string newerBuffer{};

	results_data r{ "simdjson", "Minify Test", "https://github.com/simdjson/simdjson", iterationsVal };

	simdjson::dom::parser parser{};

	auto writeResult = benchmark(
		[&]() {
			for (uint64_t x = 0; x < iterationsVal; ++x) {
				try {
					auto doc	= parser.parse(buffer);
					newerBuffer = simdjson::minify(doc);
				} catch (std ::exception& error) {
					std::cout << "Simdjson Error: " << error.what() << std::endl;
				}
			}
		},
		iterationsVal);

	file_loader fileLoader{ basePath + "/" + testName + "-simdjson.json" };
	fileLoader.saveFile(newerBuffer);
	r.writeResult = result<result_type::write>{ "cornflowerblue", newerBuffer.size() * iterationsVal, writeResult };

	if (doWePrint) {
		r.print();
	}

	return r;
}
#endif

std::string table_header = R"(
| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |)";

template<test_type type, typename test_data_type, bool minified, uint64_t iterations>
test_results jsonTests(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
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

template<> test_results jsonTests<test_type::prettify, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
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

template<> test_results jsonTests<test_type::minify, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
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

template<> test_results jsonTests<test_type::validate, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
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

static const std::string section001{ R"(> )" + jsonifier::toString(iterationsVal * iterationsVal) + R"( iterations on a 6 core (Intel i7 8700k)
)" };

static constexpr std::string_view section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: ()" };

static constexpr std::string_view section01{
	R"()

### Single Iteration Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

> 1 iteration on a 6 core (Intel i7 8700k)
)"
};

static constexpr std::string_view section02{
	R"(

### Single Iteration Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Single%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section03{
	R"(

### Multi Iteration Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section04{
	R"(

### Multi Iteration Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Multi%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static const std::string_view section05{
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

static constexpr std::string_view section06{ R"(

### ABC Test (Out of Sequence Performance - Minified) [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/JsonData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Abc%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section07{ R"(

### Discord Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section08{ R"(

### Discord Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Discord%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section09{ R"(

### Canada Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section10{ R"(

### Canada Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/CanadaData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Canada%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section11{ R"(

### Twitter Test (Prettified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Prettified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section12{ R"(

### Twitter Test (Minified) Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/TwitterData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Twitter%20Test%20(Minified)_Results.png?raw=true" 
alt="" width="400"/></p>

)" };

static constexpr std::string_view section13{
	R"(

### Minify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Prettified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Minify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section14{
	R"(

### Prettify Test Results [(View the data used in the following test)](https://github.com/RealTimeChris/Json-Performance/blob/main/Json/DiscordData-Minified.json):

----
<p align="left"><a href="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png" target="_blank"><img src="https://github.com/RealTimeChris/Json-Performance/blob/main/Graphs/Prettify%20Test_Results.png?raw=true" 
alt="" width="400"/></p>

)"
};

static constexpr std::string_view section15{
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
		jsonifier::string jsonDataNew{};
		jsonifier::jsonifier_core parser{};
		file_loader fileLoader01{ basePath + "/../ReadMe.md" };
		file_loader fileLoader02{ basePath + "/JsonData-Prettified.json" };
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(testJsonData, jsonDataNew);
		fileLoader02.saveFile(jsonDataNew);
		file_loader fileLoader03{ basePath + "/JsonData-Minified.json" };
		jsonifier::string jsonMinifiedData{ parser.minifyJson(jsonDataNew) };
		fileLoader03.saveFile(jsonMinifiedData);
		file_loader fileLoader04{ basePath + "/Results.json" };
		file_loader fileLoader05{ basePath + "/DiscordData-Prettified.json" };
		jsonifier::string discordData{ fileLoader05.operator jsonifier::string() };
		discord_message discordMessage{};
		parser.parseJson(discordMessage, discordData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(discordMessage, discordData);
		fileLoader05.saveFile(discordData);
		file_loader fileLoader06{ basePath + "/DiscordData-Minified.json" };
		jsonifier::string discordMinifiedData{ fileLoader06.operator jsonifier::string() };
		discordMinifiedData = parser.minifyJson(discordData);
		fileLoader06.saveFile(discordMinifiedData);
		file_loader fileLoader07{ basePath + "/CanadaData-Prettified.json" };
		jsonifier::string canadaData{ fileLoader07.operator jsonifier::string() };
		canada_message canadaMessage{};
		parser.parseJson(canadaMessage, canadaData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(canadaMessage, canadaData);
		fileLoader07.saveFile(canadaData);
		file_loader fileLoader08{ basePath + "/CanadaData-Minified.json" };
		jsonifier::string canadaMinifiedData{ fileLoader08.operator jsonifier::string() };
		canadaMinifiedData = parser.minifyJson(canadaData);
		fileLoader08.saveFile(canadaMinifiedData);
		file_loader fileLoader09{ basePath + "/TwitterData-Prettified.json" };
		jsonifier::string twitterData{ fileLoader09.operator jsonifier::string() };
		twitter_message twitterMessage{};
		parser.parseJson(twitterMessage, twitterData);
		parser.serializeJson<jsonifier::serialize_options{ .prettify = true }>(twitterMessage, twitterData);
		fileLoader09.saveFile(twitterData);
		file_loader fileLoader10{ basePath + "/TwitterData-Minified.json" };
		jsonifier::string twitterMinifiedData{ fileLoader10.operator jsonifier::string() };
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
		jsonifier::vector<test_results> benchmark_data{};
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		std::string newerString{ static_cast<std::string>(section00) + newTimeString + static_cast<std::string>(section01) };
		auto testResults = jsonTests<test_type::parse_and_serialize_single, Test<test_struct>, false, 1>(jsonDataNew, "Single Test (Prettified)");
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize_single, Test<test_struct>, true, 1>(jsonMinifiedData, "Single Test (Minified)");
		newerString += section02;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, false, iterationsVal>(jsonDataNew, "Multi Test (Prettified)");
		newerString += section03;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, true, iterationsVal>(jsonMinifiedData, "Multi Test (Minified)");
		newerString += section04;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, AbcTest<test_struct>, false, iterationsVal>(jsonDataNew, "Abc Test (Prettified)");
		newerString += section05;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, AbcTest<test_struct>, true, iterationsVal>(jsonMinifiedData, "Abc Test (Minified)");
		newerString += section06;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, false, iterationsVal>(discordData, "Discord Test (Prettified)");
		newerString += section07;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, true, iterationsVal>(discordMinifiedData, "Discord Test (Minified)");
		newerString += section08;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, canada_message, false, iterationsVal>(canadaData, "Canada Test (Prettified)");
		newerString += section09;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, canada_message, true, iterationsVal>(canadaMinifiedData, "Canada Test (Minified)");
		newerString += section10;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, twitter_message, false, iterationsVal>(twitterData, "Twitter Test (Prettified)");
		newerString += section11;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, twitter_message, true, iterationsVal>(twitterMinifiedData, "Twitter Test (Minified)");
		newerString += section12;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::minify, std::string, false, iterationsVal>(discordData, "Minify Test");
		newerString += section13;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::prettify, std::string, false, iterationsVal>(jsonMinifiedData, "Prettify Test");
		newerString += section14;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::validate, std::string, false, iterationsVal>(discordData, "Validate Test");
		newerString += section15;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		jsonifier::string resultsStringJson{};
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