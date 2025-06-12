# Çok Katlı Apartman İnşaatı Simülasyonu

Bu proje, **Bursa Teknik Üniversitesi** BLM0332 İşletim Sistemleri dersi kapsamında hazırlanmış olup, **işlem (process)**, **iş parçacığı (thread)** ve **senkronizasyon** kavramlarının anlaşılmasını hedefleyen bir simülasyondur.


## Amaç

Simülasyon, 10 katlı ve her katta 4 daire bulunan bir apartmanın inşaat sürecini modelleyerek aşağıdaki işletim sistemi kavramlarını öğretmeyi amaçlar:

- Çoklu işlem ve iş parçacığı yönetimi
- Kaynak paylaşımı ve erişim senkronizasyonu
- Yarış durumu (race condition) önleme teknikleri

## Teknik Özellikler

- **Program Dili:** C
- **Platform:** Windows (WinAPI)
- **Yapı:** 1 process içinde mantıksal olarak modellenmiş katlar, her daire bir thread

## Yapılandırma

- **Kat Sayısı:** 10
- **Daire Sayısı/Kat:** 4
- **Toplam Daire:** 40

### Kat Döngüsü Örneği

```c
for (int kat = 1; kat <= KAT_SAYISI; kat++) {
    for (int i = 0; i < DAIRE_SAYISI; i++) {
        int *daire_no = malloc(sizeof(int));
        *daire_no = (kat - 1) * DAIRE_SAYISI + i + 1;
        daire_threadleri[i] = CreateThread(NULL, 0, daire_insa_et, daire_no, 0, NULL);
    }
    WaitForMultipleObjects(DAIRE_SAYISI, daire_threadleri, TRUE, INFINITE);
}
```

### Thread Fonksiyonu Örneği

```c
DWORD WINAPI daire_insa_et(LPVOID arg) {
    int daire_no = *((int *)arg);
    free(arg);
    guvenli_yaz("(Daire %d) Calismaya basladi.\n", daire_no);
    // işlem adımları...
    guvenli_yaz("(Daire %d) TAMAMLANDI!\n", daire_no);
    return 0;
}
```

### Konsol Yazımı (Senkrone)

```c
void guvenli_yaz(const char* format, ...) {
    WaitForSingleObject(print_mutex, INFINITE);
    zaman_damgasi_yaz();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    ReleaseMutex(print_mutex);
}
```

## Kullanılan Senkronizasyon Araçları

- `HANDLE asansor_mutex` – Asansör kullanımı
- `HANDLE tesisat_mutex` – Tesisat işlemleri
- `HANDLE vinc_semaphore` – Vinç erişimi (aynı anda sınırlı thread)
- `HANDLE print_mutex` – Konsol çıktısı sıralaması

## Derleme ve Çalıştırma

1. Windows üzerinde Visual Studio veya destekleyen bir C derleyicisi açın.
2. Proje dosyalarını yükleyin ve derleyin.
3. Çalıştırdıktan sonra konsol ekranından ilerleyişi izleyin.

## Video Tanıtım

[C ile Çok Katlı Apartman İnşaatı Simülasyonu](https://www.youtube.com/watch?v=-cIPQktewEc)


## Lisans

Bu proje yalnızca eğitim amaçlıdır.
