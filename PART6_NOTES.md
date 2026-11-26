# Part 6 teknik notları

## Part 5 mimarisi korundu
`MainActivity` + `GLSurfaceView` + JNI + `Game` + `Renderer` hattı korunarak genişletildi. Android tarafı kontrol/HUD katmanı; oyun simülasyonu C++ tarafındadır.

## Yeni oyun sistemleri
- Karakter: grounded kontrolü, tek zıplama kilidi, yerçekimi, yürüme/koşma/eğilme hızları ve havada sınırlı hareket.
- Görev: `MissionDefinition` kayıt sistemi, aktif görev kimliği, aşama ve tamamlanma durumu. Görev aşaması SharedPreferences ile saklanır.
- Çete: bağımsız devriye, oyuncu algılama, yaklaşma ve grafik yaralanma göstermeyen arcade yakın mesafe saldırı durumu.
- Sivil: devriye/dolaşma, oyuncu yaklaşınca uzaklaşma, diğer ajanlardan ve trafikten basit ayrılma.
- Polis: devriye, şüphe, takip ve arama durumları; görev olayı wanted seviyesini başlatır.

## Kontrol ve JNI
JNI fonksiyonları `MainActivity` sınıfındaki `native...` metotlarıyla birebir eşleştirilmiştir. C++ oyun durumuna UI thread ve GL thread erişimini korumak için mutex kullanılır. Çoklu dokunmada joystick ve butonlar ayrı pointer ID'leriyle izlenir.

## Renderer
Tekrarlanan küp nesneleri instance matrisi ve renk attribute'larıyla çizilir. Renderer shader'ları ES 3.0 GLSL'dir. Shader dosyaları `native/shaders` altında kaynak olarak bulunur; aynı kod Renderer içinde gömülüdür.

## Bilinen sınırlamalar
- Karakter ve NPC görselleri prosedürel küp tabanlı animasyondur; rigged model/animasyon asset'i yoktur.
- Basit uzaklık tabanlı AI ve ayrılma davranışı, navmesh/pathfinding değildir.
- Otomatik kalite kademesi çözünürlük ölçeklemesi yapmaz.
- Android APK ve gerçek cihaz testleri bu ortamda yapılamadı.
