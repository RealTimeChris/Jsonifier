#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <jsonifier/Index.hpp>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <chrono>

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
	std::vector<int64_t> indices{};
	std::string text{};
};

struct large {
	std::string resize{};
	int64_t w{};
	int64_t h{};
};

struct sizes {
	large largeVal{};
	large medium{};
	large thumb{};
	large small{};
};

struct media {
	std::string sourceStatusIdStr{};
	std::vector<int64_t> indices{};
	std::string mediaUrlHttps{};
	std::string expandedUrl{};
	std::string displayUrl{};
	double sourceStatusId{};
	std::string mediaUrl{};
	std::string idStr{};
	std::string type{};
	std::string url{};
	sizes sizesVal{};
	double id{};
};

struct url {
	std::vector<int64_t> indices{};
	std::string expandedUrl{};
	std::string displayUrl{};
	std::string urlVal{};
};

struct user_mention {
	std::vector<int64_t> indices{};
	std::string screenName{};
	std::string idStr{};
	std::string name{};
	int64_t id{};
};

struct status_entities {
	std::vector<user_mention> userMentions{};
	std::vector<std::string> symbols{};
	std::vector<hashtag> hashtags{};
	std::vector<media> mediaVal{};
	std::vector<url> urls{};
};

struct metadata {
	std::string isoLanguageCode{};
	std::string resultType{};
};

struct description {
	std::vector<url> urls{};
};

struct user_entities {
	description descriptionVal{};
	description url{};
};

struct user {
	std::string profileBackgroundImageUrlHttps{};
	std::string profileBackgroundImageUrl{};
	std::string profileSidebarBorderColor{};
	std::string profileSidebarFillColor{};
	std::string profileBackgroundColor{};
	std::string profileImageUrlHttps{};
	bool profileUseBackgroundImage{};
	std::string profileBannerUrl{};
	std::string profileLinkColor{};
	std::string profileTextColor{};
	std::string profileImageUrl{};
	bool profileBackgroundTile{};
	bool isTranslationEnabled{};
	bool contributorsEnabled{};
	bool defaultProfileImage{};
	std::string description{};
	int64_t favouritesCount{};
	std::string screenName{};
	user_entities entities{};
	int64_t followersCount{};
	bool followRequestSent{};
	std::string createdAt{};
	int64_t statusesCount{};
	std::string location{};
	int64_t friendsCount{};
	std::string timeZone{};
	int64_t listedCount{};
	bool defaultProfile{};
	bool userProtected{};
	bool notifications{};
	std::string idStr{};
	int64_t utcOffset{};
	bool isTranslator{};
	std::string name{};
	std::string lang{};
	std::string url{};
	bool geoEnabled{};
	bool following{};
	bool verified{};
	int64_t id{};
};

struct status {
	std::string inReplyToStatusIdStr{};
	std::string inReplyToScreenName{};
	std::string inReplyToUserIdStr{};
	double inReplyToStatusId{};
	std::string contributors{};
	status_entities entities{};
	int64_t inReplyToUserId{};
	std::string coordinates{};
	bool possiblySensitive{};
	std::string createdAt{};
	int64_t favoriteCount{};
	metadata metadataVal{};
	int64_t retweetCount{};
	std::string source{};
	std::string idStr{};
	std::string place{};
	std::string text{};
	std::string lang{};
	std::string geo{};
	bool truncated{};
	bool favorited{};
	bool retweeted{};
	user userVal{};
	double id{};
};

struct twitter_message {
	search_metadata searchMetadata{};
	std::vector<status> statuses{};
};

