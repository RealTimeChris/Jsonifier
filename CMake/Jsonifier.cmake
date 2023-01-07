include(FetchContent)

fetchcontent_declare(
	 Jsonifier
	URL https://github.com/RealTimeChris/Jsonifier/releases/download/Master/Jsonifier-src.zip
	GIT_TAG f47f14a63996610474215397486394b33983c6e6
)

fetchcontent_makeavailable(Jsonifier)