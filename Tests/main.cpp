#if defined(JSONIFIER_CPU_INSTRUCTIONS)
#undef JSONIFIER_CPU_INSTRUCTIONS
#define JSONIFIER_CPU_INSTRUCTIONS (JSONIFIER_AVX | JSONIFIER_POPCNT | JSONIFIER_BMI | JSONIFIER_BMI2 | JSONIFIER_LZCNT)
#endif
#include "glaze/core/macros.hpp"
#include "glaze/glaze.hpp"
#include <jsonifier/Index.hpp>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <chrono>

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
	std::string t;
	uint64_t s;
	uint64_t op;
	guild_data d;
};

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
	static constexpr auto parseValue = createValue("t", &OTy::t, "s", &OTy::s, "op", &OTy::op, "d", &OTy::d);
};

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
		&value_type::mfaLevel, "name", &value_type::name, "icon", &value_type::icon, "banner", skip{});
};

// Specialization for discord_message
template<> struct glz::meta<discord_message> {
	using OTy					= discord_message;
	static constexpr auto value = object("t", &OTy::t, "s", &OTy::s, "op", &OTy::op, "d", &OTy::d);
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
		glz::write_json(generator, buffer);
		//std::cout << "SERIALIZED JSON (GLAZE): " << buffer << std::endl;
		buffer.clear();
		parser.serializeJson(generator, buffer);
		//std::cout << "SERIALIZED JSON: " << buffer << std::endl;
		json_data returnData{};
		returnData.theData = buffer;
		return returnData;
	}

	std::string generateString() {
		auto length{ randomizeNumber(45.0f, 25.0f) };
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
constexpr uint64_t iterations = 200;
#else
constexpr uint64_t iterations = 1;
#endif

struct results {
	std::string name{};
	std::string test{};
	std::string url{};
	uint64_t iterations{};

	std::optional<uint64_t> json_write_byte_length{};
	std::optional<uint64_t> json_read_byte_length{};
	std::optional<double> json_read{};
	std::optional<double> json_write{};

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

	std::string json_stats() {
		std::string write{};
		std::string read{};
		std::string finalstring{};
		if (json_read && json_read_byte_length) {
			double mbReadCount	   = static_cast<double>(*json_read_byte_length) / 1e+6l;
			double readSecondCount = *json_read / 1e+9l;
			std::stringstream stream01{};
			stream01 << static_cast<double>(mbReadCount / readSecondCount);
			read = stream01.str();
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

		finalstring = std::string{ "| [" + name + "](" + url + ") | " + write + " | " + read + " |" };
		return finalstring;
	}
};

class FileLoader {
  public:
	FileLoader(const char* filePathNew) {
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

	~FileLoader() {
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
	return currentLowestTime.count();
}

auto jsonifier_single_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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
	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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

	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_abc_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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

	r.json_read = result;

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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto jsonifier_discord_test(const std::string& discordDataNew, bool doWePrint = true) {
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

	r.json_read = result;

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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_single_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_abc_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
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
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto glaze_discord_test(const std::string& discordData, bool doWePrint = true) {
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

	r.json_read = result;

	r.json_read_byte_length = buffer.size();
	buffer.clear();

	result = benchmark(
		[&]() {
			glz::write_json(discordDataTest, buffer);
		},
		iterations);
	r.json_write_byte_length = buffer.size();
	r.json_write			 = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}
	return r;
}

#include "simdjson.h"

using namespace simdjson;

struct on_demand {
	bool read_in_order(Test<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

template<typename OTy2> void simdPullArray(ondemand::array newX, jsonifier::vector<OTy2>& newVector);
template<typename OTy2> void simdPullArray(ondemand::array newX, std::vector<OTy2>& newVector);

void simdPullMap(ondemand::object newX, std::unordered_map<std::string, std::string>& newVector) {
	for (auto iter = newX.begin(); iter != newX.end(); ++iter) {
		newVector.emplace(static_cast<std::string>(iter.operator*().key().value().raw()), static_cast<std::string>(iter.operator*().value().get_string().value()));
	}
}

template<> void simdPullArray<double>(ondemand::array newX, std::vector<double>& newVector) {
	for (ondemand::value value: newX) {
		double newValue{};
		if (!value.get_double().get(newValue)) {
			newVector.emplace_back(newValue);
		}
	}
}

template<> void simdPullArray<int64_t>(ondemand::array newX, std::vector<int64_t>& newVector) {
	for (ondemand::value value: newX) {
		int64_t newValue{};
		if (!value.get_int64().get(newValue)) {
			newVector.emplace_back(newValue);
		}
	}
}

template<> void simdPullArray<uint64_t>(ondemand::array newX, std::vector<uint64_t>& newVector) {
	for (ondemand::value value: newX) {
		uint64_t newValue{};
		if (!value.get_uint64().get(newValue)) {
			newVector.emplace_back(newValue);
		}
	}
}

template<> void simdPullArray<bool>(ondemand::array newX, jsonifier::vector<bool>& newVector) {
	for (ondemand::value value: newX) {
		bool newValue{};
		if (!value.get_bool().get(newValue)) {
			newVector.emplace_back(newValue);
		}
	}
}

template<> void simdPullArray<std::string>(ondemand::array newX, std::vector<std::string>& newVector) {
	for (ondemand::value value: newX) {
		std::string_view newValue{};
		if (!value.get_string().get(newValue)) {
			newVector.emplace_back(newValue);
		}
	}
}

#define SIMD_Pull(x) \
	{ \
		ondemand::array newX = doc[#x].get_array().value(); \
		ondemand::array newArray{}; \
		ondemand::object newObject{}; \
		for (ondemand::value value: newX) { \
			test_struct newStruct{}; \
			newObject = value.get_object(); \
			newArray  = newObject["testInts"].get_array().value(); \
			simdPullArray(newArray, newStruct.testInts); \
			newArray = newObject["testDoubles"].get_array().value(); \
			simdPullArray(newArray, newStruct.testDoubles); \
			newArray = newObject["testStrings"].get_array().value(); \
			simdPullArray(newArray, newStruct.testStrings); \
			newArray = newObject["testUints"].get_array().value(); \
			simdPullArray(newArray, newStruct.testUints); \
			newArray = newObject["testBools"].get_array().value(); \
			simdPullArray(newArray, newStruct.testBools); \
			obj.x.emplace_back(std::move(newStruct)); \
		} \
	}

bool on_demand::read_in_order(Test<test_struct>& obj, const padded_string& json) {
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

auto simdjson_single_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Single Test", "https://github.com/simdjson/simdjson", 1 };
	Test<test_struct> uint64Test{};

	r.json_read_byte_length = buffer.size();
	double result{};
	result = benchmark(
		[&]() {
			try {
				parser.read_in_order(uint64Test, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		1);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand parser{};

	results r{ "simdjson", "Multi Test", "https://github.com/simdjson/simdjson", iterations };
	Test<test_struct> uint64Test{};

	r.json_read_byte_length = buffer.size();
	double result{};
	result = benchmark(
		[&]() {
			try {
				parser.read_in_order(uint64Test, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read = result;
	buffer.clear();
	if (doWePrint) {
		r.print();
	}

	return r;
}

struct on_demand_abc {
	bool read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json);

  protected:
	ondemand::parser parser{};
};

bool on_demand_abc::read_out_of_order(AbcTest<test_struct>& obj, const padded_string& json) {
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

std::string getString(simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		std::string_view result;
		simdjson::error_code error{};
		field.get_string().tie(result, error);
		return static_cast<std::string>(result);
	}
	return "";
}

int64_t getInt(simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		int64_t result;
		simdjson::error_code error{};
		field.get_int64().tie(result, error);
		return result;
	}
	return 0;
}

bool getBool(simdjson::ondemand::value value, const std::string& key) {
	simdjson::ondemand::value field;
	if (!value[key].get(field)) {
		bool result;
		simdjson::error_code error{};
		field.get_bool().tie(result, error);
		return result;
	}
	return false;
}

std::vector<uint64_t> getInts(simdjson::ondemand::value value) {
	std::vector<uint64_t> roles;
	for (simdjson::ondemand::value role: value) {
		uint64_t roleValue;
		simdjson::error_code error{};
		role.get_uint64().tie(roleValue, error);
		roles.emplace_back(roleValue);
	}
	return roles;
}

std::vector<std::string> getStrings(simdjson::ondemand::value value) {
	std::vector<std::string> roles;
	for (simdjson::ondemand::value role: value) {
		std::string_view roleValue;
		simdjson::error_code error{};
		role.get_string().tie(roleValue, error);
		roles.emplace_back(roleValue);
	}
	return roles;
}

user_data parseUserData(simdjson::ondemand::value jsonData) {
	user_data user;

	simdjson::ondemand::object userObj = jsonData;
	std::string_view newString{};
	jsonData["avatar_decoration"].get_string().get(newString);
	user.avatarDecoration = std::string(newString);

	jsonData["discriminator"].get_string().get(newString);
	user.discriminator = std::string(newString);

	jsonData["global_name"].get_string().get(newString);
	user.globalName = std::string(newString);

	jsonData["user_name"].get_string().get(newString);
	user.userName = std::string(newString);

	jsonData["locale"].get_string().get(newString);
	user.locale = std::string(newString);

	jsonData["banner"].get_string().get(newString);
	user.banner = std::string(newString);

	jsonData["avatar"].get_string().get(newString);
	user.avatar = std::string(newString);

	jsonData["email"].get_string().get(newString);
	user.email = std::string(newString);

	jsonData["id"].get_string().get(newString);
	user.id = std::string(newString);

	jsonData["accent_color"].get_uint64().get(user.accentColor);
	jsonData["premium_type"].get_uint64().get(user.premiumType);
	jsonData["public_flags"].get_uint64().get(user.publicFlags);
	jsonData["mfa_enabled"].get_bool().get(user.mfaEnabled);
	jsonData["flags"].get_uint64().get(user.flags);
	jsonData["verified"].get_bool().get(user.verified);
	jsonData["system"].get_bool().get(user.system);
	jsonData["bot"].get_bool().get(user.bot);

	return user;
}

guild_scheduled_event_data parseGuildScheduledEventData(simdjson::ondemand::value json_data) {
	guild_scheduled_event_data event;

	event.scheduledStartTime = json_data["scheduled_start_time"].get_string().value();
	event.scheduledEndTime	 = json_data["scheduled_end_time"].get_string().value();
	event.description		 = json_data["description"].get_string().value();
	event.entityMetadata	 = json_data["entity_metadata"].get_int64().value();
	event.creatorId			 = json_data["creator_id"].get_string().value();
	event.channelId			 = json_data["channel_id"].get_string().value();
	event.entityId			 = json_data["entity_id"].get_string().value();
	event.privacyLevel		 = json_data["privacy_level"].get_int64().value();
	event.guildId			 = json_data["guild_id"].get_string().value();
	event.userCount			 = json_data["user_count"].get_uint64().value();
	event.entityType		 = json_data["entity_type"].get_int64().value();
	std::string_view newString{};
	simdjson::ondemand::object creator_obj = json_data["creator"];

	creator_obj["avatar_decoration"].get_string().get(newString);
	event.creator.avatarDecoration = std::string(newString);

	creator_obj["discriminator"].get_string().get(newString);
	event.creator.discriminator = std::string(newString);

	creator_obj["global_name"].get_string().get(newString);
	event.creator.globalName = std::string(newString);

	creator_obj["user_name"].get_string().get(newString);
	event.creator.userName = std::string(newString);

	creator_obj["locale"].get_string().get(newString);
	event.creator.locale = std::string(newString);

	creator_obj["banner"].get_string().get(newString);
	event.creator.banner = std::string(newString);

	creator_obj["avatar"].get_string().get(newString);
	event.creator.avatar = std::string(newString);

	creator_obj["email"].get_string().get(newString);
	event.creator.email = std::string(newString);

	creator_obj["id"].get_string().get(newString);
	event.creator.id = std::string(newString);
	creator_obj["accent_color"].get_uint64().get(event.creator.accentColor);
	creator_obj["premium_type"].get_uint64().get(event.creator.premiumType);
	creator_obj["public_flags"].get_uint64().get(event.creator.publicFlags);
	creator_obj["mfa_enabled"].get_bool().get(event.creator.mfaEnabled);
	creator_obj["flags"].get_uint64().get(event.creator.flags);
	creator_obj["verified"].get_bool().get(event.creator.verified);
	creator_obj["system"].get_bool().get(event.creator.system);
	creator_obj["bot"].get_bool().get(event.creator.bot);

	json_data["name"].get_string().get(newString);
	event.name = std::string(newString);
	json_data["status"].get_uint64().get(event.status);
	json_data["id"].get_string().get(newString);
	event.id = std::string(newString);

	return event;
}

channel_data parseChannelData(simdjson::ondemand::value jsonData) {
	channel_data channel;

	simdjson::ondemand::object channelObj = jsonData;
	std::string_view newString{};

	channelObj["default_thread_rate_limit_per_user"].get_uint64().get(channel.defaultThreadRateLimitPerUser);

	for (auto tag: channelObj["applied_tags"]) {
		tag.get_string().get(newString);
		channel.appliedTags.push_back(std::string(newString));
	}

	channelObj["default_auto_archive_duration"].get_uint64().get(channel.defaultAutoArchiveDuration);

	channelObj["last_pin_timestamp"].get_string().get(newString);
	channel.lastPinTimestamp = std::string(newString);

	channelObj["total_message_sent"].get_uint64().get(channel.totalMessageSent);
	channelObj["rate_limit_per_user"].get_uint64().get(channel.rateLimitPerUser);
	channelObj["video_quality_mode"].get_uint64().get(channel.videoQualityMode);

	channelObj["last_message_id"].get_string().get(newString);
	channel.lastMessageId = std::string(newString);

	channelObj["application_id"].get_string().get(newString);
	channel.applicationId = std::string(newString);

	channelObj["permissions"].get_string().get(newString);
	channel.permissions = std::string(newString);

	channelObj["rtc_region"].get_string().get(newString);
	channel.rtcRegion = std::string(newString);

	channelObj["message_count"].get_uint64().get(channel.messageCount);
	channelObj["member_count"].get_uint64().get(channel.memberCount);

	channelObj["parent_id"].get_string().get(newString);
	channel.parentId = std::string(newString);

	channelObj["owner_id"].get_string().get(newString);
	channel.ownerId = std::string(newString);

	channelObj["guild_id"].get_string().get(newString);
	channel.guildId = std::string(newString);

	channelObj["user_limit"].get_uint64().get(channel.userLimit);

	channelObj["topic"].get_string().get(newString);
	channel.topic = std::string(newString);

	channelObj["position"].get_uint64().get(channel.position);
	channelObj["bitrate"].get_uint64().get(channel.bitrate);

	channelObj["name"].get_string().get(newString);
	channel.name = std::string(newString);

	channelObj["icon"].get_string().get(newString);
	channel.icon = std::string(newString);

	channelObj["id"].get_string().get(newString);
	channel.id = std::string(newString);

	channelObj["flags"].get_uint64().get(channel.flags);

	channelObj["managed"].get_bool().get(channel.managed);

	channelObj["type"].get_uint64().get(channel.type);

	channelObj["nsfw"].get_bool().get(channel.nsfw);

	return channel;
}


role_data parseRoleData(simdjson::ondemand::value jsonData) {
	role_data role;

	simdjson::ondemand::object roleObj = jsonData;
	std::string_view newString{};

	jsonData["unicode_emoji"].get_string().get(newString);
	role.unicodeEmoji = std::string(newString);

	jsonData["permissions"].get_string().get(newString);
	role.permissions = std::string(newString);

	jsonData["position"].get_uint64().get(role.position);

	jsonData["name"].get_string().get(newString);
	role.name = std::string(newString);

	jsonData["icon"].get_string().get(newString);
	role.icon = std::string(newString);

	jsonData["color"].get_uint64().get(role.color);

	jsonData["id"].get_string().get(newString);
	role.id = std::string(newString);

	jsonData["flags"].get_uint64().get(role.flags);

	jsonData["mentionable"].get_bool().get(role.mentionable);

	jsonData["managed"].get_bool().get(role.managed);

	jsonData["hoist"].get_bool().get(role.hoist);

	return role;
}

guild_member_data parseGuildMemberData(simdjson::ondemand::value jsonData) {
	guild_member_data guildMember;

	simdjson::ondemand::object memberObj = jsonData;
	std::string_view newString{};

	memberObj["communicationDisabledUntil"].get_string().get(newString);
	guildMember.communicationDisabledUntil = std::string(newString);

	for (auto role: memberObj["roles"]) {
		role.get_string().get(newString);
		guildMember.roles.push_back(std::string(newString));
	}

	memberObj["premium_since"].get_string().get(newString);
	guildMember.premiumSince = std::string(newString);

	memberObj["permissions"].get_string().get(newString);
	guildMember.permissions = std::string(newString);

	memberObj["joined_at"].get_string().get(newString);
	guildMember.joinedAt = std::string(newString);

	memberObj["guild_id"].get_string().get(newString);
	guildMember.guildId = std::string(newString);

	memberObj["avatar"].get_string().get(newString);
	guildMember.avatar = std::string(newString);

	memberObj["nick"].get_string().get(newString);
	guildMember.nick = std::string(newString);

	guildMember.user = parseUserData(memberObj["user"].value());

	memberObj["flags"].get_uint64().get(guildMember.flags);
	memberObj["pending"].get_bool().get(guildMember.pending);
	memberObj["deaf"].get_bool().get(guildMember.deaf);
	memberObj["mute"].get_bool().get(guildMember.mute);

	return guildMember;
}


guild_data parseGuildData(simdjson::ondemand::value jsonData) {
	guild_data guild;

	simdjson::ondemand::object guildObj = jsonData;
	std::string_view newString{};

	for (auto event: guildObj["guild_scheduled_events"]) {
		guild_scheduled_event_data eventData = parseGuildScheduledEventData(event.value());
		guild.guildScheduledEvents.push_back(eventData);
	}

	for (auto member: guildObj["members"]) {
		guild_member_data memberData = parseGuildMemberData(member.value());
		guild.members.push_back(memberData);
	}

	guildObj["default_message_notifications"].get_uint64().get(guild.defaultMessageNotifications);

	for (auto channel: guildObj["channels"]) {
		channel_data channelData = parseChannelData(channel.value());
		guild.channels.push_back(channelData);
	}

	guildObj["max_stage_video_channel_users"].get_uint64().get(guild.maxStageVideoChannelUsers);
	guildObj["public_updates_channel_id"].get_string().get(newString);
	guild.publicUpdatesChannelId = std::string(newString);
	guildObj["premium_subscription_count"].get_uint64().get(guild.premiumSubscriptionCount);

	for (auto feature: guildObj["features"]) {
		feature.get_string().get(newString);
		guild.features.push_back(std::string(newString));
	}

	guildObj["approximate_presence_count"].get_uint64().get(guild.approximatePresenceCount);
	guildObj["safety_alerts_channel_id"].get_string().get(newString);
	guild.safetyAlertsChannelId = std::string(newString);
	guildObj["approximate_member_count"].get_uint64().get(guild.approximateMemberCount);
	guildObj["premium_progress_bar_enabled"].get_bool().get(guild.premiumProgressBarEnabled);
	guildObj["max_video_channel_users"].get_uint64().get(guild.maxVideoChannelUsers);
	guildObj["explicit_content_filter"].get_uint64().get(guild.explicitContentFilter);

	for (auto role: guildObj["roles"]) {
		role_data roleData = parseRoleData(role.value());
		guild.roles.push_back(roleData);
	}

	guildObj["system_channel_id"].get_string().get(newString);
	guild.systemChannelId = std::string(newString);
	guildObj["widget_channel_id"].get_string().get(newString);
	guild.widgetChannelId = std::string(newString);
	guildObj["preferred_locale"].get_string().get(newString);
	guild.preferredLocale = std::string(newString);
	guildObj["discovery_splash"].get_string().get(newString);
	guild.discoverySplash = std::string(newString);
	guildObj["system_channel_flags"].get_uint64().get(guild.systemChannelFlags);
	guildObj["rules_channel_id"].get_string().get(newString);
	guild.rulesChannelId = std::string(newString);
	guildObj["application_id"].get_string().get(newString);
	guild.applicationId = std::string(newString);
	guildObj["verification_level"].get_uint64().get(guild.verificationLevel);
	guildObj["vanity_url_code"].get_string().get(newString);
	guild.vanityUrlCode = std::string(newString);
	guildObj["afk_channel_id"].get_string().get(newString);
	guild.afkChannelId = std::string(newString);
	guildObj["description"].get_string().get(newString);
	guild.description = std::string(newString);
	guildObj["permissions"].get_string().get(newString);
	guild.permissions = std::string(newString);
	guildObj["max_presences"].get_uint64().get(guild.maxPresences);
	guildObj["discovery"].get_string().get(newString);
	guild.discovery = std::string(newString);
	guildObj["member_count"].get_uint64().get(guild.memberCount);
	guildObj["joined_at"].get_string().get(newString);
	guild.joinedAt = std::string(newString);
	guildObj["owner_id"].get_string().get(newString);
	guild.ownerId = std::string(newString);
	guildObj["premium_tier"].get_uint64().get(guild.premiumTier);
	guildObj["max_members"].get_uint64().get(guild.maxMembers);
	guildObj["afk_timeout"].get_uint64().get(guild.afkTimeout);
	guildObj["splash"].get_string().get(newString);
	guild.splash = std::string(newString);
	guildObj["banner"].get_string().get(newString);
	guild.banner = std::string(newString);
	guildObj["widget_enabled"].get_bool().get(guild.widgetEnabled);
	guildObj["nsfw_level"].get_uint64().get(guild.nsfwLevel);
	guildObj["name"].get_string().get(newString);
	guild.name = std::string(newString);
	guildObj["icon"].get_string().get(newString);
	guild.icon = std::string(newString);
	guildObj["mfa_level"].get_uint64().get(guild.mfaLevel);
	guildObj["unavailable"].get_bool().get(guild.unavailable);
	guildObj["id"].get_string().get(newString);
	guild.id = std::string(newString);
	guildObj["flags"].get_uint64().get(guild.flags);
	guildObj["large"].get_bool().get(guild.large);
	guildObj["owner"].get_bool().get(guild.owner);

	return guild;
}

discord_message parseDiscordMessage(simdjson::ondemand::value jsonData) {
	discord_message message;

	simdjson::ondemand::object messageObj = jsonData;
	std::string_view newString{};

	messageObj["t"].get_string().get(newString);
	message.t = std::string(newString);

	messageObj["s"].get_uint64().get(message.s);
	messageObj["op"].get_uint64().get(message.op);

	message.d = parseGuildData(messageObj["d"]);

	return message;
}

auto simdjson_discord_test(const std::string& discordData, bool doWePrint = true) {
	std::string buffer{ discordData };

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Discord Test", "https://github.com/simdjson/simdjson", iterations };

	simdjson::ondemand::parser parser;

	discord_message welcomeData;

	auto result = benchmark(
		[&]() {
			try {
				auto doc = parser.iterate(buffer);
				parseDiscordMessage(doc);
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

auto simdjson_abc_test(const jsonifier::string& bufferNew, bool doWePrint = true) {
	std::string buffer{ bufferNew };
	on_demand_abc parser{};

	AbcTest<test_struct> obj{};

	results r{ "simdjson", "Abc Test", "https://github.com/simdjson/simdjson", iterations };
	double result{};

	result = benchmark(
		[&]() {
			try {
				parser.read_out_of_order(obj, padded_string{ buffer });
			} catch (std ::exception& error) {
				std::cout << "Simdjson Error: " << error.what() << std::endl;
			}
		},
		iterations);

	r.json_read_byte_length = buffer.size();
	r.json_read				= result;
	if (doWePrint) {
		r.print();
	}

	return r;
}

static std::string table_header = R"(
| Library | Write (MB/s) | Read (MB/s) |
| ------------------------------------------------- | ---------- | ----------- |)";

std::string regular_test(const json_data& jsonData) {
	std::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string abc_test(const json_data& jsonData) {
	std::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_abc_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_abc_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_abc_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_abc_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_abc_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string discord_test(const std::string& jsonData) {
	std::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_discord_test(jsonData, false);
	}
	results.emplace_back(simdjson_discord_test(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_discord_test(jsonData, false);
	}
	results.emplace_back(glaze_discord_test(jsonData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_discord_test(jsonData, false);
	}
	results.emplace_back(jsonifier_discord_test(jsonData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
}

std::string single_test(const json_data& jsonData) {
	std::vector<results> results{};
	for (uint32_t x = 0; x < 2; ++x) {
		simdjson_single_test(jsonData.theData, false);
	}
	results.emplace_back(simdjson_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		glaze_single_test(jsonData.theData, false);
	}
	results.emplace_back(glaze_single_test(jsonData.theData));
	for (uint32_t x = 0; x < 2; ++x) {
		jsonifier_single_test(jsonData.theData, false);
	}
	results.emplace_back(jsonifier_single_test(jsonData.theData));

	std::string table{};
	const auto n = results.size();
	table += table_header + '\n';
	for (uint64_t x = 0; x < n; ++x) {
		table += results[x].json_stats();
		if (x != n - 1) {
			table += "\n";
		}
	}
	return table;
};

namespace fs = std::filesystem;

std::string readFileRecursively(const std::string& filename) {
	fs::path currentPath = fs::current_path();

	while (!currentPath.empty()) {
		fs::path filePath = currentPath / filename;

		if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
			std::ifstream fileStream(filePath);
			if (fileStream.is_open()) {
				return std::string((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
			} else {
				std::cerr << "Error opening file: " << filePath << std::endl;
				return "";
			}
		}

		currentPath = currentPath.parent_path();
	}

	std::cerr << "File not found: " << filename << std::endl;
	return "";
}

static const jsonifier::string section00{ R"(# Json-Performance
Performance profiling of JSON libraries (Compiled and run on Ubuntu-22.04 using the Clang++18 compiler)

Latest Results: ()" };

static const jsonifier::string section01{ R"()

Discord Test Results [(View the data used in the test)](https://github.com/RealTimeChris/Json-Performance/blob/main/DiscordData.json):

----
)" };

static const jsonifier::string section02{
	R"(

Single Iteration Test Results [(View the data used in the following tests)](https://github.com/RealTimeChris/Json-Performance/blob/main/JsonData.json):

----
)"
};

static const jsonifier::string section03{
	R"(

Multi Iteration Test Results:

----
)"
};

static const jsonifier::string section04{
	R"(
> )" +
	jsonifier::toString(iterations) + R"( iterations on a 6 core (Intel i7 8700k))" + R"(

## ABC Test (Out of Sequence Performance)

The JSON documents in the previous tests featured keys ranging from "a" to "z," where each key corresponds to an array of values. Notably, the documents in this test arrange these keys in reverse order, deviating from the typical "a" to "z" arrangement.

This test effectively demonstrates the challenges encountered when utilizing simdjson and iterative parsers that lack the ability to efficiently allocate memory locations through hashing. In cases where the keys are not in the expected sequence, performance is significantly compromised, with the severity escalating as the document size increases.

In contrast, hash-based solutions offer a viable alternative by circumventing these issues and maintaining optimal performance regardless of the JSON document's scale, or ordering of the keys being parsed.
)"
};

static const jsonifier::string section05{ R"(
> )" +
	jsonifier::toString(iterations) + R"( iterations on a 6 core (Intel i7 8700k))" };

using avx_simd_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<1, uint8_t, uint8_t, 2>>;

using avx_integer_list = jsonifier::concepts::type_list<jsonifier::concepts::type_holder<8, uint64_t, uint64_t, 8>, jsonifier::concepts::type_holder<4, uint32_t, uint32_t, 4>,
	jsonifier::concepts::type_holder<2, uint16_t, uint16_t, 2>, jsonifier::concepts::type_holder<1, uint8_t, uint8_t, 2>>;

template<size_t index = 0, typename char_type01, typename char_type02>
JSONIFIER_INLINE void serializeString(const char_type01* string1, char_type02* string2, uint64_t lengthNew, uint64_t& indexNew) {
	using simd_type							 = typename jsonifier::concepts::get_type_at_index<avx_simd_list, index>::type::type;
	using integer_type						 = typename jsonifier::concepts::get_type_at_index<avx_simd_list, index>::type::integer_type;
	static constexpr uint64_t bytesProcessed = jsonifier::concepts::get_type_at_index<avx_simd_list, index>::type::bytesProcessed;
	if constexpr (index < avx_simd_list::size - 1) {
		if (lengthNew < bytesProcessed / 2) {
			return serializeString<index + 1>(string1, string2, lengthNew, indexNew);
		}
	}
	static constexpr integer_type mask = jsonifier::concepts::get_type_at_index<avx_simd_list, index>::type::mask;
	while (static_cast<int64_t>(lengthNew) > 0) {
		if constexpr (index < avx_simd_list::size - 1) {
			if (lengthNew < bytesProcessed / 2) {
				return serializeString<index + 1>(string1, string2, lengthNew, indexNew);
			}
		}
		integer_type nextEscapeable = jsonifier_internal::copyAndFindSerialize<simd_type, integer_type>(string1, string2);
		if (nextEscapeable != mask) {
			auto escapeResult = jsonifier_internal::escapeTable[string1[nextEscapeable]];
			if (escapeResult == 0u) {
				indexNew += nextEscapeable;
				return;
			}
			lengthNew -= nextEscapeable;
			indexNew += nextEscapeable;
			string1 += nextEscapeable;
			string2 += nextEscapeable;
			std::memcpy(string2, &escapeResult, 2);
			indexNew += 2ULL;
			string2 += 2ULL;
			--lengthNew;
			++string1;
		} else {
			lengthNew -= bytesProcessed;
			indexNew += bytesProcessed;
			string1 += bytesProcessed;
			string2 += bytesProcessed;
		}
	}
}

int32_t main() {
	try {
		json_data jsonData{ TestGenerator<test_struct>::generateJsonData() };
		jsonifier::string newerString01{ jsonData.theData };
		jsonifier::string newerString02{};
		newerString02.resize(8);
		newerString01.resize(8);
		std::cout << "BENCHMARK 01: "
				  << benchmark(
						 [&]() {
							 for (volatile uint64_t x = 0; x < 128; ++x) {
								 uint64_t indexNew{};
								 serializeString<0>(jsonData.theData.data(), newerString01.data(), newerString01.size(), indexNew);
							 }
						 },
						 1024)
				  << std::endl;
		std::cout << "BENCHMARK 02: "
				  << benchmark(
						 [&]() {
							 for (volatile uint64_t x = 0; x < 128; ++x) {
								 uint64_t indexNew{};
								 serializeString<0>(jsonData.theData.data(), newerString01.data(), newerString01.size(), indexNew);
							 }
						 },
						 1024)
				  << std::endl;

		std::cout << "BENCHMARK 03: "
				  << benchmark(
						 [&]() {
							 for (volatile uint64_t x = 0; x < 128; ++x) {
								 uint64_t indexNew{};
								 jsonifier_internal::serializeString(jsonData.theData.data(), newerString01.data(), newerString01.size(), indexNew);
							 }
						 },
						 1024)
				  << std::endl;

#if defined(_WIN32)
		FileLoader fileLoader01{ "../../../ReadMe.md" };
		FileLoader fileLoader02{ "../../../JsonData.json" };
		fileLoader02.saveFile(jsonifier::prettify(jsonData.theData));
#else
		FileLoader fileLoader01{ "../ReadMe.md" };
		FileLoader fileLoader02{ "../JsonData.json" };
		fileLoader02.saveFile(jsonifier::prettify(jsonData.theData));
#endif
		jsonifier::string newTimeString{};
		newTimeString.resize(1024);
		std::tm resultTwo{};
		std::time_t result = std::time(nullptr);
		resultTwo		   = *localtime(&result);
		newTimeString.resize(strftime(newTimeString.data(), 1024, "%b %d, %Y", &resultTwo));
		auto discordResults	   = discord_test(readFileRecursively("DiscordData.json"));
		auto singleTestResults = single_test(jsonData);
		auto multiTestResults  = regular_test(jsonData);
		auto abcTestResults	   = abc_test(jsonData);
		jsonifier::string newerString =
			section00 + newTimeString + section01 + discordResults + section02 + singleTestResults + section03 + multiTestResults + section04 + abcTestResults + section05;
		fileLoader01.saveFile(newerString);
	} catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	} catch (std::out_of_range& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}