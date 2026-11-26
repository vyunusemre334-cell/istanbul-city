# Build verification — 2026-11-26

## Verified
- CMake configure for native host tests: PASS.
- C++20 native gameplay compilation/link: PASS.
- CTest: 1/1 passed (`game_smoke`), including movement, NPC, vehicle and multi-mission checks.
- Renderer C++ syntax check: PASS using local GLES/log API declaration stubs.

## Not verified in this workspace
- Android Gradle/NDK build, APK/AAB packaging and install: NOT RUN; Android SDK/NDK are not installed here.
- Physical Android device graphics, touch controls and performance: NOT RUN.

Use the included GitHub Actions workflow to run the actual Android build with the configured SDK/NDK versions. A passing host test is not proof of a successful Android package or device runtime.