class user_data {
  public:
	std::string avatarDecoration{};
	std::string discriminator{};
	std::string globalName{};
	std::string userName{};
	uint64_t accentColor{};
	uint64_t premiumType{};
	uint64_t publicFlags{};
	std::string locale{};
	std::string banner{};
	std::string avatar{};
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

class role_data {
  public:
	std::string unicodeEmoji{};
	std::string permissions{};
	uint64_t position{};
	std::string name{};
	std::string icon{};
	bool mentionable{};
	uint64_t color{};
	std::string id{};
	uint64_t flags{};
	bool managed{};
	bool hoist{};
};

class channel_data {
  public:
	uint64_t defaultThreadRateLimitPerUser{};
	std::vector<std::string> appliedTags{};
	uint64_t defaultAutoArchiveDuration{};
	std::vector<user_data> recipients{};
	std::string lastPinTimestamp{};
	uint64_t totalMessageSent{};
	uint64_t rateLimitPerUser{};
	uint64_t videoQualityMode{};
	std::string lastMessageId{};
	std::string applicationId{};
	std::string permissions{};
	std::string rtcRegion{};
	uint64_t messageCount{};
	uint64_t memberCount{};
	std::string parentId{};
	std::string ownerId{};
	std::string guildId{};
	uint64_t userLimit{};
	std::string topic{};
	uint64_t position{};
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
	std::string communicationDisabledUntil{};
	std::vector<std::string> roles{};
	std::string premiumSince{};
	std::string permissions{};
	std::string joinedAt{};
	std::string guildId{};
	std::string avatar{};
	std::string nick{};
	user_data user{};
	uint64_t flags{};
	bool pending{};
	bool deaf{};
	bool mute{};
};

class guild_data {
  public:
	std::vector<guild_scheduled_event_data> guildScheduledEvents{};
	std::vector<guild_member_data> members{};
	uint64_t defaultMessageNotifications{};
	std::vector<channel_data> channels{};
	uint64_t maxStageVideoChannelUsers{};
	std::string publicUpdatesChannelId{};
	uint64_t premiumSubscriptionCount{};
	std::vector<std::string> features{};
	uint64_t approximatePresenceCount{};
	std::string safetyAlertsChannelId{};
	uint64_t approximateMemberCount{};
	bool premiumProgressBarEnabled{};
	uint64_t explicitContentFilter{};
	uint64_t maxVideoChannelUsers{};
	std::vector<role_data> roles{};
	std::string systemChannelId{};
	std::string widgetChannelId{};
	std::string preferredLocale{};
	std::string discoverySplash{};
	uint64_t systemChannelFlags{};
	std::string rulesChannelId{};
	uint64_t verificationLevel{};
	std::string applicationId{};
	std::string vanityUrlCode{};
	std::string afkChannelId{};
	std::string description{};
	std::string permissions{};
	uint64_t maxPresences{};
	std::string discovery{};
	uint64_t memberCount{};
	std::string joinedAt{};
	uint64_t premiumTier{};
	std::string ownerId{};
	uint64_t maxMembers{};
	uint64_t afkTimeout{};
	std::string splash{};
	std::string banner{};
	bool widgetEnabled{};
	uint64_t nsfwLevel{};
	uint64_t mfaLevel{};
	std::string name{};
	std::string icon{};
	bool unavailable{};
	std::string id{};
	uint64_t flags{};
	bool large{};
	bool owner{};
};

struct discord_message {
	std::string t{};
	guild_data d{};
	uint64_t impl{};
	uint64_t s{};
};

template<> struct jsonifier::core<search_metadata> {
	using value_type = search_metadata;
	static constexpr auto parseValue =
		createValue("completed_in", &value_type::completedIn, "max_id", &value_type::maxId, "max_id_str", &value_type::maxIdStr, "next_results", &value_type::nextResults, "query",
			&value_type::query, "refresh_url", &value_type::refreshUrl, "count", &value_type::count, "since_id", &value_type::sinceId, "since_id_str", &value_type::sinceIdStr);
};

template<> struct jsonifier::core<hashtag> {
	using value_type				 = hashtag;
	static constexpr auto parseValue = createValue("text", &value_type::text, "indices", &value_type::indices);
};

template<> struct jsonifier::core<large> {
	using value_type				 = large;
	static constexpr auto parseValue = createValue("w", &value_type::w, "h", &value_type::h, "resize", &value_type::resize);
};

template<> struct jsonifier::core<sizes> {
	using value_type				 = sizes;
	static constexpr auto parseValue = createValue("medium", &value_type::medium, "small", &value_type::small, "thumb", &value_type::thumb, "large", &value_type::largeVal);
};

template<> struct jsonifier::core<media> {
	using value_type				 = media;
	static constexpr auto parseValue = createValue("id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices, "media_url", &value_type::mediaUrl,
		"media_url_https", &value_type::mediaUrlHttps, "display_url", &value_type::displayUrl, "expanded_url", &value_type::expandedUrl, "type", &value_type::type, "sizes",
		&value_type::sizesVal, "source_status_id", &value_type::sourceStatusId, "source_status_id_str", &value_type::sourceStatusIdStr);
};

template<> struct jsonifier::core<url> {
	using value_type				 = url;
	static constexpr auto parseValue = createValue("expanded_url", &value_type::expandedUrl, "display_url", &value_type::displayUrl, "indices", &value_type::indices);
};

template<> struct jsonifier::core<user_mention> {
	using value_type = user_mention;
	static constexpr auto parseValue =
		createValue("screen_name", &value_type::screenName, "name", &value_type::name, "id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices);
};

template<> struct jsonifier::core<status_entities> {
	using value_type				 = status_entities;
	static constexpr auto parseValue = createValue("hashtags", &value_type::hashtags, "symbols", &value_type::symbols, "urls", &value_type::urls, "user_mentions",
		&value_type::userMentions, "media", &value_type::mediaVal);
};

template<> struct jsonifier::core<metadata> {
	using value_type				 = metadata;
	static constexpr auto parseValue = createValue("result_type", &value_type::resultType, "iso_language_code", &value_type::isoLanguageCode);
};

template<> struct jsonifier::core<description> {
	using value_type				 = description;
	static constexpr auto parseValue = createValue("urls", &value_type::urls);
};

template<> struct jsonifier::core<user_entities> {
	using value_type				 = user_entities;
	static constexpr auto parseValue = createValue("description", &value_type::descriptionVal);
};

template<> struct jsonifier::core<user> {
	using value_type				 = user;
	static constexpr auto parseValue = createValue("id", &value_type::id, "id_str", &value_type::idStr, "name", &value_type::name, "screen_name", &value_type::screenName,
		"location", &value_type::location, "description", &value_type::description, "entities", &value_type::entities, "protected", &value_type::userProtected, "followers_count",
		&value_type::followersCount, "friends_count", &value_type::friendsCount, "listed_count", &value_type::listedCount, "created_at", &value_type::createdAt, "favourites_count",
		&value_type::favouritesCount, "geo_enabled", &value_type::geoEnabled, "verified", &value_type::verified, "statuses_count", &value_type::statusesCount, "lang",
		&value_type::lang, "contributors_enabled", &value_type::contributorsEnabled, "is_translator", &value_type::isTranslator, "is_translation_enabled",
		&value_type::isTranslationEnabled, "profile_background_color", &value_type::profileBackgroundColor, "profile_background_image_url", &value_type::profileBackgroundImageUrl,
		"profile_background_image_url_https", &value_type::profileBackgroundImageUrlHttps, "profile_background_tile", &value_type::profileBackgroundTile, "profile_image_url",
		&value_type::profileImageUrl, "profile_image_url_https", &value_type::profileImageUrlHttps, "profile_banner_url", &value_type::profileBannerUrl, "profile_link_color",
		&value_type::profileLinkColor, "profile_sidebar_border_color", &value_type::profileSidebarBorderColor, "profile_sidebar_fill_color", &value_type::profileSidebarFillColor,
		"profile_text_color", &value_type::profileTextColor, "profile_use_background_image", &value_type::profileUseBackgroundImage, "default_profile", &value_type::defaultProfile,
		"default_profile_image", &value_type::defaultProfileImage, "following", &value_type::following, "follow_request_sent", &value_type::followRequestSent, "notifications",
		&value_type::notifications);
};

template<> struct jsonifier::core<status> {
	using value_type				 = status;
	static constexpr auto parseValue = createValue("metadata", &value_type::metadataVal, "created_at", &value_type::createdAt, "id", &value_type::id, "id_str", &value_type::idStr,
		"text", &value_type::text, "source", &value_type::source, "truncated", &value_type::truncated, "user", &value_type::userVal, "retweet_count", &value_type::retweetCount,
		"favorite_count", &value_type::favoriteCount, "entities", &value_type::entities, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "lang",
		&value_type::lang, "possibly_sensitive", &value_type::possiblySensitive);
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

template<> struct glz::meta<large> {
	using value_type			= large;
	static constexpr auto value = object("w", &value_type::w, "h", &value_type::h, "resize", &value_type::resize);
};

template<> struct glz::meta<sizes> {
	using value_type			= sizes;
	static constexpr auto value = object("medium", &value_type::medium, "small", &value_type::small, "thumb", &value_type::thumb, "large", &value_type::largeVal);
};

template<> struct glz::meta<media> {
	using value_type			= media;
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices, "media_url", &value_type::mediaUrl,
		"media_url_https", &value_type::mediaUrlHttps, "display_url", &value_type::displayUrl, "expanded_url", &value_type::expandedUrl, "type", &value_type::type, "sizes",
		&value_type::sizesVal, "source_status_id", &value_type::sourceStatusId, "source_status_id_str", &value_type::sourceStatusIdStr);
};

template<> struct glz::meta<url> {
	using value_type			= url;
	static constexpr auto value = object("expanded_url", &value_type::expandedUrl, "display_url", &value_type::displayUrl, "indices", &value_type::indices);
};

template<> struct glz::meta<user_mention> {
	using value_type = user_mention;
	static constexpr auto value =
		object("screen_name", &value_type::screenName, "name", &value_type::name, "id", &value_type::id, "id_str", &value_type::idStr, "indices", &value_type::indices);
};

template<> struct glz::meta<status_entities> {
	using value_type			= status_entities;
	static constexpr auto value = object("hashtags", &value_type::hashtags, "symbols", &value_type::symbols, "urls", &value_type::urls, "user_mentions", &value_type::userMentions,
		"media", &value_type::mediaVal);
};

template<> struct glz::meta<metadata> {
	using value_type			= metadata;
	static constexpr auto value = object("result_type", &value_type::resultType, "iso_language_code", &value_type::isoLanguageCode);
};

template<> struct glz::meta<description> {
	using value_type			= description;
	static constexpr auto value = object("urls", &value_type::urls);
};

template<> struct glz::meta<user_entities> {
	using value_type			= user_entities;
	static constexpr auto value = object("description", &value_type::descriptionVal);
};

template<> struct glz::meta<user> {
	using value_type			= user;
	static constexpr auto value = object("id", &value_type::id, "id_str", &value_type::idStr, "name", &value_type::name, "screen_name", &value_type::screenName, "location",
		&value_type::location, "description", &value_type::description, "entities", &value_type::entities, "protected", &value_type::userProtected, "followers_count",
		&value_type::followersCount, "friends_count", &value_type::friendsCount, "listed_count", &value_type::listedCount, "created_at", &value_type::createdAt, "favourites_count",
		&value_type::favouritesCount, "geo_enabled", &value_type::geoEnabled, "verified", &value_type::verified, "statuses_count", &value_type::statusesCount, "lang",
		&value_type::lang, "contributors_enabled", &value_type::contributorsEnabled, "is_translator", &value_type::isTranslator, "is_translation_enabled",
		&value_type::isTranslationEnabled, "profile_background_color", &value_type::profileBackgroundColor, "profile_background_image_url", &value_type::profileBackgroundImageUrl,
		"profile_background_image_url_https", &value_type::profileBackgroundImageUrlHttps, "profile_background_tile", &value_type::profileBackgroundTile, "profile_image_url",
		&value_type::profileImageUrl, "profile_image_url_https", &value_type::profileImageUrlHttps, "profile_banner_url", &value_type::profileBannerUrl, "profile_link_color",
		&value_type::profileLinkColor, "profile_sidebar_border_color", &value_type::profileSidebarBorderColor, "profile_sidebar_fill_color", &value_type::profileSidebarFillColor,
		"profile_text_color", &value_type::profileTextColor, "profile_use_background_image", &value_type::profileUseBackgroundImage, "default_profile", &value_type::defaultProfile,
		"default_profile_image", &value_type::defaultProfileImage, "following", &value_type::following, "follow_request_sent", &value_type::followRequestSent, "notifications",
		&value_type::notifications);
};

template<> struct glz::meta<status> {
	using value_type			= status;
	static constexpr auto value = object("metadata", &value_type::metadataVal, "created_at", &value_type::createdAt, "id", &value_type::id, "id_str", &value_type::idStr, "text",
		&value_type::text, "source", &value_type::source, "truncated", &value_type::truncated, "user", &value_type::userVal, "retweet_count", &value_type::retweetCount,
		"favorite_count", &value_type::favoriteCount, "entities", &value_type::entities, "favorited", &value_type::favorited, "retweeted", &value_type::retweeted, "lang",
		&value_type::lang, "possibly_sensitive", &value_type::possiblySensitive);
};

template<> struct glz::meta<twitter_message> {
	using value_type			= twitter_message;
	static constexpr auto value = object("statuses", &value_type::statuses, "search_metadata", &value_type::searchMetadata);
};
#endif
template<> struct jsonifier::core<user_data> {
	using value_type				 = user_data;
	static constexpr auto parseValue = createValue("id", &value_type::id, "username", &value_type::userName, "discriminator", &value_type::discriminator, "bot", &value_type::bot,
		"system", &value_type::system, "mfa_enabled", &value_type::mfaEnabled, "accentColor", &value_type::accentColor, "locale", &value_type::locale, "verified",
		&value_type::verified, "email", &value_type::email, "flags", &value_type::flags, "premium_type", &value_type::premiumType, "public_flags", &value_type::publicFlags,
		"avatar_decoration", &value_type::avatarDecoration);
};

template<> struct jsonifier::core<role_data> {
	using value_type				 = role_data;
	static constexpr auto parseValue = createValue("id", &value_type::id, "name", &value_type::name, "color", &value_type::color, "hoist", &value_type::hoist, "position",
		&value_type::position, "permissions", &value_type::permissions, "managed", &value_type::managed, "mentionable", &value_type::mentionable, "flags", &value_type::flags);
};

template<> struct jsonifier::core<guild_member_data> {
	using value_type				 = guild_member_data;
	static constexpr auto parseValue = createValue("user", &value_type::user, "roles", &value_type::roles, "joined_at", &value_type::joinedAt, "deaf", &value_type::deaf, "mute",
		&value_type::mute, "flags", &value_type::flags, "pending", &value_type::pending, "permissions", &value_type::permissions, "guild_id", &value_type::guildId);
};

template<> struct jsonifier::core<channel_data> {
	using value_type				 = channel_data;
	static constexpr auto parseValue = createValue("default_thread_rate_limit_per_user", &value_type::defaultThreadRateLimitPerUser, "applied_tags", &value_type::appliedTags,
		"default_auto_archive_duration", &value_type::defaultAutoArchiveDuration, "recipients", &value_type::recipients, "last_pin_timestamp", &value_type::lastPinTimestamp,
		"total_message_sent", &value_type::totalMessageSent, "rate_limit_per_user", &value_type::rateLimitPerUser, "video_quality_mode", &value_type::videoQualityMode,
		"permissions", &value_type::permissions, "message_count", &value_type::messageCount, "owner_id", &value_type::ownerId, "member_count", &value_type::memberCount, "flags",
		&value_type::flags, "user_limit", &value_type::userLimit, "type", &value_type::type, "guild_id", &value_type::guildId, "position", &value_type::position, "name",
		&value_type::name, "bitrate", &value_type::bitrate, "id", &value_type::id, "managed", &value_type::managed, "nsfw", &value_type::nsfw);
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
		"afk_timeout", &value_type::afkTimeout, "max_members", &value_type::maxMembers, "mfa_level", &value_type::mfaLevel, "name", &value_type::name, "icon", &value_type::icon);
};

template<> struct jsonifier::core<discord_message> {
	using OTy						 = discord_message;
	static constexpr auto parseValue = createValue("t", &OTy::t, "s", &OTy::s, "impl", &OTy::impl, "d", &OTy::d);
};
#if !defined(ASAN)
template<> struct glz::meta<user_data> {
	using value_type			= user_data;
	static constexpr auto value = object("id", &value_type::id, "username", &value_type::userName, "discriminator", &value_type::discriminator, "bot", &value_type::bot, "system",
		&value_type::system, "mfa_enabled", &value_type::mfaEnabled, "accentColor", &value_type::accentColor, "locale", &value_type::locale, "verified", &value_type::verified,
		"email", &value_type::email, "flags", &value_type::flags, "premium_type", &value_type::premiumType, "public_flags", &value_type::publicFlags, "avatar_decoration",
		&value_type::avatarDecoration);
};

template<> struct glz::meta<role_data> {
	using value_type			= role_data;
	static constexpr auto value = object("id", &value_type::id, "name", &value_type::name, "color", &value_type::color, "hoist", &value_type::hoist, "position",
		&value_type::position, "permissions", &value_type::permissions, "managed", &value_type::managed, "mentionable", &value_type::mentionable, "flags", &value_type::flags);
};

template<> struct glz::meta<guild_member_data> {
	using value_type			= guild_member_data;
	static constexpr auto value = object("user", &value_type::user, "roles", &value_type::roles, "joined_at", &value_type::joinedAt, "deaf", &value_type::deaf, "mute",
		&value_type::mute, "flags", &value_type::flags, "pending", &value_type::pending, "permissions", &value_type::permissions, "guild_id", &value_type::guildId);
};

template<> struct glz::meta<channel_data> {
	using value_type			= channel_data;
	static constexpr auto value = object("default_thread_rate_limit_per_user", &value_type::defaultThreadRateLimitPerUser, "applied_tags", &value_type::appliedTags,
		"default_auto_archive_duration", &value_type::defaultAutoArchiveDuration, "recipients", &value_type::recipients, "last_pin_timestamp", &value_type::lastPinTimestamp,
		"total_message_sent", &value_type::totalMessageSent, "rate_limit_per_user", &value_type::rateLimitPerUser, "video_quality_mode", &value_type::videoQualityMode,
		"permissions", &value_type::permissions, "message_count", &value_type::messageCount, "owner_id", &value_type::ownerId, "member_count", &value_type::memberCount, "flags",
		&value_type::flags, "user_limit", &value_type::userLimit, "type", &value_type::type, "guild_id", &value_type::guildId, "position", &value_type::position, "name",
		&value_type::name, "bitrate", &value_type::bitrate, "id", &value_type::id, "managed", &value_type::managed, "nsfw", &value_type::nsfw);
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
		&value_type::mfaLevel, "name", &value_type::name, "icon", &value_type::icon);
};

