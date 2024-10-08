
idf_component_register(
	SRCS
	"main.cpp"
	"src/Backend.cpp"
	"src/Frontend.cpp"
	"src/Communicator.cpp"
	"src/Converter.cpp"
	INCLUDE_DIRS
	"."
	"include/"
	REQUIRES
	driver
)

component_compile_options("-std=gnu++2a" "-ffast-math" "-fipa-sra") # "-Wno-maybe-uninitialized"
