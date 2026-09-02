FetchContent_Declare (
	resources
	URL [[http://fileadmin.cs.lth.se/cs/Education/EDAF80/assignments/resources.zip]]
	URL_HASH [[SHA512=82694E9CE5388667FB0EF3C4DF73461C283C3A0F3C465E50A0EE9F82228A373DF072AF91D6B2BC30950607C2AD55A6A212E8CB588D8AEB017086333D58C4B006]]
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res"
	SOURCE_SUBDIR not-a-cmake-project
)

message (STATUS "Fetching resource archive…")
FetchContent_MakeAvailable (resources)
