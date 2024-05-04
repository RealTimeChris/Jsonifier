#if defined(JSONIFIER_CPU_INSTRUCTIONS)
//#undef JSONIFIER_CPU_INSTRUCTIONS
//#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif
#include <jsonifier/Index.hpp>
#include "glaze/glaze.hpp"
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>

struct search_metadata {
	double completedIn;
	double maxId;
	std::string maxIdStr;
	std::string nextResults;
	std::string query;
	std::string refreshUrl;
	int64_t count;
	int64_t sinceId;
	std::string sinceIdStr;
};

struct hashtag {
	std::string text;
	std::vector<int64_t> indices;
};

struct large_data {
	int64_t w;
	int64_t h;
	std::string resize;
};

struct sizes_data {
	large_data medium;
	large_data small;
	large_data thumb;
	large_data large;
};

struct media_data {
	double id;
	std::string idStr;
	std::vector<int64_t> indices;
	std::string mediaUrl;
	std::string mediaUrlHttps;
	std::string url;
	std::string displayUrl;
	std::string expandedUrl;
	std::string type;
	sizes_data sizes;
	std::optional<double> sourceStatusId;
	std::optional<std::string> sourceStatusIdStr;
};

struct url {
	std::string url;
	std::string expandedUrl;
	std::string displayUrl;
	std::vector<int64_t> indices;
};

struct user_mention {
	std::string screenName;
	std::string name;
	int64_t id;
	std::string idStr;
	std::vector<int64_t> indices;
};

struct status_entities {
	std::vector<hashtag> hashtags;
	std::vector<std::string> symbols;
	std::vector<url> urls;
	std::vector<user_mention> userMentions;
	std::optional<std::vector<media_data>> media;
};

struct metadata_data {
	std::string resultType;
	std::string isoLanguageCode;
};

struct description_data {
	std::vector<url> urls;
};

struct user_entities {
	description_data description;
	std::optional<description_data> url;
};

struct user_data_twitter {
	int64_t id;
	std::string idStr;
	std::string name;
	std::string screenName;
	std::string location;
	std::string description;
	std::optional<std::string> url;
	user_entities entities;
	bool userProtected;
	int64_t followersCount;
	int64_t friendsCount;
	int64_t listedCount;
	std::string createdAt;
	int64_t favouritesCount;
	std::optional<int64_t> utcOffset;
	std::optional<std::string> timeZone;
	bool geoEnabled;
	bool verified;
	int64_t statusesCount;
	std::string lang;
	bool contributorsEnabled;
	bool isTranslator;
	bool isTranslationEnabled;
	std::string profileBackgroundColor;
	std::string profileBackgroundImageUrl;
	std::string profileBackgroundImageUrlHttps;
	bool profileBackgroundTile;
	std::string profileImageUrl;
	std::string profileImageUrlHttps;
	std::optional<std::string> profileBannerUrl;
	std::string profileLinkColor;
	std::string profileSidebarBorderColor;
	std::string profileSidebarFillColor;
	std::string profileTextColor;
	bool profileUseBackgroundImage;
	bool defaultProfile;
	bool defaultProfileImage;
	bool following;
	bool followRequestSent;
	bool notifications;
};

struct status {
	metadata_data metadata;
	std::string createdAt;
	double id;
	std::string idStr;
	std::string text;
	std::string source;
	bool truncated;
	std::optional<double> inReplyToStatusId;
	std::optional<std::string> inReplyToStatusIdStr;
	std::optional<int64_t> inReplyToUserId;
	std::optional<std::string> inReplyToUserIdStr;
	std::optional<std::string> inReplyToScreenName;
	user_data_twitter user;
	std::optional<std::string> geo;
	std::optional<std::string> coordinates;
	std::optional<std::string> place;
	std::optional<std::string> contributors;
	int64_t retweetCount;
	int64_t favoriteCount;
	status_entities entities;
	bool favorited;
	bool retweeted;
	std::string lang;
	std::shared_ptr<status> retweetedStatus;
	std::optional<bool> possiblySensitive;
};

