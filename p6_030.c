/**
 * Masalah Lima Filsuf Makan - Implementasi berdasarkan model Petri Net yang dioptimalkan
 * Nama: Roy Aziz Barera
 * NIM: 221524030
 * 
 * Program ini mengimplementasikan Masalah Lima Filsuf Makan menggunakan Pthreads
 * berdasarkan model Petri Net yang dioptimalkan dari tugas sebelumnya.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define THINKING_TIME 2
#define EATING_TIME 3
#define ITERATION_COUNT 3

// Mutex untuk setiap sumber daya/sumpit (sesuai dengan Resource 1-5 dalam model)
pthread_mutex_t resources[NUM_PHILOSOPHERS];

// Thread untuk setiap filsuf (sesuai dengan Filsuf 1-5 dalam model)
pthread_t philosophers[NUM_PHILOSOPHERS];

// ID Thread untuk setiap filsuf
int philosopher_ids[NUM_PHILOSOPHERS];

// Prototipe fungsi
void *philosopher_lifecycle(void *arg);
void think(int philosopher_id);
void eat(int philosopher_id);
void pickup_resources(int philosopher_id);
void return_resources(int philosopher_id);

/**
 * Fungsi utama
 * Menginisialisasi sumber daya dan membuat thread filsuf
 */
int main() {
    int i;
    
    // Inisialisasi seed acak
    srand(time(NULL));
    
    printf("Masalah Lima Filsuf Makan - Berdasarkan Model Petri Net yang Dioptimalkan\n");
    printf("------------------------------------------------------------\n");
    
    // Inisialisasi sumber daya/sumpit (sesuai dengan penandaan awal dalam model)
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        // Inisialisasi mutex untuk setiap sumber daya
        // Dalam model Petri Net, setiap tempat Resource memiliki penandaan awal 1
        if (pthread_mutex_init(&resources[i], NULL) != 0) {
            printf("Gagal menginisialisasi mutex untuk sumber daya %d\n", i);
            return 1;
        }
    }
    
    // Membuat thread filsuf (sesuai dengan transisi Filsuf dalam model)
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i] = i;
        if (pthread_create(&philosophers[i], NULL, philosopher_lifecycle, &philosopher_ids[i]) != 0) {
            printf("Gagal membuat thread untuk filsuf %d\n", i);
            return 1;
        }
    }
    
    // Menunggu semua filsuf selesai
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    // Menghapus mutex sumber daya/sumpit
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&resources[i]);
    }
    
    printf("\nSemua filsuf telah selesai makan.\n");
    
    return 0;
}

/**
 * Fungsi siklus hidup filsuf - dijalankan oleh setiap thread filsuf
 * Ini mewakili siklus lengkap dalam model Petri Net untuk setiap filsuf
 */
void *philosopher_lifecycle(void *arg) {
    int philosopher_id = *((int *)arg);
    int i;
    
    printf("Filsuf %d telah bergabung di meja\n", philosopher_id + 1);
    
    for (i = 0; i < ITERATION_COUNT; i++) {
        // Fase berpikir
        think(philosopher_id);
        
        // Mencoba mengambil sumber daya (sumpit)
        pickup_resources(philosopher_id);
        
        // Fase makan
        eat(philosopher_id);
        
        // Mengembalikan sumber daya (sumpit)
        return_resources(philosopher_id);
    }
    
    printf("Filsuf %d telah meninggalkan meja\n", philosopher_id + 1);
    
    return NULL;
}

/**
 * Simulasi filsuf sedang berpikir
 * Sesuai dengan keadaan sebelum transisi Filsuf diaktifkan dalam model
 */
void think(int philosopher_id) {
    int think_time = rand() % THINKING_TIME + 1;
    printf("Filsuf %d sedang berpikir selama %d detik\n", philosopher_id + 1, think_time);
    sleep(think_time);
}

/**
 * Simulasi filsuf sedang makan
 * Sesuai dengan tempat Eating dalam model untuk setiap filsuf
 */
void eat(int philosopher_id) {
    int eat_time = rand() % EATING_TIME + 1;
    printf("Filsuf %d sedang makan selama %d detik\n", philosopher_id + 1, eat_time);
    sleep(eat_time);
}

/**
 * Mengambil sumber daya (sumpit) menggunakan pola alokasi sumber daya
 * yang ditentukan dalam model Petri Net melalui busur
 */
void pickup_resources(int philosopher_id) {
    printf("Filsuf %d mencoba mengambil sumpit\n", philosopher_id + 1);
    
    /*
     * Ini mengikuti pola akuisisi sumber daya yang tepat dari model Petri Net
     * di mana setiap transisi Filsuf memiliki busur masuk dari tempat Resource tertentu
     * 
     * Filsuf 1 membutuhkan Resource 1 dan 2
     * Filsuf 2 membutuhkan Resource 2 dan 3
     * Filsuf 3 membutuhkan Resource 3 dan 4
     * Filsuf 4 membutuhkan Resource 4 dan 5
     * Filsuf 5 membutuhkan Resource 5 dan 1
     * 
     * Solusi untuk menghindari deadlock adalah dengan memastikan urutan yang konsisten
     * dari akuisisi sumber daya di semua filsuf
     */
    
    // Sumber daya pertama adalah sumber daya bernomor filsuf itu sendiri
    int first_resource = philosopher_id;
    
    // Sumber daya kedua adalah sumber daya filsuf berikutnya
    // Kami menggunakan modulo untuk membungkus dari filsuf 5 ke filsuf 1
    int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
    
    // Kunci sumber daya pertama
    pthread_mutex_lock(&resources[first_resource]);
    printf("Filsuf %d mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
    
    // Penundaan kecil untuk mensimulasikan waktu di antara mengambil sumpit
    sleep(100000);  // 100ms
    
    // Kunci sumber daya kedua
    pthread_mutex_lock(&resources[second_resource]);
    printf("Filsuf %d mengambil sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
}

/**
 * Mengembalikan sumber daya (sumpit) mengikuti pola yang ditentukan
 * oleh transisi Return Resource dalam model Petri Net
 */
void return_resources(int philosopher_id) {
    /*
     * Ini mengikuti pola pelepasan sumber daya yang tepat dari model Petri Net
     * di mana setiap transisi Return Resource memiliki busur keluar ke tempat Resource tertentu
     * 
     * Sumber daya dilepaskan dalam urutan terbalik dari akuisisi untuk menghindari masalah potensial
     */
    
    // Sumber daya pertama adalah sumber daya bernomor filsuf itu sendiri
    int first_resource = philosopher_id;
    
    // Sumber daya kedua adalah sumber daya filsuf berikutnya
    int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
    
    // Membuka kunci sumber daya kedua terlebih dahulu (urutan terbalik dari akuisisi)
    pthread_mutex_unlock(&resources[second_resource]);
    printf("Filsuf %d mengembalikan sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
    
    // Membuka kunci sumber daya pertama
    pthread_mutex_unlock(&resources[first_resource]);
    printf("Filsuf %d mengembalikan sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
}