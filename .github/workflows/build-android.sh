#!/usr/bin/env bash
set -euo pipefail

if ! command -v gradle >/dev/null 2>&1; then
  echo 'Gradle 8.9 is required. Install it or run the GitHub Actions workflow.' >&2
  exit 2
fi

# Android SDK/NDK must be installed and ANDROID_HOME (or ANDROID_SDK_ROOT) set.
gradle --no-daemon clean assembleDebug assembleRelease bundleRelease
printf '\nBuild outputs:\n'
find app/build/outputs -type f \( -name '*.apk' -o -name '*.aab' \) -print
