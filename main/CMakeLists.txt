
idf_component_register(
	SRCS
	"main.cpp"
	"src/Settings.cpp"
	"src/Backend.cpp"
	"src/Frontend.cpp"
	"src/Communicator.cpp"
	INCLUDE_DIRS
	"."
	"include/"
	REQUIRES
	nvs_flash lwip esp_netif esp_wifi esp_event wifi_provisioning driver
)

component_compile_options("-std=gnu++2a" "-ffast-math" "-fipa-sra") # "-Wno-maybe-uninitialized"
