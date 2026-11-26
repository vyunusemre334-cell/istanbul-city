# İstanbul City — güncel oyun durumu

## Aktif oynanışa bağlı özellikler
- Mobil joystick ve dokunmatik aksiyon düğmeleri
- Üçüncü şahıs karakter hareketi: yürüme, koşma, zıplama, eğilme
- Bina ayak izlerinde karakter çarpışması ve eksen boyunca kayma
- Araca binme/inme, gaz/fren/geri vites, dönüş, bina çarpışması ve dönüş yönünü takip eden araç modeli
- Üç görevlik zincir, görev alanı etkileşimi, güvenli bölgeye ulaşma, polis takibi ve cihazda görev kaydı
- Çete, polis ve sivil NPC davranışları; can tükenince başlangıç noktasında yeniden doğma
- Trafik, yayalar, yol ağı, yaya geçitleri, kaldırımlar, sokak lambaları ve sahil bandı

## Görsel geliştirmeler
- Bina gövdeleri üzerinde katlara ayrılmış pencere cepheleri, söveler, balkonlar, çatı parapetleri, su tankları ve dükkân vitrinleri
- Prosedürel küre ve silindir mesh'leri: karakter başları, gövdeleri, uzuvlar, ağaç taçları, minareler ve araç tekerlekleri
- Dönen araç gövdesi ve tekerlekleri; ayrı cam, tampon ve far parçaları
- Altın renkli 3B görev hedef işareti
- FPS'e göre üç kademeli çizim mesafesi/ayrıntı ayarı

## Sınırlar
- Görsel varlıklar prosedürel ve stilizedir; fotogerçekçi, el yapımı GTA kalitesinde GLB/FBX karakter/araç/şehir paketleri bu arşivde bulunmuyor.
- Renderer hâlâ instanced OpenGL ES mesh'leri kullanır; tam glTF materyal/iskelet animasyonu sistemi değildir.
- Fizik sistemi arcade tarzındadır; gerçekçi araç süspansiyonu, hasar modellemesi, silah/ekonomi/mağaza/garaj sistemleri bu sürümde yoktur.
- Android APK/AAB bu çalışma alanında derlenmedi; Android SDK/NDK ve Gradle araç zinciri mevcut değildi.

## Doğrulama
- CMake host derlemesi ve CTest: PASS (1/1).
- Renderer C++ sözdizimi denetimi: PASS (Android/OpenGL başlıklarının yerini alan yerel test stub'larıyla).
- Gerçek Android cihazda açılış, dokunmatik kontrol ve FPS testi: henüz yapılmadı.
