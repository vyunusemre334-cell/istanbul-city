# Part 5 teknik notları

- GLES 3.0 fragment shader türevleriyle yüzey normali yaklaşık hesaplanır.
- CPU tarafında 115 metre yarıçap dışında kalan nesneler atlanır.
- Ek shadow-map pass veya SSR yoktur; hedef düşük maliyetli görsel iyileştirmedir.
- Bu teknikler AAA kalite sağlamaz. Yüksek kaliteli varlıklar ve kapsamlı içerik hâlâ gereklidir.
- Derleme ve cihaz testi yapılmadan performans garantisi verilemez.