struct twitter_message {
	std::vector<status> statuses;
	search_metadata searchMetadata;
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
	std::vector<permission_overwrites> permissionOverwrites{};
	std::optional<std::string> lastMessageId{};
	uint64_t defaultThreadRateLimitPerUser{};
	std::vector<std::string> appliedTags{};
	std::optional<std::string> rtcRegion{};
	uint64_t defaultAutoArchiveDuration{};
	std::optional<std::string> status{};
	std::vector<user_data> recipients{};
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
	std::vector<std::string> roles{};
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
	std::vector<guild_scheduled_event_data> guildScheduledEvents{};
	std::optional<std::string> latestOnBoardingQuestionId{};
	std::optional<std::string> safetyAlertsChannelId{};
	std::optional<std::string> inventorySettings{};
	std::optional<std::string> discoverySplash{};
	std::optional<std::string> applicationId{};
	std::optional<std::string> vanityUrlCode{};
	std::optional<std::string> description{};
	std::vector<guild_member_data> members{};
	uint64_t defaultMessageNotifications{};
	std::vector<std::string> voiceStates{};
	std::vector<channel_data> channels{};
	uint64_t maxStageVideoChannelUsers{};
	std::string publicUpdatesChannelId{};
	std::optional<std::string> hubType{};
	uint64_t premiumSubscriptionCount{};
	std::vector<std::string> features{};
	uint64_t approximatePresenceCount{};
	std::vector<std::string> stickers{};
	std::optional<std::string> splash{};
	std::vector<std::string> threads{};
	uint64_t approximateMemberCount{};
	bool premiumProgressBarEnabled{};
	uint64_t explicitContentFilter{};
	uint64_t maxVideoChannelUsers{};
	std::vector<role_data> roles{};
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

template<> struct jsonifier::core<url> {
	using value_type = url;
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

template<> struct glz::meta<url> {
	using value_type = url;
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
	using value_type				 = guild_member_data;
	static constexpr auto parseValue = createValue("user", &value_type::user, "roles", &value_type::roles, "joined_at", &value_type::joinedAt, "deaf", &value_type::deaf, "mute",
		&value_type::mute, "flags", &value_type::flags, "pending", &value_type::pending, "permissions", &value_type::permissions, "guild_id", &value_type::guildId, "premium_since",
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
	using OTy						 = discord_message;
	static constexpr auto parseValue = createValue("t", &OTy::t, "s", &OTy::s, "op", &OTy::op, "d", &OTy::d);
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
	using OTy					= discord_message;
	static constexpr auto value = object("t", &OTy::t, "s", &OTy::s, "op", &OTy::op, "d", &OTy::d);
};
#endif
struct test_struct {
	jsonifier::vector<bool> jsonifierTestBools{};
	std::vector<std::string> testStrings{};
	std::vector<bool> glazeTestBools{};
	std::vector<uint64_t> testUints{};
	std::vector<double> testDoubles{};
	std::vector<int64_t> testInts{};
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

	JSONIFIER_INLINE static jsonifier::string generateJsonData() {
		std::string buffer{};
		TestGenerator generator{};
		jsonifier::jsonifier_core parser{};
		parser.serializeJson(generator, buffer);
		return buffer;
	}

	std::string generateString() {
		auto length{ randomizeNumber(64.0f, 16.0f) };
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
			auto arraySize01 = randomizeNumber(35, 10);
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
					auto newBool = generateBool();
					v[x].jsonifierTestBools.emplace_back(newBool);
					v[x].glazeTestBools.emplace_back(newBool);
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

template<typename OTy> struct AbcTest {
	std::vector<OTy> z, y, x, w, v, u, t, s, r, q, p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a;
};

template<> struct jsonifier::core<test_struct> {
	using OTy						 = test_struct;
	static constexpr auto parseValue = createValue("testBools", &OTy::jsonifierTestBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles",
		&OTy::testDoubles, "testStrings", &OTy::testStrings);
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

template<> struct glz::meta<test_struct> {
	using OTy = test_struct;
	static constexpr auto value =
		object("testBools", &OTy::glazeTestBools, "testInts", &OTy::testInts, "testUints", &OTy::testUints, "testDoubles", &OTy::testDoubles, "testStrings", &OTy::testStrings);
};

template<> struct glz::meta<Test<test_struct>> {
	using OTy					= Test<test_struct>;
	static constexpr auto value = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i, "j", &OTy::j,
		"k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w",
		&OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};


template<> struct glz::meta<TestGenerator<test_struct>> {
	using OTy					= TestGenerator<test_struct>;
	static constexpr auto value = object("a", &OTy::a, "b", &OTy::b, "c", &OTy::c, "d", &OTy::d, "e", &OTy::e, "f", &OTy::f, "g", &OTy::g, "h", &OTy::h, "i", &OTy::i, "j", &OTy::j,
		"k", &OTy::k, "l", &OTy::l, "m", &OTy::m, "n", &OTy::n, "o", &OTy::o, "p", &OTy::p, "q", &OTy::q, "r", &OTy::r, "s", &OTy::s, "t", &OTy::t, "u", &OTy::u, "v", &OTy::v, "w",
		&OTy::w, "x", &OTy::x, "y", &OTy::y, "z", &OTy::z);
};

template<> struct glz::meta<AbcTest<test_struct>> {
	using OTy					= AbcTest<test_struct>;
	static constexpr auto value = object("z", &OTy::z, "y", &OTy::y, "x", &OTy::x, "w", &OTy::w, "v", &OTy::v, "u", &OTy::u, "t", &OTy::t, "s", &OTy::s, "r", &OTy::r, "q", &OTy::q,
		"p", &OTy::p, "o", &OTy::o, "n", &OTy::n, "m", &OTy::m, "l", &OTy::l, "k", &OTy::k, "j", &OTy::j, "i", &OTy::i, "h", &OTy::h, "g", &OTy::g, "f", &OTy::f, "e", &OTy::e, "d",
		&OTy::d, "c", &OTy::c, "b", &OTy::b, "a", &OTy::a);
};

#if defined(NDEBUG)
constexpr uint64_t iterationsVal = 100;
#else
constexpr uint64_t iterationsVal = 1;
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
	std::optional<double> json_write_speed{};
	std::optional<double> json_read_speed{};
	std::optional<double> json_read{};
	std::optional<double> json_write{};


	bool operator>(const results& other) const {
		if (json_read_speed.has_value() && other.json_read_speed.has_value()) {
			return *this->json_read_speed > *other.json_read_speed;
		} else if (json_write_speed.has_value() && other.json_write_speed.has_value()) {
			return *this->json_write_speed > *other.json_write_speed;
		}
		return false;
	}

	const void print() {
		std::cout << std::string{ "| " } + name + " " + test + ": " + url + "\n" +
				"| ------------------------------------------------------------ "
				"|\n";

		if (json_read_byte_length && json_read) {
			auto mbReadCount	 = static_cast<double>(*json_read_byte_length) / 1e+6l;
			auto readSecondCount = *json_read / 1e+9l;
			std::cout << "Read Length: " << *json_read_byte_length << std::endl;
			std::cout << "Read: " << mbReadCount / readSecondCount << " MB/s\n";
			json_read_speed = mbReadCount / readSecondCount;
		}
		if (json_write_byte_length && json_write) {
			auto mbWrittenCount	  = static_cast<double>(*json_write_byte_length) / 1e+6l;
			auto writeSecondCount = *json_write / 1e+9l;
			std::cout << "Write Length: " << *json_write_byte_length << std::endl;
			std::cout << "Write: " << mbWrittenCount / writeSecondCount << " MB/s\n";
			json_write_speed = mbWrittenCount / writeSecondCount;
		}

		std::cout << "\n---" << std::endl;
	}

	const test_result getWriteResults() {
		test_result result{};
		if (json_write_byte_length && json_write) {
			result.libraryName	  = name;
			auto mbWrittenCount	  = static_cast<double>(*json_write_byte_length) / 1e+6l;
			auto writeSecondCount = *json_write / 1e+9l;
			result.resultSpeed	  = mbWrittenCount / writeSecondCount;
			json_write_speed	  = mbWrittenCount / writeSecondCount;
			result.resultType	  = "Write";
			result.color		  = wColor;
		}
		return result;
	}

	const test_result getReadResults() {
		test_result result{};
		if (json_read_byte_length && json_read) {
			result.libraryName	 = name;
			auto mbReadCount	 = static_cast<double>(*json_read_byte_length) / 1e+6l;
			auto readSecondCount = *json_read / 1e+9l;
			result.resultSpeed	 = mbReadCount / readSecondCount;
			json_read_speed		 = mbReadCount / readSecondCount;
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
			json_read_speed = static_cast<double>(mbReadCount / readSecondCount);
			read			= stream01.str();
		} else {
			read = "N/A";
		}
		if (json_write && json_write_byte_length) {
			double mbWrittenCount	= static_cast<double>(*json_write_byte_length) / 1e+6l;
			double writeSecondCount = *json_write / 1e+9l;
			std::stringstream stream01{};
			stream01 << static_cast<double>(mbWrittenCount / writeSecondCount);
			json_write_speed = mbWrittenCount / writeSecondCount;
			write			 = stream01.str();
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
	for (uint64_t x = 0; x < iterationCount; ++x) {
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

enum class test_type {
	parse_and_serialize = 0,
	minify				= 1,
	prettify			= 2,
	validate			= 3,
};

template<test_type type = test_type::parse_and_serialize, typename test_data_type, uint64_t iterations, bool minified>
auto jsonifierTest(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", testName, "https://github.com/realtimechris/jsonifier", iterations };
	test_data_type testData{};
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.parseJson<minified>(testData, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}

	r.json_read.emplace(result);
	buffer.clear();

	result = benchmark(
		[&]() {
			parser.serializeJson(testData, buffer);
		},
		iterations);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read_byte_length.emplace(buffer.size());
	r.json_write_byte_length.emplace(buffer.size());
	r.json_write.emplace(result);
	r.wColor = "steelblue";
	r.rColor = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::minify, std::string, iterationsVal, false>(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Minify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };

	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			buffer = parser.minify(buffer);
		},
		iterationsVal);

	r.json_write.emplace(result);

	r.wColor = "steelblue";
	r.rColor = "teal";
	r.json_write_byte_length.emplace(buffer.size());
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::prettify, std::string, iterationsVal, false>(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Prettify Test", "https://github.com/realtimechris/jsonifier", iterationsVal };

	jsonifier::jsonifier_core parser{};
	uint64_t newBufferSize{};
	auto result = benchmark(
		[&]() {
			auto newBuffer = static_cast<std::string>(parser.prettify(buffer));
			newBufferSize  = newBuffer.size();
		},
		iterationsVal);

	r.json_write.emplace(result);

	r.json_write_byte_length.emplace(newBufferSize);
	r.wColor = "steelblue";
	r.rColor = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto jsonifierTest<test_type::validate, std::string, iterationsVal, false>(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "jsonifier", "Validation Test", "https://github.com/realtimechris/jsonifier", iterationsVal };
	jsonifier::jsonifier_core parser{};

	auto result = benchmark(
		[&]() {
			parser.validate(buffer);
		},
		iterationsVal);
	for (auto& value: parser.getErrors()) {
		std::cout << "Jsonifier Error: " << value << std::endl;
	}
	r.json_read.emplace(result);
	r.json_read_byte_length.emplace(buffer.size());

	r.wColor = "steelblue";
	r.rColor = "teal";
	if (doWePrint) {
		r.print();
	}

	return r;
}

#if !defined(ASAN)
template<test_type type = test_type::parse_and_serialize, typename test_data_type, uint64_t iterations, bool minified>
auto glazeTest(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "glaze", testName, "https://github.com/stephenberry/glaze", iterations };
	test_data_type testData{};
	auto result = benchmark(
		[&]() {
			try {
				if (auto error = glz::read<glz::opts{ .minified = minified }>(testData, buffer); error) {
					std::cout << "Glaze Error: " << glz::format_error(error, buffer) << std::endl;
				}
			} catch (std ::exception& error) {
				std::cout << "Glaze Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read.emplace(result);
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(testData, buffer);
		},
		iterations);
	r.json_read_byte_length.emplace(buffer.size());
	r.json_write_byte_length.emplace(buffer.size());
	r.json_write.emplace(result);
	r.wColor = "skyblue";
	r.rColor = "dodgerblue";
	if (doWePrint) {
		r.print();
	}
	return r;
}

template<> auto glazeTest<test_type::prettify, std::string, iterationsVal, false>(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Prettify Test", "https://github.com/stephenberry/glaze", iterationsVal };
	uint64_t newBufferSize{};
	auto result = benchmark(
		[&]() {
			auto newBuffer = glz::prettify_json(buffer);
			newBufferSize  = newBuffer.size();
		},
		iterationsVal);

	r.json_write.emplace(result);

	r.json_write_byte_length.emplace(newBufferSize);
	r.wColor = "skyblue";
	r.rColor = "dodgerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto glazeTest<test_type::minify, std::string, iterationsVal, false>(const jsonifier::string& bufferNew, const std::string& testName, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "glaze", "Minify Test", "https://github.com/stephenberry/glaze", iterationsVal };

	uint64_t newBufferSize{};
	auto result = benchmark(
		[&]() {
			auto newBuffer = glz::minify_json(buffer);
			newBufferSize  = newBuffer.size();
		},
		iterationsVal);

	r.json_write.emplace(result);

	r.wColor = "skyblue";
	r.rColor = "dodgerblue";
	r.json_write_byte_length.emplace(newBufferSize);
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
	getValues<int64_t>(tag.indices, jsonDataNew, "indices");

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
	getValues<int64_t>(media.indices, jsonDataNew, "indices");
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

template<> url getValue<url>(simdjson::ondemand::value jsonDataNew) {
	url urlData;

	urlData.url			= getValue<std::string>(jsonDataNew, "url");
	urlData.expandedUrl = getValue<std::string>(jsonDataNew, "expanded_url");
	urlData.displayUrl	= getValue<std::string>(jsonDataNew, "display_url");
	getValues<int64_t>(urlData.indices, jsonDataNew, "indices");

	return urlData;
}

template<> user_mention getValue<user_mention>(simdjson::ondemand::value jsonDataNew) {
	user_mention mention;

	mention.screenName = getValue<std::string>(jsonDataNew, "screen_name");
	mention.name	   = getValue<std::string>(jsonDataNew, "name");
	mention.id		   = getValue<int64_t>(jsonDataNew, "id");
	mention.idStr	   = getValue<std::string>(jsonDataNew, "id_str");
	getValues<int64_t>(mention.indices, jsonDataNew, "indices");

	return mention;
}

template<> status_entities getValue<status_entities>(simdjson::ondemand::value jsonDataNew) {
	status_entities entities;

	getValues<hashtag>(entities.hashtags, jsonDataNew, "hashtags");
	getValues<std::string>(entities.symbols, jsonDataNew, "symbols");
	getValues<url>(entities.urls, jsonDataNew, "urls");
	getValues<user_mention>(entities.userMentions, jsonDataNew, "user_mentions");

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

	getValues<url>(data.urls, jsonDataNew, "urls");

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
	st.retweetedStatus		= nullptr;// This will be set separately if retweeted_status is present
	st.possiblySensitive	= getValue<bool>(jsonDataNew, "possibly_sensitive");

	return st;
}

template<> twitter_message getValue<twitter_message>(simdjson::ondemand::value jsonDataNew) {
	twitter_message message;

	getValues<status>(message.statuses, jsonDataNew, "statuses");
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
	getValues<std::string>(memberData.roles, jsonDataNew, "roles");
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

	getValues<guild_scheduled_event_data>(guildData.guildScheduledEvents, jsonDataNew, "guild_scheduled_events");

	getValues<guild_member_data>(guildData.members, jsonDataNew, "members");

	guildData.defaultMessageNotifications = getValue<uint64_t>(jsonDataNew, "default_message_notifications");

	getValues<channel_data>(guildData.channels, jsonDataNew, "channels");

	guildData.maxStageVideoChannelUsers = getValue<uint64_t>(jsonDataNew, "max_stage_video_channel_users");
	guildData.publicUpdatesChannelId	= getValue<std::string>(jsonDataNew, "public_updates_channel_id");
	guildData.premiumSubscriptionCount	= getValue<uint64_t>(jsonDataNew, "premium_subscription_count");

	getValues<std::string>(guildData.features, jsonDataNew, "features");

	guildData.approximatePresenceCount	= getValue<uint64_t>(jsonDataNew, "approximate_presence_count");
	guildData.safetyAlertsChannelId		= getValue<std::string>(jsonDataNew, "safety_alerts_channel_id");
	guildData.approximateMemberCount	= getValue<uint64_t>(jsonDataNew, "approximate_member_count");
	guildData.premiumProgressBarEnabled = getValue<bool>(jsonDataNew, "premium_progress_bar_enabled");
	guildData.explicitContentFilter		= getValue<uint64_t>(jsonDataNew, "explicit_content_filter");
	guildData.maxVideoChannelUsers		= getValue<uint64_t>(jsonDataNew, "max_video_channel_users");

	getValues<role_data>(guildData.roles, jsonDataNew, "roles");

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
				getValues<int64_t>(newStruct.testInts, value, "testInts"); \
				getValues<double>(newStruct.testDoubles, value, "testDoubles"); \
				getValues<std::string>(newStruct.testStrings, value, "testStrings"); \
				getValues02<bool>(newStruct.jsonifierTestBools, value, "testBools"); \
				getValues<uint64_t>(newStruct.testUints, value, "testUints"); \
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

template<test_type type = test_type::parse_and_serialize, typename test_data_type, uint64_t iterations>
auto simdjsonTest(const jsonifier::string& bufferNew, const std::string& testname, bool doWePrint = true) {
	std::string buffer{ bufferNew };

	results r{ "simdjson", testname, "https://github.com/simdjson/simdjson", iterations };
	test_data_type testData{};

	simdjson::ondemand::parser parser{};

	double result{};
	result = benchmark(
		[&]() {
			try {
				auto doc = parser.iterate(buffer);
				testData = getValue<test_data_type>(doc.value());
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);
	buffer.clear();
	glz::write_json(testData, buffer);
	r.json_read.emplace(result);
	r.json_read_byte_length.emplace(buffer.size());
	r.rColor = "cadetblue";
	r.wColor = "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

template<> auto simdjsonTest<test_type::minify, std::string, iterationsVal>(const jsonifier::string& bufferNew, const std::string& testname, bool doWePrint) {
	std::string buffer{ bufferNew };

	results r{ "simdjson", "Minify Test", "https://github.com/simdjson/simdjson", iterationsVal };
	dom::parser parser{};
	auto result = benchmark(
		[&]() {
			try {
				auto doc	   = parser.parse(buffer);
				auto newBuffer = simdjson::minify(doc);
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterationsVal);

	r.json_write.emplace(result);

	r.json_write_byte_length.emplace(buffer.size());
	r.rColor = "cadetblue";
	r.wColor = "cornflowerblue";
	if (doWePrint) {
		r.print();
	}

	return r;
}

#endif
std::string table_header = R"(
| Library | Read (MB/s) | Write (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |)";

template<test_type type, typename test_data_type, bool minified, uint64_t iterationsVal>
test_results jsonTests(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonTest<type, test_data_type, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(simdjsonTest<type, test_data_type, iterationsVal>(jsonDataNew, jsonResults.testName, true));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeTest<type, test_data_type, iterationsVal, minified>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<type, test_data_type, iterationsVal, minified>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierTest<type, test_data_type, iterationsVal, minified>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<type, test_data_type, iterationsVal, minified>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results>());
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

template<> test_results jsonTests<test_type::prettify, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		glazeTest<test_type::prettify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<test_type::prettify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierTest<test_type::prettify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::prettify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results>());
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

template<> test_results jsonTests<test_type::minify, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
#if !defined(ASAN)
	for (uint32_t x = 0; x < 2; ++x) {
		simdjsonTest<test_type::minify, std::string, iterationsVal>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(simdjsonTest<test_type::minify, std::string, iterationsVal>(jsonDataNew, jsonResults.testName, true));
	for (uint32_t x = 0; x < 2; ++x) {
		glazeTest<test_type::minify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(glazeTest<test_type::minify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, true));
#endif
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierTest<test_type::minify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::minify, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results>());
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

template<> test_results jsonTests<test_type::validate, std::string, false, iterationsVal>(const jsonifier::string& jsonDataNew, const jsonifier::string& testName) {
	std::vector<results> resultsNew{};
	test_results jsonResults{};
	jsonResults.testName = testName;
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifierTest<test_type::validate, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, false);
	}
	resultsNew.emplace_back(jsonifierTest<test_type::validate, std::string, iterationsVal, false>(jsonDataNew, jsonResults.testName, true));

	std::string table{};
	const auto n = resultsNew.size();
	table += table_header + "\n";
	std::sort(resultsNew.begin(), resultsNew.end(), std::greater<results>());
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

static const std::string section001{ R"(> )" + jsonifier::toString(iterationsVal) + R"( iterations on a 6 core (Intel i7 8700k)
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
	R"()

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
		using value_type				 = data_class;
		static constexpr auto parseValue = createValue("artwork_url", &value_type::artworkUrl, "description", &value_type::description, "duration", &value_type::duration, "title",
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
		static constexpr auto parseValue = createValue("artwork_url", &value_type::artworkUrl, "description", &value_type::description, "duration", &value_type::duration, "title",
			&value_type::title, "track_authorization", &value_type::trackAuthorization, "permalink_url", &value_type::viewUrl);
	};

	template<> struct core<sound_cloud_search_results> {
		using value_type				 = sound_cloud_search_results;
		static constexpr auto parseValue = createValue("collection", &value_type::collection);
	};
}

template<typename value_type> struct core {};

int32_t main() {
	try {
		jsonifier::string jsonDataNew{ TestGenerator<test_struct>::generateJsonData() };
		jsonifier::jsonifier_core parser{};
#if defined(_WIN32)
		FileLoader fileLoader01{ findFileRecursively("../../ReadMe.md") };
		FileLoader fileLoader02{ findFileRecursively("../../Json/JsonData-Prettified.json") };
		jsonDataNew = parser.prettify(parser.minify(jsonDataNew));
		fileLoader02.saveFile(jsonDataNew);
		FileLoader fileLoader03{ findFileRecursively("../../Json/JsonData-Minified.json") };
		jsonifier::string jsonMinifiedData{ parser.minify(jsonDataNew) };
		fileLoader03.saveFile(jsonMinifiedData);
		FileLoader fileLoader04{ findFileRecursively("../../Json/Results.json") };
#else
		FileLoader fileLoader01{ findFileRecursively("../ReadMe.md") };
		FileLoader fileLoader02{ findFileRecursively("../Json/JsonData-Prettified.json") };
		jsonDataNew = parser.prettify(parser.minify(jsonDataNew));
		fileLoader02.saveFile(jsonDataNew);
		FileLoader fileLoader03{ findFileRecursively("../Json/JsonData-Minified.json") };
		jsonifier::string jsonMinifiedData{ parser.minify(jsonDataNew) };
		fileLoader03.saveFile(jsonMinifiedData);
		FileLoader fileLoader04{ findFileRecursively("../Json/Results.json") };
#endif
		FileLoader fileLoader05{ findFileRecursively("../Json/DiscordData-Prettified.json") };
		jsonifier::string bufferNew{ fileLoader05.operator jsonifier::string() };
		bufferNew = parser.prettify(parser.minify(bufferNew));
		fileLoader05.saveFile(bufferNew);
		FileLoader fileLoader06{ findFileRecursively("../Json/DiscordData-Minified.json") };
		jsonifier::string discordMinifiedData{ fileLoader06.operator jsonifier::string() };
		discordMinifiedData = parser.minify(discordMinifiedData);
		fileLoader06.saveFile(discordMinifiedData);
		FileLoader fileLoader07{ findFileRecursively("../Json/TwitterData-Prettified.json") };
		jsonifier::string twitterData{ fileLoader07.operator jsonifier::string() };
		twitterData = parser.prettify(parser.minify(twitterData));
		fileLoader07.saveFile(twitterData);
		FileLoader fileLoader08{ findFileRecursively("../Json/TwitterData-Minified.json") };
		jsonifier::string twitterMinifiedData{ fileLoader08.operator jsonifier::string() };
		twitterMinifiedData = parser.minify(twitterMinifiedData);
		fileLoader08.saveFile(twitterMinifiedData);
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
		auto testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, false, 1>(jsonDataNew, "Single Test (Prettified)");
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, Test<test_struct>, true, 1>(jsonMinifiedData, "Single Test (Minified)");
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
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, false, iterationsVal>(bufferNew, "Discord Test (Prettified)");
		newerString += section07;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::parse_and_serialize, discord_message, true, iterationsVal>(discordMinifiedData, "Discord Test (Minified)");
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
		testResults = jsonTests<test_type::minify, std::string, false, iterationsVal>(bufferNew, "Minify Test");
		newerString += section11;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::prettify, std::string, false, iterationsVal>(discordMinifiedData, "Prettify Test");
		newerString += section12;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		testResults = jsonTests<test_type::validate, std::string, false, iterationsVal>(bufferNew, "Validate Test");
		newerString += section13;
		newerString += section001;
		newerString += testResults.markdownResults;
		benchmark_data.emplace_back(testResults);
		jsonifier::string resultsStringJson{};
		parser.serializeJson<true>(benchmark_data, resultsStringJson);
		fileLoader04.saveFile(resultsStringJson);
		fileLoader01.saveFile(newerString);
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}