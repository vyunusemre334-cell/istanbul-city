# İstanbul City — oynanabilir 3B Android oyun projesi

Bu paket, verilen Part 8 Android projesinin oynanabilir oyun döngüsü ve görsel sunumu geliştirilmiş sürümüdür. Android Studio/Gradle projesi, Java dokunmatik arayüzü, JNI köprüsü, C++20 oyun mantığı ve OpenGL ES 3.0 renderer korunmuştur.

## Bu sürümde oynanabilir içerik

- Üçüncü şahıs kamera ve mobil joystick ile serbest karakter kontrolü
- Yürüme, koşma, zıplama ve eğilme
- Araca binme/inme, hızlanma, fren/geri gitme, direksiyon ve araç çarpışması
- Üç ayrı görev: **Mor Bölge**, **Sahil Operasyonu**, **Köprü Hattı**
- Görev hedefini dünyada gösteren altın 3B işaret, görev aşamaları ve ilerleme kaydı
- Rakip çete, polis ve sivil NPC davranışları; takip/kaçış ve can sistemi
- Trafik araçları, yaya hareketi, yol çizgileri, kaldırımlar, sokak lambaları ve sahil bandı
- Cephe pencereleri, kat silmeleri, balkonlar, çatı detayları, dükkân cepheleri ve mahalle camisi siluetiyle çeşitlendirilmiş şehir binaları
- İnsanlarda küresel baş/eklem ve silindirik uzuv geometrileri; araçlarda dönen tekerlekler ve yönle birlikte dönen gövde
- FPS düşünce ayrıntıyı azaltan basit kalite kademesi

## Mobil kontroller

| Kontrol | İşlev |
|---|---|
| Sol joystick | Hareket / araç yönlendirme |
| KOŞ | Basılı tutarak koş |
| ZIPLA | Zıpla |
| EĞİL | Basılı tutarak eğil |
| ETK | Görev alanında etkileşim; araca yakınken bin/in |
| GÖREV | Görev panelini aç/kapat; görev tamamlanınca sonraki göreve geç |

Görev akışı: altın işarete git → **ETK** düğmesine bas → polis takibinden kurtulup ikinci altın işarete ulaş. Görev bitince **GÖREV** düğmesi sonraki görevi başlatır. İlerleme cihazda saklanır.

## Android derlemesi

GitHub Actions ile derlemek için proje klasörünü depoya yükleyip **Actions → İstanbul City Android Build → Run workflow** yolunu izle. İş akışı önce yerel C++ testlerini, ardından Android APK/AAB paketlemesini çalıştırır. Ayrıntılar: [`docs/RELEASE_BUILD.md`](docs/RELEASE_BUILD.md).

Yerel derleme gereksinimleri: JDK 17, Android SDK Platform 35, Build Tools 35.0.0, NDK 27.2.12479018, CMake 3.22.1 ve Gradle 8.9.

## Yerel oyun mantığı testleri

```bash
cmake -S native -B build-native -DIV_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-native --parallel 2
ctest --test-dir build-native --output-on-failure
```

## Gerçekçilik kapsamı

Bu sürüm artık yalnızca düz küplerden oluşan sahne değildir; prosedürel mimari cepheler, yuvarlatılmış karakter parçaları, silindirik uzuvlar/tekerlekler, detaylı araç parçaları ve 3B görev işaretleri içerir. Yine de bu, lisanslı fotogerçekçi GTA ölçeğinde bir varlık paketi değildir: şehir, karakterler ve araçlar kodla üretilen stilize 3B modellerdir. Android cihazda son FPS ve görsel kalite testi ayrıca yapılmalıdır.
