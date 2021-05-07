from conans import ConanFile, CMake, tools

Options = [
    # Cafe
    ("CAFE_INCLUDE_TESTS", [True, False], False),

    # Cafe.TextUtils
    ("CAFE_INCLUDE_TEXT_UTILS_MISC", [True, False], True),
    ("CAFE_INCLUDE_TEXT_UTILS_FORMAT", [True, False], True),
    ("CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS", [True, False], True)
]


class CafeTextUtilsConan(ConanFile):
    name = "Cafe.TextUtils"
    version = "0.1"
    license = "MIT"
    author = "akemimadoka <chino@hotococoa.moe>"
    url = "https://github.com/akemimadoka/Cafe.TextUtils"
    description = "A general purpose C++ library"
    topics = "C++"
    settings = "os", "compiler", "build_type", "arch"
    options = {opt[0]: opt[1] for opt in Options}
    default_options = {opt[0]: opt[2] for opt in Options}

    requires = "Cafe.Encoding/0.1", "Cafe.ErrorHandling/0.1", "Cafe.Environment/0.1"

    generators = "cmake"

    exports_sources = "CMakeLists.txt", "CafeCommon*", "Format*", "Misc*", "StreamHelpers*", "Test*"

    def requirements(self):
        if self.options.CAFE_INCLUDE_TESTS:
            self.requires("catch2/3.0.0@catchorg/stable", private=True)

    def configure_cmake(self):
        cmake = CMake(self)
        for opt in Options:
            cmake.definitions[opt[0]] = getattr(self.options, opt[0])
        cmake.configure()
        return cmake

    def package(self):
        with tools.vcvars(self.settings, filter_known_paths=False) if self.settings.compiler == 'Visual Studio' else tools.no_op():
            cmake = self.configure_cmake()
            cmake.install()

    def package_id(self):
        self.info.header_only()
