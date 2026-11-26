# Part 6 — Mobil performans tasarımı

## Uygulananlar
- Tek küp mesh'i ve instance başına model matrisi/renk verisiyle `glDrawArraysInstanced`; sahnedeki tekrar eden bloklar tek instanced draw çağrısında gönderilir.
- Kamera çevresinde CPU mesafe culling ve ileri görüş için muhafazakâr frustum elemesi.
- NPC LOD: uzaktaki karakterlerde daha küçük gövde ve daha az parça; düşük kalite kademesinde uzak NPC ayrıntısı atlanır.
- NPC düşünme aralığı: 25 m içinde yaklaşık 10 Hz; daha uzakta yaklaşık 2.6 Hz; 65 m dışında AI güncellemesi durur.
- FPS ölçümü yarım saniyelik pencerede güncellenir. FPS 3 saniyeden uzun süre 44'ün altında kalırsa sahne kalite kademesi düşürülür; 56'nın üzerinde 8 saniye kalırsa kademeli olarak yükseltilir.
- Per-frame instance dizisi önceden kapasite ayırır; Android tarafında dokunma pointer'ları ayrıştırılır.

## Otomatik kalite kademesi

| Kademe | Davranış |
|---|---|
| Yüksek | 110 m çizim mesafesi, bina ışık dalgalanması ve NPC uzuv ayrıntıları |
| Orta | 90 m çizim mesafesi, daha az animasyon işi |
| Düşük | 74 m çizim mesafesi, uzak NPC ayrıntıları atlanır |

Bu kontrol çözünürlüğü dinamik olarak düşürmez; sahne ayrıntısı ve çizim yükünü azaltır. Gerçek cihaz GPU süreleri ölçülmeden bu eşikler kesin performans vaadi değildir.

## Ölçüm hedefleri
- 60 FPS için kare bütçesi 16.67 ms.
- Önce gerçek cihazda Android Studio Profiler / Android GPU Inspector ile CPU ve GPU sürelerini ölç.
- Uzun süreli testte termal kısılma ve pil tüketimini izle.
- Realme C53 (Unisoc T612 / Mali-G57 MP1) üzerinde henüz cihaz testi yapılmadı; sabit 60 FPS iddiası yoktur.

## Part 7 ekleri
- Yol, kaldırım, çizgi, yaya geçidi, sahil ve seyrek sokak lambaları aynı instanced mesh akışına eklenir.
- Araç ivmelenmesi, fren/geri hareket, hıza bağlı direksiyon ve bina bloklarına karşı basit çarpışma tepkisi oyun mantığında uygulanır.
- Yeni şehir detayları aynı küp mesh'i kullandığından ek model yükleme veya texture belleği gerektirmez.


## Part 8 mobile rendering notes (updated)

- Renderer remains OpenGL ES 3.0 and uses one instanced cube draw for the city primitives.
- Added a lightweight time-varying sun, restrained view-dependent specular highlight, filmic color compression and distance haze. These are **screen-space/stylized lighting effects**, not hardware ray tracing or ray-traced reflections.
- Frame instance uploads now orphan the streaming buffer before updating it, reducing the chance of synchronizing with the previous frame's GPU read.
- Existing adaptive quality and distance/actor LOD are retained. No Gradle, Android SDK/NDK, CMake, dependency, ABI or packaging settings were changed.

### Device expectation

The Realme C53's Unisoc T612 uses a Mali-G57-class entry GPU (regional/spec-sheet variants may differ); virtual RAM is storage-backed and is not equivalent to physical RAM. The C53 is generally listed with an LCD panel, not OLED. The renderer is deliberately ES 3.0 and low-cost. Stable frame rate depends on the exact RAM variant, Android background load, temperature and display resolution; this ZIP does not contain a benchmarked APK, so no guaranteed FPS or "AAA" performance claim is made.