// Specialization for discord_message
template<> struct glz::meta<discord_message> {
	using OTy					= discord_message;
	static constexpr auto value = object("t", &OTy::t, "s", &OTy::s, "impl", &OTy::impl, "d", &OTy::d);
};
#endif
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
		auto length{ randomizeNumber(35.0f, 45.0f) };
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
			auto arraySize01 = randomizeNumber(35, 20);
			auto arraySize02 = randomizeNumber(20, 10);
			auto arraySize03 = randomizeNumber(5, 1);
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

	void print() {
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

	test_result getWriteResults() {
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

	test_result getReadResults() {
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

auto jsonifierSingleTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Single Test", "https://github.com/realtimechris/jsonifier", 1 };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};
	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
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

auto jsonifierValidationTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Validation Test", "https://github.com/realtimechris/jsonifier", iterations };
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

	r.wColor = "steelblue";
	r.rColor = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifierTwitterTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Twitter Test", "https://github.com/realtimechris/jsonifier", iterations };
	twitter_message uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
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

auto jsonifierTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Multi Test", "https://github.com/realtimechris/jsonifier", iterations };
	Test<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
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

auto jsonifierAbcTest(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Abc Test", "https://github.com/realtimechris/jsonifier", iterations };
	AbcTest<test_struct> uint64Test{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(uint64Test, buffer);
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

auto jsonifierDiscordTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };
	auto newSize = buffer.size();

	results r{ "jsonifier", "Discord Test", "https://github.com/realtimechris/jsonifier", iterations };
	discord_message discordDataTest{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson(discordDataTest, buffer);
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

	results r{ "jsonifier", "Minify Test", "https://github.com/realtimechris/jsonifier", iterations };

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

auto jsonifierPrettifyTest(const jsonifier::string& discordDataNew, bool doWePrint = true) {
	std::string buffer{ discordDataNew };

	results r{ "jsonifier", "Prettify Test", "https://github.com/realtimechris/jsonifier", iterations };

	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.prettify(buffer);
		},
		iterations);

	r.json_write = result;

	r.json_write_byte_length = buffer.size();
	r.wColor				 = "steelblue";
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
	for (uint64_t x = 0; x < oldSize; ++x, ++iter) {
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
	for (uint64_t x = 0; x < oldSize; ++x, ++iter) {
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
	for (const simdjson::ondemand::value& role: value) {
		returnValues.emplace_back(getValue<std::string>(role));
	}
}

template<> search_metadata getValue<search_metadata>(simdjson::ondemand::value jsonData) {
	search_metadata metadata;

	metadata.completedIn = getValue<double>(jsonData, "completed_in");
	metadata.maxId		 = getValue<int64_t>(jsonData, "max_id");
	metadata.maxIdStr	 = getValue<std::string>(jsonData, "max_id_str");
	metadata.nextResults = getValue<std::string>(jsonData, "next_results");
	metadata.query		 = getValue<std::string>(jsonData, "query");
	metadata.refreshUrl	 = getValue<std::string>(jsonData, "refresh_url");
	metadata.count		 = getValue<int64_t>(jsonData, "count");
	metadata.sinceId	 = getValue<int64_t>(jsonData, "since_id");
	metadata.sinceIdStr	 = getValue<std::string>(jsonData, "since_id_str");

	return metadata;
}

template<> hashtag getValue<hashtag>(simdjson::ondemand::value jsonData) {
	hashtag tag;

	tag.text	= getValue<std::string>(jsonData, "text");
	getValues<int64_t>(tag.indices, jsonData, "indices");

	return tag;
}

template<> large getValue<large>(simdjson::ondemand::value jsonData) {
	large size;

	size.w		= getValue<uint64_t>(jsonData, "w");
	size.h		= getValue<uint64_t>(jsonData, "h");
	size.resize = getValue<std::string>(jsonData, "resize");

	return size;
}

template<> sizes getValue<sizes>(simdjson::ondemand::value jsonData) {
	sizes imageSizes;

	imageSizes.medium	= getValue<large>(jsonData, "medium");
	imageSizes.small	= getValue<large>(jsonData, "small");
	imageSizes.thumb	= getValue<large>(jsonData, "thumb");
	imageSizes.largeVal = getValue<large>(jsonData, "large");

	return imageSizes;
}

template<> media getValue<media>(simdjson::ondemand::value jsonData) {
	media mediaItem;

	mediaItem.id				= getValue<double>(jsonData, "id");
	mediaItem.idStr				= getValue<std::string>(jsonData, "id_str");
	getValues<int64_t>(mediaItem.indices, jsonData, "indices");
	mediaItem.mediaUrl			= getValue<std::string>(jsonData, "media_url");
	mediaItem.mediaUrlHttps		= getValue<std::string>(jsonData, "media_url_https");
	mediaItem.displayUrl		= getValue<std::string>(jsonData, "display_url");
	mediaItem.expandedUrl		= getValue<std::string>(jsonData, "expanded_url");
	mediaItem.type				= getValue<std::string>(jsonData, "type");
	mediaItem.sizesVal			= getValue<sizes>(jsonData, "sizes");
	mediaItem.sourceStatusId	= getValue<double>(jsonData, "source_status_id");
	mediaItem.sourceStatusIdStr = getValue<std::string>(jsonData, "source_status_id_str");

	return mediaItem;
}

template<> url getValue<url>(simdjson::ondemand::value jsonData) {
	url urlData;

	urlData.expandedUrl = getValue<std::string>(jsonData, "expanded_url");
	urlData.displayUrl	= getValue<std::string>(jsonData, "display_url");
	getValues<int64_t>(urlData.indices, jsonData, "indices");

	return urlData;
}

template<> user_mention getValue<user_mention>(simdjson::ondemand::value jsonData) {
	user_mention mention;

	mention.screenName = getValue<std::string>(jsonData, "screen_name");
	mention.name	   = getValue<std::string>(jsonData, "name");
	mention.id		   = getValue<int64_t>(jsonData, "id");
	mention.idStr	   = getValue<std::string>(jsonData, "id_str");
	getValues<int64_t>(mention.indices, jsonData, "indices");

	return mention;
}

template<> status_entities getValue<status_entities>(simdjson::ondemand::value jsonData) {
	status_entities entities;
	getValues<hashtag>(entities.hashtags, jsonData, "hashtags");
	getValues<url>(entities.urls, jsonData, "urls");
	getValues<user_mention>(entities.userMentions, jsonData, "user_mentions");
	getValues<std::string>(entities.symbols, jsonData, "symbols");

	return entities;
}

template<> metadata getValue<metadata>(simdjson::ondemand::value jsonData) {
	metadata meta;

	meta.resultType		 = getValue<std::string>(jsonData, "result_type");
	meta.isoLanguageCode = getValue<std::string>(jsonData, "iso_language_code");

	return meta;
}

template<> description getValue<description>(simdjson::ondemand::value jsonData) {
	description desc;

	getValues<url>(desc.urls, jsonData, "urls");

	return desc;
}

template<> user_entities getValue<user_entities>(simdjson::ondemand::value jsonData) {
	user_entities userEnt;

	userEnt.descriptionVal = getValue<description>(jsonData, "description");

	return userEnt;
}

template<> user getValue<user>(simdjson::ondemand::value jsonData) {
	user userData;

	userData.id								= getValue<int64_t>(jsonData, "id");
	userData.idStr							= getValue<std::string>(jsonData, "id_str");
	userData.name							= getValue<std::string>(jsonData, "name");
	userData.screenName						= getValue<std::string>(jsonData, "screen_name");
	userData.location						= getValue<std::string>(jsonData, "location");
	userData.description					= getValue<std::string>(jsonData, "description");
	userData.entities						= getValue<user_entities>(jsonData, "entities");
	userData.userProtected					= getValue<bool>(jsonData, "protected");
	userData.followersCount					= getValue<int64_t>(jsonData, "followers_count");
	userData.friendsCount					= getValue<int64_t>(jsonData, "friends_count");
	userData.listedCount					= getValue<int64_t>(jsonData, "listed_count");
	userData.createdAt						= getValue<std::string>(jsonData, "created_at");
	userData.favouritesCount				= getValue<int64_t>(jsonData, "favourites_count");
	userData.geoEnabled						= getValue<bool>(jsonData, "geo_enabled");
	userData.verified						= getValue<bool>(jsonData, "verified");
	userData.statusesCount					= getValue<int64_t>(jsonData, "statuses_count");
	userData.lang							= getValue<std::string>(jsonData, "lang");
	userData.contributorsEnabled			= getValue<bool>(jsonData, "contributors_enabled");
	userData.isTranslator					= getValue<bool>(jsonData, "is_translator");
	userData.isTranslationEnabled			= getValue<bool>(jsonData, "is_translation_enabled");
	userData.profileBackgroundColor			= getValue<std::string>(jsonData, "profile_background_color");
	userData.profileBackgroundImageUrl		= getValue<std::string>(jsonData, "profile_background_image_url");
	userData.profileBackgroundImageUrlHttps = getValue<std::string>(jsonData, "profile_background_image_url_https");
	userData.profileBackgroundTile			= getValue<bool>(jsonData, "profile_background_tile");
	userData.profileImageUrl				= getValue<std::string>(jsonData, "profile_image_url");
	userData.profileImageUrlHttps			= getValue<std::string>(jsonData, "profile_image_url_https");
	userData.profileBannerUrl				= getValue<std::string>(jsonData, "profile_banner_url");
	userData.profileLinkColor				= getValue<std::string>(jsonData, "profile_link_color");
	userData.profileSidebarBorderColor		= getValue<std::string>(jsonData, "profile_sidebar_border_color");
	userData.profileSidebarFillColor		= getValue<std::string>(jsonData, "profile_sidebar_fill_color");
	userData.profileTextColor				= getValue<std::string>(jsonData, "profile_text_color");
	userData.profileUseBackgroundImage		= getValue<bool>(jsonData, "profile_use_background_image");
	userData.defaultProfile					= getValue<bool>(jsonData, "default_profile");
	userData.defaultProfileImage			= getValue<bool>(jsonData, "default_profile_image");
	userData.following						= getValue<bool>(jsonData, "following");
	userData.followRequestSent				= getValue<bool>(jsonData, "follow_request_sent");
	userData.notifications					= getValue<bool>(jsonData, "notifications");

	return userData;
}

template<> status getValue<status>(simdjson::ondemand::value jsonData) {
	status statusData;

	statusData.metadataVal		 = getValue<metadata>(jsonData, "metadata");
	statusData.createdAt		 = getValue<std::string>(jsonData, "created_at");
	statusData.id				 = getValue<double>(jsonData, "id");
	statusData.idStr			 = getValue<std::string>(jsonData, "id_str");
	statusData.text				 = getValue<std::string>(jsonData, "text");
	statusData.source			 = getValue<std::string>(jsonData, "source");
	statusData.truncated		 = getValue<bool>(jsonData, "truncated");
	statusData.userVal			 = getValue<user>(jsonData, "user");
	statusData.retweetCount		 = getValue<int64_t>(jsonData, "retweet_count");
	statusData.favoriteCount	 = getValue<int64_t>(jsonData, "favorite_count");
	statusData.entities			 = getValue<status_entities>(jsonData, "entities");
	statusData.favorited		 = getValue<bool>(jsonData, "favorited");
	statusData.retweeted		 = getValue<bool>(jsonData, "retweeted");
	statusData.lang				 = getValue<std::string>(jsonData, "lang");
	statusData.possiblySensitive = getValue<bool>(jsonData, "possibly_sensitive");

	return statusData;
}

template<> twitter_message getValue<twitter_message>(simdjson::ondemand::value jsonData) {
	twitter_message message;
	getValues<status>(message.statuses, jsonData, "statuses");
	message.searchMetadata = getValue<search_metadata>(jsonData, "search_metadata");

	return message;
}

template<> user_data getValue<user_data>(simdjson::ondemand::value jsonData) {
	user_data userData;

	userData.avatarDecoration = getValue<std::string>(jsonData, "avatar_decoration");
	userData.discriminator	  = getValue<std::string>(jsonData, "discriminator");
	userData.globalName		  = getValue<std::string>(jsonData, "global_name");
	userData.userName		  = getValue<std::string>(jsonData, "user_name");
	userData.accentColor	  = getValue<uint64_t>(jsonData, "accent_color");
	userData.premiumType	  = getValue<uint64_t>(jsonData, "premium_type");
	userData.publicFlags	  = getValue<uint64_t>(jsonData, "public_flags");
	userData.locale			  = getValue<std::string>(jsonData, "locale");
	userData.banner			  = getValue<std::string>(jsonData, "banner");
	userData.avatar			  = getValue<std::string>(jsonData, "avatar");
	userData.email			  = getValue<std::string>(jsonData, "email");
	userData.mfaEnabled		  = getValue<bool>(jsonData, "mfa_enabled");
	userData.id				  = getValue<std::string>(jsonData, "id");
	userData.flags			  = getValue<uint64_t>(jsonData, "flags");
	userData.verified		  = getValue<bool>(jsonData, "verified");
	userData.system			  = getValue<bool>(jsonData, "system");
	userData.bot			  = getValue<bool>(jsonData, "bot");

	return userData;
}

template<> guild_scheduled_event_data getValue<guild_scheduled_event_data>(simdjson::ondemand::value jsonData) {
	guild_scheduled_event_data eventData;

	eventData.scheduledStartTime = getValue<std::string>(jsonData, "scheduled_start_time");
	eventData.scheduledEndTime	 = getValue<std::string>(jsonData, "scheduled_end_time");
	eventData.description		 = getValue<std::string>(jsonData, "description");
	eventData.entityMetadata	 = getValue<uint64_t>(jsonData, "entity_metadata");
	eventData.creatorId			 = getValue<std::string>(jsonData, "creator_id");
	eventData.channelId			 = getValue<std::string>(jsonData, "channel_id");
	eventData.privacyLevel		 = getValue<uint64_t>(jsonData, "privacy_level");
	eventData.entityId			 = getValue<std::string>(jsonData, "entity_id");
	eventData.guildId			 = getValue<std::string>(jsonData, "guild_id");
	eventData.entityType		 = getValue<uint64_t>(jsonData, "entity_type");
	eventData.userCount			 = getValue<uint64_t>(jsonData, "user_count");
	eventData.creator			 = getValue<user_data>(jsonData, "creator");
	eventData.name				 = getValue<std::string>(jsonData, "name");
	eventData.status			 = getValue<uint64_t>(jsonData, "status");
	eventData.id				 = getValue<std::string>(jsonData, "id");

	return eventData;
}

template<> channel_data getValue<channel_data>(simdjson::ondemand::value jsonData) {
	channel_data channelData;

	channelData.defaultThreadRateLimitPerUser = getValue<uint64_t>(jsonData, "default_thread_rate_limit_per_user");

	channelData.defaultAutoArchiveDuration = getValue<uint64_t>(jsonData, "default_auto_archive_duration");

	channelData.lastPinTimestamp = getValue<std::string>(jsonData, "last_pin_timestamp");
	channelData.totalMessageSent = getValue<uint64_t>(jsonData, "total_message_sent");
	channelData.rateLimitPerUser = getValue<uint64_t>(jsonData, "rate_limit_per_user");
	channelData.videoQualityMode = getValue<uint64_t>(jsonData, "video_quality_mode");
	channelData.lastMessageId	 = getValue<std::string>(jsonData, "last_message_id");
	channelData.applicationId	 = getValue<std::string>(jsonData, "application_id");
	channelData.permissions		 = getValue<std::string>(jsonData, "permissions");
	channelData.rtcRegion		 = getValue<std::string>(jsonData, "rtc_region");
	channelData.messageCount	 = getValue<uint64_t>(jsonData, "message_count");
	channelData.memberCount		 = getValue<uint64_t>(jsonData, "member_count");
	channelData.parentId		 = getValue<std::string>(jsonData, "parent_id");
	channelData.ownerId			 = getValue<std::string>(jsonData, "owner_id");
	channelData.guildId			 = getValue<std::string>(jsonData, "guild_id");
	channelData.userLimit		 = getValue<uint64_t>(jsonData, "user_limit");
	channelData.topic			 = getValue<std::string>(jsonData, "topic");
	channelData.position		 = getValue<uint64_t>(jsonData, "position");
	channelData.bitrate			 = getValue<uint64_t>(jsonData, "bitrate");
	channelData.name			 = getValue<std::string>(jsonData, "name");
	channelData.icon			 = getValue<std::string>(jsonData, "icon");
	channelData.id				 = getValue<std::string>(jsonData, "id");
	channelData.flags			 = getValue<uint64_t>(jsonData, "flags");
	channelData.type			 = getValue<uint64_t>(jsonData, "type");
	channelData.managed			 = getValue<bool>(jsonData, "managed");
	channelData.nsfw			 = getValue<bool>(jsonData, "nsfw");

	return channelData;
}

template<> role_data getValue<role_data>(simdjson::ondemand::value jsonData) {
	role_data roleData;

	roleData.unicodeEmoji = getValue<std::string>(jsonData, "unicode_emoji");
	roleData.permissions  = getValue<std::string>(jsonData, "permissions");
	roleData.position	  = getValue<uint64_t>(jsonData, "position");
	roleData.name		  = getValue<std::string>(jsonData, "name");
	roleData.icon		  = getValue<std::string>(jsonData, "icon");
	roleData.mentionable  = getValue<bool>(jsonData, "mentionable");
	roleData.color		  = getValue<uint64_t>(jsonData, "color");
	roleData.id			  = getValue<std::string>(jsonData, "id");
	roleData.flags		  = getValue<uint64_t>(jsonData, "flags");
	roleData.managed	  = getValue<bool>(jsonData, "managed");
	roleData.hoist		  = getValue<bool>(jsonData, "hoist");

	return roleData;
}

template<> guild_member_data getValue<guild_member_data>(simdjson::ondemand::value jsonData) {
	guild_member_data memberData;

	memberData.communicationDisabledUntil = getValue<std::string>(jsonData, "communication_disabled_until");
	getValues<std::string>(memberData.roles, jsonData, "roles");
	memberData.premiumSince				  = getValue<std::string>(jsonData, "premium_since");
	memberData.permissions				  = getValue<std::string>(jsonData, "permissions");
	memberData.joinedAt					  = getValue<std::string>(jsonData, "joined_at");
	memberData.guildId					  = getValue<std::string>(jsonData, "guild_id");
	memberData.avatar					  = getValue<std::string>(jsonData, "avatar");
	memberData.nick						  = getValue<std::string>(jsonData, "nick");
	memberData.user						  = getValue<user_data>(jsonData, "user");
	memberData.flags					  = getValue<uint64_t>(jsonData, "flags");
	memberData.pending					  = getValue<bool>(jsonData, "pending");
	memberData.deaf						  = getValue<bool>(jsonData, "deaf");
	memberData.mute						  = getValue<bool>(jsonData, "mute");

	return memberData;
}

template<> guild_data getValue<guild_data>(simdjson::ondemand::value jsonData) {
	guild_data guildData;

	getValues<guild_scheduled_event_data>(guildData.guildScheduledEvents, jsonData, "guild_scheduled_events");

	getValues<guild_member_data>(guildData.members, jsonData, "members");

	guildData.defaultMessageNotifications = getValue<uint64_t>(jsonData, "default_message_notifications");

	getValues<channel_data>(guildData.channels, jsonData, "channels");

	guildData.maxStageVideoChannelUsers = getValue<uint64_t>(jsonData, "max_stage_video_channel_users");
	guildData.publicUpdatesChannelId	= getValue<std::string>(jsonData, "public_updates_channel_id");
	guildData.premiumSubscriptionCount	= getValue<uint64_t>(jsonData, "premium_subscription_count");

	getValues<std::string>(guildData.features, jsonData, "features");

	guildData.approximatePresenceCount	= getValue<uint64_t>(jsonData, "approximate_presence_count");
	guildData.safetyAlertsChannelId		= getValue<std::string>(jsonData, "safety_alerts_channel_id");
	guildData.approximateMemberCount	= getValue<uint64_t>(jsonData, "approximate_member_count");
	guildData.premiumProgressBarEnabled = getValue<bool>(jsonData, "premium_progress_bar_enabled");
	guildData.explicitContentFilter		= getValue<uint64_t>(jsonData, "explicit_content_filter");
	guildData.maxVideoChannelUsers		= getValue<uint64_t>(jsonData, "max_video_channel_users");

	getValues<role_data>(guildData.roles, jsonData, "roles");

	guildData.systemChannelId	 = getValue<std::string>(jsonData, "system_channel_id");
	guildData.widgetChannelId	 = getValue<std::string>(jsonData, "widget_channel_id");
	guildData.preferredLocale	 = getValue<std::string>(jsonData, "preferred_locale");
	guildData.discoverySplash	 = getValue<std::string>(jsonData, "discovery_splash");
	guildData.systemChannelFlags = getValue<uint64_t>(jsonData, "system_channel_flags");
	guildData.rulesChannelId	 = getValue<std::string>(jsonData, "rules_channel_id");
	guildData.verificationLevel	 = getValue<uint64_t>(jsonData, "verification_level");
	guildData.applicationId		 = getValue<std::string>(jsonData, "application_id");
	guildData.vanityUrlCode		 = getValue<std::string>(jsonData, "vanity_url_code");
	guildData.afkChannelId		 = getValue<std::string>(jsonData, "afk_channel_id");
	guildData.description		 = getValue<std::string>(jsonData, "description");
	guildData.permissions		 = getValue<std::string>(jsonData, "permissions");
	guildData.maxPresences		 = getValue<uint64_t>(jsonData, "max_presences");
	guildData.discovery			 = getValue<std::string>(jsonData, "discovery");
	guildData.memberCount		 = getValue<uint64_t>(jsonData, "member_count");
	guildData.joinedAt			 = getValue<std::string>(jsonData, "joined_at");
	guildData.premiumTier		 = getValue<uint64_t>(jsonData, "premium_tier");
	guildData.ownerId			 = getValue<std::string>(jsonData, "owner_id");
	guildData.maxMembers		 = getValue<uint64_t>(jsonData, "max_members");
	guildData.afkTimeout		 = getValue<uint64_t>(jsonData, "afk_timeout");
	guildData.splash			 = getValue<std::string>(jsonData, "splash");
	guildData.banner			 = getValue<std::string>(jsonData, "banner");
	guildData.widgetEnabled		 = getValue<bool>(jsonData, "widget_enabled");
	guildData.nsfwLevel			 = getValue<uint64_t>(jsonData, "nsfw_level");
	guildData.mfaLevel			 = getValue<uint64_t>(jsonData, "mfa_level");
	guildData.name				 = getValue<std::string>(jsonData, "name");
	guildData.icon				 = getValue<std::string>(jsonData, "icon");
	guildData.unavailable		 = getValue<bool>(jsonData, "unavailable");
	guildData.id				 = getValue<std::string>(jsonData, "id");
	guildData.flags				 = getValue<uint64_t>(jsonData, "flags");
	guildData.large				 = getValue<bool>(jsonData, "large");
	guildData.owner				 = getValue<bool>(jsonData, "owner");

	return guildData;
}

template<> discord_message getValue<discord_message>(simdjson::ondemand::value jsonData) {
	discord_message message;

	message.t = getValue<std::string>(jsonData, "t");

	message.s = getValue<uint64_t>(jsonData, "s");

	message.impl = getValue<uint64_t>(jsonData, "impl");

	message.d = getValue<guild_data>(jsonData, "d");

	return message;
}

#define SIMD_Pull(x) \
		{ \
			ondemand::array newX = doc[#x].get_array().value(); \
			ondemand::array newArray{}; \
			for (ondemand::value value: newX) { \
				test_struct newStruct{}; \
				getValues<int64_t>(newStruct.testInts ,value, "testInts"); \
				getValues<double>(newStruct.testDoubles ,value, "testDoubles"); \
				getValues<std::string>(newStruct.testStrings ,value, "testStrings"); \
				getValues02<bool>(newStruct.testBools ,value, "testBools"); \
				getValues<uint64_t>(newStruct.testUints ,value, "testUints"); \
				obj.x.emplace_back(std::move(newStruct)); \
			} \
		}

bool on_demand::readInOrder(Test<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(a);
	SIMD_Pull(b);
	SIMD_Pull(c);
	SIMD_Pull(d);
	SIMD_Pull(e);
	SIMD_Pull(f);
	SIMD_Pull(g);
	SIMD_Pull(h);
	SIMD_Pull(i);
	SIMD_Pull(j);
	SIMD_Pull(k);
	SIMD_Pull(l);
	SIMD_Pull(m);
	SIMD_Pull(n);
	SIMD_Pull(o);
	SIMD_Pull(p);
	SIMD_Pull(q);
	SIMD_Pull(r);
	SIMD_Pull(s);
	SIMD_Pull(t);
	SIMD_Pull(u);
	SIMD_Pull(v);
	SIMD_Pull(w);
	SIMD_Pull(x);
	SIMD_Pull(y);
	SIMD_Pull(z);
	return false;
}

bool on_demand_abc::readOutOfOrder(AbcTest<test_struct>& obj, const padded_string& json) {
	ondemand::document doc = parser.iterate(json).value();
	SIMD_Pull(z);
	SIMD_Pull(y);
	SIMD_Pull(x);
	SIMD_Pull(w);
	SIMD_Pull(v);
	SIMD_Pull(u);
	SIMD_Pull(t);
	SIMD_Pull(s);
	SIMD_Pull(r);
	SIMD_Pull(q);
	SIMD_Pull(p);
	SIMD_Pull(o);
	SIMD_Pull(n);
	SIMD_Pull(m);
	SIMD_Pull(l);
	SIMD_Pull(k);
	SIMD_Pull(j);
	SIMD_Pull(i);
	SIMD_Pull(h);
	SIMD_Pull(g);
	SIMD_Pull(f);
	SIMD_Pull(e);
	SIMD_Pull(d);
	SIMD_Pull(c);
	SIMD_Pull(b);
	SIMD_Pull(a);
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

	discord_message welcomeData;

	auto result = benchmark(
		[&]() {
			try {
				auto doc	= parser.iterate(buffer);
				welcomeData = getValue<discord_message>(doc.value());
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
		jsonifierTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierTest(jsonData));

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
		jsonifierTwitterTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierTwitterTest(jsonData));

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
		jsonifierAbcTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierAbcTest(jsonData));

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
		jsonifierDiscordTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierDiscordTest(jsonData));

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
		jsonifierSingleTest(jsonData, false);
	}
	resultsNew.emplace_back(jsonifierSingleTest(jsonData));

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
	media mediaVal{};
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
	media mediaVal{};
};

struct welcome_element {
	jsonifier::raw_json_data data{};
	jsonifier::string hydratable{};
};

struct welcome {
	std::vector<welcome_element> data{};
};


namespace jsonifier {

	template<> struct core<data_class> {
		using value_type = data_class;
		static constexpr auto parseValue =
			createValue("artwork_url", &value_type::artworkUrl, "description", &value_type::description, "duration", &value_type::duration, "media", &value_type::mediaVal, "title",
				&value_type::title, "track_authorization", &value_type::trackAuthorization, "avatar_url", &value_type::avatarUrl, "permalink_url", &value_type::viewUrl);
	};

	template<> struct core<welcome_element> {
		using value_type				 = welcome_element;
		static constexpr auto parseValue = createValue("data", &value_type::data, "hydratable", &value_type::hydratable);
	};

	template<> struct core<welcome> {
		using value_type				 = welcome;
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
			&value_type::mediaVal, "title", &value_type::title, "track_authorization", &value_type::trackAuthorization, "permalink_url", &value_type::viewUrl);
	};

	template<> struct core<sound_cloud_search_results> {
		using value_type				 = sound_cloud_search_results;
		static constexpr auto parseValue = createValue("collection", &value_type::collection);
	};
}

template<typename... value_types> struct variant_ptr {};

int32_t main() {
	try {
		jsonifier::jsonifier_core parser{};
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
			"[{\"hydratable\":\"anonymousId\",\"data\":\"357581-895576-585319-642699\"},{\"hydratable\":\"features\",\"data\":{\"features\":[\"mobi_webauth_oauth_mode\",\"cd_repost_to_artists\",\"v2_use_onetrust_tcfv2_us_ca\",\"mobi_enable_onetrust_tcfv2\",\"mobi_tracking_send_session_id\",\"mobi_use_onetrust_eu1\",\"mobi_use_onetrust_gb\",\"mobi_use_onetrust_tcfv2_us_ca\",\"v2_use_onetrust_user_id_eu2\",\"v2_enable_sourcepoint_tcfv2\",\"mobi_peace_ukraine_logo_takeover\",\"mobi_use_auth_internal_analytics\",\"v2_peace_ukraine_logo_takeover\",\"mobi_use_onetrust_tcfv2_eu2\",\"checkout_send_segment_events_to_event_gateway\",\"mobi_use_onetrust_user_id_eu1\",\"trolley\",\"v2_nigeria_creator_banner\",\"mobi_use_onetrust_user_id_ex_us\",\"mobi_use_onetrust_tcfv2_eu1\",\"v2_post_with_caption\",\"v2_report_content_links\",\"mobi_use_dwt\",\"v2_use_onetrust_tcfv2_eu1\",\"mobi_use_onetrust_eu4\",\"featured_artists_banner\",\"v2_repost_redirect_page\",\"v2_use_onetrust_gb\",\"use_onetrust_async\",\"v2_signals_collection\",\"v2_track_level_distro_to_plan_picker\",\"v2_direct_support_link\",\"v2_api_auth_sign_out\",\"v2_ie11_support_end\",\"checkout_use_new_connect\",\"mobi_report_content_links\",\"v2_tracking_moengage_integration\",\"v2_hq_file_storage_release\",\"creator_plan_names_repositioning\",\"v2_use_onetrust_eu4\",\"v2_stories_onboarding\",\"mobi_use_onetrust_user_id_eu2\",\"mobi_tracking_moengage_integration\",\"mobi_enable_2fa_config\",\"v2_use_dwt\",\"v2_use_updated_alert_banner_quota_upsell\",\"mobi_nea_report_content_links\",\"v2_enable_onetrust\",\"mobi_nea_report_content_form\",\"v2_import_playlist_experiment\",\"v2_disable_sidebar_comments_count\",\"v2_subhub_churn_intercept\",\"checkout_use_new_plan_picker\",\"v2_signage_on_home\",\"v2_use_onetrust_eu2\",\"next_pro_first_fans\",\"fpi_messaging_drawer\",\"v2_use_onetrust_us\",\"v2_comment_sorting\",\"checkout_use_recurly_with_paypal\",\"v2_use_onetrust_tcfv2_ex_us\",\"v2_show_for_artists_link\",\"mobi_use_onetrust_eu3\",\"mobi_use_onetrust_elsewhere\",\"mobi_report_content_form\",\"v2_use_onetrust_eu3\",\"mobi_use_onetrust_us\",\"v2_oscp_german_tax_fields_support\",\"v2_fallback_queue_for_search\",\"v2_use_onetrust_user_id_ex_us\",\"v2_use_new_connect\",\"v2_use_onetrust_tcfv2_eu2\",\"v2_next_pro_brazil_banner\",\"v2_send_segment_events_to_event_gateway\",\"v2_use_onetrust_eu1\",\"v2_enable_sourcepoint\",\"v2_repost_with_caption_graphql\",\"mobi_use_onetrust_tcfv2_ex_us\",\"v2_tags_recent_tracks\",\"mobi_use_onetrust_eu2\",\"v2_enable_new_web_errors\",\"v2_use_onetrust_elsewhere\",\"checkout_use_dwt\",\"mobi_sign_in_experiment\",\"mobi_enable_onetrust\",\"v2_nea_report_content_links\",\"v2_webauth_use_local_tracking\",\"v2_can_see_insights\",\"fpi_20_fans_rollout\",\"mobi_trinity\",\"v2_like_comments\",\"v2_webauth_oauth_mode\",\"v2_google_one_tap\",\"v2_enable_pwa\",\"mobi_use_hls_hack\",\"v2_stories\",\"v2_next_pro_referral_banner\",\"v2_use_onetrust_user_id_eu1\",\"v2_use_onetrust_user_id_global\",\"use_recurly_checkout\",\"v2_show_side_by_side_upsell_experience\",\"v2_enable_onetrust_tcfv2\",\"v2_enable_tcfv2_consent_string_cache\",\"v2_track_manager_redirection\",\"mobi_send_segment_events_to_event_gateway\",\"use_on_soundcloud_short_links\""
				"]}},{\"hydratable\":\"experiments\",\"data\":{}},{\"hydratable\":\"geoip\",\"data\":{\"country_code\":\"CA\",\"country_name\":\"Canada\",\"region\":\"ON\",\"city\":\"Scarborough\",\"postal_code\":\"M1L\",\"latitude\":43.7108,\"longitude\":-79.2812}},{\"hydratable\":\"privacySettings\",\"data\":{\"allows_messages_from_unfollowed_users\":false,\"analytics_opt_in\":true,\"communications_opt_in\":true,\"targeted_advertising_opt_in\":false,\"legislation\":[]}},{\"hydratable\":\"trackingBrowserTabId\",\"data\":\"6254f4\"},{\"hydratable\":\"user\",\"data\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-uTHPYYgM5YCScfMN-Qu5qcw-large.jpg\",\"city\":\"The Poolrooms\",\"comments_count\":0,\"country_code\":\"US\",\"created_at\":\"2020-05-25T03:52:47Z\",\"creator_subscriptions\":[{\"product\":{\"id\":\"free\"}}],\"creator_subscription\":{\"product\":{\"id\":\"free\"}},\"description\":\"[WAKE UP]\\\n[THIS IS ALL JUST A DREAM]\\\n\\\n\\\n[19]\",\"followers_count\":72,\"followings_count\":874,\"first_name\":\"Nick\",\"full_name\":\"Nick Okroy\",\"groups_count\":0,\"id\":828548983,\"kind\":\"user\",\"last_modified\":\"2023-07-30T15:59:12Z\",\"last_name\":\"Okroy\",\"likes_count\":10345,\"playlist_likes_count\":587,\"permalink\":\"nick-okroy\",\"permalink_url\":\"https://soundcloud.com/nick-okroy\",\"playlist_count\":35,\"reposts_count\":null,\"track_count\":0,\"uri\":\"https://api.soundcloud.com/users/828548983\",\"urn\":\"soundcloud:users:828548983\",\"username\":\"umbra somnia\",\"verified\":false,\"visuals\":{\"urn\":\"soundcloud:users:828548983\",\"enabled\":true,\"visuals\":[{\"urn\":\"soundcloud:visuals:197782830\",\"entry_time\":0,\"visual_url\":\"https://i1.sndcdn.com/visuals-e5vBeRRCc5ncBCi5-ctOgmQ-original.jpg\"}],\"tracking\":null},\"badges\":{\"pro\":false,\"pro_unlimited\":false,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:828548983\",\"station_permalink\":\"artist-stations:828548983\",\"url\":\"/nick-okroy\"}},{\"hydratable\":\"playlist\",\"data\":{\"artwork_url\":\"https://i1.sndcdn.com/artworks-rzc34GwRwLlIktBm-kUuccQ-large.jpg\",\"created_at\":\"2023-08-03T03:30:04Z\",\"description\":\"\",\"duration\":55609622,\"embeddable_by\":\"all\",\"genre\":\"\",\"id\":1663340599,\"kind\":\"playlist\",\"label_name\":\"\",\"last_modified\":\"2023-12-27T19:17:45Z\",\"license\":\"all-rights-reserved\",\"likes_count\":3,\"managed_by_feeds\":false,\"permalink\":\"2023-selects-catalogue-ii\",\"permalink_url\":\"https://soundcloud.com/nick-okroy/sets/2023-selects-catalogue-ii\",\"public\":true,\"purchase_title\":null,\"purchase_url\":null,\"release_date\":null,\"reposts_count\":1,\"secret_token\":null,\"sharing\":\"public\",\"tag_list\":\"\",\"title\":\"2023 SELECTS CATALOGUE II\",\"uri\":\"https://api.soundcloud.com/playlists/1663340599\",\"user_id\":828548983,\"set_type\":\"\",\"is_album\":false,\"published_at\":\"2023-08-13T04:25:36Z\",\"display_date\":\"2023-08-13T04:25:36Z\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-uTHPYYgM5YCScfMN-Qu5qcw-large.jpg\",\"city\":\"The Poolrooms\",\"comments_count\":0,\"country_code\":\"US\",\"created_at\":\"2020-05-25T03:52:47Z\",\"creator_subscriptions\":[{\"product\":{\"id\":\"free\"}}],\"creator_subscription\":{\"product\":{\"id\":\"free\"}},\"description\":\"[WAKE UP]\\\n[THIS IS ALL JUST A DREAM]\\\n\\\n\\\n[19]\",\"followers_count\""
				":72,\"followings_count\":874,\"first_name\":\"Nick\",\"full_name\":\"Nick Okroy\",\"groups_count\":0,\"id\":828548983,\"kind\":\"user\",\"last_modified\":\"2023-07-30T15:59:12Z\",\"last_name\":\"Okroy\",\"likes_count\":10345,\"playlist_likes_count\":587,\"permalink\":\"nick-okroy\",\"permalink_url\":\"https://soundcloud.com/nick-okroy\",\"playlist_count\":35,\"reposts_count\":null,\"track_count\":0,\"uri\":\"https://api.soundcloud.com/users/828548983\",\"urn\":\"soundcloud:users:828548983\",\"username\":\"umbra somnia\",\"verified\":false,\"visuals\":{\"urn\":\"soundcloud:users:828548983\",\"enabled\":true,\"visuals\":[{\"urn\":\"soundcloud:visuals:197782830\",\"entry_time\":0,\"visual_url\":\"https://i1.sndcdn.com/visuals-e5vBeRRCc5ncBCi5-ctOgmQ-original.jpg\"}],\"tracking\":null},\"badges\":{\"pro\":false,\"pro_unlimited\":false,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:828548983\",\"station_permalink\":\"artist-stations:828548983\"},\"tracks\":[{\"artwork_url\":null,\"caption\":null,\"commentable\":true,\"comment_count\":223,\"created_at\":\"2023-07-26T06:28:25Z\",\"description\":\"TRACK 1: https://soundcloud.com/extretch\\\nTRACK 2: https://soundcloud.com/neo-krono\\\n\\\nCOMMENT WHO YOU THINK WON\",\"downloadable\":false,\"download_count\":0,\"duration\":234136,\"full_duration\":234136,\"embeddable_by\":\"all\",\"genre\":\"\",\"has_downloads_left\":false,\"id\":1574716762,\"kind\":\"track\",\"label_name\":null,\"last_modified\":\"2023-07-27T02:45:37Z\",\"license\":\"all-rights-reserved\",\"likes_count\":178,\"permalink\":\"season-2-round-2-extretch-vs\",\"permalink_url\":\"https://soundcloud.com/dubduels/season-2-round-2-extretch-vs\",\"playback_count\":3005,\"public\":true,\"publisher_metadata\":{\"id\":1574716762,\"urn\":\"soundcloud:tracks:1574716762\",\"contains_music\":true},\"purchase_title\":null,\"purchase_url\":null,\"release_date\":null,\"reposts_count\":18,\"secret_token\":null,\"sharing\":\"public\",\"state\":\"finished\",\"streamable\":true,\"tag_list\":\"\",\"title\":\"SEASON 2 ROUND 2: EXTRETCH VS NEOKRONO [ WINNER: EXTRETCH ]\",\"track_format\":\"single-track\",\"uri\":\"https://api.soundcloud.com/tracks/1574716762\",\"urn\":\"soundcloud:tracks:1574716762\",\"user_id\":1198194871,\"visuals\":null,\"waveform_url\":\"https://wave.sndcdn.com/9D4omb8rW4oz_m.json\",\"display_date\":\"2023-07-26T06:28:25Z\",\"media\":{\"transcodings\":[{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1574716762/d0207db7-4f28-4260-ab9b-ae411336a072/stream/hls\",\"preset\":\"mp3_1_0\",\"duration\":234136,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1574716762/d0207db7-4f28-4260-ab9b-ae411336a072/stream/progressive\",\"preset\":\"mp3_1_0\",\"duration\":234136,\"snipped\":false,\"format\":{\"protocol\":\"progressive\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1574716762/6900e688-0aff-498c-866e-66e510f4f23c/stream/hls\",\"preset\":\"opus_0_0\",\"duration\":234109,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/ogg; codecs=\"opus\"\"},\"quality\":\"sq\"}]},\"station_urn\":\"soundcloud:system-playlists:"
				"track-stations:1574716762\",\"station_permalink\":\"track-stations:1574716762\",\"track_authorization\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJnZW8iOiJDQSIsInN1YiI6IiIsInJpZCI6Ijg5Yjc2N2M1LTVhNGMtNDFlNC05NzBlLWMwMmNhNDA4ODJjYSIsImlhdCI6MTcwMzcxNTAyN30.-8Eq90zu8ebZCLk07FPFeUFw2dorVS_8DObOHpvAatQ\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-jjVXdNsdv7zpBL55-i6vpqg-large.jpg\",\"first_name\":\"\",\"followers_count\":1600,\"full_name\":\"\",\"id\":1198194871,\"kind\":\"user\",\"last_modified\":\"2023-07-19T13:31:12Z\",\"last_name\":\"\",\"permalink\":\"dubduels\",\"permalink_url\":\"https://soundcloud.com/dubduels\",\"uri\":\"https://api.soundcloud.com/users/1198194871\",\"urn\":\"soundcloud:users:1198194871\",\"username\":\"DUB DUELS\",\"verified\":false,\"city\":null,\"country_code\":null,\"badges\":{\"pro\":false,\"pro_unlimited\":false,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:1198194871\",\"station_permalink\":\"artist-stations:1198194871\"}},{\"artwork_url\":\"https://i1.sndcdn.com/artworks-xESukFyLjWKewEyy-rw3pjA-large.jpg\",\"caption\":null,\"commentable\":true,\"comment_count\":249,\"created_at\":\"2023-07-27T00:54:40Z\",\"description\":\"YOOOOOOOOO THANK YOU ALL SO MUCH FOR 2K FOLLOWERS!!! i really really appreciate it and i cant thank you all enough for the insane support \\\u003c3\\\nhere's the full ep + the sample pack! putting up the stems on twitter\\\nILY ALL SO MUCH\\\n\\\nDL LINKS AT THE BOTTOM OF THE BIO\\\n----------------------------------------------------------------------\\\n\\\nsupport me!\\\n\\\u003e@sakuro-dubstep\\\n\\\u003etwitter.com/sakurodubs\\\n\\\u003eyoutube.com/@sakurodubs\\\n\\\u003elinktr.ee/sakurodubs\\\n\\\n----------------------------------------------------------------------\\\nDL LINK FOR THE TRACK\\\nhttps://hypeddit.com/sakuro/sakuroymir\\\nDIRECT DLS ARE ON\\\n\\\nDL LINK FOR THE SAMPLE PACK\\\nhypeddit.com/sakuro/soundsofsakurovol7\\\n\\\nDL LINK FOR THE STEMS\\\nhttps://hypeddit.com/sakuro/stalkerstems\\\n\\\nif you like this track please consider leaving a like, following, or sharing this with others you think would enjoy this!\\\nAgain, thank you guys so much for this! I never would've got here without you. \\\u003c3\\\n\\\n- sakuro\",\"downloadable\":true,\"download_count\":327,\"duration\":279458,\"full_duration\":279458,\"embeddable_by\":\"all\",\"genre\":\"Dubstep\",\"has_downloads_left\":true,\"id\":1575337162,\"kind\":\"track\",\"label_name\":null,\"last_modified\":\"2023-07-31T22:45:55Z\",\"license\":\"all-rights-reserved\",\"likes_count\":1211,\"permalink\":\"sakuro-ymir\",\"permalink_url\":\"https://soundcloud.com/sakuro-dubstep/sakuro-ymir\",\"playback_count\":13388,\"public\":true,\"publisher_metadata\":{\"id\":1575337162,\"urn\":\"soundcloud:tracks:1575337162\",\"artist\":\"SAKURO\",\"contains_music\":true,\"explicit\":false,\"writer_composer\":\"SAKURO\"},\"purchase_title\":\"FREE DL\",\"purchase_url\":\"https://hypeddit.com/sakuro/sakuroymir\",\"release_date\":\"2023-07-31T00:00:00Z\",\"reposts_count\":254,\"secret_token\":null,\"sharing\":\"public\",\"state\":\"finished\",\"streamable\":"
				"true,\"tag_list\":\"trench riddim \"riddim dubstep\" \"og riddim\" \"classic riddim\" \"real riddim\" flow 140 ID titan ep free freebie \"free download\" download sample \"sample pack\" pack 2k\",\"title\":\"SAKURO - YMIR [2K FREEBIE + PACK]\",\"track_format\":\"single-track\",\"uri\":\"https://api.soundcloud.com/tracks/1575337162\",\"urn\":\"soundcloud:tracks:1575337162\",\"user_id\":1040500045,\"visuals\":null,\"waveform_url\":\"https://wave.sndcdn.com/4hjCkQn7YH01_m.json\",\"display_date\":\"2023-07-31T22:45:55Z\",\"media\":{\"transcodings\":[{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337162/e824a482-9646-4bf5-a8ed-8a7358d83851/stream/hls\",\"preset\":\"mp3_1_0\",\"duration\":279458,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337162/e824a482-9646-4bf5-a8ed-8a7358d83851/stream/progressive\",\"preset\":\"mp3_1_0\",\"duration\":279458,\"snipped\":false,\"format\":{\"protocol\":\"progressive\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337162/d5c146f5-c739-44e6-b301-68de9426b0c8/stream/hls\",\"preset\":\"opus_0_0\",\"duration\":279435,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/ogg; codecs=\"opus\"\"},\"quality\":\"sq\"}]},\"station_urn\":\"soundcloud:system-playlists:track-stations:1575337162\",\"station_permalink\":\"track-stations:1575337162\",\"track_authorization\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJnZW8iOiJDQSIsInN1YiI6IiIsInJpZCI6ImQzZWEzZGI4LWRjNTktNGVhOC1hNjliLTYwMmI0NTE3ZDg2ZiIsImlhdCI6MTcwMzcxNTAyN30.wQ-u1wZogmSCFdhBdDnPVx0fQJgEFe9w3jHLN1ubcXM\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-iPoJOpFqEyWPhmoT-bT2chQ-large.jpg\",\"first_name\":\"\",\"followers_count\":3518,\"full_name\":\"\",\"id\":1040500045,\"kind\":\"user\",\"last_modified\":\"2023-12-24T00:51:13Z\",\"last_name\":\"\",\"permalink\":\"sakuro-dubstep\",\"permalink_url\":\"https://soundcloud.com/sakuro-dubstep\",\"uri\":\"https://api.soundcloud.com/users/1040500045\",\"urn\":\"soundcloud:users:1040500045\",\"username\":\"Sakuro\",\"verified\":false,\"city\":\"internet\",\"country_code\":null,\"badges\":{\"pro\":false,\"pro_unlimited\":true,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:1040500045\",\"station_permalink\":\"artist-stations:1040500045\"}},{\"artwork_url\":\"https://i1.sndcdn.com/artworks-xESukFyLjWKewEyy-rw3pjA-large.jpg\",\"caption\":null,\"commentable\":true,\"comment_count\":29,\"created_at\":\"2023-07-27T00:54:39Z\",\"description\":\"YOOOOOOOOO THANK YOU ALL SO MUCH FOR 2K FOLLOWERS!!! i really really appreciate it and i cant thank you all enough for the insane support \\\u003c3\\\nhere's the full ep + the sample pack! putting up the stems on twitter\\\nILY ALL SO MUCH\\\n\\\nDL LINKS AT THE BOTTOM OF THE BIO\\\n----------------------------------------------------------------------\\\n\\\nsupport me!\\\n\\\u003e@sakuro-dubstep\\\n\\\u003etwitter.com/sakurodubs\\\n\\\u003eyoutube.com/@sakurodubs\\\n\\\u00"
				"3elinktr.ee/sakurodubs\\\n\\\n----------------------------------------------------------------------\\\nDL LINK FOR THE TRACK\\\nhttps://hypeddit.com/sakuro/sakuroliminal\\\nDIRECT DLS ARE ON\\\n\\\nDL LINK FOR THE SAMPLE PACK\\\nhypeddit.com/sakuro/soundsofsakurovol7\\\n\\\nDL LINK FOR THE STEMS\\\nhttps://hypeddit.com/sakuro/stalkerstems\\\n\\\nif you like this track please consider leaving a like, following, or sharing this with others you think would enjoy this!\\\nAgain, thank you guys so much for this! I never would've got here without you. \\\u003c3\\\n\\\n- sakuro\",\"downloadable\":true,\"download_count\":103,\"duration\":168046,\"full_duration\":168046,\"embeddable_by\":\"all\",\"genre\":\"Dubstep\",\"has_downloads_left\":true,\"id\":1575337153,\"kind\":\"track\",\"label_name\":null,\"last_modified\":\"2023-07-31T22:46:04Z\",\"license\":\"all-rights-reserved\",\"likes_count\":208,\"permalink\":\"liminal\",\"permalink_url\":\"https://soundcloud.com/sakuro-dubstep/liminal\",\"playback_count\":3035,\"public\":true,\"publisher_metadata\":{\"id\":1575337153,\"urn\":\"soundcloud:tracks:1575337153\",\"contains_music\":true},\"purchase_title\":null,\"purchase_url\":null,\"release_date\":\"2023-07-26T00:00:00Z\",\"reposts_count\":29,\"secret_token\":null,\"sharing\":\"public\",\"state\":\"finished\",\"streamable\":true,\"tag_list\":\"\"deep dubstep\" Deep \"og dubstep\" \"real dubstep\" 140 liminal titan ep freebie free dl download \"free download\" \"free sample pack\" \"sample pack\" pack sakuro\",\"title\":\"SAKURO - LIMINAL [2K FREEBIE + PACK]\",\"track_format\":\"single-track\",\"uri\":\"https://api.soundcloud.com/tracks/1575337153\",\"urn\":\"soundcloud:tracks:1575337153\",\"user_id\":1040500045,\"visuals\":null,\"waveform_url\":\"https://wave.sndcdn.com/XBaZpTQ9feFs_m.json\",\"display_date\":\"2023-07-31T22:46:04Z\",\"media\":{\"transcodings\":[{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337153/d974930b-e1f6-47e4-9068-db41593d13b3/stream/hls\",\"preset\":\"mp3_1_0\",\"duration\":168046,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337153/d974930b-e1f6-47e4-9068-db41593d13b3/stream/progressive\",\"preset\":\"mp3_1_0\",\"duration\":168046,\"snipped\":false,\"format\":{\"protocol\":\"progressive\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1575337153/48040373-928e-4fdd-98f8-509b65534acc/stream/hls\",\"preset\":\"opus_0_0\",\"duration\":168007,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/ogg; codecs=\"opus\"\"},\"quality\":\"sq\"}]},\"station_urn\":\"soundcloud:system-playlists:track-stations:1575337153\",\"station_permalink\":\"track-stations:1575337153\",\"track_authorization\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJnZW8iOiJDQSIsInN1YiI6IiIsInJpZCI6IjcxZTk2NjNjLTEzZWYtNGYzMS1iNDdlLWNlZDM4OGVjZDk2NyIsImlhdCI6MTcwMzcxNTAyN30.9gNwyJsUsbRnEczlJnTWC2Md4cC9xr2HlWhvVCex0Vg\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-iPoJOpFqEyWPhmoT-"
				"bT2chQ-large.jpg\",\"first_name\":\"\",\"followers_count\":3518,\"full_name\":\"\",\"id\":1040500045,\"kind\":\"user\",\"last_modified\":\"2023-12-24T00:51:13Z\",\"last_name\":\"\",\"permalink\":\"sakuro-dubstep\",\"permalink_url\":\"https://soundcloud.com/sakuro-dubstep\",\"uri\":\"https://api.soundcloud.com/users/1040500045\",\"urn\":\"soundcloud:users:1040500045\",\"username\":\"Sakuro\",\"verified\":false,\"city\":\"internet\",\"country_code\":null,\"badges\":{\"pro\":false,\"pro_unlimited\":true,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:1040500045\",\"station_permalink\":\"artist-stations:1040500045\"}},{\"artwork_url\":\"https://i1.sndcdn.com/artworks-mdrszI3w3dz7isGu-yfFXmA-large.jpg\",\"caption\":null,\"commentable\":true,\"comment_count\":15,\"created_at\":\"2023-08-03T21:37:00Z\",\"description\":\"Follow \\\u0026 support CRYSTALLICA:\\\nSC: @user-814616964\\\nVK: vk.com/artofcrystallica\\\n\\\nFollow \\\u0026 support Dust:\\\nSC: @dustyboig\\\nBandcamp: dust7.bandcamp.com/music\\\n---\\\n\"Obsession\" will be available soon on all the streaming services.\\\nVisual conception is made by @slammerunderground.\",\"downloadable\":false,\"download_count\":0,\"duration\":198243,\"full_duration\":198243,\"embeddable_by\":\"all\",\"genre\":\"Crowd Murdering\",\"has_downloads_left\":false,\"id\":1582391163,\"kind\":\"track\",\"label_name\":\"Murder The Crowd\",\"last_modified\":\"2023-08-03T21:40:28Z\",\"license\":\"all-rights-reserved\",\"likes_count\":79,\"permalink\":\"obsession-dust-remix\",\"permalink_url\":\"https://soundcloud.com/murderthecrowd/obsession-dust-remix\",\"playback_count\":1193,\"public\":true,\"publisher_metadata\":{\"id\":1582391163,\"urn\":\"soundcloud:tracks:1582391163\",\"artist\":\"Dust\",\"album_title\":\"Obsession (Single)\",\"contains_music\":true,\"publisher\":\"Murder The Crowd\",\"isrc\":\"QM8DG2385682\",\"writer_composer\":\"Denno Samuelz\",\"release_title\":\"CRYSTALLICA - Obsession (Single)\"},\"purchase_title\":\"BUY\",\"purchase_url\":\"https://murderthecrowd.bandcamp.com/album/crystallica-obsession-single\",\"release_date\":\"2023-08-03T00:00:00Z\",\"reposts_count\":26,\"secret_token\":null,\"sharing\":\"public\",\"state\":\"finished\",\"streamable\":true,\"tag_list\":\"\"Murder The Crowd\" CRYSTALLICA Dust Minatory Breakcore \"Existentia Teeth\" \"Boiled Blood Addicted\" \"Seared 1: Embodied Ailment\"\",\"title\":\"CRYSTALLICA - Obsession (Dust Remix)\",\"track_format\":\"single-track\",\"uri\":\"https://api.soundcloud.com/tracks/1582391163\",\"urn\":\"soundcloud:tracks:1582391163\",\"user_id\":568250649,\"visuals\":null,\"waveform_url\":\"https://wave.sndcdn.com/0L23E37MMJhY_m.json\",\"display_date\":\"2023-08-03T21:40:28Z\",\"media\":{\"transcodings\":[{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1582391163/24bc0e70-6e65-4804-ad90-7819c6dfa820/stream/hls\",\"preset\":\"mp3_1_0\",\"duration\":198243,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1582391163/24bc0e70-6e65-4804-ad90-7819c6dfa820/stream/progressive\",\"preset\":\"mp3_1_0\",\"duration\":198243,\"snipped\":false,\"format\":{\"protocol\""
				":\"progressive\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1582391163/046a29eb-b73b-4d86-a82e-af20846b3c1a/stream/hls\",\"preset\":\"opus_0_0\",\"duration\":198216,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/ogg; codecs=\"opus\"\"},\"quality\":\"sq\"}]},\"station_urn\":\"soundcloud:system-playlists:track-stations:1582391163\",\"station_permalink\":\"track-stations:1582391163\",\"track_authorization\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJnZW8iOiJDQSIsInN1YiI6IiIsInJpZCI6ImY2NjZkNWJhLTFlZGMtNDMwNC1iZDMzLWM1MDVlYzI3NmMzYiIsImlhdCI6MTcwMzcxNTAyN30.mHB-s3jRArSpOSi2Ipz1hs4lmtISb01BpqY3ht8oGnY\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-o0JO1ewKogPhLuAg-F9Pl2A-large.jpg\",\"first_name\":\"\",\"followers_count\":2464,\"full_name\":\"\",\"id\":568250649,\"kind\":\"user\",\"last_modified\":\"2023-12-23T06:54:25Z\",\"last_name\":\"\",\"permalink\":\"murderthecrowd\",\"permalink_url\":\"https://soundcloud.com/murderthecrowd\",\"uri\":\"https://api.soundcloud.com/users/568250649\",\"urn\":\"soundcloud:users:568250649\",\"username\":\"Murder The Crowd\",\"verified\":false,\"city\":null,\"country_code\":null,\"badges\":{\"pro\":false,\"pro_unlimited\":true,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:568250649\",\"station_permalink\":\"artist-stations:568250649\"}},{\"artwork_url\":\"https://i1.sndcdn.com/artworks-nGBA8bpdNQtyU5pM-iGIyYg-large.jpg\",\"caption\":null,\"commentable\":true,\"comment_count\":22,\"created_at\":\"2023-07-14T04:31:51Z\",\"description\":\"[BUY = FREE DOWNLOAD]\\\n\\\n- ALL MY SOCIALS -\\\nlinktr.ee/prodsyzen\",\"downloadable\":false,\"download_count\":0,\"duration\":216033,\"full_duration\":216033,\"embeddable_by\":\"all\",\"genre\":\"Dubstep\",\"has_downloads_left\":false,\"id\":1564619803,\"kind\":\"track\",\"label_name\":null,\"last_modified\":\"2023-07-14T04:37:33Z\",\"license\":\"all-rights-reserved\",\"likes_count\":180,\"permalink\":\"carnage\",\"permalink_url\":\"https://soundcloud.com/syz3n/carnage\",\"playback_count\":3448,\"public\":true,\"publisher_metadata\":{\"id\":1564619803,\"urn\":\"soundcloud:tracks:1564619803\",\"contains_music\":true},\"purchase_title\":null,\"purchase_url\":\"https://www.toneden.io/syz3n/post/syz3n-carnage\",\"release_date\":\"2023-07-14T00:00:00Z\",\"reposts_count\":35,\"secret_token\":null,\"sharing\":\"public\",\"state\":\"finished\",\"streamable\":true,\"tag_list\":\"Tearout Riddim Trench Deathstep\",\"title\":\"CARNAGE\",\"track_format\":\"single-track\",\"uri\":\"https://api.soundcloud.com/tracks/1564619803\",\"urn\":\"soundcloud:tracks:1564619803\",\"user_id\":1103034523,\"visuals\":null,\"waveform_url\":\"https://wave.sndcdn.com/hoZOvP3MvHDm_m.json\",\"display_date\":\"2023-07-14T04:31:51Z\",\"media\":{\"transcodings\":[{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1564619803/f8685768-7dfb-4758-b91e-d076d4e3b8ac/stream/hls\",\"preset\":\"mp3_1_0\",\"duration\":216033,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1564619803/f8685768-7dfb-"
				"4758-b91e-d076d4e3b8ac/stream/progressive\",\"preset\":\"mp3_1_0\",\"duration\":216033,\"snipped\":false,\"format\":{\"protocol\":\"progressive\",\"mime_type\":\"audio/mpeg\"},\"quality\":\"sq\"},{\"url\":\"https://api-v2.soundcloud.com/media/soundcloud:tracks:1564619803/1b13ea26-13da-4fd3-9d0c-7a7755f85e68/stream/hls\",\"preset\":\"opus_0_0\",\"duration\":216007,\"snipped\":false,\"format\":{\"protocol\":\"hls\",\"mime_type\":\"audio/ogg; codecs=\"opus\"\"},\"quality\":\"sq\"}]},\"station_urn\":\"soundcloud:system-playlists:track-stations:1564619803\",\"station_permalink\":\"track-stations:1564619803\",\"track_authorization\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJnZW8iOiJDQSIsInN1YiI6IiIsInJpZCI6IjM0YWEwODhhLWI2OGYtNDc1My1iY2M5LWM1MDA5N2JkZDUzMCIsImlhdCI6MTcwMzcxNTAyN30.giv5d_E8wAEHsZVGDRdT4OW58lwH5OT62Hon6YRPgsk\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\",\"user\":{\"avatar_url\":\"https://i1.sndcdn.com/avatars-Hr1nAKZxMRRey53w-3Jl08w-large.jpg\",\"first_name\":\"\",\"followers_count\":336,\"full_name\":\"\",\"id\":1103034523,\"kind\":\"user\",\"last_modified\":\"2023-10-15T01:49:59Z\",\"last_name\":\"\",\"permalink\":\"syz3n\",\"permalink_url\":\"https://soundcloud.com/syz3n\",\"uri\":\"https://api.soundcloud.com/users/1103034523\",\"urn\":\"soundcloud:users:1103034523\",\"username\":\"SYZ3N\",\"verified\":false,\"city\":\"\",\"country_code\":\"US\",\"badges\":{\"pro\":false,\"pro_unlimited\":false,\"verified\":false},\"station_urn\":\"soundcloud:system-playlists:artist-stations:1103034523\",\"station_permalink\":\"artist-stations:1103034523\"}},{\"id\":1581967583,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1581967567,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1579468258,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1582836779,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1583375555,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1583387411,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1399212733,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1572262966,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1584561735,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1571283373,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1586239603,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1585007395,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1587857567,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1587857947,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1583655335,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1580095586,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1588697707,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1407589612,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\""
				"id\":1589668891,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1575909670,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1576068862,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1579223694,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1576067165,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1576221265,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1588804987,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1549825525,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1522192678,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1432154356,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1582681691,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1576062451,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1468316275,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1591584679,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1582371119,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1590273943,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1590273987,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1593304053,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1587294479,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1593011256,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1588313643,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1593258888,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1593257109,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1306578352,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1594241751,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1596260049,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1571408767,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1593861450,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1575921889,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1594181319,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1594201905,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1555226110,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1594606539,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1595706951,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1571744317,\"kind\":\"track\",\"monetizat"
				"ion_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1586902155,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1580545814,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1601098242,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1499016568,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1508281711,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1601251872,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1526380660,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1526383711,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1589774511,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1580245703,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1588229695,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1601663874,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1577913786,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1597389681,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1601789253,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1600467024,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1601894241,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1602430053,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1602670665,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1603071219,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1549185055,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1595663046,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1603889217,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1603608732,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1582646999,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1603719096,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1603719159,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1602357150,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1602979563,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1605274071,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1603624782,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1597369659,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1602353253,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1599325143,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1"
				"559250289,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1555656592,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1605672186,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1589080579,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1596930243,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1605703140,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1589780971,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1605243966,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1587471119,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1606154943,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1605628320,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1607082594,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1455257878,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1608055110,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1523406175,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1596186147,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1606050438,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1593163296,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1590661811,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1610902785,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1604345286,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1599970620,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1611185220,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1594883445,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1581636843,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1611573684,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1336054402,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1606288668,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1611725601,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1536314680,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1601430852,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1468308118,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1197885613,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1612520067,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1613970588,\"kind\":\"track\",\"monetization_model\":\"B"
				"LACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1616015412,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1539735040,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1295862802,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1435320067,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1616609367,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1619378079,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1616947947,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1411182547,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1528971583,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1622368404,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1618238274,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1618240719,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1618250832,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1618256028,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1621315770,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1619345256,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1622949615,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1592569699,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1609665873,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1628030493,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1628030379,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1628030259,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1628640660,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1629087747,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1631771976,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1630059885,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1633341207,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1633915755,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1633915833,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1581346907,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1621644291,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1633993008,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1629935634,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1639827912,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1631636748,\"kind\":\"track\",\"mo"
				"netization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1636275825,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1632357588,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1637010738,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1634882577,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1642438617,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1630403334,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1640635056,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1643214897,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1644041757,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1638087315,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1638086547,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1645350984,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1645351092,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1622938410,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1617333219,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1645514565,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1507491745,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1644084357,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1594591110,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1627211841,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1649531412,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1642910997,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1647286860,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1637811681,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1643974668,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1650973071,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1650849183,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1652558022,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1652661237,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1639610802,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1641801495,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1641808986,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1641806166,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1653947451,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"poli"
				"cy\":\"MONETIZE\"},{\"id\":1655647629,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1653821295,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1558028359,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1645953915,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1645954572,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1650893271,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1655973447,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1656061698,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1658269458,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1661417946,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1659269193,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1611502032,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1661825097,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1662628125,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1667168874,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1667168853,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1553200144,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1643272692,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1643271198,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1662585012,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1661533422,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1661533389,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1661533263,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1661533233,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1670317986,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1658710965,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1666760586,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1672962045,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1666556298,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1666612758,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1673713581,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1671889359,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1675339479,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1643939694,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1652787258,\"kind\":\"track\",\"monetization_model\":"
				"\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1649926443,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1674376842,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1675324536,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1676992875,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1662049116,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1629671265,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1568430712,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1633661871,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1679078247,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1679463618,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1490829199,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1680058794,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1677713106,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1681073631,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1644190962,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1681866063,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1683160458,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1681485900,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1683950445,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1679244261,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1664216424,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1691310672,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1608651246,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1619578203,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1553051944,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1693502820,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1678654554,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1695210216,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1631866944,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1676728017,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1676727981,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1697053668,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1664303004,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1697855313,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1697142837,\"kind\":\"track\",\"monetizat"
				"ion_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1699066962,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1697695908,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1697564538,\"kind\":\"track\",\"monetization_model\":\"AD_SUPPORTED\",\"policy\":\"MONETIZE\"},{\"id\":1679664033,\"kind\":\"track\",\"monetization_model\":\"NOT_APPLICABLE\",\"policy\":\"ALLOW\"},{\"id\":1699547535,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"},{\"id\":1701201624,\"kind\":\"track\",\"monetization_model\":\"BLACKBOX\",\"policy\":\"MONETIZE\"}],\"track_count\":272,\"url\":\"/nick-okroy/sets/2023-selects-catalogue-ii\"}}]"
		};
		welcome resultsNew{};
		parser.parseJson(resultsNew, rawData);
		jsonifier::vector<int32_t> valuesNew{ 1, 2, 3, 4, 5, 6, 7 };
		jsonifier::string collectionString{};
		for (auto& value : valuesNew) {
			std::cout << "CURRENT VALUE (REAL): " << value << std::endl;
		}
		jsonifier::string avatarUrl{};

		for (auto& value: resultsNew.data) {
			if (value.data.getType() == jsonifier::json_type::Object) {
				auto newObject = value.data.operator jsonifier::raw_json_data::object_type();
				if (value.hydratable == "playlist") {
					auto newArray = value.data.operator jsonifier::raw_json_data::object_type();
					for (auto& [key, value]: newArray) {
						if (key == "tracks") {
							auto newArray = value.operator jsonifier::raw_json_data::array_type();
							bool isItFound{ false };
							std::cout << "CURRENT SIZE: " << newArray.size() << std::endl;
							uint32_t currentIndex{};
							auto arraySize = newArray.size();
							for (auto& newValue: newArray) {
								newObject = newValue.operator jsonifier::raw_json_data::object_type();
								for (auto& valueNew:
									newObject["media"].operator jsonifier::raw_json_data::object_type()["transcodings"].operator jsonifier::raw_json_data::array_type()) {
									if (valueNew.operator jsonifier::raw_json_data::object_type()["preset"].operator jsonifier::string() == "opus_0_0") {
										isItFound						   = true;
										jsonifier::string firstDownloadUrl = valueNew.operator jsonifier::raw_json_data::object_type()["url"].operator jsonifier::string();
										jsonifier::string songId		   = valueNew.operator jsonifier::raw_json_data::object_type()["url"].operator jsonifier::string();
									}
								}
								collectionString += newValue.operator jsonifier::raw_json_data::object_type()["id"].rawJson();
								if (currentIndex < arraySize - 1) {
									collectionString += "%2C";
								}
								++currentIndex;
								std::cout << "CURRENT VALUE: " << newValue.operator jsonifier::raw_json_data::object_type()["id"].rawJson() << std::endl;
							}
							collectionString += "&client_id=" + jsonifier::string{ "&%5Bobject%20Object%5D=&app_version=" } + "&app_locale=en";
							std::cout << "CURRENT VALUE: " << collectionString << std::endl;
						}
					}
				}
			}
		}
		jsonifier::raw_json_data rawData02{ "3434" };
		std::cout << "EQUAL?: " << (rawData02 == 2323) << std::endl;
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