# İstanbul City — GitHub ile APK üretme

## Tek tıkla derleme

1. Bu ZIP'i açıp içindeki `ISTANBUL-CITY-Part8` klasörünün tamamını GitHub deposuna yükle.
2. Depoda **Actions** sekmesini aç.
3. **İstanbul City Android Build** iş akışını seç ve **Run workflow** düğmesine bas.
4. `native-tests` ve `android` işleri yeşil tamamlanırsa çalışmanın **Artifacts** bölümünden `istanbul-city-android-*` paketini indir.
5. İndirilen paketin içinde `app-debug.apk`, `app-release-unsigned.apk` ve `app-release.aab` bulunur.

`app-release-unsigned.apk` imzasızdır; telefona dağıtılacak/satılacak sürüm için kendi Android imzalama anahtarınla imzalanmalıdır. Debug APK test içindir. AAB, Play Console yükleme formatıdır.

## Yerel derleme

- JDK 17
- Android SDK Platform 35 ve Build Tools 35.0.0
- Android NDK 27.2.12479018
- CMake 3.22.1
- Gradle 8.9

SDK/NDK kurulduktan ve `ANDROID_HOME` ayarlandıktan sonra proje kökünde:

```bash
./scripts/build-android.sh
```

## Uygulama kimliği ve sürüm

- Görünen uygulama adı: **İstanbul City**
- Android applicationId: `com.istanbulvice.underground` (mevcut kurulum/kayıt uyumluluğu için korundu)
- versionName: `1.1.0`
- versionCode: `7`
- Paketlenen ABI'ler: `arm64-v8a`, `armeabi-v7a`
- Grafik API'si: OpenGL ES 3.0

## Yaş derecelendirmesi ve içerik

AndroidManifest içindeki uygulama etiketi veya `targetSdk` bir mağaza yaş derecelendirmesi belirlemez. 18+ derecelendirmesi Google Play Console içerik derecelendirme anketinde doğru cevaplarla alınmalıdır. ZIP'e çıplaklık/çıplak karakter modeli eklenmemiştir; kaynak arşivde bu tür model/texture dosyaları bulunmamaktadır. Kullanılacak her harici varlık için dağıtım ve ticari kullanım lisansı ayrıca doğrulanmalıdır.

## Boyut

2 GB hedefi yapay dolgu dosyalarıyla şişirilmemiştir. APK boyutu gerçek, lisanslı oyun varlıklarına ve ABI'lere bağlıdır. Play dağıtımı için AAB tercih edilir; Google Play cihaza uygun APK parçalarını üretir.
