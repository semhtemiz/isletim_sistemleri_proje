#include <stdio.h>       // standart giris cikis islemleri icin gerekli kutuphane
#include <stdlib.h>      // bellek ayirma ve cikis islemleri icin kutuphane
#include <windows.h>     // windows API fonksiyonlari icin gerekli
#include <time.h>        // zaman islemleri icin kutuphane

#define KAT_SAYISI 10     // apartmanda toplam kat sayisi
#define DAIRE_SAYISI 4    // her kattaki toplam daire sayisi


HANDLE asansor_mutex;        // asansor kullanimi icin mutex (ayni anda sadece bir thread)
HANDLE tesisat_mutex;        // tesisat islemi icin mutex (ayni anda sadece bir thread)
HANDLE vinc_semaphore;       // vinc icin semaphore (ayni anda belirli sayida thread kullanimi)
HANDLE print_mutex;          // konsola cikti yazarken siralarin karismamasi icin mutex

// zaman damgasi fonksiyonu
void zaman_damgasi_yaz()
{
    SYSTEMTIME zaman;              // sistem zamanini tutacak degisken
    GetLocalTime(&zaman);          // sistm saatini al
    printf("[%02d:%02d:%02d] ", zaman.wHour, zaman.wMinute, zaman.wSecond); // saat formatinda yazdirma
}

// Mutex guvenli yazdirma fonksiyonu
void guvenli_yaz(const char* format, ...)
{
    WaitForSingleObject(print_mutex, INFINITE); // print_mutex'i al (baska thread cikti yazmasin)

    zaman_damgasi_yaz();        // Baslik olarak zaman bilgisi yazdir

    va_list args;               //degisken parametre listesi
    va_start(args, format);    // parametreleri format'a gore ayarlama
    vprintf(format, args);     // formatli sekilde parametreleri yazdirma
    va_end(args);              // parametre listesini kapatma

    ReleaseMutex(print_mutex); // mutex'i birakma (diger threadler yazabilir)
}

// her bir daire insaati icin calisacak thread fonksiyonu
DWORD WINAPI daire_insa_et(LPVOID arg)
{
    int daire_no = *((int *)arg); // parametre olarak gelen daire numarasini alma
    free(arg);                    // dinamik olarak ayrilan bellegi serbest birakma

    guvenli_yaz(">> (Daire %d) Calismaya basladi.\n", daire_no); // baslangic mesaji
    Sleep(1500); // hazirlik icin 1.5 saniye bekleme

    // asansor kullanimi
    WaitForSingleObject(asansor_mutex, INFINITE); // asansor mutex'ini bekleme
    guvenli_yaz("   (Daire %d) Asansoru kullaniyor.\n", daire_no);
    Sleep(2000); // asansor kullanimi 2 saniye
    ReleaseMutex(asansor_mutex); // mutex'i birakma

    // vinc kullanimi
    WaitForSingleObject(vinc_semaphore, INFINITE); // vinc kaynak bekleme
    guvenli_yaz("   (Daire %d) Vinc ile yukler tasiniyor.\n", daire_no);
    Sleep(2500); // vinc islemi 2.5 saniye surer
    ReleaseSemaphore(vinc_semaphore, 1, NULL); // kaynagi serbest birakma

    // tesisat kurulumu
    WaitForSingleObject(tesisat_mutex, INFINITE); // tesisat mutex'ini alma
    guvenli_yaz("   (Daire %d) Tesisat kuruluyor.\n", daire_no);
    Sleep(2000); // tesisat kurulum suresi
    ReleaseMutex(tesisat_mutex); // tesisat mutex'ini serbest birakma

    // ic duzenleme islemi
    guvenli_yaz("   (Daire %d) Ic duzenleme yapiliyor.\n", daire_no);
    Sleep(3000); // ic dekorasyon 3 saniye

    guvenli_yaz("<< (Daire %d) TAMAMLANDI!\n", daire_no); // tamamlama bildirimi
    return 0; // thread islemi tamamlandi
}

