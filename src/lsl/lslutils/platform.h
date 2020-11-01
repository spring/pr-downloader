
namespace LSL {
namespace Util {
enum class Platform {
	kLinux32,
	kLinux64,
	kMacosx,
	kWindows32,
	kWindows64
};

enum Platform GetPlatform();
const char* GetPlatformString(enum Platform platform);

} //namespace Util
} //Namespace LSL