int main()
{
    // senkronizasyon nesnelerini olusturma
    asansor_mutex = CreateMutex(NULL, FALSE, NULL);         // asansor mutex
    tesisat_mutex = CreateMutex(NULL, FALSE, NULL);         // tesisat mutex
    vinc_semaphore = CreateSemaphore(NULL, 1, 1, NULL);     // vinc semaphore (1 kaynak)
    print_mutex = CreateMutex(NULL, FALSE, NULL);           // yazdirma mutex

    // giris ekrani
    printf("\n*****************************************\n");
    printf("     APARTMAN INSAATI SIMULASYONU       \n");
    printf("*****************************************\n");
    guvenli_yaz("[PROJE] Apartman insaati basladi.\n");
    printf("- Toplam Kat: %d\n", KAT_SAYISI);
    printf("- Kat Basina Daire: %d\n", DAIRE_SAYISI);
    printf("- Toplam Daire: %d\n", KAT_SAYISI * DAIRE_SAYISI);
    printf("*****************************************\n\n");

    // her kat icin dongu
    for (int kat = 1; kat <= KAT_SAYISI; kat++)
    {
        printf("*****************************************\n");
        printf("*               %2d. KAT INSAATI           *\n", kat);
        printf("*****************************************\n");

        guvenli_yaz(">> [KAT %d] Temel atiliyor...\n", kat);
        Sleep(3000); // temel atma 3 saniye surer

        guvenli_yaz(">> [KAT %d] Temel tamam! Daireler basliyor.\n", kat);
        printf("*****************************************\n");

        HANDLE daire_threadleri[DAIRE_SAYISI]; // thread handle dizisi

        for (int i = 0; i < DAIRE_SAYISI; i++)
        {
            int *daire_no = malloc(sizeof(int)); // daire numarasi icin bellek ayirma
            *daire_no = (kat - 1) * DAIRE_SAYISI + i + 1; // daire numarasini hesaplama

            daire_threadleri[i] = CreateThread(
                NULL,         // guvenlik bilgisi
                0,            // stack boyutu
                daire_insa_et,// fonksiyon adi
                daire_no,     // parametre
                0,            // thread hemen baslasin
                NULL);        // thread id'si gerekmez

            if (daire_threadleri[i] == NULL) // hata kontrolu
            {
                WaitForSingleObject(print_mutex, INFINITE);
                fprintf(stderr, "Thread olusturulamadi! Hata: %d\n", GetLastError());
                ReleaseMutex(print_mutex);
                exit(1);
            }
        }

        // tum thread'lerin bitmesini bekleme
        WaitForMultipleObjects(DAIRE_SAYISI, daire_threadleri, TRUE, INFINITE);

        // thread kaynaklarini temizleme
        for (int i = 0; i < DAIRE_SAYISI; i++)
        {
            CloseHandle(daire_threadleri[i]);
        }

        printf("*****************************************\n");
        guvenli_yaz("<< [KAT %d] Insaat tamamlandi!\n", kat);
        printf("*****************************************\n\n");
    }

    // proje tamamlama bildirimi
    printf("\n************************************************\n");
    printf("          PROJE BASARIYLA TAMAMLANDI!          \n");
    printf("************************************************\n");
    guvenli_yaz(">> [PROJE] Apartman insaati tamamlandi!\n");
    printf("   Toplam %d kat, %d daire basariyla insa edildi.\n", KAT_SAYISI, KAT_SAYISI * DAIRE_SAYISI);
    printf("************************************************\n\n");

    // tum senkronizasyon nesnelerini serbest birakma
    CloseHandle(asansor_mutex);
    CloseHandle(tesisat_mutex);
    CloseHandle(vinc_semaphore);
    CloseHandle(print_mutex);

    return 0; // program basariyla sonlandirma
}
